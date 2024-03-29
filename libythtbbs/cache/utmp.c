#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include "ytht/shmop.h"
#include "ytht/random.h"
#include "bmy/iphash.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/session.h"
#include "ythtbbs/user.h"
#include "cache-internal.h"

// 用于 iphash
#define NHASH 67

struct UTMPFILE {
	struct user_info uinfo[USHM_SIZE];
	time_t uptime;
	unsigned short activeuser;
	unsigned short maxuser;	//add by gluon
	unsigned short maxtoday;
	unsigned short wwwguest;
	time_t activetime;	//time of updating activeuser
	int ave_score;
	int allprize;
	time_t watchman;
	unsigned int unlock;
	int nouse[5];
};

static struct UTMPFILE *shm_utmp;

// 不再统计 wwwguest，因为当前不再产生 wwwguest 会话
static int count_active(const struct user_info *uentp, void *);

void ythtbbs_cache_utmp_resolve() {
	if (shm_utmp == NULL) {
		shm_utmp = get_shm(UTMP_SHMKEY, sizeof(*shm_utmp));
		if (shm_utmp == NULL) {
			shm_err(UTMP_SHMKEY);
		}
	}
}

void ythtbbs_cache_utmp_update(int utmp_idx, const struct user_info *ptr_info) {
	// TODO lock
	ythtbbs_cache_UserTable_remove_utmp_idx(shm_utmp->uinfo[utmp_idx].uid, utmp_idx);
	memcpy(&shm_utmp->uinfo[utmp_idx], ptr_info, sizeof(struct user_info));
	// TODO unlock
}

int ythtbbs_cache_utmp_insert(struct user_info *ptr_user_info) {
	int               utmpfd;          /* 文件锁 */
	struct user_info *ptr_utmp_entry;  /* utmp 中的指针 */
	time_t            local_now;
	int               i, j, n, num[2];
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
		flock(utmpfd, LOCK_UN);
		close(utmpfd);
		return -1;
	}

	ythtbbs_session_generate_id(ptr_user_info->sessionid, 40 /* defined in ythtbbs/cache.h */);
	memcpy(ptr_utmp_entry, ptr_user_info, sizeof(struct user_info));
	if (ythtbbs_cache_UserTable_add_utmp_idx(ptr_user_info->uid, j, ptr_user_info->login_type) < 0) {
		// 如果插入失败，则撤销上一步的 memcpy
		memset(ptr_utmp_entry, 0, sizeof(struct user_info));
		snprintf(local_buf, sizeof(local_buf), "failed to insert UT for %s", ptr_user_info->userid);
		newtrace(local_buf);
		flock(utmpfd, LOCK_UN);
		close(utmpfd);
		return -1;
	}
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
					snprintf(local_buf, sizeof(local_buf), "%s drop www/api", ptr_utmp_entry->userid);
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
	flock(utmpfd, LOCK_UN);
	close(utmpfd);
	return j;
}

void ythtbbs_cache_utmp_remove(int utmp_idx) {
	struct user_info *ptr_info;

	if (utmp_idx < 0 || utmp_idx >= USHM_SIZE)
		return;

	ptr_info = &shm_utmp->uinfo[utmp_idx];
	if (!ptr_info->active)
		return;

	ythtbbs_cache_UserTable_remove_utmp_idx(ptr_info->uid, utmp_idx);
	ythtbbs_session_del(ptr_info->sessionid);
	memset(ptr_info, 0, sizeof(struct user_info));
}

int ythtbbs_cache_utmp_apply(ythtbbs_cache_utmp_apply_callback fptr, void *x_param) {
	int i;

	ythtbbs_cache_utmp_resolve();
	for (i = 0; i <= USHM_SIZE - 1; i++) {
		if (shm_utmp->uinfo[i].active == 0)
			continue;
		if ((*fptr) (&shm_utmp->uinfo[i], x_param) == QUIT)
			return QUIT;
	}
	return 0;
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

int ythtbbs_cache_utmp_count_active(void) {
	time_t now = time(NULL);
	ythtbbs_cache_utmp_resolve();
	if (now <= shm_utmp->activetime + 1)
		return shm_utmp->activeuser;
	shm_utmp->activetime = now;
	shm_utmp->activeuser = 0;
	ythtbbs_cache_utmp_apply(count_active, &(shm_utmp->activeuser));
	return shm_utmp->activeuser;
}

void ythtbbs_cache_utmp_increase_unreadmsg(const struct user_info *ptr_info) {
	unsigned long offset;
	if (ptr_info >= shm_utmp->uinfo) {
		offset = ((void *) ptr_info - (void *) shm_utmp->uinfo) / sizeof(struct user_info);
		if (offset < USHM_SIZE) {
			shm_utmp->uinfo[offset].unreadmsg++;
		}
	}
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

		fprintf(fp, "%d, %s, %s\n", i, info->userid, ythtbbs_user_get_login_type_str(info->login_type));
	}
}

struct user_info *ythtbbs_cache_utmp_get_by_idx(int idx) {
	return &shm_utmp->uinfo[idx];
}

static int count_active(const struct user_info *uentp, void *x_param) {
	int *p_i = x_param;
	if (!uentp->active || !uentp->pid)
		return 0;
	*p_i = *p_i + 1;
	return 1;
}

time_t ythtbbs_cache_utmp_get_watchman(void) {
	return shm_utmp->watchman;
}

void ythtbbs_cache_utmp_set_watchman(time_t t) {
	shm_utmp->watchman = t;
}

unsigned int ythtbbs_cache_utmp_get_unlock(void) {
	return shm_utmp->unlock;
}

void ythtbbs_cache_utmp_set_unlock(void) {
	ytht_get_random_int(&(shm_utmp->unlock));
}

unsigned short ythtbbs_cache_utmp_get_activeuser(void) {
	return shm_utmp->activeuser;
}

unsigned short ythtbbs_cache_utmp_get_maxtoday(void) {
	return shm_utmp->maxtoday;
}

void ythtbbs_cache_utmp_set_maxtoday(unsigned short m) {
	shm_utmp->maxtoday = m;
}

unsigned short ythtbbs_cache_utmp_get_maxuser(void) {
	return shm_utmp->maxuser;
}

void ythtbbs_cache_utmp_set_maxuser(unsigned short m) {
	shm_utmp->maxuser = m;
}

unsigned short ythtbbs_cache_utmp_get_wwwguest(void) {
	return shm_utmp->wwwguest;
}

void ythtbbs_cache_utmp_set_www_kicked(int utmp_idx) {
	// TODO check null ptr
	// TODO lock
	shm_utmp->uinfo[utmp_idx].wwwinfo.iskicked = 1;
}

