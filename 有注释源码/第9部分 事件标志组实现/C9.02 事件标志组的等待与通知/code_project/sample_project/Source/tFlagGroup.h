/*************************************** Copyright (c)******************************************************
** File name            :   tFlagGroup.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的事件标志实现
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
#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tFlagGroup
{
	// 事件控制块
    tEvent event;

    // 当前事件标志
    uint32_t flags;
}tFlagGroup;


#define	TFLAGGROUP_CLEAR		(0x0 << 0)
#define	TFLAGGROUP_SET			(0x1 << 0)
#define	TFLAGGROUP_ANY			(0x0 << 1)
#define	TFLAGGROUP_ALL			(0x1 << 1)

#define TFLAGGROUP_SET_ALL		(TFLAGGROUP_SET | TFLAGGROUP_ALL)
#define	TFLAGGROUP_SET_ANY		(TFLAGGROUP_SET | TFLAGGROUP_ANY)
#define TFLAGGROUP_CLEAR_ALL	(TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)
#define TFLAGGROUP_CLEAR_ANY	(TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)

#define	TFLAGGROUP_CONSUME		(0x1 << 7)

/**********************************************************************************************************
** Function name        :   tFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   flags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags);

/**********************************************************************************************************
** Function name        :   tFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   flagGroup 等待的事件标志组
** parameters           :   waitType 等待的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** parameters           :   waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tFlagGroupWait (tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag,
						uint32_t * resultFlag, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   tFlagGroupWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   flagGroup 获取的事件标志组
** parameters           :   waitType 获取的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** Returned value       :   获取结果,tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tFlagGroupNoWaitGet (tFlagGroup * flagGroup, uint32_t waitType, uint32_t requstFlag, uint32_t * requestFlag);

/**********************************************************************************************************
** Function name        :   tSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void tFlagGroupNotify (tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags);

#endif /* TFLAGGROUP_H */
