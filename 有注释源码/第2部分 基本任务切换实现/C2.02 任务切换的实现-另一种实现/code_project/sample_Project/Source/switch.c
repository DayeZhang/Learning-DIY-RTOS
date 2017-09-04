/*************************************** Copyright (c)******************************************************
** File name            :   switch.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS任务切换中与CPU相关的函数。
**
**--------------------------------------------------------------------------------------------------------
** Created by           :   01课堂 lishutong
** Created date         :   2016-06-01
** Version              :   1.0
** Descriptions         :   The original version
**
**--------------------------------------------------------------------------------------------------------
** Copyright            :   版权所有，禁止用于商业用途
** Author Blog          :   http://ilishutong.com
**********************************************************************************************************/
#include "tinyOS.h"
#include "ARMCM3.h"

// 在任务切换中，主要依赖了PendSV进行切换。PendSV其中的一个很重要的作用便是用于支持RTOS的任务切换。
// 实现方法为：
// 1、首先将PendSV的中断优先配置为最低。这样只有在其它所有中断完成后，才会触发该中断；
//    实现方法为：向NVIC_SYSPRI2写NVIC_PENDSV_PRI
// 2、在需要中断切换时，设置挂起位为1，手动触发。这样，当没有其它中断发生时，将会引发PendSV中断。
//    实现方法为：向NVIC_INT_CTRL写NVIC_PENDSVSET
// 3、在PendSV中，执行任务切换操作。
#define NVIC_INT_CTRL       0xE000ED04      // 中断控制及状态寄存器
#define NVIC_PENDSVSET      0x10000000      // 触发软件中断的值
#define NVIC_SYSPRI2        0xE000ED22      // 系统优先级寄存器
#define NVIC_PENDSV_PRI     0x000000FF      // 配置优先级

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)


/**********************************************************************************************************
** Function name        :   tTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t tTaskEnterCritical (void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();        // CPSID I
    return primask;
}

/**********************************************************************************************************
** Function name        :   tTaskExitCritical
** Descriptions         :   退出临界区,恢复之前的临界区状态
** parameters           :   status 进入临界区之前的CPU
** Returned value       :   进入临界区之前的临界区状态值
***********************************************************************************************************/
void tTaskExitCritical (uint32_t status) {
    __set_PRIMASK(status);
}

__asm void PendSV_Handler (void) { 
    IMPORT saveAndLoadStackAddr
    
    // 切换第一个任务时,由于设置了PSP=MSP，所以下面的STMDB保存会将R4~R11
    // 保存到系统启动时默认的堆栈中，而不是某个任务
    MRS     R0, PSP                 
    STMDB   R0!, {R4-R11}               // 将R4~R11保存到当前任务栈，也就是PSP指向的堆栈
    BL      saveAndLoadStackAddr        // 调用函数：参数通过R0传递，返回值也通过R0传递 
    LDMIA   R0!, {R4-R11}               // 从下一任务的堆栈中，恢复R4~R11
    MSR     PSP, R0
    
    MOV     LR, #0xFFFFFFFD             // 指明返回异常时使用PSP。注意，这时LR不是程序返回地址
    BX      LR
}

uint32_t saveAndLoadStackAddr (uint32_t stackAddr) {
    if (currentTask != (tTask *)0) {                    // 第一次切换时，当前任务为0
        currentTask->stack = (uint32_t *)stackAddr;     // 所以不会保存
    }
    currentTask = nextTask;                     
    return (uint32_t)currentTask->stack;                // 取下一任务堆栈地址
}

/**********************************************************************************************************
** Function name        :   tTaskRunFirst
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskRunFirst () {
    // 这里设置了一个标记，PSP = MSP, 用于与tTaskSwitch()区分，用于在PEND_SV
    // 中判断当前切换是tinyOS启动时切换至第1个任务，还是多任务已经跑起来后执行的切换
    __set_PSP(__get_MSP());

    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;   // 向NVIC_SYSPRI2写NVIC_PENDSV_PRI，设置其为最低优先级

    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV

    // 可以看到，这个函数是没有返回
    // 这是因为，一旦触发PendSV后，将会在PendSV后立即进行任务切换，切换至第1个任务运行
    // 此后，tinyOS将负责管理所有任务的运行，永远不会返回到该函数运行
}

/**********************************************************************************************************
** Function name        :   tTaskSwitch
** Descriptions         :   进行一次任务切换，tinyOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至
**                          nextTask运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSwitch () {
    // 和tTaskRunFirst, 这个函数会在某个任务中调用，然后触发PendSV切换至其它任务
    // 之后的某个时候，将会再次切换到该任务运行，此时，开始运行该行代码, 返回到
    // tTaskSwitch调用处继续往下运行
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}
