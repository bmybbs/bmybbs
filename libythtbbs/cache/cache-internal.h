#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "ythtbbs/misc.h"

/**
 * @brief 向 UserTable 中添加 utmp 记录
 * 方法实现参考了 src/bbs/bcache.c::add_uindex，但是缓存的结构发生了变化，因此对应做了调整（包括传入参数的意义）。
 * 存放在 UserTable 中的 utmp 记录还是从 1 开始索引，0 用于表示记录不存在。
 * @param uid 用户编号（从1开始索引）
 * @param utmp_idx utmp 索引（从0开始）
 */
void ythtbbs_cache_UserTable_add_utmp_idx(int uid, int utmp_idx);

/**
 * @brief 从 UserTable 中移除 utmp 记录
 * 参考了 src/bbs/bcache.c::remove_uindex 的实现。
 * @param uid 用户编号（从1开始索引）
 * @param utmp_idx utmp 索引（从0开始）
 */
void ythtbbs_cache_UserTable_remove_utmp_idx(int uid, int utmp_idx);

/**
 * @brief 输出错误并退出
 * @param key SHM 键
 */
static inline void shm_err(key_t key) {
	char buf[64];

	sprintf(buf, "SHM Error! key = %d.\n", key);
	newtrace(buf);
	exit(1);
}

