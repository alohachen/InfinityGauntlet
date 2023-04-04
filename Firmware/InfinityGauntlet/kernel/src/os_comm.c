#include "os_config.h"
#include "os_board.h"
#include "os_task.h"
#include "os_comm.h"

void OSSemBinCreat(OS_EVENT *EventHandle,unsigned int State)
{
	unsigned int Addr;	
	EventHandle->Type = TET_SEM_BIN;
	if(State>0)
		EventHandle->Cnt=1;
	else
		EventHandle->Cnt=0;
	EventHandle->WaitList[0]=0x00000000;
	EventHandle->WaitList[1]=0x00000000;
	Addr=(unsigned int)(EventHandle->WaitList);
	EventHandle->WaitListBitBand=(unsigned int *)(0x22000000 + (Addr-0x20000000)*32);
}

void OSSemBinGive(OS_EVENT *EventHandle)
{
	OSEnterCritical();
	if(EventHandle->Cnt==1)
		;
	else if(EventHandle->Cnt==0)
	{
		unsigned int HighestPrioInWait;
		HighestPrioInWait=OSTaskGetPrio(EventHandle->WaitList);
		if(HighestPrioInWait==NO_TASK_IN_THIS_STATE)
			EventHandle->Cnt=1;
		else if(HighestPrioInWait!=NO_TASK_IN_THIS_STATE)
		{
			EventHandle->WaitListBitBand[HighestPrioInWait]=0;
			if(GET_BLOCKSUSPEND_LIST(HighestPrioInWait))
				OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCKSUSPEND_TO_SUSPEND);
			else if(GET_BLOCK_LIST(HighestPrioInWait))
				OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCK_TO_READY);
		}
	}
	OSExitCritical();	
}

unsigned char OSSemBinTake(OS_EVENT *EventHandle,unsigned int Ticks)
{
	unsigned int PrioInRun;
	OSEnterCritical();
	if(Ticks==OS_NO_DELAY)
	{
		if(EventHandle->Cnt==1)
		{
			OSExitCritical();
			return 1;
		}
		else
		{
			OSExitCritical();
			return 0;
		}
	}
	if(EventHandle->Cnt==1)
	{
		EventHandle->Cnt=0;
		OSExitCritical();
		return 1;
	}
	else
	{
		PrioInRun=OSTaskGetPrio(OSTaskRunList);
		EventHandle->WaitListBitBand[PrioInRun]=1;
		OSTaskAllTask[PrioInRun].BlockTicks = Ticks;
		OSTaskAllTask[PrioInRun].WaitEvent = EventHandle;
		OSTaskStateMachineChangeState(PrioInRun,TSCR_RUN_TO_BLOCK);
		OSExitCritical();
		if(OSTaskAllTask[PrioInRun].IfOverTime == 1)
		{
			OSTaskAllTask[PrioInRun].IfOverTime = 0;
			return 0;
		}
		else
			return 1;
	}
}

void OSSemCntCreat(OS_EVENT *EventHandle,unsigned int Cnt)
{
	unsigned int Addr;	
	EventHandle->Type = TET_SEM_CNT;
	EventHandle->Cnt=Cnt;
	EventHandle->WaitList[0]=0x00000000;
	EventHandle->WaitList[1]=0x00000000;
	Addr=(unsigned int)(EventHandle->WaitList);
	EventHandle->WaitListBitBand=(unsigned int *)(0x22000000 + (Addr-0x20000000)*32);
}

void OSSemCntGive(OS_EVENT *EventHandle)
{
	OSEnterCritical();
	if(EventHandle->Cnt>0)
		EventHandle->Cnt++;
	else if(EventHandle->Cnt==0)
	{
		unsigned int HighestPrioInWait;
		HighestPrioInWait=OSTaskGetPrio(EventHandle->WaitList);
		if(HighestPrioInWait==NO_TASK_IN_THIS_STATE)
			EventHandle->Cnt=1;
		else if(HighestPrioInWait!=NO_TASK_IN_THIS_STATE)
		{
			EventHandle->WaitListBitBand[HighestPrioInWait]=0;
			if(GET_BLOCKSUSPEND_LIST(HighestPrioInWait))
			   OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCKSUSPEND_TO_SUSPEND);
			else if(GET_BLOCK_LIST(HighestPrioInWait))
				OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCK_TO_READY);
		}
			
	}
	OSExitCritical();	
}

