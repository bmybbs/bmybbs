#include "bbs.h"

char * setbfile(char *buf, char *boardname, char *filename) {
	snprintf(buf, sizeof buf, MY_BBS_HOME "/boards/%s/%s", boardname, filename);
	return buf;
}

char * setbdir(char *buf, char *boardname) {
	char dir[STRLEN];

	strncpy(dir, DOT_DIR, STRLEN);
	dir[STRLEN - 1] = '\0';
	snprintf(buf, sizeof buf, MY_BBS_HOME "/boards/%s/%s", boardname, dir);
	return buf;
}

void getcross(char *filepath, char *filepath2, char *nboard, char *posttitle) {
	FILE *inf, *of;
	char buf[256];
	time_t now;

	now = time(0);
	inf = fopen(filepath2, "r");
	if (inf == NULL)
		return;
	of = fopen(filepath, "w");
	if (of == NULL) {
		fclose(inf);
		return;
	}

	fprintf(of, "发信人: XJTU-XANET (自动发信系统), 信区: %s\n", nboard);
	fprintf(of, "标  题: %s\n", posttitle);
	fprintf(of, "发信站: 自动发信系统 (%24.24s)\n\n", ctime(&now));
	fprintf(of, "【此篇文章是由自动发信系统所张贴】\n\n");
	while (fgets(buf, 256, inf) != NULL)
		fprintf(of, "%s", buf);
	fclose(inf);
	fclose(of);
}

int post_cross(char *filename, char *nboard, char *posttitle, char *owner) {
	struct fileheader postfile;
	char filepath[STRLEN];
	char buf[256], buf4[STRLEN];
	int fp;
	time_t now;

	memset(&postfile, 0, sizeof (postfile));

	now = time(0);
	setbfile(filepath, nboard, "");
	now = trycreatefile(filepath, "M.%ld.A", now, 100);
	if (now < 0)
		return -1;
	postfile.filetime = now;
	postfile.thread = now;
	fh_setowner(&postfile, owner, 0);
	ytht_strsncpy(postfile.title, posttitle, sizeof(postfile.title));

	getcross(filepath, filename, nboard, posttitle);
	chmod(filepath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	setbdir(buf, nboard);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		printf("post recored fail!\n");
		return 0;
	}
	printf("post sucessful\n");
	return 1;
}

int cmpbnames(void *a, void *b) {
	char *bname = (char *) a;
	struct boardheader *brec = (struct boardheader *) b;
	if (!strncmp(bname, brec->filename, sizeof (brec->filename)))
		return 1;
	else
		return 0;
}

int
postfile(char *filename, char *owner, char *nboard, char *posttitle)
{
	struct boardheader fh;

	if (search_record(MY_BBS_HOME "/" BOARDS, &fh, sizeof (fh), cmpbnames, nboard) <= 0) {
		printf("%s 讨论区找不到", nboard);
		return -1;
	}
	post_cross(filename, nboard, posttitle, owner);
	return 0;
}

void securityreport(char * owner, char * str, char * title)
{
	FILE *se;
	char fname[STRLEN];

	sprintf(fname, "tmp/security.%s", owner);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "系统安全记录系统\n\x1b[1m原因：%s\x1b[m\n", str);
		fclose(se);
		postfile(fname, owner, "syssecurity", title);
		unlink(fname);
	}
}

void deliverreport(char * board, char * title, char * str)
{
	FILE *se;
	char fname[STRLEN];

	sprintf(fname, "tmp/deliver.%s.%lu", board, time(NULL));
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
		postfile(fname, "XJTU-XANET", board, title);
		unlink(fname);
	}
}
