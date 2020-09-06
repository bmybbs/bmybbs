#include "bbs.h"
#include "stdio.h"
#include "ythtlib.h"
#include "ythtbbs.h"
#include <string.h>

char ignore[10]="Y";

int cmpboardt(const void *aa, const void *bb) {
	struct boardheader *a = (struct boardheader *)aa;
	struct boardheader *b = (struct boardheader *)bb;
	return (b->board_ctime - a->board_ctime);
}
int
main() {
	struct mmapfile mf = {ptr:NULL};
	struct boardheader *ptr, *ptr1;
	int i, size, j;
	FILE *fp;
	int now_t;
	now_t = time(NULL);
	chdir(MY_BBS_HOME);
	if (mmapfile(".BOARDS", &mf) < 0)
		return -1;
	size = mf.size;
	ptr = malloc(size);
	memcpy(ptr, mf.ptr, size);
	mmapfile(NULL, &mf);
	qsort(ptr, size/sizeof(struct boardheader), sizeof(struct boardheader), cmpboardt);
	fp = fopen("wwwtmp/newboards.tmp", "w");
	if (fp == NULL)
		return -1;
	fprintf(fp, "<b>ÐÂ¿ª°æÃæ</b><br>");
	fprintf(fp, "<table cellspacing=0 cellpadding=2>\n");
	i = 0;
	ptr1 = ptr;
	size /= sizeof(struct boardheader);
	for (j=0; j < size && i < 3; j++) {
		if ((!strchr(ignore, ptr1->sec1[0])) && (now_t - ptr1->board_ctime < 60*60*24*7) && (ptr1->level == 0) && (ptr1->clubnum == 0)) {
			i++;
			fprintf(fp, "<tr><td><a href=bbshome?board=%s>%s</a></td></tr>\n", ptr1->filename, ptr1->title);
		}
		ptr1++;
	}
	fprintf(fp,"</table>");
	fclose(fp);
	if (i) {
		rename("wwwtmp/newboards.tmp", "wwwtmp/newboards.txt");
	} else {
		unlink("wwwtmp/newboards.tmp");
	}
}

