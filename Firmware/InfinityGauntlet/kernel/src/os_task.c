#include "os_config.h"
#include "os_board.h"
#include "os_task.h"
#include "string.h"
#include "stdio.h"
#if OS_CONFIG_STK_CHECK_EN
#define OS_TASK_STACK_FILL_BYTE 0xa5 
#endif
OS_TCB OSTaskAllTask[OS_CONFIG_TASK_NUM];
static volatile unsigned int priOSTaskSysTicks=0;
static volatile unsigned int priOSTaskSchedLock = 0;
volatile unsigned int OSTaskReadyList[2];
volatile unsigned int OSTaskDelayList[2];
volatile unsigned int OSTaskSuspendList[2];
volatile unsigned int OSTaskBlockList[2];
volatile unsigned int OSTaskRunList[2];
volatile unsigned int OSTaskDelaySuspendList[2];
volatile unsigned int OSTaskBlockSuspendList[2];
volatile unsigned int *OSTaskReadyListBitBand;
volatile unsigned int *OSTaskDelayListBitBand;
volatile unsigned int *OSTaskSuspendListBitBand;
volatile unsigned int *OSTaskBlockListBitBand;
volatile unsigned int *OSTaskRunListBitBand;						
volatile unsigned int *OSTaskDelaySuspendListBitBand;
volatile unsigned int *OSTaskBlockSuspendListBitBand;

unsigned int OSTaskGetPrio(volatile unsigned int *TaskList)
{
	if(TaskList[0])
	{	
		return (31-OSCntLeadZeros(TaskList[0]));
	}
	else if(TaskList[1])
	{
		return (63-OSCntLeadZeros(TaskList[1]));
	}
	else
		return NO_TASK_IN_THIS_STATE;	
}

/*
 * 初始化任务状态机
 */
static void priOSTaskStateMachineInit(void)
{
	OSTaskReadyList[0]=0x00000000;
	OSTaskReadyList[1]=0x00000000; // 就绪态任务表初始化
	OSTaskDelayList[0]=0x00000000;
	OSTaskDelayList[1]=0x00000000; // 延时态任务表初始化
	OSTaskSuspendList[0]=0x00000000;
	OSTaskSuspendList[1]=0x00000000; // 挂起态任务表初始化
	OSTaskBlockList[0]=0x00000000;
	OSTaskBlockList[1]=0x00000000; // 阻塞态任务表初始化
	OSTaskRunList[0]=0x00000000;
	OSTaskRunList[1]=0x00000000; // 运行态任务表初始化
	OSTaskDelaySuspendList[0]=0x00000000;
	OSTaskDelaySuspendList[1]=0x00000000; // 延迟挂起态任务表初始化
	OSTaskBlockSuspendList[0]=0x00000000;
	OSTaskBlockSuspendList[1]=0x00000000; // 阻塞挂起态任务表初始化
    /*
     * 位带绑定任务表,绑定后可用位带直接读写任务表的指定位
     * 例如：OSTaskReadyListBitBand[3]=1 等价于直接将OSTaskReadyList的第3位直接赋1
     */
	OSTaskReadyListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskReadyList-0x20000000)<<5));
	OSTaskDelayListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskDelayList-0x20000000)<<5));
	OSTaskSuspendListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskSuspendList-0x20000000)<<5));
	OSTaskBlockListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskBlockList-0x20000000)<<5));
	OSTaskRunListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskRunList-0x20000000)<<5));
	OSTaskDelaySuspendListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskDelaySuspendList-0x20000000)<<5));
	OSTaskBlockSuspendListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)OSTaskBlockSuspendList-0x20000000)<<5));
}

static void OSTaskSch(void)
{
	unsigned int HighestPrioInReady;
	unsigned int PrioInRun;
	OSEnterCritical();
	if(priOSTaskSchedLock == 0)
	{
		HighestPrioInReady=OSTaskGetPrio(OSTaskReadyList);
		PrioInRun=OSTaskGetPrio(OSTaskRunList);
		if(HighestPrioInReady == NO_TASK_IN_THIS_STATE )
		{
			OSExitCritical();
			return;
		}
		if(PrioInRun == NO_TASK_IN_THIS_STATE )
		{
			OSTaskStateMachineChangeState(HighestPrioInReady,TSCR_READY_TO_RUN);
			OSRoutTo(&(OSTaskAllTask[HighestPrioInReady].RoutineHandle));
		}
		else if( PrioInRun < HighestPrioInReady )
		{
			OSTaskStateMachineChangeState(PrioInRun,TSCR_RUN_TO_READY);
			OSTaskStateMachineChangeState(HighestPrioInReady,TSCR_READY_TO_RUN);
			OSRoutTo(&(OSTaskAllTask[HighestPrioInReady].RoutineHandle));
		}
	}
	OSExitCritical();
}

