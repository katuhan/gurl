/*****************************************
 * function : 调试信息输出
 * author   : gpan
 * time     : Tue Jan 10 17:32:47 CST 2017
 ****************************************/
#ifndef __debug_h__
#define __debug_h___

#define BASIC_DEBUG  1   
#define DETAIL_DEBUG 2  
#define SUPER_DEBUG  4 
#ifndef DEBUG
#define DEBUG 0
#endif


#if (DEBUG & BASIC_DEBUG)  
#define GLOG1(fmt,...) do{\
        printf("[%s]--<%s>(%d) : ",__func__,__FILE__,__LINE__);\
        printf(fmt,##__VA_ARGS__);\
	}while(0)
#else
#define GLOG1(fmt,...) 
#endif

#if (DEBUG & DETAIL_DEBUG)
#define GLOG2(fmt,...) do{\
        printf("[%s]--<%s>(%d) : ",__func__,__FILE__,__LINE__);\
        printf(fmt,##__VA_ARGS__);\
	}while(0)
#else
#define GLOG2(fmt,...) 
#endif

#if (DEBUG & SUPER_DEBUG)
#define GLOG3(fmt,...) do{\
        printf("[%s]--<%s>(%d) : ",__func__,__FILE__,__LINE__);\
        printf(fmt,##__VA_ARGS__);\
	}while(0)
#else
#define GLOG3(fmt,...) 
#endif

#endif //__debug_h__
