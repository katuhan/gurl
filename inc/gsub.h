/*******************************************
 * function :  gurl子函数声明
 * author   :  gpan
 * time     :  Tue Dec 27 09:49:08 CST 2016
 * ****************************************/
#ifndef __gsub_h__
#define __gsub_h__

#include "gpub.h"
#include "url.h"

void gurl_init();                                                          //初始化
void gurl_close();                                                         //结束
void usage(int sta);                                                       //使用帮助
void show_version();                                                       //版本信息
void set_save_dir(char *path);                                             //保存目录
void print_to_screen(const char *fmt,...);                                 //屏幕输出
int  inter_with_screen(const char *ans, int length, const char *fmt,...);  //屏幕交互
void *deal_url(void *url);                                                 //url处理函数

#endif