void OSTaskStateMachineChangeState(unsigned int TaskPrio,OS_TASK_STATE_CHANGE_ROUTE Route)
{
	OSEnterCritical();
	switch (Route)
	{
		case TSCR_SLEEP_TO_READY:
			SET_READY_LIST(TaskPrio);
			break;
		case TSCR_READY_TO_RUN:
			CLR_READY_LIST(TaskPrio);
			SET_RUN_LIST(TaskPrio);
			break;
		case TSCR_READY_TO_SUSPEND:
			CLR_READY_LIST(TaskPrio);
			SET_SUSPEND_LIST(TaskPrio);
			break;
		case TSCR_RUN_TO_BLOCK:
			CLR_RUN_LIST(TaskPrio);
			SET_BLOCK_LIST(TaskPrio);
			OSTaskSch();
			break;
		case TSCR_RUN_TO_READY:
			CLR_RUN_LIST(TaskPrio);
			SET_READY_LIST(TaskPrio);
			break;
		case TSCR_RUN_TO_SUSPEND:
			CLR_RUN_LIST(TaskPrio);
			SET_SUSPEND_LIST(TaskPrio);
			OSTaskSch();
			break;
		case TSCR_RUN_TO_DELAY:
			CLR_RUN_LIST(TaskPrio);
			SET_DELAY_LIST(TaskPrio);
			OSTaskSch();
			break;
		case TSCR_DELAY_TO_READY:
			CLR_DELAY_LIST(TaskPrio);
			SET_READY_LIST(TaskPrio);
			OSTaskSch();
			break;
		case TSCR_DELAY_TO_DELAYSUSPEND:
			CLR_DELAY_LIST(TaskPrio);
			SET_DELAYSUSPEND_LIST(TaskPrio);
			break;
		case TSCR_SUSPEND_TO_READY:
			CLR_SUSPEND_LIST(TaskPrio);
			SET_READY_LIST(TaskPrio);
			OSTaskSch();
			break;
		case TSCR_BLOCK_TO_BLOCKSUSPEND:
			CLR_BLOCK_LIST(TaskPrio);
			SET_BLOCKSUSPEND_LIST(TaskPrio);
			break;
		case TSCR_BLOCK_TO_READY:
			CLR_BLOCK_LIST(TaskPrio);
			SET_READY_LIST(TaskPrio);
			OSTaskSch();
			break;
		case TSCR_BLOCKSUSPEND_TO_BLOCK:
			CLR_BLOCKSUSPEND_LIST(TaskPrio);
			SET_BLOCK_LIST(TaskPrio);
			break;
		case TSCR_BLOCKSUSPEND_TO_SUSPEND:
			CLR_BLOCKSUSPEND_LIST(TaskPrio);
			SET_SUSPEND_LIST(TaskPrio);
			break;
		case TSCR_DELAYSUSPEND_TO_DELAY:
			CLR_DELAYSUSPEND_LIST(TaskPrio);
			SET_DELAY_LIST(TaskPrio);
			break;
		case TSCR_DELAYSUSPEND_TO_SUSPEND:
			CLR_DELAYSUSPEND_LIST(TaskPrio);
			SET_SUSPEND_LIST(TaskPrio);
			break;
		default:
			break;
	}
	OSExitCritical();
}

unsigned char OSCreatTask(void (*Code)(void *), void *Para, void *StkBuf,unsigned int StkSize, const char *TaskName,unsigned int Prio)
{
	if(Prio>=OS_CONFIG_TASK_NUM || StkSize<64 )
		return 0;
	OSEnterCritical();
	#if OS_CONFIG_STK_CHECK_EN
	memset( StkBuf, OS_TASK_STACK_FILL_BYTE, StkSize );
	OSTaskAllTask[Prio].StkBuf  = StkBuf;
	OSTaskAllTask[Prio].StkSize = StkSize;
	#endif
	OSTaskAllTask[Prio].RoutineHandle = OSRoutCreat(Code,Para,(unsigned int)StkBuf+StkSize);
	OSTaskAllTask[Prio].Name = TaskName;
	OSTaskAllTask[Prio].DelayTicks = 0;
	OSTaskAllTask[Prio].BlockTicks = 0;
	OSTaskAllTask[Prio].WaitEvent =  0;
	OSTaskAllTask[Prio].IfOverTime = 0;
	OSTaskStateMachineChangeState(Prio,TSCR_SLEEP_TO_READY);
	OSExitCritical();
	return 1;
}

void OSInit(void)
{
	OSBoardInit();
	priOSTaskStateMachineInit();
}

void OSStart(void)
{
	unsigned int HighestPrioInReady;	
	HighestPrioInReady=OSTaskGetPrio(OSTaskReadyList);
	CLR_READY_LIST(HighestPrioInReady);
	SET_RUN_LIST(HighestPrioInReady);
	OSRoutStart(&(OSTaskAllTask[HighestPrioInReady].RoutineHandle));
}

void OSDelayTask(unsigned int Ticks)
{
	unsigned int PrioInRun;
	OSEnterCritical();
	PrioInRun = OSTaskGetPrio(OSTaskRunList);
	OSTaskAllTask[PrioInRun].DelayTicks = Ticks;
	OSTaskStateMachineChangeState(PrioInRun,TSCR_RUN_TO_DELAY);
	OSExitCritical();
}

