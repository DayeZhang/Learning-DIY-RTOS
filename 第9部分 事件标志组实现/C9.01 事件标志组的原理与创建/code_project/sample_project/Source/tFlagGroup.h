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

/**********************************************************************************************************
** Function name        :   tFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   flags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags);

#endif /* TFLAGGROUP_H */
