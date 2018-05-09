/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

/* 节点结构 */
struct map_node_t 
{
    unsigned hash;                                                  //1-key的hash值
    void *value;                                                    //2-指向5的位置
    map_node_t *next;                                               //3-next指针
    /* char key[]; */                                               //4-真正存储结构里键-值对中的键
    /* char value[]; */                                             //5-真正存储结构里键-值对中的值
};

/* hash函数 */
static unsigned map_hash(const char *str) 
{
    unsigned hash = 5381;
    while (*str) 
    {
        hash = ((hash << 5) + hash) ^ *str++;
    }
    return hash;
}

/* 创建一个新节点 */
static map_node_t *map_newnode(const char *key, void *value, int vsize)
{
    map_node_t *node;                                            
    int ksize = strlen(key) + 1;                                     //键长度+1
    int voffset = ksize + ((sizeof(void*) - ksize) % sizeof(void*)); //key的存储空间大小,sizeof(void *)的整数倍
    node = malloc(sizeof(*node) + voffset + vsize);                  //存储结构map_node_t + key + value
    if (!node) 
        return NULL;
    memcpy(node + 1, key, ksize);
    node->hash = map_hash(key);
    node->value = ((char*) (node + 1)) + voffset;                    //value存储起始位置
    memcpy(node->value, value, vsize);
    return node;
}

/* 获取桶下标 */
static int map_bucketidx(map_base_t *m, unsigned hash)
{
    /* If the implementation is changed to allow a non-power-of-2 bucket count,
     * the line below should be changed to use mod instead of AND */
    return hash & (m->nbuckets - 1);                                 //返回桶下标,桶个数为2的幂
}

/* 添加新的节点到m  */
static void map_addnode(map_base_t *m, map_node_t *node) 
{
    int n = map_bucketidx(m, node->hash);                            //根据hash得知应该放入下标为n的桶
    node->next = m->buckets[n];                                      //在下标为n的桶表示的链表头添加节点
    m->buckets[n] = node;
}

/* 更改map容器的大小，即重新设置桶的个数 */
static int map_resize(map_base_t *m, int nbuckets) 
{
    map_node_t *nodes, *node, *next;
    map_node_t **buckets;
    int i; 

    nodes = NULL;                                                    //把所有桶中的节点链在一起,头节点为nodes
    i = m->nbuckets;                                                 //原来桶个数
    while (i--) 
    {
        node = (m->buckets)[i];                                      //下标为i的桶的第一个节点
        while (node)
        {                                                            //把该桶中每一个节点加入到nodes
            next = node->next;                                       //暂存下一个节点位置
            node->next = nodes;                                      //把node加在nodes前面
            nodes = node;                                        
            node = next;                                             //处理该桶中下一个节点
        }
    }
    buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);   //根据新的桶个数，从新分配MAP容器空间
    if (buckets != NULL) 
    {
        m->buckets = buckets;
        m->nbuckets = nbuckets;
    }
    if (m->buckets) 
    {
        memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
        node = nodes;                                                //把所有节点重新转入重置后的各个桶中
        while (node)
        {
            next = node->next;
            map_addnode(m, node);
            node = next;
        }
    }
    return (buckets == NULL) ? -1 : 0;
}

/* 查找键为key的节点,返回节点的二级指针 */
static map_node_t **map_getref(map_base_t *m, const char *key) 
{
    unsigned hash = map_hash(key);                                   //获取key的hash值
    map_node_t **next;
    if (m->nbuckets > 0)
    {
        next = &m->buckets[map_bucketidx(m, hash)];                  //有hash值得桶下标,并去掉该桶内第一个节点
        while (*next)
        {                                                            //hash与键都相同
            if ((*next)->hash == hash && !strcmp((char*) (*next + 1), key)) 
            {
                return next;
            }
            next = &(*next)->next;
        }
    }
    return NULL;
}

/* 销毁map容器 */
void map_deinit_(map_base_t *m) 
{
    map_node_t *next, *node;
    int i = m->nbuckets;
    while (i--)                                                      //遍历容器内的每个桶
    {
        node = m->buckets[i];
        while (node)                                                 //销毁桶内的每个节点
        {
            next = node->next;
            free(node);
            node = next;
        }
    }
    free(m->buckets);                                                  
}

/* 获取键为key对应的值 */
void *map_get_(map_base_t *m, const char *key) 
{
    map_node_t **next = map_getref(m, key);
    return next ? (*next)->value : NULL;
}

/* 添加或重置map对 */
int map_set_(map_base_t *m, const char *key, void *value, int vsize)
{
    int n, err;
    map_node_t **next, *node;
    next = map_getref(m, key);                                       //查找键为key的节点
    if (next) {                                                      //若节点存在，更改对应的值
      memcpy((*next)->value, value, vsize);
      return 0;
    }
                                                                     //否则，把新节点加入map容器
    node = map_newnode(key, value, vsize);
    if (node == NULL) goto fail;
    if (m->nnodes >= m->nbuckets)                                    //???桶个数有必要大于等于节点个数????
    {
        n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1;              //桶个数x2
        err = map_resize(m, n);
        if (err) goto fail;
    }
    map_addnode(m, node);
    m->nnodes++;
    return 0;
    fail:
    if (node) free(node);
    return -1;
}

/* 删除键为key的对 */
void map_remove_(map_base_t *m, const char *key) 
{
    map_node_t *node;
    map_node_t **next = map_getref(m, key);
    if (next) 
    {
        node = *next;
        *next = (*next)->next;
        free(node);
        m->nnodes--;
    }
}


map_iter_t map_iter_(void) 
{
    map_iter_t iter;
    iter.bucketidx = -1;
    iter.node = NULL;
    return iter;
}


const char *map_next_(map_base_t *m, map_iter_t *iter) 
{
    if (iter->node) 
    {
        iter->node = iter->node->next;
        if (iter->node == NULL) 
            goto nextBucket;
    } 
    else 
    {
        nextBucket:
        do {
            if (++iter->bucketidx >= m->nbuckets) 
            {
                return NULL;
            }
            iter->node = m->buckets[iter->bucketidx];
        }while (iter->node == NULL);
    }
    return (char*) (iter->node + 1);
}
