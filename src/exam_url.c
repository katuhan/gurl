#include "url.h"

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

#if 1
void url_field_print(url_field_t *url)
{
   char *str_hosttype[] = { "host ipv4", "host ipv6", "host domain", NULL };
   if (!url) return;
   fprintf(stdout, "\nurl field:\n");
   fprintf(stdout, "  - href:     '%s'\n", url->href);
   fprintf(stdout, "  - schema:   '%s'\n", url->schema);
   if (url->username)
      fprintf(stdout, "  - username: '%s'\n", url->username);
   if (url->password)
      fprintf(stdout, "  - password: '%s'\n", url->password);
   fprintf(stdout, "  - host:     '%s' (%s)\n", url->host, str_hosttype[url->host_type]);
   if (url->port)
      fprintf(stdout, "  - port:     '%s'\n", url->port);
   if (url->path)
   fprintf(stdout, "  - path:     '%s'\n", url->path);
   if (url->query_num > 0)
   {
      fprintf(stdout, "  - query\n");
      for (int i = 0; i < url->query_num; i++)
      {
         fprintf(stdout, "    * %s : %s\n", url->query[i].name, url->query[i].value);
      }
   }
   if (url->fragment)
      fprintf(stdout, "  - fragment: '%s'\n", url->fragment);
}

int main()
{
   //stp-1 ：url_parse解析,返回url_field_t *
   //stp-2 : 使用url_field_t *指向的值
   //stp-3 : url_free一定要释放
   url_field_t *url = url_parse("abc://username:password@example.com:123/path/data?key=value&key2=value2#fragid1");
   url_field_print(url);
   url_free(url);
   return 0;
}
#endif

