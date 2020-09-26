/* timeop.h */
#ifndef __TIMEOP_H
#define __TIMEOP_H

/**
 * @brief ctime(3) 的封装
 * 会额外截断转换后字符串结尾的换行符
 * @return 时间的字符串形式
 * @warning 多线程不安全
 */
char *ytht_ctime(const time_t clock);

/** 比较当前时间和目标时间的差异，并返回适当的字符串。
 * 例如 5秒钟前，10分钟后
 * @param compared_time 需要对比的时间。
 * @return 更容易理解的文字
 * @warning 该方法不是线程安全的！并且会强制转换为 int 类型。
 */
char *ytht_Difftime(time_t compared_time);

/**
 * @brief ytht_Difftime 对应的安全版本
 */
char *ytht_Difftime_s(time_t compared_time, char *buf, size_t buf_len);

/**
 * @brief 获得某一天 0:00:00 的时间戳(UTC+8 时区)
 * @param tm
 * @return 时间戳
 */
time_t get_time_of_the_biginning_of_the_day(struct tm *tm);
#endif
