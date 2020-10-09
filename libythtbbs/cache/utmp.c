#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include "ytht/shmop.h"
#include "bmy/iphash.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/session.h"
#include "cache-internal.h"

// 用于 iphash
#define NHASH 67
// 最长发呆时间，3天
#define MAX_IDEL_TIME (3 * 24 * 3600)
// 最长会话时间，7天强制登出
#define MAX_SESS_TIME (7 * 24 * 3600)

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
	char              local_buf[64];

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

	ythtbbs_session_generate_id(ptr_user_info->sessionid, 40 /* defined in ythtbbs/cache.h */);
	memcpy(ptr_utmp_entry, ptr_user_info, sizeof(struct user_info));
	ythtbbs_cache_UserTable_add_utmp_idx(ptr_user_info->uid, j);
	ythtbbs_session_set(ptr_utmp_entry->sessionid, ptr_utmp_entry->userid, j);

	time(&local_now);
	if (local_now > shm_utmp->uptime + 60) {
		num[0] = num[1] = 0;
		shm_utmp->uptime = local_now;

		for (n = 0; n < USHM_SIZE; n++) {
			ptr_utmp_entry = &(shm_utmp->uinfo[n]);
			if (ptr_utmp_entry->pid == 1) {
				// 对于来自 nju09 和 api 的会话清理
				if (ptr_utmp_entry->active && (local_now - ptr_utmp_entry->lasttime > MAX_IDEL_TIME || local_now - ptr_utmp_entry->wwwinfo.login_start_time > MAX_SESS_TIME || ptr_utmp_entry->wwwinfo.iskicked)) {
					sprintf(local_buf, "%s drop www/api", ptr_utmp_entry->userid);
					newtrace(local_buf);
					ythtbbs_cache_UserTable_remove_utmp_idx(ptr_utmp_entry->uid, n);
					ythtbbs_session_del(ptr_utmp_entry->sessionid);
					memset(ptr_utmp_entry, 0, sizeof(struct user_info));
				}
			} else {
				// 对于来自 telnet 和 ssh 的会话清理
				if (!ptr_utmp_entry->active || local_now - ptr_utmp_entry->lasttime < 120) {
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

int ythtbbs_cache_utmp_get_ave_score() {
	return shm_utmp->ave_score;
}

void ythtbbs_cache_utmp_set_ave_score(int value) {
	shm_utmp->ave_score = value;
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

