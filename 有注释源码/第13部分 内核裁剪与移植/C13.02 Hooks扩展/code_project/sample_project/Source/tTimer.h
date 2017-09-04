/*************************************** Copyright (c)******************************************************
** File name            :   tTimer.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的软定时器实现
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
#ifndef TTIMER_H
#define TTIMER_H

#include "tConfig.h"
#include "tEvent.h"

// 定时器状态
typedef enum _tTimerState
{
    tTimerCreated,          // 定时器已经创建
    tTimerStarted,          // 定时器已经启动
    tTimerRunning,          // 定时器正在执行回调函数
    tTimerStopped,          // 定时器已经停止
    tTimerDestroyed         // 定时器已经销毁
}tTimerState;

// 软定时器结构
typedef struct _tTimer
{
    // 链表结点
    tNode linkNode;

    // 初次启动延后的ticks数
    uint32_t startDelayTicks;

    // 周期定时时的周期tick数
    uint32_t durationTicks;

    // 当前定时递减计数值
    uint32_t delayTicks;

    // 定时回调函数
    void (*timerFunc) (void * arg);

    // 传递给回调函数的参数
    void * arg;

    // 定时器配置参数
    uint32_t config;

    // 定时器状态
    tTimerState state;
}tTimer;

// 软定时器状态信息
typedef struct _tTimerInfo
{
    // 初次启动延后的ticks数
    uint32_t startDelayTicks;

    // 周期定时时的周期tick数
    uint32_t durationTicks;

    // 定时回调函数
    void (*timerFunc) (void * arg);

    // 传递给回调函数的参数
    void * arg;

    // 定时器配置参数
    uint32_t config;

    // 定时器状态
    tTimerState state;
}tTimerInfo;

// 软硬定时器
#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

/**********************************************************************************************************
** Function name        :   tTimerInit
** Descriptions         :   初始化定时器
** parameters           :   timer 等待初始化的定时器
** parameters           :   delayTicks 定时器初始启动的延时ticks数。
** parameters           :   durationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   timerFunc 定时器回调函数
** parameters           :   arg 传递给定时器回调函数的参数
** parameters           :   timerFunc 定时器回调函数
** parameters           :   config 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void * arg), void * arg, uint32_t config);

/**********************************************************************************************************
** Function name        :   tTimerStart
** Descriptions         :   启动定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void tTimerStart (tTimer * timer);

/**********************************************************************************************************
** Function name        :   tTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void tTimerStop (tTimer * timer);

/**********************************************************************************************************
** Function name        :   tTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   timer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void tTimerDestroy (tTimer * timer);

/**********************************************************************************************************
** Function name        :   tTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   timer 查询的定时器
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tTimerGetInfo (tTimer * timer, tTimerInfo * info);

/**********************************************************************************************************
** Function name        :   tTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimerModuleTickNotify (void);

/**********************************************************************************************************
** Function name        :   tTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimerModuleInit (void);

/**********************************************************************************************************
** Function name        :   tTimerInitTask
** Descriptions         :   初始化软定时器任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimerInitTask (void);

#endif /* TTIMER_H */
