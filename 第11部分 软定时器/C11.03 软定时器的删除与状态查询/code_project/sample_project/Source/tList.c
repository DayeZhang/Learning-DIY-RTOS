/*************************************** Copyright (c)******************************************************
** File name            :   tList.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS所用的双向链表数据结构。
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
#include "tLib.h"

/**********************************************************************************************************
** Function name        :   tNodeInit
** Descriptions         :   初始化结点类型
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tNodeInit (tNode * node)
{
    node->nextNode = node;
    node->preNode = node;
}

// 以下是简化代码编写添加的宏
#define firstNode   headNode.nextNode
#define lastNode    headNode.preNode

/**********************************************************************************************************
** Function name        :   tListInit
** Descriptions         :   链表初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tListInit (tList * list)
{
	list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   tListCount
** Descriptions         :   返回链表中结点的数量
** parameters           :   无
** Returned value       :   结点数量
***********************************************************************************************************/
uint32_t tListCount (tList * list)
{
	return list->nodeCount;
}

/**********************************************************************************************************
** Function name        :   tListFirst
** Descriptions         :   返回链表的首个结点
** parameters           :   list 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
***********************************************************************************************************/
tNode * tListFirst (tList * list)
{
    tNode * node = (tNode *)0;
	
	if (list->nodeCount != 0) 
	{
		node = list->firstNode;
	}    
    return  node;
}

/**********************************************************************************************************
** Function name        :   tListLast
** Descriptions         :   返回链表的最后一个结点
** parameters           :   list 查询的链表
** Returned value       :   最后的结点，如果链表为空，则返回0
***********************************************************************************************************/
tNode * tListLast (tList * list)
{
    tNode * node = (tNode *)0;
	
	if (list->nodeCount != 0) 
	{
		node = list->lastNode;
	}    
    return  node;
}

/**********************************************************************************************************
** Function name        :   tListPre
** Descriptions         :   返回链表中指定结点的前一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   前一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
tNode * tListPre (tList * list, tNode * node)
{
	if (node->preNode == node) 
	{
		return (tNode *)0;
	} 
	else 
	{
		return node->preNode;
	}
}

/**********************************************************************************************************
** Function name        :   tListnextNode
** Descriptions         :   返回链表中指定结点的后一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   后一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
tNode * tListNext (tList * list, tNode * node)
{
	if (node->nextNode == node) 
	{
		return (tNode *)0;
	}
	else 
	{
		return node->nextNode;
	}
}

/**********************************************************************************************************
** Function name        :   tListRemoveAll
** Descriptions         :   移除链表中的所有结点
** parameters           :   list 待清空的链表
** Returned value       :   无
***********************************************************************************************************/
void tListRemoveAll (tList * list)
{
    uint32_t count;
    tNode * nextNode;
        
    // 遍历所有的结点
	  nextNode = list->firstNode;
    for (count = list->nodeCount; count != 0; count-- )
    {
    	// 先纪录下当前结点，和下一个结点
    	// 必须纪录下一结点位置，因为在后面的代码中当前结点的next会被重置
        tNode * currentNode = nextNode;
        nextNode = nextNode->nextNode;
        
        // 重置结点自己的信息
        currentNode->nextNode = currentNode;
        currentNode->preNode = currentNode;
    }
    
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   tListAddFirst
** Descriptions         :   将指定结点添加到链表的头部
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void tListAddFirst (tList * list, tNode * node)
{
    node->preNode = list->firstNode->preNode;
    node->nextNode = list->firstNode;

    list->firstNode->preNode = node;
    list->firstNode = node;
    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   tListAddLast
** Descriptions         :   将指定结点添加到链表的末尾
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void tListAddLast (tList * list, tNode * node)
{
	node->nextNode = &(list->headNode);
    node->preNode = list->lastNode;

    list->lastNode->nextNode = node;
    list->lastNode = node;
    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   tListRemoveFirst
** Descriptions         :   移除链表中的第1个结点
** parameters           :   list 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
***********************************************************************************************************/
tNode * tListRemoveFirst (tList * list)
{
    tNode * node = (tNode *)0;

	if( list->nodeCount != 0 )
    {
        node = list->firstNode;

        node->nextNode->preNode = &(list->headNode);
        list->firstNode = node->nextNode;
        list->nodeCount--;
    }
    return  node;
}

/**********************************************************************************************************
** Function name        :   tListInsertAfter
** Descriptions         :   将指定的结点插入到某个结点后面
** parameters           :   list 			待插入的链表
** parameters           :   nodeAfter 		参考结点
** parameters           :   nodeToInsert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void tListInsertAfter (tList * list, tNode * nodeAfter, tNode * nodeToInsert)
{
    nodeToInsert->preNode = nodeAfter;
    nodeToInsert->nextNode = nodeAfter->nextNode;

    nodeAfter->nextNode->preNode = nodeToInsert;
    nodeAfter->nextNode = nodeToInsert;

    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   tListRemove
** Descriptions         :   将指定结点从链表中移除
** parameters           :   list 	待移除的链表
** parameters           :   node 	待移除的结点
** Returned value       :   无
***********************************************************************************************************/
void tListRemove (tList * list, tNode * node)
{
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    list->nodeCount--;
}
