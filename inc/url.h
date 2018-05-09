/*
Distributed under the MIT License (MIT)
    Copyright (c) 2017 katuhan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished 
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/

/*
 * 该模块实现对URL的解析
 * 使用方法详见src/exam_url.c
 * 修改自https://github.com/Xsoda/url
 */
#ifndef _URL_H
#define _URL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//host format
typedef enum
{
   HOST_IPV4,
   HOST_IPV6,
   HOST_DOMAIN
} host_type_t;

typedef struct _url_field
{
   host_type_t host_type;             //主机类型  
   char *href;                        //原始url
   char *schema;
   char *username;
   char *password;
   char *host;
   char *port;
   char *path;
   int query_num;                     //query对个数
   struct {
      char *name;
      char *value;
   } *query;
   char *fragment;
} url_field_t;

#ifdef __cplusplus
extern "C" {
#endif

url_field_t *url_parse(const char *str);
void url_free(url_field_t *url);

#ifdef __cplusplus
}
#endif

#endif //!_URI_H
