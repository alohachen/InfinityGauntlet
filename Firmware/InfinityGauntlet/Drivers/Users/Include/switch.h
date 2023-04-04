/*
 * switch.h
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */

#ifndef USERS_INCLUDE_SWITCH_H_
#define USERS_INCLUDE_SWITCH_H_

#include "stm32f4xx_hal.h"

#endif /* USERS_INCLUDE_SWITCH_H_ */

#define SW_UNKNOWN		((uint16_t)0x0000)
#define SW_PA			((uint16_t)0x0001)
#define SW_BF			((uint16_t)0x0002)
#define SW_RC			((uint16_t)0x0004)
#define SW_TE			((uint16_t)0x0008)
#define SW_SNAP			((uint16_t)0x0010)

uint16_t SW_GetStates(void);
