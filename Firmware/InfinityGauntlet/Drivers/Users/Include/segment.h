/*
 * segment.h
 *
 *  Created on: 2022年4月22日
 *      Author: aloha
 */

#ifndef USERS_INCLUDE_SEGMENT_H_
#define USERS_INCLUDE_SEGMENT_H_

#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

void Segment_Init(void);
void Segment_Deinit(void);
void Segment_ShowErrorCode(uint8_t reason);
void Segment_ShowNum(uint8_t num);
void Segment_ShowNull(void);
void Segment_ShowDash(void);
void Segment_ShowPA(void);
void Segment_ShowBF(void);
void Segment_ShowRC(void);
void Segment_ShowTE(void);
void Segment_ShowBFState(uint32_t num);
void Segment_ShowRCState(uint32_t num);

#endif /* USERS_INCLUDE_SEGMENT_H_ */
