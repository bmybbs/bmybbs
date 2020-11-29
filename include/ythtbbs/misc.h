/* misc.c */
#ifndef __MISC_H
#define __MISC_H
#include <stddef.h>

int u2g(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int is_utf_special_byte(unsigned char c);
int is_utf(char * inbuf, size_t inlen);

int badstr(char *s);

/**
 * @brief 获取不超过4张进站图片
 * 该方法将把当前进站图片的信息输出到 pics_list 中。每条信息使用单个半角分号将图片和
 * 链接隔开。多条信息使用两个半角分号隔开。
 * @warning 该函数最早于 2011.09.05 由 IronBlood 编写在 nju09/bbsindex.c 中。为了
 * api 复用，调整到了 libythtlib/misc.h 中，并变更了函数原型。
 */
void get_no_more_than_four_login_pics(char *buf, size_t len);
#endif