unsigned char OSSemCntTake(OS_EVENT *EventHandle,unsigned int Ticks)
{
	OSEnterCritical();
	if(Ticks==OS_NO_DELAY)
	{
		if(EventHandle->Cnt>0)
		{
			OSExitCritical();
			return 1;
		}
		else
		{
			OSExitCritical();
			return 0;
		}
	}
	if(EventHandle->Cnt>0)
	{
		EventHandle->Cnt--;
		OSExitCritical();
		return 1;
	}
	else
	{
		unsigned int PrioInRun;
		PrioInRun=OSTaskGetPrio(OSTaskRunList);
		EventHandle->WaitListBitBand[PrioInRun]=1;
		OSTaskAllTask[PrioInRun].BlockTicks = Ticks;
		OSTaskAllTask[PrioInRun].WaitEvent = EventHandle;
		OSTaskAllTask[PrioInRun].IfOverTime = 0;
		OSTaskStateMachineChangeState(PrioInRun,TSCR_RUN_TO_BLOCK);
		OSExitCritical();
		if(OSTaskAllTask[PrioInRun].IfOverTime == 1)
		{
			OSTaskAllTask[PrioInRun].IfOverTime = 0;
			return 0;
		}
		else
			return 1;
	}
	

}

void OSMutexCreat(OS_EVENT *EventHandle)
{
	unsigned int Addr;	
	EventHandle->Type = TET_MUTEX;
	EventHandle->State=TES_IDLE;
	EventHandle->WaitList[0]=0x00000000;
	EventHandle->WaitList[1]=0x00000000;
	Addr=(unsigned int)(EventHandle->WaitList);
	EventHandle->WaitListBitBand=(unsigned int *)(0x22000000 + (Addr-0x20000000)*32);
}

void OSMutexGive(OS_EVENT *EventHandle)
{
	OSEnterCritical();
	if(EventHandle->State==TES_IDLE)
		;
	else if(EventHandle->State==TES_BUSY)
	{
		unsigned int PrioInRun;
		unsigned int HighestPrioInWait;
		PrioInRun=OSTaskGetPrio(OSTaskRunList);
		if(PrioInRun!=EventHandle->Info)
			;
		else if(PrioInRun==EventHandle->Info)
		{
			HighestPrioInWait=OSTaskGetPrio(EventHandle->WaitList);
			if(HighestPrioInWait==NO_TASK_IN_THIS_STATE)
				EventHandle->State=TES_IDLE;
			else if(HighestPrioInWait!=NO_TASK_IN_THIS_STATE)
			{
				EventHandle->WaitListBitBand[HighestPrioInWait]=0;
				if(GET_BLOCKSUSPEND_LIST(HighestPrioInWait))
					OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCKSUSPEND_TO_SUSPEND);
				else if(GET_BLOCK_LIST(HighestPrioInWait))
					OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCK_TO_READY);
			}	
		}
	}
	OSExitCritical();	
}

void OSMutexTake(OS_EVENT *EventHandle)
{
	unsigned int PrioInRun;
	OSEnterCritical();
	PrioInRun=OSTaskGetPrio(OSTaskRunList);
	if(EventHandle->State==TES_IDLE)
	{
		EventHandle->State=TES_BUSY;
		EventHandle->Info=PrioInRun;
	}
	else
	{
		if(PrioInRun==EventHandle->Info)
			;
		else if(PrioInRun!=EventHandle->Info)
		{
			EventHandle->WaitListBitBand[PrioInRun]=1;
			OSTaskStateMachineChangeState(PrioInRun,TSCR_RUN_TO_BLOCK);
		}
	}
	OSExitCritical();
}

void OSQueueCreat(OS_EVENT *EventHandle,unsigned int MaxNum,void * *Buf)
{
	if(MaxNum==0)
		return;	
	EventHandle->Type = TET_QUEUE;
	EventHandle->State= TES_EMPTY;
	EventHandle->Buf  = Buf;
	EventHandle->Volume = MaxNum;
	EventHandle->Cnt  = 0;
    EventHandle->Front = 0;
	EventHandle->Rear = 0;
	EventHandle->WaitList[0]=0x00000000;
	EventHandle->WaitList[1]=0x00000000;
	EventHandle->WaitListBitBand=(unsigned int *)(0x22000000 + (((unsigned int)(EventHandle->WaitList)-0x20000000)<<5));
}

