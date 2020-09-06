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
