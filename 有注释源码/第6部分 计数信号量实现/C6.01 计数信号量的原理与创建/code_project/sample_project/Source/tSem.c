/*************************************** Copyright (c)******************************************************
** File name            :   tSem.c
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
#include "tSem.h"

/**********************************************************************************************************
** Function name        :   tSemInit
** Descriptions         :   初始化信号量
** parameters           :   startCount 初始的计数
** parameters           :   maxCount 最大计数，如果为0，则不限数量
** Returned value       :   无
***********************************************************************************************************/
void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount) 
{
	tEventInit(&sem->event, tEventTypeSem);

	sem->maxCount = maxCount;
	if (maxCount == 0)
	{
		sem->count = startCount;
	} 
	else 
	{
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
}
