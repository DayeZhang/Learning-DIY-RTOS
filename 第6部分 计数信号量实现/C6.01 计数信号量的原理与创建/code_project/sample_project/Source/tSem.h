/*************************************** Copyright (c)******************************************************
** File name            :   tSem.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的信号量实现
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
#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"
#include "tEvent.h"

// 信号量类型
typedef struct _tSem 
{
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	tEvent event;

	// 当前的计数
	uint32_t count;

	// 最大计数
	uint32_t maxCount;
}tSem;

/**********************************************************************************************************
** Function name        :   tSemInit
** Descriptions         :   初始化信号量
** parameters           :   startCount 初始的计数
** parameters           :   maxCount 最大计数，如果为0，则不限数量
** Returned value       :   无
***********************************************************************************************************/
void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount);

#endif /* TSEM_H */
