/*****************************************
 * function : 相关gurl公共函数的定义
 * author   : gpan
 * time     : Wed Dec 28 23:34:46 CST 2016
 * ***************************************/
#include "gpub.h"

static char cunit[] = {'B', 'K', 'M', 'G'};
static size_t save2file(void *ptr, size_t size, size_t nmemb, void *stream);
/***********************************************
 * cat_path: 从path,文件名name,组合出全路径到out
 * *********************************************/
int cat_path(const char *path, const char *name, char *out, int size)
{
    int re = -1;
    bzero(out, size);
    if (name)
    {
        if (path)
        {
            int iLen = strlen(path);
            if ('/' != path[iLen - 1])
            {
                if (iLen >= (size-1)) 
                    goto _cat_path_END;
                memcpy(out, path, iLen);
                out[iLen] = '/';
            }
            else
            {
                if (iLen >= size) 
                    goto _cat_path_END;
                memcpy(out, path, iLen);
            }
        }
        strncat(out, name, size - strlen(out));
        re = 0;
    }
_cat_path_END:
    return re;
}

/*******************************************************
 * filename_from_path : 从全路径path获取文件名到filename
 * *****************************************************/
int filename_from_path(const char *path, char *filename, int size)
{
    int re = -1;
    bzero(filename, size);
    if (path)
    {
        int length = strlen(path);
        if (length > 0)
        {
            int index = length - 1;
            while ((0 <= index) && ('/' != path[index]))
                index--;
            if ((size > (length - index  - 1)) && (length != (index + 1)))
            {
                memcpy(filename, &path[index+1], length - index -1);
                re = 0;
            }
        }
    } 
    return re;
}

/******************************
 * change_filename : 更改文件名
 * ***************************/
int change_filename(const char *path, const char *old, const char *new)
{
    int re = -1;
    if (old && new)
    {
        char sTmp1[PATH_LEN];
        char sTmp2[PATH_LEN];
        bzero(sTmp1, sizeof(sTmp1));
        bzero(sTmp2, sizeof(sTmp2));
        if (cat_path(path, old, sTmp1, sizeof(sTmp1)))
            goto _change_filename_END;
        if (cat_path(path, new, sTmp2, sizeof(sTmp2)))
            goto _change_filename_END;
        re = rename(sTmp1, sTmp2);
    }
_change_filename_END:
    return re;
}

/************************
 * remove_file : 删除文件
 * **********************/
void remove_file(const char *path, const char *filename)
{
    char sPath[PATH_LEN];
    bzero(sPath, sizeof(sPath));
    if (!cat_path(path, filename, sPath, sizeof(sPath)))
    {
        if (!access(sPath, F_OK)) 
            unlink(sPath);
    }
}

/************************************
 * open_file : 新建path/filename文件w
 * **********************************/
FILE *open_file(char *path,char *filename, char *mode)
{
    FILE *fp = NULL;
    char sPath[PATH_LEN];
    bzero(sPath, sizeof(sPath));
    if (!cat_path(path, filename, sPath, sizeof(sPath)))
        fp = fopen(sPath, mode);
    return fp;
}

/***************************
 * file_exist : 文件是否存在
 ***************************/
int file_exist(char *path,char *filename)
{
    char sPath[PATH_LEN];
    bzero(sPath, sizeof(sPath));
    if (!cat_path(path, filename, sPath, sizeof(sPath)))
    {
        if (!access(sPath, F_OK))
            return 0;
    }
    return -1;
}

/*************************************
 * single_http_download : 下载文件到fd
 * return:    -1,获取eaay handle失败
 *     	-2,下载失败
 * ***********************************/
int single_http_download(void *url, FILE *fd, char *pMsg, int size)         
{
    int re = -1;
    char *psta = NULL;
    http_handle_t *handle = http_init();
    if (!handle)
        goto _single_http_download_END;
 
    fseek(fd,  0, SEEK_END); 
    int offset = ftell(fd);             //获取文件偏移量
    if (offset > 0)                     //如果原文件中有内容
    {
        offset--;                       //-1 ?
        fseek(fd,  -1, SEEK_CUR); 
    }
 
    http_set_opt(handle, "HTTP_URL", url);    
    http_set_opt(handle, "HTTP_DATA", fd);    
    http_set_opt(handle, "HTTP_CALLBACK", save2file);    
    char range[20];
    bzero(range, sizeof(range));
    sprintf(range, "bytes=%d-",offset);       //offset以后的范围
    http_set_opt(handle, "Range", range);    
 
    int res = http_perform(handle);
    if ( 0 != res)                                   
    {
        re = -2;
        goto _single_http_download_END;
    }

    void *p = http_get_info(handle, "HTTP_STA");
    if (p && (!strcmp(p,"200") || !strcmp(p,"206"))) 
    {
        long downLength = 0;
        p = http_get_info(handle, "HTTP_LEN");
        if (p)
        {
            downLength = *(long *)p;
            if (-1.0 == downLength)
                sprintf(pMsg + strlen(pMsg),"  size = unknown");
            else
            {
                sprintf(pMsg + strlen(pMsg),"  size = %.2lf%c",\
                downLength/pow(1024,size_unit), cunit[size_unit]);
            }
        }
        double usetime = 0.0;
        p = http_get_info(handle, "HTTP_TIME");
        if (p)
        {
            usetime = *(double *)p;
            sprintf(pMsg + strlen(pMsg),"  time = %.2lfs", usetime/1000000);
        }
        if (downLength && usetime)
        {
            sprintf(pMsg + strlen(pMsg),"  speed = %.2lf%c/s", \
            (downLength/pow(1024, size_unit))/(usetime/1000000), cunit[size_unit]);
        }
        re = 0; goto _single_http_download_END;
    }
    else if(p)
    {
        psta = p;
    }
    re = -3; 

_single_http_download_END:
    if (!re)
        sprintf(pMsg + strlen(pMsg),"  status = OK\n");
    else if (psta)
        sprintf(pMsg + strlen(pMsg),"  status = failed(%s)\n",psta);
    else
        sprintf(pMsg + strlen(pMsg),"  status = failed : %s\n", handle->errmsg);
    if (handle)
        http_cleanup(handle);
    return re;
}

/************************************
 * save2file : 写数据到文件的回调函数
 * *********************************/
static size_t save2file(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}
