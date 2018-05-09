/*****************************************
 * function : gurl公共头文件
 * author   : gpan
 * time     : Wed Dec 28 23:44:50 CST 2016
 ****************************************/
#ifndef __gpub_h__
#define __gpub_h__

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <libgen.h>
#include <pthread.h>
//#include <curl/curl.h>
#include "http.h"                                        //不用curl,换用自写的http库
#include "debug.h"

#define APPNAME "gurl"                                   //应用名称
#define APPDSCR "simple HTTP download tool"              //应用简介
#define APPVER  "0.3"                                    //版本号
#define URL_NUM 30                                       //支持的最大URLs参数个数
#define MAXNTHREADS 3                                    //最大线程数
#define PATH_LEN 200                                     //路径长度
#define MAX_LINE 1024                                    //路径长度
#define ERRCODE_LEN 6                                    //路径长度

//static char cunit[] = {'B', 'K', 'M', 'G'};
typedef enum unit{B, K, M, G}units;
extern units size_unit;
extern char *const  g_pSaveDir;                          //下载文件保存目录
extern long         g_counter;                           //任务计数器
pthread_mutex_t counter_lock;                            //任务结束去上的互斥锁
pthread_cond_t  counter_over;                            //任务计数器上的条件变量
pthread_mutex_t screen_lock;                             //屏幕互斥锁

int cat_path(const char *path, const char *name, char *out, int size);
int filename_from_path(const char *path, char *filename, int size);
int change_filename(const char *path, const char *old, const char *new);
void remove_file(const char *path, const char *filename);
FILE *open_file(char *path,char *filename, char *mode);
int file_exist(char *path,char *filename);
int single_http_download(void *url, FILE *fd, char *pMsg, int size);
//static size_t save2file(void *ptr, size_t size, size_t nmemb, void *stream);

#endif

