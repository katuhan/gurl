/********************************************
 * function : 线程池函数定义-----优先队列版本
 * author   : gpan
 * time     : Mon Jan  9 14:42:21 CST 2017
 * ******************************************/
#include "threadpool.h"

extern int pthread_setconcurrency(int new_level);
/****************************************
 * thread_pool_create : 创建线程池
 * param              : max,预启max个线程
 * **************************************/
thread_pool *thread_pool_create(int max)
{  
    thread_pool *pool = (thread_pool *)malloc(sizeof(thread_pool));   
    if (NULL == pool)
        goto _thread_pool_create_END;
    pthread_mutex_init(&(pool->queue_lock), NULL);  
    pthread_cond_init(&(pool->queue_ready), NULL);  
    pool->heap = priority_queue_create(100*max);                     //简单的定义队列容量为线程数的100倍   
    if (NULL == pool->heap)
        goto _thread_pool_create_1_END;
    pool->max_thread_num = max;;  
    pool->is_destroy = 0;  
    pool->tids = (pthread_t *)malloc(max * sizeof(pthread_t));  
    if (pool->tids)
    {
        pthread_setconcurrency(max);
        for (int i = 0; i < max; i++)  
        {
            pthread_create(&(pool->tids[i]), NULL, (void *)thread_pool_process_task, pool);  
            GLOG3("new threadid = [%ld]\n",pool->tids[i]);
        }
        goto _thread_pool_create_END;
    }
    free(pool->heap);
    pool->heap = NULL;
_thread_pool_create_1_END:
    free(pool);
    pool = NULL;
_thread_pool_create_END:
    return pool;
}  
  
/*******************************************************
 * function : 向线程池pool的任务队列中加入任务
 * param    : callbak ,该任务调用callback回调函数进行处理
 *            priority,任务的优先级
 *            args    ,处理函数的参数
 * *****************************************************/  
long thread_pool_add_task(thread_pool *pool, long priority, CALLBACKFUN callback, void *args)
{  
    call_back fun;                        
    fun.callbackfun = callback;                                      //回调函数的地址
    fun.args = args;                                                 //回调函数的参数
 
    task_node node;
    node.priority = priority;                                        //任务的优先级
    node.size = sizeof(call_back);                                   //具体任务数据的大小
    node.fun = fun;                                                  //任务数据 
 
    pthread_mutex_lock (&(pool->queue_lock));                        //把任务加入队列
    long re = priority_queue_insert(pool->heap, (queue_element *)&node, sizeof(task_node));
    pthread_mutex_unlock(&(pool->queue_lock));  
    pthread_cond_signal(&(pool->queue_ready));                       //尝试唤醒一个等待线程
    GLOG3("add one task to queue, node size = %ld\n",node.size);
    return re;  
}  
  
/***********************
 * function : 销毁线程池
 * *********************/
int thread_pool_destroy(thread_pool *pool)
{  
    if (pool->is_destroy)
        return -1;                                                    //防止二次销毁
    else
        pool->is_destroy = 1;                                         //准备销毁线程池
    pthread_cond_broadcast(&(pool->queue_ready));                     //唤醒所有线程，线程池要销毁  
    for (int i = 0; i < pool->max_thread_num; i++)                    //一大波僵尸来袭前，统统扼杀在拐角
        pthread_join (pool->tids[i], NULL);   
    GLOG3("all threads exit \n");
    free(pool->tids);                                                 //释放存放线程id的内存
    priority_queue_destroy(pool->heap);                               //销毁队列
    pthread_mutex_destroy(&(pool->queue_lock));                       //销毁互斥锁
    pthread_cond_destroy(&(pool->queue_ready));                       //销毁条件变量
    free(pool);                                                       
    pool=NULL;  
    GLOG3("destroy pool\n");
    return 0;  
}  
  
/*******************************
 * function : 预启的线程处理函数
 * *****************************/
void thread_pool_process_task(thread_pool *pool)
{  
    while (1)  
    {  
        pthread_mutex_lock(&(pool->queue_lock));  
        while ((!pool->is_destroy) && (0 >= get_deep(pool->heap)))
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));  
        if(pool->is_destroy)                                         //若要销毁线程池
        {  
            pthread_mutex_unlock(&(pool->queue_lock));               //要先解锁再退出
            pthread_exit (NULL);  
        }  
        task_node node;      
        priority_queue_get(pool->heap, (queue_element *)(&node), sizeof(task_node));
        GLOG3("get a task ,will call callback function \n");
        pthread_mutex_unlock(&(pool->queue_lock));  
        (*(node.fun.callbackfun))(node.fun.args);                    //执行回调函数
    }  
}  
