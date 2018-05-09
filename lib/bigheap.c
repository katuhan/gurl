/******************************************
 * function : 大顶堆优先队列接口实现
 * author   : gpan
 * time     : Fri Jan 13 10:01:57 CST 2017
 * ***************************************/
#include "bigheap.h"  
#include "debug.h"  
  
/*********************************************************
 * function : 创建容量为capacity，元素大小为size的优先队列
 * *******************************************************/
priority_queue *priority_queue_create(long capacity) 
{  
    if (MIN_SIZE_PRIO_QUEUE > capacity)                    //新建队列不能小于最小容量
        capacity = MIN_SIZE_PRIO_QUEUE;   
    priority_queue *queue = NULL;
    if ((queue = (priority_queue*)malloc(sizeof(priority_queue))))  
    {
        queue->elems = (queue_element *)malloc((capacity + 1) * SIZE_OF_ELEMENT);  
        if (NULL == queue->elems)                          //存储元素的空间不能为空
        {
            free(queue);
            queue = NULL;
        }
        else
        {
            queue->capacity = capacity;  
            queue->deep = 0;  
        }
    }
    return queue;  
}  

/*************************
 * function : 销毁优先队列
 * ***********************/
void priority_queue_destroy(priority_queue *queue) 
{  
    if (queue)
    {
        if (queue->elems)
        {
            free(queue->elems);
            queue->elems = NULL;
        }
        free(queue);
        queue = NULL;
    }
}  

/******************************************
 * function : 向队列中加入元素
 * return   : -1 ,出错
 *            -2 ,数据过长
 *            -3 ,element中data必须要有长度
 *             0 ,队列已满
 *             >0,插入成功,返回当前队列深度
 * ****************************************/
long priority_queue_insert(priority_queue *queue, queue_element *element, long size) 
{  
    if (queue)  
    {
        if (queue->capacity == queue->deep)                                             //队列已满
            return 0;
        if (size > MAX_LEN_DATA)                                                        //元素数据过长 
            return -2;
        if (element->len <= 0)                                                         
            return -3;                                                                  //data长度必须大于0
        long index = ++queue->deep;                                                     //队列中元素编号从1开始的,当前要插入的是编号为index的元素
        bzero(ELEMENT(queue,index), SIZE_OF_ELEMENT); 
        memcpy(ELEMENT(queue,index), element, size);                                    //把元素放入最后一个编号index位置
        GLOG3("priority= %ld,len = %ld\n", element->priority, element->len);
                                                                                        //向上调整堆序，与父亲比较优先级
        int endflg = 0;                                                                 //插入元素位置调整完成标志
        if (1 == index) return queue->deep;                                             //如果在堆顶，就不用调整了
        while (1 != index && 0 == endflg)                                           
        {
            if ((ELEMENT(queue,index))->priority > (ELEMENT(queue,index/2))->priority)  //若当前元素的优先级比其父高
            {                                                                           //父子交换 第0个元素做中转
                memcpy(ELEMENT(queue,0), ELEMENT(queue,index), SIZE_OF_ELEMENT);
                memcpy(ELEMENT(queue,index), ELEMENT(queue,index/2), SIZE_OF_ELEMENT);
                memcpy(ELEMENT(queue,index/2), ELEMENT(queue,0), SIZE_OF_ELEMENT);
            }
            else                                                                        //否则表示位置调整完成
            {
                endflg = 1;
            }
            index = index / 2;                                                         
        }
        return queue->deep;
    }
    return -1;
}  

/******************************************
 * function : 从队列中获取优先级最高的元素
 * return   : -1 ,出错
 *             0 ,提供的size过小
 *            >0 ,元素总大小
 * ****************************************/
int priority_queue_get(priority_queue *queue, queue_element *element, long size) 
{
    if (queue && (0 < queue->deep))
    {
        bzero(element, size);
        int len = ELEMENT(queue,1)->len + SIZE_OF_ELEMENT - MAX_LEN_DATA; 
        if (len > size)
           return 0;
        else
           memcpy(element, ELEMENT(queue,1), len);                                   //取出编号为1的元素
		if (1 != queue->deep)
            memcpy(ELEMENT(queue,1), ELEMENT(queue,queue->deep), SIZE_OF_ELEMENT);   //把最后一个元素移到第一位
        bzero(ELEMENT(queue,queue->deep), SIZE_OF_ELEMENT);                          //取走的区域初始化         
        --queue->deep;                                                               //队列深度-1
        GLOG3("priority= %ld,len = %ld\n", element->priority, element->len);
                                                                                     //向下调整堆序,与两个儿子中较大的一个进行交换
        long index = 1;                                                              //从第一个元素开始
        int endflg = 0;                                                              //堆序调整结束标志
        while ((index*2 <= queue->deep) && (0 == endflg))
        {
            long index_tmp = index;                                                  //先认为当前元素优先级较高
            if (ELEMENT(queue,index_tmp)->priority < ELEMENT(queue,index * 2)->priority)
                index_tmp = index * 2;                                               //如果左儿子的优先级要高,tmp记录优先级高的下标编号
            if (ELEMENT(queue,index_tmp)->priority < ELEMENT(queue,index * 2 + 1)->priority)
                index_tmp = index * 2 + 1;                                           //如果右儿子的优先级要高,mp记录优先级高的下标编号
            if (index_tmp != index)                                                  //如果优先级高的不是自己
            {                                                                        //那么父子交换
                memcpy(ELEMENT(queue,0), ELEMENT(queue,index), SIZE_OF_ELEMENT);
                memcpy(ELEMENT(queue,index), ELEMENT(queue,index_tmp), SIZE_OF_ELEMENT);
                memcpy(ELEMENT(queue,index_tmp), ELEMENT(queue,0), SIZE_OF_ELEMENT);
                index = index_tmp;                                                   //继续向下调整
            }
            else
                endflg = 1;                                                          //否则说明不需要调整了
        }
        return len;
    }
    return -1;
}

/*****************************
 * function : 返回当前队列深度
 * ***************************/
long get_deep(priority_queue *queue)
{
   return queue->deep;
}

