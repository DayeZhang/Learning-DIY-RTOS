/*************************************** Copyright (c)******************************************************
** File name            :   tMutex.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的互斥信号量实现
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
#ifndef TMUTEX_H
#define TMUTEX_H

#include "tConfig.h"
#include "tEvent.h"

// 互斥信号量类型
typedef struct  _tMutex
{
    // 事件控制块
    tEvent event;

    // 已被锁定的次数
    uint32_t lockedCount;

    // 拥有者
    tTask * owner;

    // 拥有者原始的优先级
    uint32_t ownerOriginalPrio;
}tMutex;

// 互斥信号量查询结构
typedef struct  _tMutexInfo
{
    // 等待的任务数量
    uint32_t taskCount;

    // 拥有者任务的优先级
    uint32_t ownerPrio;

    // 继承优先级
    uint32_t inheritedPrio;

    // 当前信号量的拥有者
    tTask * owner;

    // 锁定次数
    uint32_t lockedCount;
}tMutexInfo;

/**********************************************************************************************************
** Function name        :   tMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void tMutexInit (tMutex * mutex);

/**********************************************************************************************************
** Function name        :   tMutexWait
** Descriptions         :   等待信号量
** parameters           :   mutex 等待的信号量
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   tMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   tMutex 获取的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tMutexNoWaitGet (tMutex * mutex);

/**********************************************************************************************************
** Function name        :   tMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t tMutexNotify (tMutex * mutex);

/**********************************************************************************************************
** Function name        :   tMutexDestroy
** Descriptions         :   销毁信号量
** parameters           :   mutex 销毁互斥信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t tMutexDestroy (tMutex * mutex);

/**********************************************************************************************************
** Function name        :   tMutexGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mutex 查询的互斥信号量
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tMutexGetInfo (tMutex * mutex, tMutexInfo * info);

#endif /* TMUTEX_H */
