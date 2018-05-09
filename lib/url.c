/*
 * modify from https://github.com/Xsoda/url
 */
#include "url.h"

static int host_is_ipv4(char *str);
static void parse_query(url_field_t *url, char *query);
//static char *strndup(const char *str, int n);

/* * str是否为点分十进制ipv4地址,不能有额外的空格 */
static int host_is_ipv4(char *str)
{
    if (!str) return 0;
    while (*str)
    {
        if ((*str >= '0' && *str <= '9') || *str == '.')
            str++;
        else
            return 0;
    }
    return 1;
}

static void parse_query(url_field_t *url, char *query)
{
    //int length;
    //int offset;
    char *chr;
    //length = strlen(query);
    //offset = 0;
    chr = strchr(query, '=');
    while (chr)
    {
        if (url->query)
            url->query = realloc(url->query, (url->query_num + 1) * sizeof(*url->query));
        else
            url->query = malloc(sizeof(*url->query));
        url->query[url->query_num].name = strndup(query, chr - query);
        query = chr + 1;
        chr = strchr(query, '&');
        if (chr)
        {
            url->query[url->query_num].value = strndup(query, chr - query);
            url->query_num++;
            query = chr + 1;
            chr = strchr(query, '=');
        }
        else
        {
            url->query[url->query_num].value = strndup(query, strlen(query));
            url->query_num++;
            break;
        }
    }
}

/******************************************************************************************
 *                hierarchical part                                                       *
 *           ┌───────────────────┴─────────────────────┐                                  *
 *                       authority               path                                     *
 *           ┌───────────────┴───────────────┐┌───┴────┐                                  *
 *     abc://username:password@example.com:123/path/data?key=value&key2=value2#fragid1    *
 *     └┬┘   └───────┬───────┘ └────┬────┘ └┬┘           └─────────┬─────────┘ └──┬──┘    *
 *   scheme  user information     host     port                  query         fragment   *
 *                                                                                        *
 *     urn:example:mammal:monotreme:echidna                                               *
 *     └┬┘ └──────────────┬───────────────┘                                               *
 *   scheme              path                                                             *
 ******************************************************************************************/
url_field_t *url_parse (const char *str)
{
    char *query;
    query = NULL;

    url_field_t *url = NULL;;
    if (NULL == str)
        goto __fail;      
    if ((url = (url_field_t *)malloc(sizeof(url_field_t))) == NULL)
        goto __fail;      
    memset(url, 0, sizeof(url_field_t));

    url->href = strndup(str, strlen(str));
    const char *pch = strchr(str, ':');             /* parse schema */
    if (pch && pch[1] == '/' && pch[2] == '/')
    {
        url->schema = strndup(str, pch - str); 
        str = pch + 3;
    }
    else
        goto __fail;

    pch = strchr(str, '@');                         /* parse user info */
    if (pch)
    {
        pch = strchr(str, ':');
        if (pch)
        {
            url->username = strndup(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '@');
            if (pch)
            {
                url->password = strndup(str, pch - str);
                str = pch + 1;
            }
         else
            goto __fail;
        }
        else
            goto __fail;
    }
                                                    /* parse host info */
    if (str[0] == '[')                              /* IPv6 addresses must be enclosed in brackets ([ ]).*/
    {
        str++;
        pch = strchr(str, ']');
        if (pch)
        {
            url->host = strndup(str, pch - str);
            str = pch + 1;
            if (str[0] == ':')                      /* port information */
            {
                str++;
                pch = strchr(str, '/');
                if (pch)
                {
                    url->port = strndup(str, pch - str);
                    str = pch + 1;
                }
                else
                {
                    url->port = strndup(str, strlen(str));
                    str = str + strlen(str);
                }
            }
            url->host_type = HOST_IPV6;
        }
        else
            goto __fail;
    }
    else
    {
        const char *pch_slash;
        pch = strchr(str, ':');                     /* have port info */
        pch_slash = strchr(str, '/');
        if (pch && (!pch_slash || (pch_slash && pch<pch_slash)))
        {
            url->host = strndup(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '/');
            if (pch)
            {
                url->port = strndup(str, pch - str);
                str = pch + 1;
            }
            else
            {
                url->port = strndup(str, strlen(str));
                str = str + strlen(str);
            }
        }
        else                                        /* no port info */
        {
            pch = strchr(str, '/');
            if (pch)
            {
                url->host = strndup(str, pch - str);
                str = pch + 1;
            }
            else
            {
                url->host = strndup(str, strlen(str));
                str = str + strlen(str);
            }
        }
        url->host_type = host_is_ipv4(url->host) ? HOST_IPV4 : HOST_DOMAIN;
    }
    if (str[0])                                     /* parse path, query and fragment */
    {
        pch = strchr(str, '?');
        if (pch)
        {
            url->path = strndup(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '#');
            if (pch)
            {
                query = strndup(str, pch - str);
                str = pch + 1;
                url->fragment = strndup(str, strlen(str));
            }
            else
            {
                query = strndup(str, strlen(str));
                str = str + strlen(str);
            }
            parse_query(url, query);
            free(query);
        }
        else
        {
            pch = strchr(str, '#');
            if (pch)
            {
                url->path = strndup(str, pch - str);
                str = pch + 1;
                url->fragment = strndup(str, strlen(str));
                str = str + strlen(str);
            }
            else
            {
                url->path = strndup(str, strlen(str));
                str = str + strlen(str);
            }
        }
    }
    else
    {
__fail:
        url_free(url);
        return NULL;
    }
    return url;
}

void url_free(url_field_t *url)
{
    if (!url) return;
    if (url->href) free(url->href);
    if (url->schema) free(url->schema);
    if (url->username) free(url->username);
    if (url->password) free(url->password);
    if (url->host) free(url->host);
    if (url->port) free(url->port);
    if (url->path) free(url->path);
    if (url->query)
    {
        for (int i = 0; i < url->query_num; i++)
        {
            free(url->query[i].name);
            free(url->query[i].value);
        }
        free(url->query);
    }
    if (url->fragment) free(url->fragment);
    free(url);
}

/**
static char *strndup(const char *str, int n)
{
    char *dst;
    if (!str) return NULL;
    if (n < 0) n = strlen(str);
    if (n == 0) return NULL;
    if ((dst = (char *)malloc(n + 1)) == NULL)
        return NULL;
    memcpy(dst, str, n);
    dst[n] = 0;
    return dst;
}
**/
