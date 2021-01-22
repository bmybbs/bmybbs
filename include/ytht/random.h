#ifndef BMYBBS_RANDOM_H
#define BMYBBS_RANDOM_H
#include <stddef.h>

/**
 * @brief 给 buf 填充随机值
 * 借助 /dev/urandom 读取随机值，和其他 getrandom* 方法不同，
 * 本函数不会将 buf 最后变更为 0 作为字符串终止符。
 */
void ytht_get_random_buf(char *buf, size_t len);
int ytht_get_random_int(unsigned int *s);
void ytht_get_random_str(char *s);
/**
 * getrandomstr 方法的变种
 * @param s 字符串
 * @param len 长度
 */
int ytht_get_random_str_r(char *s, size_t len);

/**
 * @brief 加盐的方法
 * 来自 nju09
 * @param salt
 */
int ytht_get_salt(char *salt);
#endif
