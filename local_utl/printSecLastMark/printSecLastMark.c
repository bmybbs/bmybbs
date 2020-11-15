#include <stdio.h>
#include "bbs.h"
#include "www.h"
#define MAXL 50
#define MAXNLINE  20
#define MAXLNRAND 4
#define MAXFILTER 100

int makeallseclastmark(const struct sectree *sec);
void makeseclastmark(const struct sectree *sec);
int printseclastmark(FILE * fp, const struct sectree *sec, int nline);

struct lastmark {
	char title[100];
	char board[30];
	char boardtitle[80];
	int thread;
} lastmarklist[MAXBOARD];
int numlastmark;
struct {
	int score[MAXL];
	struct lastmark lastmark[MAXL];
	int score2[MAXL];
	struct lastmark lastmark2[MAXL];
	int n;
	int n2;
} seclastmark;

char filterstr[MAXFILTER][60];
int nfilterstr = 0;

struct BCACHE *shm_bcache;

int
initfilter(char *filename)
{
	FILE *fp;
	char *ptr, *p;
	fp = fopen(filename, "r");
	if (!fp)
		return -1;
	for (nfilterstr = 0; nfilterstr < MAXFILTER; nfilterstr++) {
		ptr = filterstr[nfilterstr];
		if (fgets(filterstr[nfilterstr], sizeof (filterstr[nfilterstr]), fp) == NULL)
			break;
		if ((p = strchr(ptr, '\n')) != NULL)
			*p = 0;
		if ((p = strchr(ptr, '\r')) != NULL)
			*p = 0;
		while (ptr[0] == ' ')
			memmove(&ptr[0], &ptr[1], strlen(ptr));
		while (ptr[0] != 0 && ptr[strlen(ptr) - 1] == ' ')
			ptr[strlen(ptr) - 1] = 0;
		if (!ptr[0])
			nfilterstr--;
	}
	fclose(fp);
	return 0;
}

int
dofilter(char *str)
{
	int i;
	if (!nfilterstr)
		return 0;
	for (i = 0; i < nfilterstr; i++) {
		if (strstr(str, filterstr[i]))
			break;
	}
	if (i < nfilterstr)
		return 1;
	return 0;
}

int
readlastmark(char *board, int *thread, char *title)
{
	char buf[200], *ptr;
	FILE *fp;
	int found = 0;
	sprintf(buf, MY_BBS_HOME "/wwwtmp/lastmark/%s", board);
	*thread = 0;
	if (!file_exist(buf))
		goto END;
	if ((fp = fopen(buf, "r")) == NULL)
		goto END;
	while (fgets(buf, 200, fp)) {
		if (dofilter(buf))
			continue;
		ptr = strchr(buf, '\t');
		if (ptr == NULL)
			break;
		ptr++;
		*thread = atoi(ptr);
		if (*thread == 0)
			break;
		ptr = strchr(ptr, '\t');
		if (!ptr)
			break;
		ptr++;
		strcpy(title, ptr);
		ptr = strchr(title, '\n');
		if (ptr)
			*ptr = 0;
		found = 1;
	}
	fclose(fp);
END:
	return found;
}

/* select n different numbers from {x | x>=0, x<max} */
void
makerand(int w[], int n, int max)
{
	int i, j, l;
	if (n > max) {
		printf("Warning! makerand: n>max!\n");
		exit(0);
	}
	// i: we have selected i different numbers
	for (i = 0; i < n; i++) {
		l = random() % max;
		// check the nuw random number l: is l different from the i selected numbers?
		for (j = 0; j < i; j++)
			if (w[j] == l)
				break;
		if (j < i) {
			i--;
			continue;
		}
		w[i] = l;
	}
}

int
testperm(struct boardmem *x)
{
	if (!x->header.filename[0])
		return 0;
	if (x->header.clubnum != 0) {
		if ((x->header.flag & CLUBTYPE_FLAG))
			return 1;
		return 0;
	}
	if (x->header.level == 0)
		return 1;
	if (x->header.level & (PERM_POSTMASK | PERM_NOZAP))
		return 1;
	return 0;
}

int
main()
{
	int i, j;
	struct boardmem x;
	initfilter(MY_BBS_HOME "/etc/filtertitle");
	srandom(time(NULL));
	shm_bcache = (struct BCACHE *) get_old_shm(BCACHE_SHMKEY, sizeof (struct BCACHE));
	for (i = 0; i < shm_bcache->number; i++) {
		x = shm_bcache->bcache[i];
		lastmarklist[i].thread = 0;
		if (!testperm(&x))
			continue;
		strcpy(lastmarklist[i].board, x.header.filename);
		strcpy(lastmarklist[i].boardtitle, x.header.title);
		readlastmark(x.header.filename, &lastmarklist[i].thread, lastmarklist[i].title);
	}
	numlastmark = i;
	makeallseclastmark(&sectree);
}

