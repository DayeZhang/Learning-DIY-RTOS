/*************************************** Copyright (c)******************************************************
** File name            :   tMemBlock.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的存储块的实现
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
#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tConfig.h"
#include "tEvent.h"

typedef struct _tMemBlock
{
	// 事件控制块
    tEvent event;

    // 存储块的首地址
    void * memStart;

    // 每个存储块的大小
    uint32_t blockSize;

    // 总的存储块的个数
    uint32_t maxCount;

    // 存储块列表
    tList blockList;
}tMemBlock;

/**********************************************************************************************************
** Function name        :   tMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   memBlock 等待初始化的存储控制块
** parameters           :   memStart 存储区的起始地址
** parameters           :   blockSize 每个块的大小
** parameters           :   blockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);

/**********************************************************************************************************
** Function name        :   tMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tMemBlockWait (tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   tMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tMemBlockNoWaitGet (tMemBlock * memBlock, void ** mem);

/**********************************************************************************************************
** Function name        :   tMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void tMemBlockNotify (tMemBlock * memBlock, uint8_t * mem);

#endif /* TMEMBLOCK_H */
