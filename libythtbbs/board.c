#include "ythtbbs.h"
#include "bbs.h"

char *
bm2str(buf, bh)
char *buf;
struct boardheader *bh;
{
	int i;
	buf[0] = 0;
	for (i = 0; i < 4; i++)
		if (bh->bm[i][0] == 0)
			break;
		else {
			if (i != 0)
				strcat(buf, " ");
			strcat(buf, bh->bm[i]);
		}
	return buf;
}

char *
sbm2str(buf, bh)
char *buf;
struct boardheader *bh;
{
	int i;
	buf[0] = 0;
	for (i = 4; i < BMNUM; i++)
		if (bh->bm[i][0] == 0)
			break;
		else {
			if (i != 0)
				strcat(buf, " ");
			strcat(buf, bh->bm[i]);
		}
	return buf;
}

int
chk_BM(struct userec *user, struct boardheader *bh, int isbig)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (bh->bm[i][0] == 0)
			break;
		if (!strcmp(bh->bm[i], user->userid)
		    && bh->hiretime[i] >= user->firstlogin)
			return i + 1;
	}
	if (isbig)
		return 0;
	for (i = 4; i < BMNUM; i++) {
		if (bh->bm[i][0] == 0)
			break;
		if (!strcmp(bh->bm[i], user->userid)
		    && bh->hiretime[i] >= user->firstlogin)
			return i + 1;
	}
	return 0;
}

int
chk_BM_id(char *user, struct boardheader *bh)
{
	int i;
	for (i = 0; i < BMNUM; i++) {
		if (bh->bm[i][0] == 0) {
			if (i < 4) {
				i = 3;
				continue;
			}
			break;
		}
		if (!strcmp(bh->bm[i], user))
			return i + 1;
	}
	return 0;
}

int
bmfilesync(struct userec *user)
{
	char path[256];
	struct myparam1 mp;
	sethomefile(path, user->userid, "mboard");
	if (file_time(path) > file_time(".BOARDS"))
		return 0;
	memcpy(&(mp.user), user, sizeof (struct userec));
	mp.fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);	//touch a new file
	if (mp.fd == -1) {
		errlog("touch new mboard error");
		return -1;
	}
	mp.bid = 0;
	new_apply_record(".BOARDS", sizeof (struct boardheader), (void *)fillmboard, &mp);
	close(mp.fd);
	return 0;
}

int
fillmboard(struct boardheader *bh, struct myparam1 *mp)
{
	struct boardmanager bm;
	int i;
	if ((i = chk_BM(&(mp->user), bh, 0))) {
		bzero(&bm, sizeof (bm));
		strncpy(bm.board, bh->filename, 24);
		bm.bmpos = i - 1;
		bm.bid = mp->bid;
		write(mp->fd, &bm, sizeof (bm));
	}
	(mp->bid)++;
	return 0;
}


struct boardmem * getboardbyname(const char *board_name) {
	int i;
	if (board_name[0] == 0)
		return NULL;

	struct BCACHE * shm_bcache = (struct BCACHE *) get_old_shm(BCACHE_SHMKEY, sizeof (struct BCACHE));
	// ¥” shm_bcache ÷–µ›πÈ≤È’“
	for (i=0; i<MAXBOARD && i<shm_bcache->number; ++i) {
		if(!strcasecmp(board_name, shm_bcache->bcache[i].header.filename))
			return &shm_bcache->bcache[i];
	}

	return NULL;
}

int board_is_junkboard(char *board_name)
{
	return seek_in_file("etc/junkboards", board_name);
}
