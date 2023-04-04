#ifndef OS_BOARD_H
#define OS_BOARD_H

#include "os_config.h"

void OSBoardInit(void);
void OSEnterCritical(void);
void OSExitCritical(void);
unsigned int OSCntLeadZeros(unsigned int Num);
void OSIntMask(void);
void OSClearIntMask(void);
void *OSRoutCreat(void(*Code)(void *), void *Para, unsigned int Sp);
void OSRoutStart(void **Routi);
void OSRoutTo(void **Routi);

#define SYSCLK_FREQ  96000000 // 根据芯片运行时钟进行设置

#endif
