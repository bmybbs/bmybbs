#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include "ythtbbs/override.h"
#include "ythtbbs/user.h"
#include "ythtbbs/record.h"

static const char *FRIENDS_FILE = "friends";
static const char *REJECTS_FILE = "rejects";
static const char *FRIENDS_FILE_LOCK = "friends.lock";
static const char *REJECTS_FILE_LOCK = "rejects.lock";

static int ythtbbs_override_callback_cmp_userid(void *a, void *b);
int ythtbbs_override_lock(const char *userid, const enum ythtbbs_override_type override_type) {
	char buf[128];
	int lockfd;

	sethomefile_s(buf, sizeof(buf), userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? FRIENDS_FILE_LOCK : REJECTS_FILE_LOCK);
	lockfd = open(buf, O_RDWR | O_CREAT, 0600);
	flock(lockfd, LOCK_EX);

	return lockfd;
}

void ythtbbs_override_unlock(int lockfd) {
	close(lockfd);
}

int ythtbbs_override_add(const char *userid, const struct ythtbbs_override *ptr_override, const enum ythtbbs_override_type override_type) {
	char buf[128];

	sethomefile_s(buf, sizeof(buf), userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? FRIENDS_FILE : REJECTS_FILE);
	return append_record(buf, ptr_override, sizeof(struct ythtbbs_override));
}

int ythtbbs_override_count(const char *userid, const enum ythtbbs_override_type override_type) {
	char buf[128];

	sethomefile_s(buf, sizeof(buf), userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? FRIENDS_FILE : REJECTS_FILE);
	return ythtbbs_record_count_records(buf, sizeof(struct ythtbbs_override));
}

int ythtbbs_override_included(const char *userid, const enum ythtbbs_override_type override_type, const char *search_id) {
	char buf[128];
	struct ythtbbs_override tmp_override_buf;

	sethomefile_s(buf, sizeof(buf), userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? FRIENDS_FILE : REJECTS_FILE);
	return search_record(buf, &tmp_override_buf, sizeof(struct ythtbbs_override), ythtbbs_override_callback_cmp_userid, (void *) search_id);
}

long ythtbbs_override_get_records(const char *userid, struct ythtbbs_override *array, const size_t count, const enum ythtbbs_override_type override_type) {
	char buf[128];

	sethomefile_s(buf, sizeof(buf), userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? FRIENDS_FILE : REJECTS_FILE);
	return ythtbbs_record_get_records(buf, array, sizeof(struct ythtbbs_override), 1, count);
}

void ythtbbs_override_set_records(const char *userid, const struct ythtbbs_override *array, const size_t count, const enum ythtbbs_override_type override_type) {
	char buf[128];
	FILE *fp;
	unsigned int i, j;

	sethomefile_s(buf, sizeof(buf), userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? FRIENDS_FILE : REJECTS_FILE);

	fp = fopen(buf, "w+");
	for(i = 0, j = 0; j < count; ++i) {
		if(array[i].id[0]) {
			fwrite(&(array[i]), sizeof(struct ythtbbs_override), 1, fp);
			j++;
		}
	}
	ftruncate(fileno(fp), count * sizeof(struct ythtbbs_override)); // 截断
	fclose(fp);
}

static int ythtbbs_override_callback_cmp_userid(void *a, void *b) {
	const char *userid = (const char *)a;
	const struct ythtbbs_override *ptr_override = (const struct ythtbbs_override *)b;
	return !strcmp(userid, ptr_override->id);
}

