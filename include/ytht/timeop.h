/* timeop.h */
#ifndef __TIMEOP_H
#define __TIMEOP_H
#include <time.h>

/**
 * @brief ctime(3) 的封装
 * 会额外截断转换后字符串结尾的换行符
 * @return 时间的字符串形式
 * @warning 多线程不安全
 * @see ytht_ctime_r
 */
char *ytht_ctime(const time_t clock) __attribute__((deprecated("use ytht_ctime_r instead")));

/**
 * @brief ctime_r(3) 的封装
 * buf 至少为 26 字节
 * 会额外截断转换后字符串结尾的换行符
 * @return 时间的字符串形式
 */
char *ytht_ctime_r(const time_t clock, char *buf);

/** 比较当前时间和目标时间的差异，并返回适当的字符串。
 * 例如 5秒钟前，10分钟后
 * @param compared_time 需要对比的时间。
 * @return 更容易理解的文字
 * @warning 该方法不是线程安全的！并且会强制转换为 int 类型。
 */
char *ytht_Difftime(time_t compared_time) __attribute__((deprecated("use ytht_Difftime_s instead")));

/**
 * @brief ytht_Difftime 对应的安全版本
 */
char *ytht_Difftime_s(time_t compared_time, char *buf, size_t buf_len);

/**
 * @brief 生成符合 RFC 7231 的日期格式
 * 该格式广泛用于 HTTP 协议中。
 * 参考了 https://stackoverflow.com/a/7548846/803378 的实现。在 NJU09 中亦有多处使用。
 * 使用了多线程安全的方式。
 * @return 返回值参见 strftime(3) 的返回值说明。若返回值为 0，说明 buf 缓冲区不够大。
 */
size_t ytht_utc_time_s(char *buf, const size_t len, const time_t *t);

/**
 * @brief 获得某一天 0:00:00 的时间戳(UTC+8 时区)
 * @param tm
 * @return 时间戳
 */
time_t get_time_of_the_biginning_of_the_day(struct tm *tm);
#endif
