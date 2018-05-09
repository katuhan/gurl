/*
 * Copyright (c) 2017 gpan
 */
#include "http.h"

typedef enum {GET, POST, HEAD, PUT, DELETE, TRACE, CONNECT, OPTIONS, MAX_MOTHOD} http_mothod_e;
static const char *http_mothod[] = { 
    "GET",                           //请求获取Request-URI所标识的资源
    "POST",                          //在Request-URI所标识的资源后附加新的数据
    "HEAD",                          //请求获取由Request-URI所标识的资源的响应消息报头
    "PUT",                           //请求服务器存储一个资源，并用Request-URI作为其标识
    "DELETE",                        //请求服务器删除Request-URI所标识的资源
    "TRACE",                         //请求服务器回送收到的请求信息，主要用于测试或诊断
    "CONNECT",                       //保留将来使用
    "OPTIONS"                        //请求查询服务器的性能，或者查询与资源相关的选项和需求
};

typedef enum {ONE, MAX_VERSION} http_version_e;
static const char *http_version[] = {    
        "1.1"
};

/**************************以下为对http_opt.h的引用*********************/
typedef enum {
#define HTTP_OPT_ENUM_INFO           //定义可以设置HTTP属性枚举类型
#include "http_opt.h"
            HTTP_OPT_MAX_ENUM
}http_opt_e;

const char *const HTTP_OPT_STR[] = {
#define HTTP_OPT_STR_INFO            //属性相关字符串数组
#include "http_opt.h"
};
                                     //属性设置函数的类型
typedef void (*HTTP_SET_FUN)(http_handle_t *, const char *, const void *);

#define HTTP_OPT_FUN_DECLARE_INFO    //属性设置函数的声明
#include "http_opt.h"

static const HTTP_SET_FUN HTTP_OPT_FUN[] = {
#define HTTP_OPT_FUN_INFO            //所有属性设置函数指针数组
#include "http_opt.h"
};

/**************************以下为对http_get.h的引用*********************/
typedef enum {
#define HTTP_GET_ENUM_INFO           //获取相关枚举
#include "http_get.h"
                HTTP_GET_MAX_ENUM
}http_get_e;

const char *const HTTP_GET_STR[] = {
#define HTTP_GET_STR_INFO            //获取相关字符串
#include "http_get.h"
};
                                     //所有获取函数类型
typedef void *(*HTTP_INFO_FUN)(http_handle_t *handle, const char *key);

#define HTTP_GET_FUN_DECLARE_INFO    //所获取函数声明
#include "http_get.h"

static const HTTP_INFO_FUN HTTP_GET_FUN[] = {
#define HTTP_GET_FUN_INFO            //所有获取函数指针数组
#include "http_get.h"
};

/***************以下为接口用到的其他静态函数的声明及定义****************/
static void http_set_ret(http_handle_t *handle, int retcd, const char *fmt, ...);
static size_t http_call_back(void *ptr, size_t size, size_t nmemb, void *stream);
static void http_data_deal(http_handle_t *handle, int len);
static void *http_get_header(http_handle_t *handle, const char *key);

/* 用于http接口函数设置返回码，错误信息 */
static void http_set_ret(http_handle_t *handle, int retcd, const char *fmt, ...)
{
    handle->retcd = retcd;
    bzero(handle->errmsg, sizeof(handle->errmsg));
    va_list args;
    va_start(args, fmt);
    vsnprintf(handle->errmsg, HTTP_MAX_LINE_LEN, fmt, args);
    va_end(args);
}

/* 默认数据处理回调函数，把数据print,因为默认输出到stdout */
static size_t http_call_back(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	fflush((FILE *)stream);
    return written;
}

/* 对将要接收的len长度数据进行处理 */
static void http_data_deal(http_handle_t *handle, int len)
{
    int bufsize = sizeof(handle->buf)-1;                                            //buf大小
    int remainlen = len;                                                            //剩余长度
	while (remainlen > 0)
    {
        int curlen = (remainlen > bufsize) ? bufsize : remainlen;                   //一次接收不能大于bufsize
        bzero(handle->buf, sizeof(handle->buf));
        int rcvlen = sock_rcv(handle->sockid, handle->buf, curlen);
        if (rcvlen < 0) break;                                                      //如果接收出错，退出
        //int cou = (*(handle->callback))(handle->buf, rcvlen, 1, handle->data);      //调用回调函数进行数据处理
        (*(handle->callback))(handle->buf, rcvlen, 1, handle->data);      //调用回调函数进行数据处理
        remainlen -= rcvlen;
    }
}

/* 从请求获取HTTP头域 */
static void *http_get_header(http_handle_t *handle, const char *key) 
{
    if (handle && key)
    {
        char **p = map_get(&(handle->header_map), key);
        if (p)
            return (*p);
    }
    return NULL;
}

