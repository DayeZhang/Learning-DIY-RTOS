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

// 下面的代码中，用到了C文件嵌入ARM汇编
// 基本语法为:__asm 返回值 函数名(参数声明) {....}， 更具体的用法见Keil编译器手册，此处不再详注。

/**********************************************************************************************************
** Function name        :   tTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t tTaskEnterCritical (void)
{
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

/**********************************************************************************************************
** Function name        :   PendSV_Handler
** Descriptions         :   PendSV异常处理函数。很有些会奇怪，看不到这个函数有在哪里调用。实际上，只要保持函数头不变
**                          void PendSV_Handler (), 在PendSV发生时，该函数会被自动调用
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
__asm void PendSV_Handler ()
{
    IMPORT  currentTask               // 使用import导入C文件中声明的全局变量
    IMPORT  nextTask                  // 类似于在C文文件中使用extern int variable

    MRS     R0, PSP                   // 获取当前任务的堆栈指针
    CBZ     R0, PendSVHandler_nosave  // if 这是由tTaskSwitch触发的(此时，PSP肯定不会是0了，0的话必定是tTaskRunFirst)触发
                                      // 不清楚的话，可以先看tTaskRunFirst和tTaskSwitch的实现
    STMDB   R0!, {R4-R11}             //     那么，我们需要将除异常自动保存的寄存器这外的其它寄存器自动保存起来{R4, R11}
                                      //     保存的地址是当前任务的PSP堆栈中，这样就完整的保存了必要的CPU寄存器,便于下次恢复
    LDR     R1, =currentTask          //     保存好后，将最后的堆栈顶位置，保存到currentTask->stack处
    LDR     R1, [R1]                  //     由于stack处在结构体stack处的开始位置处，显然currentTask和stack在内存中的起始
    STR     R0, [R1]                  //     地址是一样的，这么做不会有任何问题

PendSVHandler_nosave                  // 无论是tTaskSwitch和tTaskSwitch触发的，最后都要从下一个要运行的任务的堆栈中恢复
                                      // CPU寄存器，然后切换至该任务中运行
    LDR     R0, =currentTask          // 好了，准备切换了
    LDR     R1, =nextTask
    LDR     R2, [R1]
    STR     R2, [R0]                  // 先将currentTask设置为nextTask，也就是下一任务变成了当前任务

    LDR     R0, [R2]                  // 然后，从currentTask中加载stack，这样好知道从哪个位置取出CPU寄存器恢复运行
    LDMIA   R0!, {R4-R11}             // 恢复{R4, R11}。为什么只恢复了这么点，因为其余在退出PendSV时，硬件自动恢复

    MSR     PSP, R0                   // 最后，恢复真正的堆栈指针到PSP
    ORR     LR, LR, #0x04             // 标记下返回标记，指明在退出LR时，切换到PSP堆栈中(PendSV使用的是MSP)
    BX      LR                        // 最后返回，此时任务就会从堆栈中取出LR值，恢复到上次运行的位置
}

/**********************************************************************************************************
** Function name        :   tTaskRunFirst
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskRunFirst()
{
    // 这里设置了一个标记，PSP = 0, 用于与tTaskSwitch()区分，用于在PEND_SV
    // 中判断当前切换是tinyOS启动时切换至第1个任务，还是多任务已经跑起来后执行的切换
    __set_PSP(0);

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
void tTaskSwitch()
{
    // 和tTaskRunFirst, 这个函数会在某个任务中调用，然后触发PendSV切换至其它任务
    // 之后的某个时候，将会再次切换到该任务运行，此时，开始运行该行代码, 返回到
    // tTaskSwitch调用处继续往下运行
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}
