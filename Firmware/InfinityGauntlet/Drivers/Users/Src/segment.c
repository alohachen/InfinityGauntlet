/*
 * segment.c
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */

#include "segment.h"

const uint8_t dig[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};

void Segment_Init(void){
	uint8_t cmd = 0x91;

	HAL_I2C_Master_Transmit(&hi2c2, 0x48, &cmd, 0x1, 10000);
}

void Segment_ShowErrorCode(uint8_t reason){
	uint8_t b;

	b = 0x83;
	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &b, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, (uint8_t *)&dig[reason], 1, 10000);
}

void Segment_ShowNum(uint8_t num){
	HAL_I2C_Master_Transmit(&hi2c2, 0x68, (uint8_t *)&dig[num/10], 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, (uint8_t *)&dig[num%10], 1, 10000);
}

void Segment_ShowRCState(uint32_t num){
	static uint8_t step=0;

	switch(step){
		case 0:
			Segment_ShowRC();
			if(num<100)
				step = 3;
			else if(num<10000)
				step = 2;
			else if(num<1000000)
				step = 1;
			else
				step = 0;
			break;
		case 1:
			Segment_ShowNum((num/10000)%100);
			step = 2;
			break;
		case 2:
			Segment_ShowNum((num/100)%100);
			step = 3;
			break;
		case 3:
			Segment_ShowNum(num%100);
			step=0;
			break;
		default:
			break;
	}
}

void Segment_ShowBFState(uint32_t num){
	static uint8_t step=0;

	switch(step){
		case 0:
			Segment_ShowBF();
			if(num<100)
				step = 3;
			else if(num<10000)
				step = 2;
			else if(num<1000000)
				step = 1;
			else
				step = 0;
			break;
		case 1:
			Segment_ShowNum((num/10000)%100);
			step = 2;
			break;
		case 2:
			Segment_ShowNum((num/100)%100);
			step = 3;
			break;
		case 3:
			Segment_ShowNum(num%100);
			step=0;
			break;
		default:
			break;
	}
}

void Segment_ShowNull(void){
	uint8_t null;

	null = 0xff;

	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &null, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, &null, 1, 10000);
}

void Segment_ShowDash(void){
	uint8_t dash;

	dash = 0xbf;

	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &dash, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, &dash, 1, 10000);
}

void Segment_ShowPA(void){
	uint8_t P;
	uint8_t A;

	P = 0x8c;
	A = 0x88;

	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &P, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, &A, 1, 10000);
}

void Segment_ShowBF(void){
	uint8_t b;
	uint8_t F;

	b = 0x83;
	F = 0x8E;

	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &b, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, &F, 1, 10000);

}

void Segment_ShowRC(void){
	uint8_t r;
	uint8_t C;

	r = 0xCE;
	C = 0xC6;

	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &r, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, &C, 1, 10000);
}

void Segment_ShowTE(void){
	uint8_t T;
	uint8_t E;

	T = 0xF8;
	E = 0x86;

	HAL_I2C_Master_Transmit(&hi2c2, 0x68, &T, 1, 10000);
	HAL_I2C_Master_Transmit(&hi2c2, 0x6A, &E, 1, 10000);

}
