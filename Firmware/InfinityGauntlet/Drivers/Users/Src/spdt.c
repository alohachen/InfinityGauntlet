/*
 * spdt.c
 *
 *  Created on: Apr 24, 2022
 *      Author: aloha
 */

#include "spdt.h"

void SPDT_FakeMISO_On(void){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
}

void SPDT_FakeMISO_Off(void){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
}

void SPDT_ReadMOSI(void){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void SPDT_ReadMISO(void){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}
