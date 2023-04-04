/*
 * switch.c
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */
#include "switch.h"

uint16_t SW_GetStates(void){
	uint16_t states = SW_UNKNOWN;

	states += (uint16_t)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
	states += (uint16_t)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2)<<1;
	states += (uint16_t)HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15)<<2;
	states += (uint16_t)HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7)<<3;
	states += (1-(uint16_t)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))<<4;
	return states;
}
