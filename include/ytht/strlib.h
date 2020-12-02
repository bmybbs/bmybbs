#ifndef BMYBBS_STRLIB_H
#define BMYBBS_STRLIB_H
#include <stdlib.h>

// 类似于 strstr(3)
char *ytht_strnstr(const char *haystack, const char *needle, size_t haystacklen);

// 类似于 strcasestr(3)
char *ytht_strncasestr(const char *haystack, const char *needle, size_t haystacklen);

/**
 * @brief 将字符串中包含的 "/" 转换为 ":"
 * 猜测便于生成对应的 *NIX 文件路径
 * @param buf 原字符串
 */
void ytht_normalize(char *buf);

/**
 * @brief 安全版本的 strncpy
 * 这可能是被使用得最广泛的库函数了，没有之一，几乎主要模块所有的代码文件中都依赖它。
 * @param s1 目标缓冲区
 * @param s2 源
 * @param n  长度
 */
void ytht_strsncpy(char *s1, const char *s2, int n);

/**
 * @brief 从左侧裁剪空白字符
 * 包括 " ", "\t", "\r", "\n"
 * @param s 待裁剪的字符串
 * @return 原字符串中首个非空字符的地址
 */
char *ytht_strltrim(char *s);

/**
 * @brief 从右侧裁剪空白字符
 * @warning 多线程不安全
 * @param s 待裁剪的字符串
 * @return 内部静态变量的地址
 */
char *ytht_strrtrim(char *s);

/**
 * @warning 多线程不安全
 */
#define ytht_strtrim(s) ytht_strltrim(ytht_strrtrim(s))

char *ytht_str_to_uppercase(char *str);

char *ytht_str_to_lowercase(char *str);

int ytht_badstr(const char *s);
#endif //BMYBBS_STRLIB_H
