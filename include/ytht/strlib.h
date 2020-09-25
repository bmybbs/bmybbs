#ifndef BMYBBS_STRLIB_H
#define BMYBBS_STRLIB_H
#include <stdlib.h>

// 类似于 strstr(3)
char *ytht_strnstr(const char *haystack, const char *needle, size_t haystacklen);

// 类似于 strcasestr(3)
char *ytht_strncasestr(const char *haystack, const char *needle, size_t haystacklen);
void normalize(char *buf);

/**
 * @brief 安全版本的 strncpy
 * 这可能是被使用得最广泛的库函数了，没有之一，几乎主要模块所有的代码文件中都依赖它。
 * @param s1 目标缓冲区
 * @param s2 源
 * @param n  长度
 */
void ytht_strsncpy(char *s1, const char *s2, int n);
char *strltrim(char *s);
char *strrtrim(char *s);
#define strtrim(s) strltrim(strrtrim(s))
#endif //BMYBBS_STRLIB_H
