#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "ythtbbs/misc.h"

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

