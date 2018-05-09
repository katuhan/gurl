/*Copyright (c) 2017 gpan
 *
 *http报文请求行格式:
 *        Method URL HTTP-VersionCRLF  
 *        \____/ \_/ \__________/\__/
 *          |     |       |        |
 *       请求方法 |   HTTP协议版本 |
 *           投以资源标识符        |
 *               车换行,除结尾外,不许单独的CR或LF出现
 *报文头部格式:
 *      field-name:field-valueCRLF
 *               ...
 *      field-name:field-valueCRLF
 *      \________/ \_________/\__/
 *          |           |       |
 *      头部字段名     值    回车换行
 */
#ifndef _HTTP_H
#define _HTTP_H

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include "url.h"
#include "map.h"
#include "sock.h"
#include "debug.h"

#define HTTP_MAX_URL_LEN 1024        //URL的最大长度
#define HTTP_MAX_LINE_LEN 1024       //行的最大长度
#define HTTP_MAX_BUF_LEN 4096        //buf长度
                                     //http_handle_t中回调函数类型
typedef size_t callbackfun(void *ptr, size_t size, size_t nmemb, void *stream);
typedef struct http_handle_s
{
    char url[HTTP_MAX_URL_LEN+1];    //url
    const char *mothod;              //请求方法,从http_mothod中取值
    const char *ver;                 //请求的http版本 从http_version中取值,组报文时加HTTP/前缀
    map_str_t header_map;            //http请求头域
    int  sockid;                     //在perform函数执行是判断是否新建链接              
    char repver[20];                 //应答版本
    char repsta[20];                 //应答行中的状态    
    char repmsg[HTTP_MAX_LINE_LEN+1];//应答中的错误信息
    map_str_t response_map;          //http应答头域
    long len;                        //数据的长度
    void *data;                      //数据存放位置，为callback第四个参数
    callbackfun *callback;           //数据处理函数
    char buf[HTTP_MAX_BUF_LEN+1];    //buf
    double usec;                     //用时
    int  retcd;                      //下面http接口函数错误码
    char errmsg[HTTP_MAX_LINE_LEN+1];//下面http接口函数的错误信息
}http_handle_t;

#ifdef __cplusplus
extern "C" {
#endif
http_handle_t *http_init();
void http_cleanup(http_handle_t *handle);
int http_perform(http_handle_t *handle);
void *http_get_info(http_handle_t *handle, const char *key);
void http_set_opt(http_handle_t *handle, const char *key, const void *value);
#ifdef __cplusplus
}
#endif

#endif //!_HTTP_H
