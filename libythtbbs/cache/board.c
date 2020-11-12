#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "ytht/common.h"
#include "ytht/shmop.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/article.h"
#include "ythtbbs/record.h"
#include "cache-internal.h"

static struct BCACHE *shm_board;

static int fillbcache(void *, void *);
static void bmonlinesync(void);

void ythtbbs_cache_Board_resolve() {
	struct stat st;
	time_t local_now, local_now2;
	char local_buf[80];
	int lockfd, countboard;

	if (shm_board == NULL) {
		shm_board = get_shm(BCACHE_SHMKEY, sizeof(*shm_board));
		if (shm_board == NULL) {
			shm_err(BCACHE_SHMKEY);
		}
	}

	lockfd = open(MY_BBS_HOME "./CACHE.board.lock", O_RDONLY | O_CREAT, 0600);
	if (lockfd < 0)
		return;
	flock(lockfd, LOCK_EX);

	time(&local_now);
	if (stat(BOARDS, &st) < 0) {
		errlog("BOARDS stat error");
		st.st_mtime = local_now - 3600;
	}

	if (shm_board->uptime <= st.st_mtime || shm_board->uptime < local_now - 3600) {
		shm_board->uptime = local_now;
		countboard = 0;

		new_apply_record(BOARDS, sizeof(struct boardheader), fillbcache, &countboard);

		shm_board->number = countboard;

		snprintf(local_buf, sizeof(local_buf), "system reload bcache %d", shm_board->number);
		newtrace(local_buf);

		bmonlinesync();

		time(&local_now2);
		if (stat(BOARDS, &st) >= 0 && st.st_mtime < local_now)
			shm_board->uptime = local_now2;
	}
	close(lockfd);
}

struct boardmem *ythtbbs_cache_Board_get_bcache() {
	return shm_board->bcache;
}

int ythtbbs_cache_Board_set_bm_hat_v(void *b, va_list ap) {
	struct boardmanager *bm = b;
	bool *online, *invisible;

	if (shm_board == NULL)
		return 0;

	online    = va_arg(ap, bool *);
	invisible = va_arg(ap, bool *);

	if (strcmp(shm_board->bcache[bm->bid].header.filename, bm->board)) {
		errlog("error board name %s, %s", shm_board->bcache[bm->bid].header.filename, bm->board);
		return -1;
	}
	if (*online) {
		shm_board->bcache[bm->bid].bmonline |= (1 << bm->bmpos);
		if (*invisible)
			shm_board->bcache[bm->bid].bmcloak |= (1 << bm->bmpos);
		else
			shm_board->bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	} else {
		shm_board->bcache[bm->bid].bmonline &= ~(1 << bm->bmpos);
		shm_board->bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	}
	return 0;
}

/***** implementations of private functions *****/
static int getlastpost(char *board, int *lastpost, int *total) {
	struct fileheader fh;
	struct stat st;
	char filename[STRLEN * 2];
	int fd, atotal;

	snprintf(filename, sizeof (filename), MY_BBS_HOME "/boards/%s/.DIR", board);
	if ((fd = open(filename, O_RDONLY)) < 0)
		return 0;
	fstat(fd, &st);
	atotal = st.st_size / sizeof (fh);
	if (atotal <= 0) {
		*lastpost = 0;
		*total = 0;
		close(fd);
		return 0;
	}
	*total = atotal;
	lseek(fd, (atotal - 1) * sizeof (fh), SEEK_SET);
	if (read(fd, &fh, sizeof (fh)) > 0) {
		if (fh.edittime == 0)
			*lastpost = fh.filetime;
		else
			*lastpost = fh.edittime;
	}
	close(fd);
	return 0;
}

static int fillbcache(void *a, void *b) {
	struct boardheader *fptr = (struct boardheader *)a;
	int *pcountboard = (int *)b;
	struct boardmem *bptr;

	if (*pcountboard >= MAXBOARD)
		return 0;
	bptr = &(shm_board->bcache[*pcountboard]);
	(*pcountboard)++;
	memcpy(&(bptr->header), fptr, sizeof (struct boardheader));
	getlastpost(bptr->header.filename, &bptr->lastpost, &bptr->total);
	return 0;
}

static void bmonlinesync() {
	int j, k;
	for (j = 0; j < shm_board->number; j++) {
		if (!shm_board->bcache[j].header.filename[0])
			continue;
		shm_board->bcache[j].bmonline = 0;
		shm_board->bcache[j].bmcloak = 0;
		for (k = 0; k < BMNUM; k++) {
			if (shm_board->bcache[j].header.bm[k][0] == 0) {
				if (k < 4) {
					k = 3;      // 继续检查小版主
					continue;
				}
				break;          // 小版主也检查完了
			}
			if (ythtbbs_cache_UserTable_is_user_online(shm_board->bcache[j].header.bm[k]))
				shm_board->bcache[j].bmonline |= (1 << k);
			if (ythtbbs_cache_UserTable_is_user_invisible(shm_board->bcache[j].header.bm[k]))
				shm_board->bcache[j].bmcloak  |= (1 << k);
		}
	}

	newtrace("system reload bmonline");
}

