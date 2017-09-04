/*************************************** Copyright (c)******************************************************
** File name            :   tMBox.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的事件控制结构实现
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
#include "tMBox.h"
#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   tMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   mbox 等待初始化的邮箱
** parameters           :   msgBuffer 消息存储缓冲区
** parameters           :   maxCount 最大计数
** Returned value       :   无
***********************************************************************************************************/
void tMboxInit (tMbox * mbox, void ** msgBuffer, uint32_t maxCount) 
{
	tEventInit(&mbox->event, tEventTypeMbox);

	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->read = 0;
	mbox->write = 0;
	mbox->count = 0;
}
