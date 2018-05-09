/*
 * Copyright (c) 2017 gpan
 */


#include "sock.h"

/* 关闭连接 */
void sock_close(SOCK_ID sock_id)
{
    close(sock_id);
}

/* 客户端建立连接,sec <= 0则阻塞式链接 */
SOCK_ID sock_connect(const char *host, const char *serv, int sec)
{
    int res;
    struct addrinfo hope_addr;                       //期待返回类型
    bzero(&hope_addr, sizeof(hope_addr));
    hope_addr.ai_family = AF_UNSPEC;                 //期待返回的套接口地址结构的类型 AF_INET，AF_INET6
    hope_addr.ai_socktype = SOCK_STREAM;

    struct addrinfo *res_addr;                       //返回类型
    if( 0 != getaddrinfo(host, serv, &hope_addr, &res_addr))
        return -1;                                   //get server info error!
    struct addrinfo *tmp_addr = res_addr;            //中间变量保存服务端信息
    
    SOCK_ID sock_id = -1;
    GLOG3("before connect...\n");
    do {                                             //获取一个描述字
        sock_id = socket(res_addr->ai_family, res_addr->ai_socktype, res_addr->ai_protocol);
        GLOG3("try to get a fd = %d\n",sock_id);
        if (sock_id < 0)
            continue;                                //忽略这一个
        if (sec > 0)                                 //有超时控制
        {                                            //设置成非阻塞  
            fcntl(sock_id, F_SETFL, fcntl(sock_id, F_GETFL, 0)|O_NONBLOCK); 
        }
        if (0 == (res = connect(sock_id, res_addr->ai_addr, res_addr->ai_addrlen)))
        {
            GLOG3("connect ok at onece\n");
            if (sec > 0)                             //有超时控制，恢复阻塞 
                fcntl(sock_id,F_SETFL,fcntl(sock_id,F_GETFL,0) & ~O_NONBLOCK);  
            break;                                   //连接成功
        }
        else                                         //连接失败
        {
            GLOG3("connect return != 0\n");
            if (0 < sec)                             //有超时控制的情况
            {
                GLOG3("chk errno = %d\n",errno);
                if (errno != EINPROGRESS)             //不是正在处理状态115
                    goto _connect_err_FAIL;
                else
                {
                    GLOG3("errno =  EINPROGRESS\n");
                    fd_set wset,rset;                //要检测写变化的文件描述符集合
                    FD_ZERO(&wset);                  //清空集合
                    FD_SET(sock_id, &wset);          //把文件描述符sock_id加入集合wset
                    rset = wset;
                    struct timeval timeout;       
                    timeout.tv_sec = sec;
                    timeout.tv_usec = 0;
                    GLOG3("set out time = %d\n",sec);
                    res = select(sock_id + 1, &rset, &wset, NULL, &timeout);  
                    if (-1 == res)   
                        goto _connect_err_FAIL;      //select出错
                    else if (0 == res) 
                        goto _connect_err_FAIL;      //超时
                    else                             //=1
                    {                                //服务端连接的本机时，没启服务情况下，select也返回描述字可写????
                        GLOG3("res = %d,return by select\n",res);
                        if (FD_ISSET(sock_id, &wset) || FD_ISSET(sock_id, &rset))//sock_id是否还在集合wset,rset中
                        {  
                            int error;
                            socklen_t len = sizeof(error);
                            GLOG3("getsockopt : sock_id = %d\n",sock_id);
                                                     //获取套接口的待处理错误，若连接成功该值为0，若连接错误该值为errno值
                            if (getsockopt(sock_id, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
                                goto _connect_err_FAIL; //getsockopt错误
                            GLOG3("error =  %d\n",error);
                            if (error)               //连接错误
                                goto _connect_err_FAIL; 
                            fcntl(sock_id, F_SETFL, fcntl(sock_id,F_GETFL,0) & (~O_NONBLOCK));  
                            GLOG3("connect ok\n");
                            break;                   //连接成功
                        }  
                        else
                        {
                            goto _connect_err_FAIL;  //其他错误
                        }
                    }
                }
            }
        }
        _connect_err_FAIL:
        sock_close(sock_id);            
        sock_id = -1;   
        GLOG3("connect fail\n");
    } while (NULL != (res_addr = res_addr->ai_next));

    if (NULL == res_addr)
        sock_id = -1;
    GLOG3("sock_id = %d\n",sock_id);
    freeaddrinfo(tmp_addr);
    return sock_id;
}

/* 发送数据 */
int sock_snd(SOCK_ID sock_id, const char *data, int len)
{
    return(send(sock_id, data, len, 0));
}

/* 按长度接收数据 */
int sock_rcv(SOCK_ID sock_id, char *data, int len)
{
    if (!data)
        return (-1);
    int haveLen = 0;
    int curLen = 0;
    for(;;)
    {
        curLen = recvfrom(sock_id, data+haveLen, len-haveLen, 0,NULL,NULL);
        if ( -1 == curLen )
        {
            if ( ( (errno == EINTR) || errno == EWOULDBLOCK) || (errno == EAGAIN) )
                continue;
            return (-1);
        }
        haveLen += curLen;   
        if ( (0 == curLen) || (haveLen >= len) ) 
            return haveLen; 
    }
}

/* 读取一行数据 */
void sock_read_line(SOCK_ID sock_id, char *buf, int size)
{
    FILE *pf = fdopen(sock_id, "r");
    fgets(buf, size, pf); 
    fclose(pf);
}

/* 读取一行数据,返回读取的长度 */
int sock_get_line(SOCK_ID sock_id, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock_id, &c, 1, 0);             //读取一个字符
        if (n > 0)                               //读到了这个字符
        {
            if (c == '\r')                       //若是'\r'就继续读，因为可能是"\r\n"换行
            {
                n = recv(sock_id, &c, 1, MSG_PEEK);//MSG_PEEK参数类似于预读取，本次读取的值仍在缓冲区仍可重复被读
                if ((n > 0) && (c == '\n'))      //看看预读的数是否是'\n'
                    recv(sock_id, &c, 1, 0);     //如果是，就把'\n'彻底的从缓冲区中拿出来
                else
                    c = '\n';                    //如果下个数不是'\n'，就把'\r'也替换成'\n'
            }
            buf[i] = c;
            i++;
        }
        else                                     //没读取到这个字符
            c = '\n';  
    }
    buf[i] = '\0';                               //最后以'\0'结尾
    return(i);
}

/* 像fprintf一样写入数据 */
void sock_write_line(SOCK_ID sock_id, const char *fmt, ...)
{
    FILE *pf = fdopen(sock_id, "a");
    va_list args;
    va_start(args, fmt);
    vfprintf(pf, fmt, args);
    va_end(args);
    //fclose(pf);
}
