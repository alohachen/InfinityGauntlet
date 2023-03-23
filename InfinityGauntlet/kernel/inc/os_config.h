#ifndef OS_CONFIG_H
#define OS_CONFIG_H
#define OS_CONFIG_TASK_NUM 4
#define OS_CONFIG_STK_CHECK_EN 1
#define OS_CONFIG_TICK_RATE_HZ 1000

//#define OS_CONFIG_SUPER_ISR_PRIO     16 //请直接在汇编文件中进行设置 
#define OS_CONFIG_PENDSV_PRIO        255
#define OS_CONFIG_SYSTICK_PRIO       255

#endif
