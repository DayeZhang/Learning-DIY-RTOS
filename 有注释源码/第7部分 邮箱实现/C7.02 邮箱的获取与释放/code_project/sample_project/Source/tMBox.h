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

#define tMBOXSendNormal             0x00        // 正常发送发送至缓冲区
#define tMBOXSendFront              0x01        // 消息发送至缓冲区头部

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

/**********************************************************************************************************
** Function name        :   tMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   mbox 等待的邮箱
** parameters           :   msg 消息存储缓存区
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tMboxWait (tMbox * mbox, void **msg, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   tMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   mbox 获取消息的邮箱
** parameters           :   msg 消息存储缓存区
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tMboxNoWaitGet (tMbox * mbox, void **msg);

/**********************************************************************************************************
** Function name        :   tMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t tMboxNotify (tMbox * mbox, void * msg, uint32_t notifyOption);

#endif /* TMBOX_H */ 
