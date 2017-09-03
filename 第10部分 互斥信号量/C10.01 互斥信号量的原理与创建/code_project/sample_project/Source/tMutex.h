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

/**********************************************************************************************************
** Function name        :   tMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void tMutexInit (tMutex * mutex);

#endif /* TMUTEX_H */
