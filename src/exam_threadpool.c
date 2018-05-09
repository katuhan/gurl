/***************************
 * function : 线程池使用例子 
 * *************************/
#include "threadpool.h"
int count = 0;

void * myprocess (void *arg)
{
    count++;
    printf ("tid is %ld, working on task %d\n", pthread_self (),*(int *) arg);
sleep(5);
    return NULL;
}

int main (int argc, char **argv)
{
    thread_pool *pool = thread_pool_create(3);
    int *workingnum = (int *) malloc (sizeof (int) * 80);
    for (int i = 0; i < 80; i++)
    {
        workingnum[i] = i;
        //thread_pool_add_task(pool, myprocess, &workingnum[i]);
        thread_pool_add_task(pool, 1,myprocess, &workingnum[i]);
sleep(1);
    }
    while (count < 80)
       getpid();
    thread_pool_destroy(pool);
    free (workingnum);
    return 0;
}

