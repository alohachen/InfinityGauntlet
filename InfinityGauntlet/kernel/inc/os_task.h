#ifndef OS_TASK_H
#define OS_TASK_H
#include "os_config.h"
#include "os_comm.h"
#define NO_TASK_IN_THIS_STATE 0xff
typedef struct{
	void  *RoutineHandle;
	unsigned int   DelayTicks;
	unsigned int   BlockTicks;
	OS_EVENT *WaitEvent;
	char IfOverTime;
	const char *Name;
	#if OS_CONFIG_STK_CHECK_EN
	void  *StkBuf;
	unsigned int   StkSize;
	#endif
	void *Msg;
}OS_TCB;
extern OS_TCB OSTaskAllTask[OS_CONFIG_TASK_NUM];
extern volatile unsigned int OSTaskReadyList[2];
extern volatile unsigned int OSTaskDelayList[2];
extern volatile unsigned int OSTaskSuspendList[2];
extern volatile unsigned int OSTaskBlockList[2];
extern volatile unsigned int OSTaskRunList[2];
extern volatile unsigned int OSTaskDelaySuspendList[2];
extern volatile unsigned int OSTaskBlockSuspendList[2];
extern volatile unsigned int *OSTaskReadyListBitBand;
extern volatile unsigned int *OSTaskDelayListBitBand;
extern volatile unsigned int *OSTaskSuspendListBitBand;
extern volatile unsigned int *OSTaskBlockListBitBand;
extern volatile unsigned int *OSTaskRunListBitBand;						
extern volatile unsigned int *OSTaskDelaySuspendListBitBand;
extern volatile unsigned int *OSTaskBlockSuspendListBitBand;
#define SET_READY_LIST(TASK_PRIO) do{OSTaskReadyListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_READY_LIST(TASK_PRIO) do{OSTaskReadyListBitBand[TASK_PRIO]=0;}while(0)
#define GET_READY_LIST(TASK_PRIO) OSTaskReadyListBitBand[TASK_PRIO]
#define SET_DELAY_LIST(TASK_PRIO) do{OSTaskDelayListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_DELAY_LIST(TASK_PRIO) do{OSTaskDelayListBitBand[TASK_PRIO]=0;}while(0)
#define GET_DELAY_LIST(TASK_PRIO) OSTaskDelayListBitBand[TASK_PRIO]
#define SET_SUSPEND_LIST(TASK_PRIO) do{OSTaskSuspendListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_SUSPEND_LIST(TASK_PRIO) do{OSTaskSuspendListBitBand[TASK_PRIO]=0;}while(0)
#define GET_SUSPEND_LIST(TASK_PRIO) OSTaskSuspendListBitBand[TASK_PRIO]
#define SET_BLOCK_LIST(TASK_PRIO) do{OSTaskBlockListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_BLOCK_LIST(TASK_PRIO) do{OSTaskBlockListBitBand[TASK_PRIO]=0;}while(0)
#define GET_BLOCK_LIST(TASK_PRIO) OSTaskBlockListBitBand[TASK_PRIO]
#define SET_RUN_LIST(TASK_PRIO) do{OSTaskRunListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_RUN_LIST(TASK_PRIO) do{OSTaskRunListBitBand[TASK_PRIO]=0;}while(0)
#define GET_RUN_LIST(TASK_PRIO) OSTaskRunListBitBand[TASK_PRIO]
#define SET_DELAYSUSPEND_LIST(TASK_PRIO) do{OSTaskDelaySuspendListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_DELAYSUSPEND_LIST(TASK_PRIO) do{OSTaskDelaySuspendListBitBand[TASK_PRIO]=0;}while(0)
#define GET_DELAYSUSPEND_LIST(TASK_PRIO) OSTaskDelaySuspendListBitBand[TASK_PRIO]
#define SET_BLOCKSUSPEND_LIST(TASK_PRIO) do{OSTaskBlockSuspendListBitBand[TASK_PRIO]=1;}while(0)
#define CLR_BLOCKSUSPEND_LIST(TASK_PRIO) do{OSTaskBlockSuspendListBitBand[TASK_PRIO]=0;}while(0)
#define GET_BLOCKSUSPEND_LIST(TASK_PRIO) OSTaskBlockSuspendListBitBand[TASK_PRIO]
typedef enum{
	TSCR_SLEEP_TO_READY,
	TSCR_READY_TO_RUN,
	TSCR_READY_TO_SUSPEND,
	TSCR_RUN_TO_BLOCK,
	TSCR_RUN_TO_READY,
	TSCR_RUN_TO_SUSPEND,
	TSCR_RUN_TO_DELAY,
	TSCR_DELAY_TO_READY,
	TSCR_DELAY_TO_DELAYSUSPEND,
	TSCR_SUSPEND_TO_READY,
	TSCR_BLOCK_TO_BLOCKSUSPEND,
	TSCR_BLOCK_TO_READY,
	TSCR_BLOCKSUSPEND_TO_BLOCK,
	TSCR_BLOCKSUSPEND_TO_SUSPEND,
	TSCR_DELAYSUSPEND_TO_DELAY,
	TSCR_DELAYSUSPEND_TO_SUSPEND 
}OS_TASK_STATE_CHANGE_ROUTE;
unsigned char OSCreatTask(void (*Code)(void *), void *Para, void *StkBuf,unsigned int StkSize, const char *TaskName,unsigned int Prio);
void OSStart(void);
void OSDelayTask(unsigned int Ticks);
void OSSuspendTask(unsigned int Prio);
void OSResumeTask(unsigned int Prio);
void OSSchedLock(void);
void OSSchedUnlock(void);
unsigned int OSGetCurrentTick(void);
unsigned int OSGetCurrentSysMs(void);
#if OS_CONFIG_STK_CHECK_EN
typedef struct{
    unsigned int StkBase;
	unsigned int StkSize;
	unsigned int StkRem;
}OS_STK_INFO;
void OSStkCheck(unsigned int TaskPrio, OS_STK_INFO *StkInfo);
#endif
void OSTaskStateMachineChangeState(unsigned int TaskPrio,OS_TASK_STATE_CHANGE_ROUTE Route);
unsigned int OSTaskGetPrio(volatile unsigned int *TaskList);
void OSTaskTick(void);
void OSInit(void);
#endif