void OSTaskTick(void)
{
	unsigned int Prio;
	priOSTaskSysTicks++;
	for(Prio=0;Prio<OS_CONFIG_TASK_NUM;Prio++)
	{
		if(GET_DELAY_LIST(Prio))
		{
			if( OSTaskAllTask[Prio].DelayTicks>0 )
			{
				(OSTaskAllTask[Prio].DelayTicks)--;
				if(OSTaskAllTask[Prio].DelayTicks==0)
					OSTaskStateMachineChangeState(Prio,TSCR_DELAY_TO_READY);
			}		
		}
		else if(GET_DELAYSUSPEND_LIST(Prio))
		{
			if( OSTaskAllTask[Prio].DelayTicks>0 )
			{
				(OSTaskAllTask[Prio].DelayTicks)--;
				if(OSTaskAllTask[Prio].DelayTicks==0)
					OSTaskStateMachineChangeState(Prio,TSCR_DELAYSUSPEND_TO_SUSPEND);
			}			
		}
		else if(GET_BLOCK_LIST(Prio))
		{
			if( OSTaskAllTask[Prio].BlockTicks>0 )
			{
				(OSTaskAllTask[Prio].BlockTicks)--;
				if(OSTaskAllTask[Prio].BlockTicks==0)
				{
					(OSTaskAllTask[Prio].WaitEvent)->WaitListBitBand[Prio]=0;
					OSTaskAllTask[Prio].IfOverTime = 1;
					OSTaskStateMachineChangeState(Prio,TSCR_BLOCK_TO_READY);
				}
			}		
		}
		else if(GET_BLOCKSUSPEND_LIST(Prio))
		{
			if( OSTaskAllTask[Prio].BlockTicks>0 )
			{
				(OSTaskAllTask[Prio].BlockTicks)--;
				if(OSTaskAllTask[Prio].BlockTicks==0)
				{
					(OSTaskAllTask[Prio].WaitEvent)->WaitListBitBand[Prio]=0;
					OSTaskAllTask[Prio].IfOverTime = 1;
					OSTaskStateMachineChangeState(Prio,TSCR_BLOCKSUSPEND_TO_SUSPEND);
				}
			}		
		}	
	}	
}

void OSSuspendTask(unsigned int Prio)
{
	if(Prio>=OS_CONFIG_TASK_NUM)
		return ;
	OSEnterCritical();
	if(GET_SUSPEND_LIST(Prio) || GET_DELAYSUSPEND_LIST(Prio) || GET_BLOCKSUSPEND_LIST(Prio) )
	{
		OSExitCritical();
		return ;
	}
	if(GET_READY_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_READY_TO_SUSPEND);
	else if(GET_RUN_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_RUN_TO_SUSPEND);
	else if(GET_DELAY_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_DELAY_TO_DELAYSUSPEND);
	else if(GET_BLOCK_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_BLOCK_TO_BLOCKSUSPEND);
	OSExitCritical();
}

void OSResumeTask(unsigned int Prio)
{
	if(Prio>=OS_CONFIG_TASK_NUM)
		return;
	OSEnterCritical();
	if( GET_SUSPEND_LIST(Prio)==0 && GET_DELAYSUSPEND_LIST(Prio)==0 && GET_BLOCKSUSPEND_LIST(Prio)==0 )
	{
		OSExitCritical();
		return;
	}
	if(GET_SUSPEND_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_SUSPEND_TO_READY);
	else if(GET_DELAYSUSPEND_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_DELAYSUSPEND_TO_DELAY);
	else if(GET_BLOCKSUSPEND_LIST(Prio))
		OSTaskStateMachineChangeState(Prio,TSCR_BLOCKSUSPEND_TO_BLOCK);
	OSExitCritical();
}

void OSSchedLock(void)
{
	priOSTaskSchedLock++;
}

void OSSchedUnlock(void)
{
	OSEnterCritical();
	if(priOSTaskSchedLock>0)
		priOSTaskSchedLock--;
	else
		OSTaskSch();
	OSExitCritical();
}

unsigned int OSGetCurrentTick(void)
{
	return priOSTaskSysTicks;
}

unsigned int OSGetCurrentSysMs(void)
{
	return priOSTaskSysTicks*1000/OS_CONFIG_TICK_RATE_HZ;
}

#if OS_CONFIG_STK_CHECK_EN
void OSStkCheck(unsigned int TaskPrio, OS_STK_INFO *StkInfo)
{
    int i;
    for(i=0;i<OSTaskAllTask[TaskPrio].StkSize;i++)
    {
        if(((unsigned char*)(OSTaskAllTask[TaskPrio].StkBuf))[i]!=OS_TASK_STACK_FILL_BYTE)
            break;
    }
    StkInfo->StkBase = (unsigned int)OSTaskAllTask[TaskPrio].StkBuf;
    StkInfo->StkSize = OSTaskAllTask[TaskPrio].StkSize;
    StkInfo->StkRem = i;
}
#endif
