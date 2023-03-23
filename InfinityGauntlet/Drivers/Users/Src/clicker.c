/*
 * clicker.c
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */

#include "beep.h"

void Clicker_Press(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}

void Clicker_Release(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}
