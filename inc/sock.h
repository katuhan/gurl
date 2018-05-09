#ifndef _SOCK_H
#define _SOCK_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//netdb.h不是C11标准里定义的函数,是posix标准里的，-std=c11 会导致netdb.h无效
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include "debug.h"


typedef int SOCK_ID;

//SOCK_ID sock_connect(const char *host, const char *serv);
SOCK_ID sock_connect(const char *host, const char *serv, int sec);
int sock_snd(SOCK_ID sock_id, const char *data, int len);
int sock_rcv(SOCK_ID sock_id, char *data, int len);
void sock_read_line(SOCK_ID sock_id, char *buf, int size);
int sock_get_line(SOCK_ID sock_id, char *buf, int size);
void sock_write_line(SOCK_ID sock_id, const char *fmt, ...);
void sock_close(SOCK_ID sock_id);

#endif //_SOCK_H
