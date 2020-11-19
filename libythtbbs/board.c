#include "ythtbbs/ythtbbs.h"
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
	return ythtbbs_cache_Board_get_board_by_name(board_name);
}

int board_is_junkboard(char *board_name)
{
	return seek_in_file("etc/junkboards", board_name);
}
