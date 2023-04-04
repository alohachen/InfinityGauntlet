/*
 * spdt.h
 *
 *  Created on: Apr 24, 2022
 *      Author: aloha
 */

#ifndef USERS_INCLUDE_SPDT_H_
#define USERS_INCLUDE_SPDT_H_

#include "stm32f4xx_hal.h"

void SPDT_FakeMISO_On(void);
void SPDT_FakeMISO_Off(void);

void SPDT_ReadMOSI(void);
void SPDT_ReadMISO(void);

#endif /* USERS_INCLUDE_SPDT_H_ */
