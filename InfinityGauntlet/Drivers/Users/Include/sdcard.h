/*
 * sdcard.h
 *
 *  Created on: Apr 25, 2022
 *      Author: aloha
 */

#ifndef USERS_INCLUDE_SDCARD_H_
#define USERS_INCLUDE_SDCARD_H_

#include "stm32f4xx_hal.h"

void SD_Init(void);
uint8_t SD_CheckInjectCntFile(__IO uint32_t *p_inject_cnt);
uint8_t SD_ReadFingerprintBMP(uint32_t inject_cnt, char * path_name, uint8_t *sd_bmp_buf,uint32_t buf_size);
uint8_t SD_SaveInjectCntFile(uint32_t inject_cnt);
void SD_DelInjectCntFile(void);
uint8_t SD_Log(uint32_t inject_cnt, uint8_t trystate, uint8_t reason);
void SD_SaveFingerprintRAW(uint32_t capcnt, char * path_name,uint8_t *try0_rev_buf,uint8_t *try1_rev_buf);
void SD_SaveFingerprintBMP(uint32_t capcnt, char * path_name,uint8_t *try0_rev_buf,uint8_t *try1_rev_buf);

#endif /* USERS_INCLUDE_SDCARD_H_ */
