/* Copyright (c) 2017 gpan
 * http属性设置辅助头文件
 */
#ifdef HTTP_OPT_ENUM_INFO
#define HTTP_OPT_INFO(opt,str,fun) opt,
#endif 
#ifdef HTTP_OPT_STR_INFO
#define HTTP_OPT_INFO(opt,str,fun) str,
#endif
#ifdef HTTP_OPT_FUN_INFO
#define HTTP_OPT_INFO(opt,str,fun) fun,
#endif
#ifdef HTTP_OPT_FUN_DECLARE_INFO
#define HTTP_OPT_INFO(opt,str,fun) \
    static void fun(http_handle_t *handle, const char *key, const void *ver);
#endif

//OPT_INFO的参数分别为：属性枚举、属性字符串、设置该属性时被http_set_opt调用的函数
#ifdef HTTP_OPT_INFO
HTTP_OPT_INFO(HTTP_HEADER,     "HTTP_HEADER",    http_set_header)
HTTP_OPT_INFO(HTTP_URL,        "HTTP_URL",       http_set_url)
HTTP_OPT_INFO(HTTP_MOTHOD,     "HTTP_MOTHOD",    http_set_mothod)
HTTP_OPT_INFO(HTTP_VER,        "HTTP_VER",       http_set_version)
HTTP_OPT_INFO(HTTP_DATA,       "HTTP_DATA",      http_set_data)
HTTP_OPT_INFO(HTTP_CALLBACK,   "HTTP_CALLBACK",  http_set_callback)
#undef HTTP_OPT_INFO
#endif

#ifdef HTTP_OPT_FUN_DECLARE_INFO
#undef HTTP_OPT_FUN_DECLARE_INFO
#endif
#ifdef HTTP_OPT_FUN_INFO
#undef HTTP_OPT_FUN_INFO
#endif
#ifdef HTTP_OPT_STR_INFO
#undef HTTP_OPT_STR_INFO
#endif
#ifdef HTTP_OPT_ENUM_INFO
#undef HTTP_OPT_ENUM_INFO
#endif
