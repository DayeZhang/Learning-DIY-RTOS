/*************************************** Copyright (c)******************************************************
** File name            :   tMutex.c
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
#include "tMutex.h"
#include "tinyOS.h"
 
/**********************************************************************************************************
** Function name        :   tMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void tMutexInit (tMutex * mutex)
{
    tEventInit(&mutex->event, tEventTypeMutex);

    mutex->lockedCount = 0;
    mutex->owner = (tTask *)0;
    mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;
}
