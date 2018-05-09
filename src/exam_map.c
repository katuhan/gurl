/*****************************************************
 * function :  hash map库使用例子,更多参考exam_map.md
 * author   :  gpan
 * time     :  Thu Jan 19 16:55:54 CST 2017
 * **************************************************/
#include <stdio.h>  
#include "map.h"                                       //stp1-包含头文件

int   
main()  
{  
    char *str1 = "line_1";
    char *str2 = "line_2";
    char *str3 = "line_3";

    map_str_t smap;                                    //stp2-map变量
    map_init(&smap);                                   //stp3-初始化
    map_set(&smap, "key1", str1);                    //stp4-添加或修改map对
    map_set(&smap, "key2", str2);
    map_set(&smap, "key3", str3);

    map_get(&smap, "key1");                            //stp5-根据键获取值
    printf("key = key1, value = %s\n", *(smap.ref) );

    map_remove(&smap, "key1");                         //stp6-可以删除map对
    map_get(&smap, "key1");                            //必须从新获取
    if( smap.ref && *(smap.ref) )
        printf("key = key1, value = %s\n", *(smap.ref) );

    map_get(&smap, "key2");                          
    printf("key = key2, value = %s\n", *(smap.ref) );
    map_get(&smap, "key3");                         
    printf("key = key3, value = %s\n", *(smap.ref) );

	map_iter_t iter = map_iter(&smap);
	const char *key;
    while ((key = map_next(&smap, &iter)))
	{
		printf("%s -> %s\n", key, *map_get(&smap, key));
	}

    map_deinit(&smap);                                 //stp6-销毁
    return 0;  
}  

