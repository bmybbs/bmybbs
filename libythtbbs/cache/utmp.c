#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include "ytht/shmop.h"
#include "bmy/iphash.h"
#include "ythtbbs/cache.h"
#include "cache-internal.h"

#define NHASH 67

static struct UTMPFILE *shm_utmp;

void ythtbbs_cache_utmp_resolve() {
	if (shm_utmp == NULL) {
		shm_utmp = get_shm(UTMP_SHMKEY, sizeof(*shm_utmp));
		if (shm_utmp == NULL) {
			shm_err(UTMP_SHMKEY);
		}
	}
}

int ythtbbs_cache_utmp_insert(struct user_info *ptr_user_info) {
	int               utmpfd;          /* 文件锁 */
	struct user_info *ptr_utmp_entry;  /* utmp 中的指针 */
	time_t            local_now;
	int               i, j, n, num[2];
	pid_t             pid;

	utmpfd = open(MY_BBS_HOME "/.CACHE.utmp.lock", O_RDWR | O_CREAT, 0600);
	if (utmpfd < 0)
		return -1;

	ythtbbs_cache_utmp_resolve();
	flock(utmpfd, LOCK_EX);

	for (j = bmy_iphash(ptr_user_info->from, NHASH) * (MAXACTIVE / NHASH), i = 0; i < USHM_SIZE; i++, j++) {
		if (j >= USHM_SIZE) j = 0;

		ptr_utmp_entry = &(shm_utmp->uinfo[j]);
		if (!ptr_utmp_entry->active || !ptr_utmp_entry->pid) {
			// TODO check
			break;
		}
	}

	if (j >= USHM_SIZE) {
		close(utmpfd);
		return -1;
	}

	memcpy(ptr_utmp_entry, ptr_user_info, sizeof(struct user_info));
	ythtbbs_cache_UserTable_add_utmp_idx(ptr_user_info->uid, j);

	time(&local_now);
	if (local_now > shm_utmp->uptime + 60) {
		num[0] = num[1] = 0;
		shm_utmp->uptime = local_now;

		for (n = 0; n < USHM_SIZE; n++) {
			ptr_utmp_entry = &(shm_utmp->uinfo[n]);
			if (!ptr_utmp_entry->active || ptr_utmp_entry->pid <= 1 || local_now - ptr_utmp_entry->lasttime < 120) {
				// TODO check
				continue;
			}

			// 向该进程发送空信号
			if (kill(ptr_utmp_entry->pid, 0) == -1) {
				// 该进程不存在
				ythtbbs_cache_UserTable_remove_utmp_idx(ptr_utmp_entry->uid, n);
				memset(ptr_utmp_entry, 0, sizeof(struct user_info));
			} else {
				num[(ptr_utmp_entry->invisible == true) ? 1 : 0]++; // TODO 尚未使用的在线/隐身人数统计
			}
		}
	}
	close(utmpfd);
	return j;
}

int ythtbbs_cache_utmp_check_active_by_idx(int idx) {
	return shm_utmp->uinfo[idx].active;
}

int ythtbbs_cache_utmp_check_uid_by_idx(int idx, int uid) {
	return shm_utmp->uinfo[idx].uid == uid;
}

/**
 * @brief 将 utmp 缓存中的用户信息序列化出来
 * 输出形式 utmp_idx, userid："0, SYSOP\n"
 * @warning 非公开接口
 */
void ythtbbs_cache_utmp_dump(FILE *fp) {
	int i;
	struct user_info *info;

	ythtbbs_cache_utmp_resolve();
	fprintf(fp, "===== UTMP =====\n");
	for (i = 0; i < USHM_SIZE; i++) {
		info = &(shm_utmp->uinfo[i]);
		if (info->userid[0] == '\0')
			continue;

		fprintf(fp, "%d, %s\n", i, info->userid);
	}
}

