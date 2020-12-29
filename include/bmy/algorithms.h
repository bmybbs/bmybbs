#ifndef BMY_ALGORITHMS
#define BMY_ALGORITHMS
#include <stddef.h>

/**
 * @brief 将整形数组转化为字符串的函数
 * e.g. ([1, 2, 3], 3, ',') => "1,2,3"
 * @warn 使用结束需要调用 free(3) 释放返回的字符串
 * @param array 需要转换的数组
 * @param num 数组元素个数
 * @param delimiter 分隔符
 * @return 字符串（若参数不合法，或者中途内存不足，返回 NULL）
 */
char *bmy_algo_join_int_array_to_string(const int array[], size_t num, char delimiter);
#endif

