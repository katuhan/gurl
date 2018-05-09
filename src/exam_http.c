#include "http.h"

#if 1
int main()
{
    char url[] = "http://127.0.0.1/gurltest/gfile.4";


    //stp-1 ： init
    http_handle_t *handle = http_init();
    if (NULL == handle)
        return -1;
    //stp-2 ： set options
    http_set_opt(handle, "HTTP_URL", url);   
	//...更多属性查看http_opt.h
    //stp-3 ： perform
    if (!http_perform(handle))
    {
        //stp-4 ： get infomation
	    //...更多属性查看http_get.h
        void *p = http_get_info(handle, "HTTP_STA");
        printf("status is %s\n", (char *)p);
        p = http_get_info(handle, "HTTP_MSG");
        printf("msg is %s\n",(char *)p);
        p = http_get_info(handle, "HTTP_LEN");
        printf("len is %ld\n", *(long *)p);
        p = http_get_info(handle, "HTTP_TIME");
        printf("usec is %lf\n", *(double *)p);
    }
    //stp-5 ： cleanup
    http_cleanup(handle);
    return 0;
}
#endif

