#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "config.h"
#include "ythtbbs/article.h"
#include "bbslib.h"

static int
getlastpost(char *board, int *lastpost, int *total)
{
	struct fileheader fh;
	struct stat st;
	char filename[STRLEN * 2];
	int fd, atotal;

	sprintf(filename, "%s/boards/%s/.DIR", BBSHOME, board);
	if ((fd = open(filename, O_RDWR)) < 0)
		return -1;
	fstat(fd, &st);
	atotal = st.st_size / sizeof (fh);
	if (*total <= 0) {
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

int
updatelastpost(char *board)
{
	struct boardmem *bptr;
	if (shm_bcache == NULL)
		shm_init();
	bptr = ythtbbs_cache_Board_get_board_by_name(board);
	if (bptr == NULL)
		return -1;
	return getlastpost(bptr->header.filename, &bptr->lastpost, &bptr->total);
}
