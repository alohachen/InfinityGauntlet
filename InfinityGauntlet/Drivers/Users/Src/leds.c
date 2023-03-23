/*
 * leds.c
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */
#include "leds.h"

void LED_ON(uint16_t LED){
	HAL_GPIO_WritePin(GPIOB,LED,GPIO_PIN_SET);
}

void LED_OFF(uint16_t LED){
	HAL_GPIO_WritePin(GPIOB,LED,GPIO_PIN_RESET);
}

void LED_Toggle(uint16_t LED){
	HAL_GPIO_TogglePin(GPIOB, LED);
}
