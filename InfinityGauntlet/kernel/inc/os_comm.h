#ifndef OS_TASK_COM_H
#define OS_TASK_COM_H
#include "os_config.h"
#define OS_ALWAYS_DELAY 0
#define OS_NO_DELAY 0xffffffff
typedef enum{
	TET_SEM_BIN,
	TET_SEM_CNT,
	TET_MUTEX,
	TET_QUEUE
}OS_TASKCOM_EVENT_TYPE;
typedef enum{
	TES_IDLE,
	TES_BUSY,
	TES_EMPTY,
	TES_NOT_EMPTY
}OS_TASKCOM_ENENT_STATE;
typedef struct os_event{
	OS_TASKCOM_EVENT_TYPE  Type;
	OS_TASKCOM_ENENT_STATE State;
	unsigned short Cnt;
	signed int Info;
	unsigned short Volume;
	unsigned short Front;
	unsigned short Rear;
	void * *Buf;
	volatile unsigned int WaitList[2];
	unsigned int *WaitListBitBand;
}OS_EVENT;
void OSSemBinCreat(OS_EVENT *EventHandle,unsigned int State);
void OSSemBinGive(OS_EVENT *EventHandle);
unsigned char OSSemBinTake(OS_EVENT *EventHandle,unsigned int Ticks);
void OSSemCntCreat(OS_EVENT *EventHandle,unsigned int Cnt);
void OSSemCntGive(OS_EVENT *EventHandle);
unsigned char OSSemCntTake(OS_EVENT *EventHandle,unsigned int Ticks);
void OSMutexCreat(OS_EVENT *EventHandle);
void OSMutexGive(OS_EVENT *EventHandle);
void OSMutexTake(OS_EVENT *EventHandle);
void OSQueueCreat(OS_EVENT *EventHandle,unsigned int MaxNum,void * *Buf);
void OSQueueSend(OS_EVENT *EventHandle,void *Msg);
unsigned char OSQueueReceive(OS_EVENT *EventHandle,void * *pMsg,unsigned int Ticks);
#endif
