/* Copyright (c) 2017 gpan
 * http信息获取辅助头文件
 */
#ifdef HTTP_GET_ENUM_INFO
#define HTTP_GET_INFO(enu,str,fun) enu,
#endif 
#ifdef HTTP_GET_STR_INFO
#define HTTP_GET_INFO(enu,str,fun) str,
#endif
#ifdef HTTP_GET_FUN_INFO
#define HTTP_GET_INFO(enu,str,fun) fun,
#endif
#ifdef HTTP_GET_FUN_DECLARE_INFO
#define HTTP_GET_INFO(enu,str,fun)  static void *fun(http_handle_t *handle, const char *key);
#endif

//GET_INFO的参数分别为：属性枚举、属性字符串、获取http信息时被http_get_info调用的函数
#ifdef HTTP_GET_INFO
HTTP_GET_INFO(HTTP_HEAD,"HTTP_HEADER",http_get_reheader)   //获取应答头域
HTTP_GET_INFO(HTTP_STA,"HTTP_STA",http_get_sta)            //获取应答状态
HTTP_GET_INFO(HTTP_MSG,"HTTP_MSG",http_get_msg)            //获取应答信息
HTTP_GET_INFO(HTTP_LEN,"HTTP_LEN",http_get_len)            //获取接收长度
HTTP_GET_INFO(HTTP_TIME,"HTTP_TIME",http_get_time)         //获取下载时间
#undef HTTP_GET_INFO
#endif

#ifdef HTTP_GET_FUN_DECLARE_INFO
#undef HTTP_GET_FUN_DECLARE_INFO
#endif
#ifdef HTTP_GET_FUN_INFO
#undef HTTP_GET_FUN_INFO
#endif
#ifdef HTTP_GET_STR_INFO
#undef HTTP_GET_STR_INFO
#endif
#ifdef HTTP_GET_ENUM_INFO
#undef HTTP_GET_ENUM_INFO
#endif
