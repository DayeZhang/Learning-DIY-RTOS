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
#ifndef TEVENT_H
#define TEVENT_H

#include "tConfig.h"
#include "tLib.h"

// Event类型
typedef enum  _tEventType {   
    tEventTypeUnknown   = 0, 				// 未知类型
 }tEventType;

// Event控制结构
typedef struct _tEvent {
    tEventType type;						// Event类型

    tList waitList;							// 任务等待列表
}tEvent;

/**********************************************************************************************************
** Function name        :   tEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   event 事件控制块
** parameters           :   type 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void tEventInit (tEvent * event, tEventType type);

#endif /* TEVENT_H */
