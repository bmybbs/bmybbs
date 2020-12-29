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

char *ythtbbs_board_set_board_file(char *buf, size_t len, const char *boardname, const char *filename) {
	snprintf(buf, len, MY_BBS_HOME "/boards/%s/%s", boardname, filename);
	return buf;
}

static enum YTHTBBS_BOARD_INFO_STATUS ythtbbs_board_load_internal(char *buf, size_t len, const char *filename) {
	struct mmapfile mf;

	memset(&mf, 0, sizeof(mf));

	if (mmapfile(filename, &mf) == -1)
		return YTHTBBS_BOARD_INFO_NOT_FOUND;

	if (mf.size == 0) {
		mmapfile(NULL, &mf);
		return YTHTBBS_BOARD_INFO_NOT_FOUND;
	}

	strncpy(buf, mf.ptr, len);
	mmapfile(NULL, &mf);
	return YTHTBBS_BOARD_INFO_FOUND;
}

enum YTHTBBS_BOARD_INFO_STATUS ythtbbs_board_load_intro(char *buf, size_t len, const struct boardheader *bh) {
	char filename[128];
	FILE *fp;

	ythtbbs_board_set_board_file(filename, sizeof(filename), bh->filename, "introduction");
	return ythtbbs_board_load_internal(buf, len, filename);
}

enum YTHTBBS_BOARD_INFO_STATUS ythtbbs_board_load_note(char *buf, size_t len, const struct boardheader *bh) {
	char filename[128];
	snprintf(filename, sizeof(filename), MY_BBS_HOME "/vote/%s/notes", bh->filename);
	return ythtbbs_board_load_internal(buf, len, filename);
}

