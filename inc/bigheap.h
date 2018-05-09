/******************************************
 * function : 大顶堆实现的优先队列
 * author   : gpan
 * time     : Fri Jan 13 09:58:07 CST 2017
 * ***************************************/
#ifndef _BIGHEAP_H  
#define _BIGHEAP_H  

#include <stdio.h>  
#include <string.h>  
#include <strings.h>  
#include <stdlib.h>  

typedef struct 
{
    long priority;                                      //元素优先级
    long len;                                           //数据data长度,必须提供
    char data[1];                                       //元素数据首地址 实际空间长度为MAX_LEN_ELEMENT
}queue_element;                                         //队列上的元素结构
#define MAX_LEN_DATA    256                             //元素data最大大小
#define SIZE_OF_ELEMENT (2*sizeof(long) + MAX_LEN_DATA) //每个元素占空间大小

typedef struct heap
{
    long capacity;                                      //队列容量
    long deep;                                          //队列深度
    queue_element *elems;                               //元素存储首地址,0下标位置不存储元素，做中间量用
}priority_queue;                                        //优先队列结构  (大顶堆)
#define MIN_SIZE_PRIO_QUEUE 10                          //优先队列最小容量
                                                        //指向Q优先队列上第X个元素
#define ELEMENT(Q,X) ((queue_element *)((char *)((Q)->elems) + ((SIZE_OF_ELEMENT) * (X))))
                                                        //提供的接口
priority_queue *priority_queue_create(long capacity);   
void priority_queue_destroy(priority_queue *queue);
long priority_queue_insert(priority_queue *queue, queue_element *element, long size);
int  priority_queue_get(priority_queue *queue, queue_element *element, long size);
long get_deep(priority_queue *queue);                    //队列深度

#endif
