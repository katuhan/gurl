/**********************************************
 * function :  用大顶堆实现的优先级队列使用例子
 * author   :  gpan
 * time     : Fri Jan 13 10:16:33 CST 2017
 * ********************************************/
#include "bigheap.h"                                                  //包含头文件,当然链接时需引用相应库
#include <stdio.h>  

typedef struct mem_s
{
    long priority;
    long size;
    int  data;
}mem_t;                                                                //需定义往优先队列中放入元素的结构，
                                                                       //mem_t中data可以是任意类型但长度不能大于256
  
int   
main()  
{  
    mem_t a;                                                            
    priority_queue *H = priority_queue_create(50);                      // stp1----创建一个容量为50的优先队列
    int ar[] = { 32, 21, 16, 24, 31, 19, 68, 65, 26, 13 };  
    for (int i = 0; i < 10; i++ )  
    {
        a.priority = ar[i];                                             //给元素a定义优先级
        a.data = i;                                                     //元素a的具体数据
        a.size = sizeof(int);                                           //元素a的具体数据的大小，不能大于256(默认)
        priority_queue_insert(H, (queue_element *)&a, sizeof(mem_t));   // stp2----向优先队列H 中插入元素a
    }
    for (int i = 0; i < 10; i++ )  
    {  
       priority_queue_get(H, (queue_element *)&a, sizeof(mem_t));       // stp3----从优先队列H 中取出优先级最高的元素到a
       printf( "优先级%ld--值长度%ld---值%d\n", a.priority, a.size, a.data);  
  
    }  
    priority_queue_destroy(H);                                          // stp4----用完后一定记得销毁队列
    return 0;  
}  