/**************************以下为HTTP库提供的接口实现*********************/
/* 创建并初始化http_handle */
http_handle_t *http_init()
{
    http_handle_t *handle = (http_handle_t *)malloc(sizeof(http_handle_t));
    if (NULL == handle)
        return handle;
    bzero(handle, sizeof(http_handle_t)); 
    http_set_opt(handle, "HTTP_MOTHOD", "GET");                                     //默认mothod为GET
    http_set_opt(handle, "HTTP_VER", "1.1");                                        //默认协议版本1.1
    map_init(&(handle->header_map));                                                //初始化头部
    handle->sockid = -1;
    map_init(&(handle->response_map));                                              //初始化应答头部
    handle->len = 0;                                                                //接收数据长度0
    handle->data = (void *)stdout;                                                  //默认输出数据到标准输出 
    handle->callback = http_call_back;                                              //默认数据处理回调函数
    handle->usec = 0;                                                               //默认用时0
    handle->retcd = 0;                                                              //默认返回成功 0
    http_set_opt(handle, "Accept", "text/html, application/xhtml+xml, image/jxr, */*");    
    http_set_opt(handle, "Accept-Encoding", "gzip, deflate");                      
    http_set_opt(handle, "Accept-Language", "en,zh");
    http_set_opt(handle, "Connection", "Keep-Alive");
    return handle;
}

/* 销毁http_handle */
void http_cleanup(http_handle_t *handle)                                              
{
    if (handle)
    {
        map_deinit(&(handle->response_map));
        if (handle->sockid)
            sock_close(handle->sockid);     
        map_deinit(&(handle->header_map));
        free(handle);
    }
}

