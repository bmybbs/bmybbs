#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "config.h"
#include "ytht/shmop.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/misc.h"
#include "ythtbbs/record.h"
#include "ythtbbs/user.h"

static struct UTMPFILE                       *shm_utmp;
static struct ythtbbs_cache_userid_hashtable *shm_userid_hashtable;
static struct ythtbbs_cache_user_table       *shm_user_table;

/**
 * @brief 输出错误并退出
 * @param key SHM 键
 */
static void shm_err(key_t key);

static int fill_ucache_v(void *user_ec, va_list ap);
static int ythtbbs_cache_resolve_user_hash();
static int ythtbbs_cache_userid_hashtable_insert(struct ythtbbs_cache_userid_hashitem *ptr_items, size_t size, char *userid, int idx);

unsigned int ythtbbs_cache_hash_userid(char *id) {
	unsigned int n1 = 0;
	unsigned int n2 = 0;
	const unsigned int _HASH_SIZE = 26;

	while (*id) {
		n1 += ((unsigned int) toupper(*id)) % _HASH_SIZE;
		id++;
		if (!*id) break;

		n2 += ((unsigned int ) toupper(*id)) % _HASH_SIZE;
		id++;
	}

	n1 %= _HASH_SIZE;
	n2 %= _HASH_SIZE;
	return n1 * _HASH_SIZE + n2;
}

void ythtbbs_cache_resolve_utmp() {
	if (shm_utmp == NULL) {
		shm_utmp = get_shm(UTMP_SHMKEY, sizeof(*shm_utmp));
		if (shm_utmp == NULL) {
			shm_err(UTMP_SHMKEY);
		}
	}
}

void ythtbbs_cache_resolve_user() {
	int fd;
	struct stat st;
	static volatile time_t lasttime = -1;
	time_t local_now;
	char   local_buf[100];
	int    local_usernumber;

	fd = open(MY_BBS_HOME "/.CACHE.user.lock", O_RDWR | O_CREAT, 0660);
	flock(fd, LOCK_EX);

	time(&local_now);
	if (local_now - lasttime < 10) {
		// do not update if already updated in the last 10 seconds
		close(fd);
		return;
	}

	lasttime = local_now;

	if (shm_user_table == NULL) {
		shm_user_table = get_shm(UCACHE_SHMKEY, sizeof(*shm_user_table));
		if (shm_user_table == NULL)
			shm_err(UCACHE_SHMKEY);
	}

	if (stat(FLUSH, &st) < 0) {
		st.st_mtime++;
	}

	if (shm_user_table->update_time < st.st_mtime) {
		local_usernumber = 0;
		ythtbbs_record_apply_v(PASSFILE, fill_ucache_v, sizeof(struct userec), &local_usernumber);

		shm_user_table->number = local_usernumber;
		shm_user_table->update_time = st.st_mtime;
		shm_user_table->usersum = 0; // TODO;

		sprintf(local_buf, "system reload ucache %d", shm_user_table->usersum);
		// TODO detach shm and reattach?
	}
	close(fd);

	ythtbbs_cache_resolve_user_hash();
}

static void shm_err(key_t key) {
	char buf[64];

	sprintf(buf, "SHM Error! key = %d.\n", key);
	newtrace(buf);
	exit(1);
}

static int fill_ucache_v(void *user_ec, va_list ap) {
	int           *ptr_local_usernumber;
	struct userec *ptr;

	ptr_local_usernumber = va_arg(ap, int *);
	ptr = (struct userec *)user_ec;
	if (*ptr_local_usernumber < MAXUSERS) {
		strncpy(shm_user_table->users[*ptr_local_usernumber].userid, ptr->userid, IDLEN+1);
		shm_user_table->users[*ptr_local_usernumber].userid[IDLEN] = '\0';
		memset(shm_user_table->users[*ptr_local_usernumber].utmp_indices, 0, MAX_LOGIN_PER_USER);
		(*ptr_local_usernumber)++;
	}
	return 0;
}

static int ythtbbs_cache_resolve_user_hash() {
	int fd, i;
	char local_buf[64];

	fd = open(MY_BBS_HOME "/.CACHE.hash.lock", O_RDWR | O_CREAT, 0660);
	flock(fd, LOCK_EX);
	if (shm_userid_hashtable == NULL) {
		shm_userid_hashtable = get_shm(UCACHE_HASH_SHMKEY, sizeof(*shm_userid_hashtable));
		if (shm_userid_hashtable == NULL) {
			shm_err(UCACHE_HASH_SHMKEY);
		}
	}

	if (shm_userid_hashtable->update_time < shm_user_table->update_time) {
		shm_userid_hashtable->update_time = shm_user_table->update_time;

		for(i = 0; i < shm_user_table->number; i++) {
			ythtbbs_cache_userid_hashtable_insert(shm_userid_hashtable->items, UCACHE_HASH_SIZE, shm_user_table->users[i].userid, i);
		}

		sprintf(local_buf, "system reload shm_userid_hashtable %d", shm_user_table->number);
		newtrace(local_buf);

		// TODO detach and reattach?
	}

	close(fd);
	return 0;
}

static int ythtbbs_cache_userid_hashtable_insert(struct ythtbbs_cache_userid_hashitem *ptr_items, size_t size, char *userid, int idx) {
	unsigned int h, s, i, j = 0;
	if (!*userid)
		return -1;

	h = ythtbbs_cache_hash_userid(userid);
	s = size / 26 / 26;
	i = h * s;

	// 找到第一个可用的位置，最多跨越 5 块区域
	while ((j < s * 5) && ptr_items[i].user_num > 0 && ptr_items[i].user_num != idx + 1) {
		j++;
		i++;
		if (i >= size)
			i %= size;
	}

	if (j == s * 5) {
		// 如果跨越 5 块区域都没有找到可用的位置，则 hash 失败
		return -1;
	}

	ptr_items[i].user_num = idx + 1;
	strcpy(ptr_items[i].userid, userid);
	return 0;
}

