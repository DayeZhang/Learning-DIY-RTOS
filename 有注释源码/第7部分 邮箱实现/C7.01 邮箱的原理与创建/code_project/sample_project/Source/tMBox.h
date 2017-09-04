/*************************************** Copyright (c)******************************************************
** File name            :   tEvent.h
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
#ifndef TMBOX_H
#define TMBOX_H

#include "tConfig.h"
#include "tEvent.h"

// 邮箱类型
typedef struct _tMbox
{
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	tEvent event;

	// 当前的消息数量
    uint32_t count;

    // 读取消息的索引
    uint32_t read;

    // 写消息的索引
    uint32_t write;

    // 最大允许容纳的消息数量
    uint32_t maxCount;

    // 消息存储缓冲区
    void ** msgBuffer;
}tMbox;

/**********************************************************************************************************
** Function name        :   tMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   mbox 等待初始化的邮箱
** parameters           :   msgBuffer 消息存储缓冲区
** parameters           :   maxCount 最大计数
** Returned value       :   无
***********************************************************************************************************/
void tMboxInit (tMbox * mbox, void ** msgBuffer, uint32_t maxCount);

#endif /* TMBOX_H */ 
