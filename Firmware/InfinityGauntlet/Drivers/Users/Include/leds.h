/*
 * leds.h
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */

#ifndef USERS_INCLUDE_LEDS_H_
#define USERS_INCLUDE_LEDS_H_

#include "stm32f4xx_hal.h"


#define EYE_LED				GPIO_PIN_13  /* EYE LED selected    */
#define STATE_LED			GPIO_PIN_8  /* STATE LED selected    */
#define ALL_LED				(GPIO_PIN_13|GPIO_PIN_8)

void LED_ON(uint16_t LED);
void LED_OFF(uint16_t LED);
void LED_Toggle(uint16_t LED);


#endif /* USERS_INCLUDE_LEDS_H_ */
