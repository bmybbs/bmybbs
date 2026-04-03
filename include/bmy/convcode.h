/**
 * 原 libythtbbs/misc.c 中有关 gbk <--> utf8 互转的代码，提取出来作为公用。
 * 需要注意的是和 src/bbs/convcode 不同，后者用于 gbk <--> big5。
 */
#ifndef BMY_CONVCODE_H
#define BMY_CONVCODE_H
#include <stddef.h>

/**
 * @brief 将 UTF-8 编码内容转换为 GBK
 * @param inbuf 输入缓冲区
 * @param inlen 要转换的输入缓冲区字节数；对于 `\0` 结尾的字符串，通常可传入 `strlen(inbuf)`。
 * @param outbuf 输出缓冲区
 * @param outlen 输出缓冲区容量，单位为字节，通常使用 `sizeof` 计算
 * @return 成功返回 0，失败返回 -1
 */
int u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen);

/**
 * @brief 将 GBK 编码内容转换为 UTF-8
 * @param inbuf 输入缓冲区
 * @param inlen 要转换的输入缓冲区字节数；对于 `\0` 结尾的字符串，通常可传入 `strlen(inbuf)`。
 * @param outbuf 输出缓冲区
 * @param outlen 输出缓冲区容量，单位为字节，通常使用 `sizeof` 计算
 * @return 成功返回 0，失败返回 -1
 */
int g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int is_utf_special_byte(unsigned char c);
int is_utf(const char * inbuf, size_t inlen);

#endif