int
makeallseclastmark(const struct sectree *sec)
{
	int i, n, nline, separate;
	const struct sectree *subsec;
	char buf[200], tmp[200];
	FILE *fp;
	if (!sec->introstr[0])
		return -1;
	sprintf(tmp, "%s/wwwtmp/lastmark.sec%s.new", MY_BBS_HOME, sec->basestr);
	fp = fopen(tmp, "w");
	if (!fp)
		return -1;
	fprintf(fp, "<table><tr><td valign=top width=49%%>\n");
	n = strlen(sec->introstr);
	if(strchr(sec->introstr, '|'))
		separate = strchr(sec->introstr, '|') - sec->introstr;
	else
		separate = n/2;
	nline = 8;
	if (n <= 4||sec->basestr[0])
		nline =11;
	for (i = 0; i < n; i++) {
		if(i==separate)
			fprintf(fp, "</td><td>&nbsp;</td><td valign=top width=49%%>\n");
		if(sec->introstr[i]=='|')
			continue;
		sprintf(buf, "%s%c", sec->basestr, sec->introstr[i]);
		subsec = getsectree(buf);
		if (!subsec)
			continue;
		makeseclastmark(subsec);
		printseclastmark(fp, subsec, nline);
	}
	fprintf(fp, "</td></tr></table>\n");
	fclose(fp);
	sprintf(buf, "%s/wwwtmp/lastmark.sec%s", MY_BBS_HOME, sec->basestr);
	rename(tmp, buf);
	for (i = 0; i < sec->nsubsec; i++)
		makeallseclastmark(sec->subsec[i]);
	return 0;
}

void
makeseclastmark(const struct sectree *sec)
{
	int i, len, j, k;
	struct boardmem x;
	len = strlen(sec->basestr);
	bzero(&seclastmark, sizeof (seclastmark));
	for (i = 0; i < numlastmark; i++) {
		x = shm_bcache->bcache[i];
		if(!testperm(&x))
			continue;
		if (strncmp(sec->basestr, x.header.sec1, len) && strncmp(sec->basestr, x.header.sec2, len))
			continue;
		if (lastmarklist[i].thread) {
			for (j = 0; j < seclastmark.n; j++) {
				if (x.score > seclastmark.score[j])
					break;
			}
			if (j == MAXL)
				continue;
			if (seclastmark.n == MAXL)
				seclastmark.n--;
			for (k = seclastmark.n; k > j; k--) {
				seclastmark.score[k] = seclastmark.score[k - 1];
				seclastmark.lastmark[k] = seclastmark.lastmark[k - 1];
			}
			seclastmark.lastmark[j] = lastmarklist[i];
			seclastmark.score[j] = x.score;
			seclastmark.n++;
		} else {
			for (j = 0; j < seclastmark.n2; j++) {
				if (x.score > seclastmark.score2[j])
					break;
			}
			if (j == MAXL)
				continue;
			if (seclastmark.n2 == MAXL)
				seclastmark.n2--;
			for (k = seclastmark.n2; k > j; k--) {
				seclastmark.score2[k] = seclastmark.score2[k - 1];
				seclastmark.lastmark2[k] = seclastmark.lastmark2[k - 1];
			}
			seclastmark.lastmark2[j] = lastmarklist[i];
			seclastmark.score2[j] = x.score;
			seclastmark.n2++;

		}
	}
}

int
printlastmarkline(FILE * fp, struct lastmark *lm)
{
	char *title = lm->title;
	fprintf(fp, "<tr><td valign=top>¡¤</td>"
		"<td><a href='tfind?B=%s&th=%d&T=%s'>",
		lm->board, lm->thread, encode_url(title));
	if (!strncmp(title, "[×ªÔØ] ", 7) && strlen(title) > 20)
		title += 7;
	if (strlen(title) > 45)
		title[45] = 0;
	fprintf(fp, "%s</a> ", void1(nohtml(title)));
	fprintf(fp, "&lt;<a href='home?B=%s' class=blk>%s</a>&gt;</td></tr>\n",
		lm->board, nohtml(lm->boardtitle));
	return 0;
}

int
printseclastmark(FILE * fp, const struct sectree *sec, int nline)
{
	int j, k;
	int r[MAXNLINE];
	int count = 0;

	//fprintf(fp, "#%s %s\n", sec->basestr, nohtml(void1(sec->title)));
	fprintf(fp, "<table cellpadding=2 cellspacing=0 border=0 width=100%%>"
		"<tr class=tb2_blk><td width=15><font class=star>¡ï</font></td>"
		"<td><a href=boa?secstr=%s class=blk><B>%s</B></td></tr>\n",
		sec->basestr, nohtml(sec->title));
	if (nline > MAXNLINE)
		nline = MAXNLINE;
	if (nline < MAXLNRAND)
		nline = MAXLNRAND;
	if (seclastmark.n <= nline) {
		for (j = 0; j < seclastmark.n; j++) {
			count++;
			printlastmarkline(fp, &seclastmark.lastmark[j]);
		}
		goto END;
	}
	for (j = 0; j < MAXLNRAND; j++) {
		count++;
		printlastmarkline(fp, &seclastmark.lastmark[j]);
	}
	makerand(r, nline - MAXLNRAND, seclastmark.n - MAXLNRAND);
	for (k = 0; k < nline - MAXLNRAND; k++) {
		j = r[k] + MAXLNRAND;
		count++;
		printlastmarkline(fp, &seclastmark.lastmark[j]);
	}
END:
	if (count < MAXLNRAND && seclastmark.n2) {
		makerand(r, min(seclastmark.n2, 3), seclastmark.n2);
		fprintf(fp, "<tr><td colspan=2>");
		for (k = 0; k < 3 && k < seclastmark.n2; k++)
			fprintf(fp,
				"&lt;<a href='home?B=%s' class=blk>%s</a>&gt; ",
				seclastmark.lastmark2[r[k]].board,
				nohtml(seclastmark.lastmark2[r[k]].boardtitle));
		fprintf(fp, "¡­</td></tr>\n");
		count++;
	}
	if (!count)
		fprintf(fp, "<tr><td colspan=2>&nbsp;</td></tr>\n");
	fprintf(fp, "</table>\n");
	return 0;
}

