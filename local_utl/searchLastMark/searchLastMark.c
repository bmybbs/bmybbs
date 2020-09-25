//查找版面最靠后的mark文章      ylsdd 2002/4/3
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "config.h"
#include "ytht/fileop.h"
#include "ytht/strlib.h"
#include "ythtbbs/article.h"
#include "ythtbbs/board.h"

#define MAXFOUNDD 9
#define MAXAUTHOR 5
struct markeditem {
	char title[100];
	int n;
	char author[MAXAUTHOR][16];
	int time;		//used for yc
	int thread;
};

struct markedlist {
	int n;
	struct markeditem mi[MAXFOUNDD];
};

int
addmiauthor(struct markeditem *mi, char *author)
{
	int i;
	for (i = 0; i < mi->n; i++) {
		if (!strncmp(mi->author[i], author, sizeof (mi->author[i]))) {
			return mi->n;
		}
	}
	if (i >= MAXAUTHOR)
		return -1;
	strsncpy(mi->author[i], author, sizeof (mi->author[i]));
	mi->n++;
	return mi->n;
}

int
addmarkedlist(struct markedlist *ml, char *title, char *author, int thread)
{
	int i;
	for (i = 0; i < ml->n; i++) {
		if (!strncmp(ml->mi[i].title, title, sizeof (ml->mi[i].title))) {
			addmiauthor(&ml->mi[i], author);
			return ml->n;
		}
	}
	if (i >= MAXFOUNDD)
		return ml->n;
	strsncpy(ml->mi[i].title, title, sizeof (ml->mi[0].title));
	addmiauthor(&ml->mi[i], author);
	ml->mi[i].thread = thread;
	ml->n++;
	return ml->n;
}

int
searchLastMark(char *filename, struct markedlist *ml, int addscore)
{
	struct fileheader fhdr;
	int fd, total, n, old = 0;
	time_t now;

	bzero(ml, sizeof (*ml));

	if ((total = file_size(filename) / sizeof (fhdr)) <= 0)
		return 0;

	time(&now);
	if ((fd = open(filename, O_RDONLY, 0)) == -1) {
		return 0;
	}
	for (n = total - 1; n >= 0 && total - n < 500; n--) {
		if (lseek(fd, n * sizeof (fhdr), SEEK_SET) < 0)
			break;
		if (read(fd, &fhdr, sizeof (fhdr)) != sizeof (fhdr))
			break;
		if (now - fhdr.filetime > 3600 * 24 * 6) {
			old++;
			if (old > 4)
				break;
			continue;
		}
		if (fhdr.owner[0] == '-' || !strcmp(fhdr.owner, "deliver")
		    || strstr(fhdr.title, "[警告]"))
			continue;
		if(fhdr.sizebyte > 20)
		if ((fhdr.accessed & FH_DIGEST)
		    || (fhdr.accessed & FH_MARKED)
		    || (fhdr.hasvoted > 1 + addscore
			&& fhdr.staravg50 * (int) fhdr.hasvoted / 50 > 4 + addscore * 2)) {
			if (addmarkedlist
			    (ml, fhdr.title, fh2owner(&fhdr),
			     fhdr.thread) >= MAXFOUNDD)
				break;
		}
	}
	close(fd);
	return ml->n;
}

int
main()
{
	int b_fd, i;
	struct boardheader bh;
	int size, foundd;
	char buf[200], recfile[200];
	struct markedlist ml;

	size = sizeof (bh);

	chdir(MY_BBS_HOME);
	if ((b_fd = open(MY_BBS_HOME "/.BOARDS", O_RDONLY)) == -1)
		return -1;
	while (read(b_fd, &bh, size) == size) {
		if (!bh.filename[0])
			continue;
		sprintf(buf, MY_BBS_HOME "/boards/%s/.DIR", bh.filename);
		sprintf(recfile, MY_BBS_HOME "/wwwtmp/lastmark/%s",
			bh.filename);
		if (file_time(buf) < file_time(recfile))
			continue;
		searchLastMark(buf, &ml, 1);
		if ((foundd = ml.n) > 0) {
			FILE *fp = fopen(recfile, "w");
			while (foundd > 0) {
				foundd--;
				i = ml.mi[foundd].n;
				while (i > 0) {
					i--;
					fprintf(fp, "%s",
						ml.mi[foundd].author[i]);
					if (i)
						fprintf(fp, " ");
				}
				fprintf(fp, "\t%d", ml.mi[foundd].thread);
				fprintf(fp, "\t%s\n", ml.mi[foundd].title);
			}
			fclose(fp);
		} else
			unlink(recfile);
	}
	close(b_fd);
	return 0;
}
