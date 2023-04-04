#include "os_config.h"
#include "os_board.h"
#include "os_task.h"
#include "os_comm.h"

#define  OS_CM4_NVIC_ST_CTRL    (*((volatile unsigned int *)0xE000E010))
#define  OS_CM4_NVIC_ST_RELOAD  (*((volatile unsigned int *)0xE000E014))
#define  OS_CM4_NVIC_SYS_PRIO	(*(volatile unsigned long *) 0xe000ed20 )

#define  OS_CM4_NVIC_ST_CTRL_CLK_SRC 0x00000004
#define  OS_CM4_NVIC_ST_CTRL_INTEN   0x00000002
#define  OS_CM4_NVIC_ST_CTRL_ENABLE  0x00000001


static void OSTickInit(void)
{
	OS_CM4_NVIC_ST_RELOAD = (SYSCLK_FREQ/OS_CONFIG_TICK_RATE_HZ - 1);
	OS_CM4_NVIC_ST_CTRL  |= OS_CM4_NVIC_ST_CTRL_CLK_SRC | OS_CM4_NVIC_ST_CTRL_INTEN | OS_CM4_NVIC_ST_CTRL_ENABLE ;
}

static void OSIsrPrioInit(void)
{
	OS_CM4_NVIC_SYS_PRIO |= ((unsigned long)OS_CONFIG_PENDSV_PRIO<16);
	OS_CM4_NVIC_SYS_PRIO |= ((unsigned long)OS_CONFIG_SYSTICK_PRIO<<24);
}

void OSBoardInit(void)
{
	OSTickInit();
	OSIsrPrioInit();
}

void HAL_IncTick(void);
void SysTick_Handler(void)
{
	OSTaskTick();
	//HAL_IncTick();
}

static volatile unsigned int OSCriticalNesting = 0;

void OSEnterCritical( void )
{
	OSIntMask();
	OSCriticalNesting++;
}

void OSExitCritical( void )
{
	OSCriticalNesting--;
	if( OSCriticalNesting == 0 )
		OSClearIntMask();
}

