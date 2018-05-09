/*****************************************
 * function : 模仿curl，实现简单的http下载
 * author   : gpan   
 * time     : Fri Dec 23 10:58:22 CST 2016
 *****************************************/
                               
#include "gsub.h"
#include "threadpool.h"

units   size_unit = B;
char    g_sSaveDir[PATH_LEN], *const g_pSaveDir = g_sSaveDir;                      //文件保存位置
long    g_counter = 0;                                                             //任务计数器
pthread_mutex_t counter_lock;   
pthread_cond_t counter_over;
pthread_mutex_t screen_lock;   

int main(int argc, char **argv)
{
    gurl_init();                                                                   //初始化操作
    int iOpt;                                                                      //选项参数
    while ((iOpt= getopt(argc, argv, "2o:vhbmkg")) != -1)                          //: -o后要求有参数
    {
        switch (iOpt)
        {
            case 'o': set_save_dir(optarg);   break;                                //保存到指定位置
            case 'v': show_version();         break;                                //版本
            case 'h': usage(0);               break;                                //帮助
            case '?': usage(-1);              break;                                //非法option
            case ':': usage(-2);              break;                                //非法option
            case 'b': size_unit = B;          break;                                //文件大小单位b
            case 'k': size_unit = K;          break;                                //文件大小单位k
            case 'm': size_unit = M;          break;                                //文件大小单位m
            case 'g': size_unit = G;          break;                                //文件大小单位g
            default : usage(-3);              break;
        }
    }
    if (( (argc -1) < optind ) || ( (argc - optind) > URL_NUM ))                    //不带选项的URL参数不能超URL_NUM
        usage(-4);                                                                  //至少要有一个不带选项的URL参数
 
    thread_pool *pool = thread_pool_create(MAXNTHREADS);                            //创建线程池,预启动MAXNTHREADS线程
 
    char *pUrl[URL_NUM];
    for (int iLoop = 0; iLoop < URL_NUM; iLoop++)
        pUrl[iLoop] = NULL;          
    pUrl[0] = argv[optind];
    //thread_pool_add_task(pool, deal_url, argv[optind]);
    thread_pool_add_task(pool, 1, deal_url, argv[optind]);
    int total = 1;                                                                  //任务总数
 
    for (int iLoop = optind + 1; iLoop < argc; iLoop++)                             //添加余下的非重复任务到队列
    {
        char  **ppTmp;
        int   iRepeat = 0;
        for (ppTmp = pUrl; *ppTmp; ppTmp++)
            if ((iRepeat = !strcmp(argv[iLoop], *ppTmp)))                             //相同iRepeat为1
                break;
        if (iRepeat)
        {
            printf("Duplicate data:[%s]\n", argv[iLoop]);
            continue;                                                               //若argv[iLoop]与之前后相同，则抛弃
        }
        else
        {
            *ppTmp = argv[iLoop];                                                   //否则把它加入pUrl中
            thread_pool_add_task(pool, 1, deal_url, argv[iLoop]);                   //添加一个任务到队列
            total += 1;                                                             //任务总数加1
        }
    }
 
    GLOG1("wait_all_task\n");
    pthread_mutex_lock(&counter_lock);
    while (g_counter < total)                                                       //等待队列中的所有任务处理完成
        pthread_cond_wait(&counter_over, &counter_lock);
    pthread_mutex_unlock(&counter_lock);
 
    GLOG1("will_destroy_pool\n");
    thread_pool_destroy(pool);                                                      //销毁线程池
    gurl_close();                                                                   //最后清理工作
    return 0;
}
