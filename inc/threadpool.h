/******************************************
 * function : 线程池头文件
 * author   : gpan
 * time     : Wed Jan 11 14:44:01 CST 2017
 * ***************************************/
#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include "debug.h"
#include "bigheap.h"

typedef void * (*CALLBACKFUN)(void *arg); //回调函数,用于线程处理具体任务,若有多个参数，需传结构
typedef struct 
{
    CALLBACKFUN callbackfun;              //回调函数
    void *args;                           //回调函数参数
}call_back;

typedef struct task  
{
    long priority;                        //任务优先级
    long size;                            //fun的大小
    call_back  fun;                       //数据data为一个call_back类型的fun
}task_node;                               //任务节点,任务队列中的基本单元

typedef struct                          
{
    pthread_mutex_t queue_lock;           //任务队列互斥锁
    pthread_cond_t queue_ready;           //条件变量用于有任务时、或要销毁线程池时唤醒等待线程
    priority_queue *heap;                 //优先队列，用于任务排队
    int max_thread_num;                   //预启线程数
    int is_destroy;                       //销毁线程池标志  1-销毁 0-初始
    pthread_t *tids;                      //指向存储max_thread_num个线程id的数据空间首地址
}thread_pool;                             //线程池结构

thread_pool *thread_pool_create(int max);
long thread_pool_add_task(thread_pool *pool, long priority, CALLBACKFUN callback, void *args);
int  thread_pool_destroy(thread_pool *pool);
void thread_pool_process_task(thread_pool *pool);

#endif
