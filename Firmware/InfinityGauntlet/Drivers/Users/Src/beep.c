/*
 * beep.c
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */

#include "beep.h"

void Beep_On(void){
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
}

void Beep_Off(void){
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
}
