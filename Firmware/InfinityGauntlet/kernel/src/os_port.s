  .syntax unified
    .cpu cortex-m3
    .thumb

.equ OS_CONFIG_SUPER_ISR_PRIO, 16
.equ NVIC_INT_CTRL, 0xe000ed04
.equ NVIC_SYSPRI2,  0xe000ed20
.equ NVIC_PENDSVSET,    0x10000000
.equ NVIC_PENDSV_PRI,   0x00ff0000

.global PendSV_Handler
.global SVC_Handler
.global OSIntMask
.global OSClearIntMask
.global OSRoutCreat
.global OSRoutStart
.global OSRoutTo
.global OSCntLeadZeros

    .section    .data
OSCurRout:
    .word 0
OSRoutRdy:
    .word 0

    .section    .text.PendSV_Handler
    .type   PendSV_Handler, %function
PendSV_Handler:
    mrs   r0, psp
    stmdb r0!, {r4-r11}
    ldr   r1, =OSCurRout
    ldr   r12, [r1]
    str   r0, [r12]
    ldr   r12, =OSRoutRdy
    ldr   r2,[r12]
    ldr   r0, [r2]
    ldmia r0!, {r4-r11}
    str   r2  , [r1]
    msr   psp, r0
    bx    lr
    .size PendSV_Handler, .-PendSV_Handler

   .section    .text.SVC_Handler
   .type   SVC_Handler, %function
SVC_Handler:
    ldr   r12, =OSRoutRdy
    ldr   r12, [r12]
    ldr   r0, [r12]
    ldmia r0!,{R4-R11}
    msr   psp, r0
    ldr   r12, =NVIC_SYSPRI2
    ldr   r0,  =NVIC_PENDSV_PRI
    ldr   r1, [r12]
    orr   r0, r1,r0
    str   r0, [r12]
    ldr   r0, =NVIC_INT_CTRL
    ldr   r1, =NVIC_PENDSVSET
    str   r1, [r0]
    orr   lr, lr,#0xd
    bx    lr
    .size SVC_Handler, .-SVC_Handler

   .section    .text.OSIntMask
   .type   OSIntMask, %function
OSIntMask:
    push { r0 }
    ldr r0, =OS_CONFIG_SUPER_ISR_PRIO
    msr basepri, r0
    pop { r0 }
    bx  lr
    .size OSIntMask, .-OSIntMask

   .section    .text.OSClearIntMask
   .type   OSClearIntMask, %function
OSClearIntMask:
    push { r0 }
    mov r0, #0
    msr basepri, r0
    pop { r0 }
    bx lr
    .size OSClearIntMask, .-OSClearIntMask

   .section    .text.OSRoutCreat
   .type   OSRoutCreat, %function
OSRoutCreat:
    sub    r2,  #0x40
    str    r1, [r2,#0x20]
    ldr    r12, =0xfffffff0
    str    r12, [r2,#0x34]
    str    r0, [r2,#0x38]
    ldr    r12, =0x01000000
    str    r12, [r2,#0x3c]
    mov    r0,  r2
    bx     lr
    .size OSRoutCreat, .-OSRoutCreat

   .section    .text.OSRoutStart
   .type   OSRoutStart, %function
OSRoutStart:
    ldr   r12, =OSRoutRdy
    str   r0, [r12]
    ldr   r12, =OSCurRout
    str   r0, [r12]
    ldr   r12, =0xE000ED08
    ldr   r12, [r12]
    ldr   r12, [r12]
    msr   msp, r12
    svc   0
    bx    lr
    .size OSRoutStart, .-OSRoutStart

    .section    .text.OSRoutTo
    .type   OSRoutTo, %function
OSRoutTo:
    ldr   r12, =OSRoutRdy
    str   r0, [r12]
    ldr   r12, =NVIC_INT_CTRL
    ldr   r1, =NVIC_PENDSVSET
    str   r1, [r12]
    bx    lr
    .size OSRoutTo, .-OSRoutTo

    .section    .text.OSCntLeadZeros
    .type   OSCntLeadZeros, %function
OSCntLeadZeros:
	clz   r0, r0
	bx    lr 
    .size OSCntLeadZeros, .-OSCntLeadZeros