void OSQueueSend(OS_EVENT *EventHandle,void *Msg)
{
	OSEnterCritical();
	if(EventHandle->State == TES_NOT_EMPTY)
	{
		if( EventHandle->Cnt == EventHandle->Volume )
			;
		else if( EventHandle->Cnt != EventHandle->Volume )
		{
			(EventHandle->Buf)[EventHandle->Rear] = Msg;
			EventHandle->Rear = (EventHandle->Rear+1)%(EventHandle->Volume);
			EventHandle->Cnt++;
		}
	}
	else if(EventHandle->State == TES_EMPTY )
	{
		unsigned int HighestPrioInWait;
		HighestPrioInWait=OSTaskGetPrio(EventHandle->WaitList);
		if(HighestPrioInWait==NO_TASK_IN_THIS_STATE)
		{
			if( EventHandle->Cnt == EventHandle->Volume )
				;
			else if( EventHandle->Cnt != EventHandle->Volume )
			{
				(EventHandle->Buf)[EventHandle->Rear] = Msg;
				EventHandle->Rear = (EventHandle->Rear+1)%(EventHandle->Volume);
				EventHandle->Cnt++;
			}
			EventHandle->State = TES_NOT_EMPTY;
		}
		else if(HighestPrioInWait!=NO_TASK_IN_THIS_STATE)
		{
			OSTaskAllTask[HighestPrioInWait].Msg = Msg;
			EventHandle->WaitListBitBand[HighestPrioInWait]=0;
			if(GET_BLOCKSUSPEND_LIST(HighestPrioInWait))
			   OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCKSUSPEND_TO_SUSPEND);
			else if(GET_BLOCK_LIST(HighestPrioInWait))
				OSTaskStateMachineChangeState(HighestPrioInWait,TSCR_BLOCK_TO_READY);
		}
	}
	OSExitCritical();
}

unsigned char OSQueueReceive(OS_EVENT *EventHandle,void * *pMsg,unsigned int Ticks)
{	
	unsigned int PrioInRun;
	OSEnterCritical();
	if(Ticks==OS_NO_DELAY)
	{
		if(EventHandle->Cnt>0)
		{
			(*pMsg)=(EventHandle->Buf)[EventHandle->Front];
			EventHandle->Front=(EventHandle->Front+1)%(EventHandle->Volume);
			EventHandle->Cnt--;
			if(EventHandle->Cnt==0)
				EventHandle->State = TES_EMPTY;
			OSExitCritical();
			return 1;
		}
		else
		{
			(*pMsg)=0;
			OSExitCritical();
			return 0;
		}
	}
	if(EventHandle->State == TES_NOT_EMPTY)
	{
		if( EventHandle->Cnt ==0 )
		{
			OSExitCritical();
			return 0;
		}
		else if( EventHandle->Cnt !=0 )
		{
			(*pMsg)=(EventHandle->Buf)[EventHandle->Front];
			EventHandle->Front=(EventHandle->Front+1)%(EventHandle->Volume);
			EventHandle->Cnt--;
			if(EventHandle->Cnt==0)
				EventHandle->State = TES_EMPTY;
			OSExitCritical();
			return 1;
		}	
	}
	else if(EventHandle->State == TES_EMPTY)
	{
		PrioInRun=OSTaskGetPrio(OSTaskRunList);
		EventHandle->WaitListBitBand[PrioInRun]=1;
		OSTaskAllTask[PrioInRun].BlockTicks = Ticks; 
		OSTaskAllTask[PrioInRun].WaitEvent = EventHandle;
		OSTaskAllTask[PrioInRun].IfOverTime = 0;
		OSTaskStateMachineChangeState(PrioInRun,TSCR_RUN_TO_BLOCK);
		OSExitCritical();	
		if(OSTaskAllTask[PrioInRun].IfOverTime == 1)
		{
			OSTaskAllTask[PrioInRun].IfOverTime = 0;
			(*pMsg)=0;
			return 0;
		}
		else
		{
			(* pMsg)=OSTaskAllTask[PrioInRun].Msg;
			return 1;
		}		
	}
	return 0;	
}

