/*****************************************
 * function : 相关gurl子函数的定义
 * author   : gpan
 * time     : Wed Dec 28 23:34:46 CST 2016
 *****************************************/
#include "gsub.h"
    
/*************************
 * init : 对gurl进行初始化
 ************************/
void gurl_init()
{
    bzero(g_pSaveDir, PATH_LEN);
    memcpy(g_pSaveDir, "./", 2);  
    g_counter = 0;
    pthread_mutex_init(&counter_lock, NULL);
    pthread_cond_init(&counter_over, NULL);
    pthread_mutex_init(&screen_lock, NULL);
    //curl_global_init(CURL_GLOBAL_ALL);              
}

/***************************
 * gurl_close : gurl结束清理
 * ************************/
void gurl_close()
{
    pthread_mutex_destroy(&(screen_lock));
    pthread_cond_destroy(&counter_over);
    pthread_mutex_destroy(&(counter_lock));
    //curl_global_cleanup(); 
}

/*******************
 * usage :  使用帮助
 ******************/
void usage(int sta)
{
    static int  flg = 1;   
    if (!flg) return ;
    flg = 0;
    printf("-------------------gurl usage---------------------\n");
    printf("NAME\n");
    printf("     %s - %s\n", APPNAME, APPDSCR);
    printf("SYNOPSIS\n");
    printf("    gurl [ v|h ] | [ [-o <dir>|b|m|k|g] URL... ]\n");
    printf("OPTIONS\n");
    printf("    -v,        show version\n");
    printf("    -h,        help\n");
    printf("    -o,        save location\n");
    printf("    -b,m,k,g,  file size unit\n");
    printf("URL\n");
    printf("    can specify multiple URLs,but not more than %d\n",URL_NUM);
    printf("--------------------------------------------------\n");
    if (sta)                                                    //-h与别的选项连用时，不至于显示后退出而没处理别的选项 
        exit(sta);
}

/*************************
 * show_version : 版本信息
 ************************/
void show_version()
{
    printf("%s -- %s\n", APPNAME, APPDSCR);
    printf("Version : %s\n", APPVER);
    printf("Protocols : http\n");
    exit(0);                                                    //查看版本后立即结束
}

/*********************************
 * set_save_dir : 设置文件保存位置
 * ******************************/
void set_save_dir(char *path)
{
    if (path && (strlen(path) < PATH_LEN))
    {
        bzero(g_pSaveDir, PATH_LEN);
        memcpy(g_pSaveDir, path, strlen(path));
    }
}  

/****************************
 * print_to_screen : 屏幕输出
 * **************************/
void print_to_screen(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    pthread_mutex_lock(&screen_lock);
    vprintf(fmt, args);          
    pthread_mutex_unlock(&screen_lock);
    va_end(args);
} 

/******************************
 * inter_with_screen : 屏幕交互
 * ****************************/
int inter_with_screen(const char *ans, int length, const char *fmt,...)
{
    char sInput[MAX_LINE];
    bzero(sInput, sizeof(sInput));
    va_list args;
    va_start(args, fmt);
    pthread_mutex_lock(&screen_lock);
    vprintf(fmt, args);          
    //fflush(stdin);  //标准好像对stdin的fflush行为未定义,避免使用它
    //scanf("%s", &sInput); 
	fgets(sInput, sizeof(sInput), stdin);
    pthread_mutex_unlock(&screen_lock);
    va_end(args);
	//fgets得到的字符串sInput中包含输入的\n,由length避开
    return strncasecmp(sInput, ans, length);
} 

/****************************
 * deal_url : URL线程处理函数
 * *************************/
void *deal_url(void *url)
{
    GLOG2("begin\n");
    char *purl = (char *)url;
    if (purl)                      
    {
        GLOG2("url=[%s]\n",purl);
        char sPath[PATH_LEN];
		url_field_t *url_field = url_parse(purl);             //从url中获取文件路径
		if (url_field && url_field->path && (strlen(url_field->path) < sizeof(sPath)))
        {
            bzero(sPath, sizeof(sPath));
		    memcpy(sPath, url_field->path, strlen(url_field->path));
		    url_free(url_field);
        }
		else
        {
			if (url_field)
		        url_free(url_field);
            print_to_screen("Can not parse url '%s'\n", purl);          
            goto _deal_url_END;
        }
        GLOG2("path=[%s]\n",sPath);
  
        char sFilenm[PATH_LEN];
        bzero(sFilenm, sizeof(sFilenm));                      //从文件路径获取文件名
        if (0 > filename_from_path(sPath, sFilenm, sizeof(sFilenm)))
        {
            print_to_screen("Can not parse filename from '%s'\n", purl);          
            goto _deal_url_END;
        }
        GLOG2("filename=[%s]\n",sFilenm);
  
        if (!file_exist(g_pSaveDir, sFilenm))                 //要下载的文件已在本地存在
        {
            if (inter_with_screen("y", 1,"File '%s' exists,overwrite(y/n)?\n", sFilenm)) 
                goto _deal_url_END;
        }
  
        char  sFilenmTmp[PATH_LEN + 4];                       //组tmp文件名
        bzero(sFilenmTmp, sizeof(sFilenmTmp));   
        memcpy(sFilenmTmp, sFilenm, sizeof(sFilenm));
        strcat(sFilenmTmp, ".tmp");                           
        GLOG2("filename.tmp=[%s]\n",sFilenmTmp);
  
        FILE *fp = NULL;
        char resumeflg = 0;                                   //断点续传标志 1-需要续传
        if (!file_exist(g_pSaveDir, sFilenmTmp))              //tmp文件已经存在
            if (!inter_with_screen("y", 1,"Find a break about '%s',resume from it?(y/n)\n", sFilenm)) 
                resumeflg = 1;                                //断点续传
  
        GLOG2("resumeflg=[%d]\n",resumeflg);
        fp = open_file(g_pSaveDir, sFilenmTmp, (resumeflg ? "r+" : "w+"));
        if (NULL == fp)
        {
            print_to_screen("Open file '%s' error\n", sFilenmTmp);          
            goto _deal_url_END;
        }
  
        GLOG2("call_single_http_download\n");
        char sMsg[MAX_LINE];
        bzero(sMsg, sizeof(sMsg));
        snprintf(sMsg, sizeof(sMsg), "%s : ", sFilenm);
        if (single_http_download(purl, fp, sMsg, sizeof(sMsg) - strlen(sMsg)))
            remove_file(g_pSaveDir, sFilenm);
        else
            change_filename(g_pSaveDir, sFilenmTmp, sFilenm);
        print_to_screen("%s", sMsg);          
        fclose(fp);
    }
 
_deal_url_END:
    pthread_mutex_lock(&counter_lock);
    g_counter++;                   
    GLOG2("counter=[%ld]\n",g_counter);
    pthread_mutex_unlock(&counter_lock);
    pthread_cond_signal(&counter_over);
    GLOG2("end\n");
    return NULL;
}
