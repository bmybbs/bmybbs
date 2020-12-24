/**
 * 原 libythtbbs/misc.c 中有关 gbk <--> utf8 互转的代码，提取出来作为公用。
 * 需要注意的是和 src/bbs/convcode 不同，后者用于 gbk <--> big5。
 */
#ifndef BMY_CONVCODE_H
#define BMY_CONVCODE_H
#include <stddef.h>

int u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int code_convert(const char *from_charset, const char *to_charset, const char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int is_utf_special_byte(unsigned char c);
int is_utf(const char * inbuf, size_t inlen);

#endif