/* 执行使设置生效 */
int http_perform(http_handle_t *handle)
{
    char dataline[HTTP_MAX_LINE_LEN+1];
    struct timeval starttime,endtime;    
    gettimeofday(&starttime,0);

    GLOG3("begin\n");
    url_field_t *url = url_parse(handle->url);                                      //解析目标地址
    if (!url || !(url->host))
    {
        http_set_ret(handle, -1, "%s", "parse url error!");
        goto _http_perform_END;
    }
    if (0 >= handle->sockid)                                                        //判断建立或使用原有socket连接
    {
        bzero(dataline, sizeof(dataline));
        if (url->port)
            memcpy(dataline, url->port, strlen(url->port));
        else
            memcpy(dataline, "80", 2);
        GLOG3("connect ...\n");
        //handle->sockid = sock_connect(url->host, dataline);
        handle->sockid = sock_connect(url->host, dataline, 20);
    }
    if (0 >= handle->sockid)                                                        //建立socket链接失败
    {
        http_set_ret(handle, -1, "%s", "socket connect error");
        goto _http_perform_END;
    }
    GLOG3("connected \n");
    http_set_opt(handle, "Host", url->host);                                        //一定要写的头域

    bzero(handle->buf, sizeof(handle->buf));
    snprintf(handle->buf+strlen(handle->buf),sizeof(handle->buf)-strlen(handle->buf), \
        "%s %s HTTP/%s\r\n", handle->mothod, handle->url, handle->ver); 
    map_iter_t iter = map_iter(&(handle->header_map));                              //遍历写入头域
    const char *key;
    while ((key = map_next(&(handle->header_map), &iter)))
    {
        char **p = map_get(&(handle->header_map), key);
        if (p)
            snprintf(handle->buf+strlen(handle->buf),sizeof(handle->buf)-strlen(handle->buf), \
                "%s: %s\r\n", key, *p); 
    }
    snprintf(handle->buf+strlen(handle->buf),sizeof(handle->buf)-strlen(handle->buf),"\r\n");
    GLOG3("send...\n");
    GLOG3("[%s]\n",handle->buf);
    sock_snd(handle->sockid, handle->buf, strlen(handle->buf));

    bzero(dataline, sizeof(dataline));
    sock_get_line(handle->sockid, dataline, sizeof(dataline));                      //接收第一行
    bzero(handle->repver, sizeof(handle->repver));
    bzero(handle->repsta, sizeof(handle->repsta));
    bzero(handle->repmsg, sizeof(handle->repmsg));
    sscanf(dataline, "%s %s %s", handle->repver,handle->repsta, handle->repmsg);
    GLOG3("repver = %s, repsta = %s, repmsg = %s\n", handle->repver, handle->repsta, handle->repmsg);
    int rcvlen = 0;                                                                 //接收报体的总长度
    do {                                                                            //接收头
        char *p = NULL;
        bzero(dataline, sizeof(dataline));
        while (0 == sock_get_line(handle->sockid, dataline, sizeof(dataline)))                          
            ;
        GLOG3("get line[%s]\n",dataline);
        if (!strcmp(dataline, "\n"))                                                //一直读到空行,sock_get_line只要一个'\n'的行
            break;                                                                  //也许应该加一个超时控制，以应对接收到没有空行的错误数据的情况
        if (NULL != (p = strstr(dataline, ":")))
        {
            *p++ = '\0';
            map_set(&(handle->response_map), dataline, p);
            if (!strcmp(dataline,"Transfer-Encoding") && (!strcmp(p,"chunked")))
                rcvlen = -1;                                                        //有chunked编码
            if ((!strcmp(dataline,"Content-Length") && (-1 != rcvlen)))             //有Content-Length且没有chunked编码
                rcvlen = atoi(p);
        }
    } while (1);
    GLOG3("rcvlen = %d\n",rcvlen);
    if (-1 == rcvlen)                                                               //按chunked编码方式接收数据
    {                                                                               //chunked块格式:十六进制长度\r\n实际内容，最后一个块长度为0
        rcvlen = 0;
        do {
            bzero(dataline, sizeof(dataline));
            sock_get_line(handle->sockid, dataline, sizeof(dataline));              //获取chunked块的长度
            int chunkedlen;
            sscanf(dataline, "%x", &chunkedlen);                                    //strtol(chunkedlen, data, 16);
            if (0 == chunkedlen)                                                    //忽略最后一个footer的内容
                break;
            rcvlen += chunkedlen;
            http_data_deal(handle, chunkedlen);
        } while (1);
    }
    else                                                                            //按长度数据接收
    {
        http_data_deal(handle, rcvlen);
    }
    gettimeofday(&endtime,0);
    handle->usec = 1000000*(endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec- starttime.tv_usec;
    handle->len = rcvlen;                                                           //数据总长度
    http_set_ret(handle, 0, "%s", "ok!");
    GLOG3("usec = %lf\n",handle->usec);
    GLOG3("len = %ld\n",handle->len);

_http_perform_END:
    ;                                                                               //标签是语句的一部分，声明不是一个语句
    int closeflg = 1;                                                               //判断是否需要关闭socket
    char *tmpvalue = http_get_header(handle, "Connection");
    if (tmpvalue && !strcmp(tmpvalue, "Keep-Alive"))
        closeflg = 0;                                                               //如果请求时让保持连接,那么不关闭连接
    tmpvalue = http_get_reheader(handle, "Connection");
    if (tmpvalue && !strcasecmp(tmpvalue, "close"))
         closeflg = 1;                                                              //如果服务端告知连接已经关闭,那么关闭连接
    if (closeflg && handle->sockid)
        sock_close(handle->sockid);
    url_free(url);
    GLOG3("end__retcd = %d ,retmsg = %s\n",handle->retcd, handle->errmsg);
    return handle->retcd;
}

/* 设置HTTP属性 */
void http_set_opt(http_handle_t *handle, const char *key, const void *value) 
{
    if (handle && key && value)
    {
        const char *key_tmp = key;
        if (strncmp(key, "HTTP_", 5))                                               //若key键不为HTTP_开头
            key_tmp = "HTTP_HEADER";                                                //则执行HTTP_HEADER对应的函数,加入到头域
        http_opt_e max = HTTP_OPT_MAX_ENUM;                                         //否则执行key对应的函数
        int index;
        for (index = 0; index < max; index++)
        {
            if (!strcmp(key_tmp, HTTP_OPT_STR[index]))        
                break;
        }
        GLOG3("index = %d,max = %d,key = %s\n",index,max,key);
        if (index < max)
            (*(HTTP_OPT_FUN[index]))(handle, key, value);                           //执行设置函数
    }
}

/* 获取http信息 */
void *http_get_info(http_handle_t *handle, const char *key)  
{
    if (handle && key)
    {
        const char *key_tmp = key;
        if (strncmp(key, "HTTP_", 5))                                               //若key键不为HTTP_开头
            key_tmp = "HTTP_HEADER";                                                //则执行HTTP_HEADER对应的函数,加入到头域
        http_get_e max = HTTP_GET_MAX_ENUM;                                         //否则执行key对应的函数
        int index;
        for (index = 0; index < max; index++)
        {
            if (!strcmp(key_tmp, HTTP_GET_STR[index]))        
                break;
        }
        if (index < max)
            return (*(HTTP_GET_FUN[index]))(handle, key);                           //执行设置函数
    }
    return NULL;
}

/*********以下为http完成后获取信息函数，由http_get_info调用***************/
/* 从应答获取HTTP头域 */
static void *http_get_reheader(http_handle_t *handle, const char *key) 
{
    if (handle && key)
    {
        GLOG3("get__reheader\n");
        char **p = map_get(&(handle->response_map), key);
        if (p)
            return (*p);
    }
    return NULL;
}

/* 获取应答状态 */
static void *http_get_sta(http_handle_t *handle, const char *key)
{
    char *p = NULL;
    if (handle && key && !strcmp(key, "HTTP_STA"))
        p = handle->repsta;
    return p;
}

/* 获取应答描述 */
static void *http_get_msg(http_handle_t *handle, const char *key)
{
    char *p = NULL;
    if (handle && key && !strcmp(key, "HTTP_MSG"))
        p = handle->repmsg;
    return p;
}

/* 获取下载的长度 */
static void *http_get_len(http_handle_t *handle, const char *key)
{
    if (handle && key && !strcmp(key, "HTTP_LEN"))
        return &handle->len;
    return NULL;
}

/* 获取耗时 */
static void *http_get_time(http_handle_t *handle, const char *key)
{
    if (handle && key && !strcmp(key, "HTTP_TIME"))
        return &handle->usec;
    return NULL;
}

/**************以下为http属性设置函数，由http_set_opt调用*****************/
/* 设置或重置HTTP头域 */
static void http_set_header(http_handle_t *handle, const char *key, const void *value)
{
    if (handle && key && value)
        map_set(&(handle->header_map), key, (char *)value);
}

/* 设置URL*/
static void http_set_url(http_handle_t *handle, const char *key, const void *url)
{
    GLOG3("key = %s url = %s\n",key,url);
    url_field_t *url_old = url_parse(handle->url);         
    bzero(handle->url, sizeof(handle->url));                                       //一旦进入就初始化
    if (handle && url && key && strlen((char *)url)<=HTTP_MAX_URL_LEN && (!strcmp(key, "HTTP_URL")))
    {
        if (url_old)                                                               //如果原来设置过url
        {
            GLOG3("have old\n");
            url_field_t *url_new = url_parse((char *)url);                         //解析新的url
            if (NULL == url_new)
                goto _http_set_url_END;                                        
            GLOG3("parse new ok\n");
            if ( !url_new->host || !url_old->host || strcmp(url_new->host, url_old->host)) 
            {                                                                      //如果新的url与原有host不相同
                GLOG3("host !=\n");
                url_free(url_new);
                goto _http_set_url_END;
            }
            else                                                                   //在两个url的host相同情况下
            {
                if (url_old->port)                                                 //原有url包含端口号
                {
                    if (url_new->port)                                             //新的URL也有端口号的话
                    {
                        if (strcmp(url_new->port, url_old->port))                  //比较两个端口
                        {
                            GLOG3("port!=1\n");
                            url_free(url_new);
                            goto _http_set_url_END;
                        }
                    }
                    else if (strcmp(url_old->port, "80"))                          //新的没有端口的话,那么判断原有端口号是否为80
                    {
                        GLOG3("port!=2\n");
                        url_free(url_new);
                        goto _http_set_url_END;
                    }
                } 
                else if (url_new->port && strcmp(url_new->port, "80"))             //原有url不包含端口号，那么新url如果包含端口号，一定要为80
                {
                    GLOG3("port!=3\n");
                    url_free(url_new);
                    goto _http_set_url_END;
                }
            }
            url_free(url_new);
        }
        memcpy(handle->url, (char *)url, strlen((char *)url));
        GLOG3("set url = %s\n",handle->url);
    }
_http_set_url_END:
    url_free(url_old);
}

/* 设置MOTHOD*/
static void http_set_mothod(http_handle_t *handle, const char *key, const void *mothod)
{
    if (handle && mothod && key && (!strcmp(key, "HTTP_MOTHOD")))
    {
        http_mothod_e max = MAX_MOTHOD;
        for (int i = 0; i< max; i++)
        {
            if (!strcmp((char *)mothod, http_mothod[i]))
            {
                handle->mothod = http_mothod[i];
                GLOG3("set mothod= %s\n",handle->mothod);
                break;
             }
        }
    }
}

/* 设置HTTP版本*/
static void http_set_version(http_handle_t *handle, const char *key, const void *ver)
{
    if (handle && ver && key && (!strcmp(key, "HTTP_VER")))
    {
        http_version_e max = MAX_VERSION;
        for (int i = 0; i< max; i++)
        {
            if (!strcmp((char *)ver, http_version[i]))
            {
                handle->ver = http_version[i];
                GLOG3("set ver = %s\n",handle->ver);
                break;
             }
        }
    }
}

/* 设置数据存放位置 */
static void http_set_data(http_handle_t *handle, const char *key, const void *data)
{
    if (handle && data && key && (!strcmp(key, "HTTP_DATA")))
    {
        handle->data = (void *)data;
    }
}

/* 设置回调函数 */
static void http_set_callback(http_handle_t *handle, const char *key, const void *fun)
{
    if (handle && fun && key && (!strcmp(key, "HTTP_CALLBACK")))
    {
        handle->callback = (callbackfun *)fun;
    }
}
