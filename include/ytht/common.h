#ifndef BMYBBS_COMMON_H
#define BMYBBS_COMMON_H
void _errlog(char *fmt, ...);

/**
 * @brief extract and copy tokens from string
 *
 * 注意，和 strtok(3) 不同
 * @param buf    需要被处理的字符串
 * @param c      分隔符
 * @param tmp    存放结果的缓冲区，需要预先准备
 * @param max    最多分段
 */
int ytht_strtok(char *buf, int c, char **tmp, int max);

#define errlog(format, args...) _errlog(__FILE__ ":%s line %d " format, __FUNCTION__,__LINE__ , ##args)
#endif //BMYBBS_COMMON_H
