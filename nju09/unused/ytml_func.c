#include "bbslib.h"
#define NAVFILE      "nav.txt"

struct content_files cfs = { config:C_FILES_LIST, max:CONTENT_FILE_MAX };

enum {
	INIT,
	DOWORK
};

enum {
	CLASS
};

struct a2i check_type[] = {
	{"class", CLASS},
	{NULL}
};

static struct boardmem *
findboard(struct oneclass *bs, char *bn, int *no)
{
	int i;
	struct boardmem *x;
	for (i = 0; i < bs->bcount; i++) {
		int t;
		t = cl->bdata[bs->group[0].start + i].bno;
		x = &(shm_bcache->bcache[t]);
		if (!strcmp(x->header.filename, bn)
		    && !hideboard(x->header.filename)) {
			*no = i;
			return x;
		}
	}
	return NULL;
}

static int *boards;
static struct oneclass *c;

static int
process_nav_line(int dowhat, int t, char *add, void *fw)
{
	static int num;
	char bname[STRLEN], fname[STRLEN];
	char dir[STRLEN];
	struct boardmem *x;
	struct fileheader *fh = NULL;
	struct mmapfile mf = { ptr:NULL };
	int i, sum, cnt, no, bmax, day;

	switch (dowhat) {
	case INIT:
		num = 0;
		switch (t) {
		case CLASS:
			c = get_class(add);
			if (NULL == c)
				return -1;
			*((int *) fw) = c->bcount;
			break;
		}
		return 0;
	case DOWORK:
		if (sscanf(add, "%d%*s%d%40s%40s", &sum, &cnt, bname, fname) !=
		    4) return 0;
		if (strncmp(fname, "M.", 2) && strncmp(fname, "G.", 2))
			return 0;
		i = atoi(fname + 2);
		x = findboard(c, bname, &no);
		if (!x)
			return 0;
		day = cl->bdata[no + c->group[0].start].nav_day;
		bmax = cl->bdata[no + c->group[0].start].nav_max;
		if (!day)
			day = NAV_DAY;
		if (!bmax)
			bmax = NAV_MAXOB;
		if (now_t - i > day * 24 * 3600)
			return 0;
		sprintf(dir, "boards/%s/.DIR", x->header.filename);
		MMAP_TRY {
			if (mmapfile(dir, &mf) == -1) {
				MMAP_RETURN(0);
			}
			i = -1;
			fh = findbarticle(&mf, fname, &i, 1);
		}
		MMAP_CATCH {
			fh = NULL;
		}
		MMAP_END mmapfile(NULL, &mf);
		if (!fh || boards[no] == bmax)
			return 0;
		boards[no]++;
		fprintf(fw, "%f %d %s %s %s %d %s\n", sum * 1.0 / cnt, cnt,
			x->header.filename, fh2owner(fh), fname, fh->thread,
			fh->title);
		if (++num > NAV_MAX)
			return 1;
		else
			return 0;
	}
	return -1;
}

int
generatenav(char *argv[])
{
	char buf[256];
	time_t t1, t2;
	int i, bcount;
	FILE *fr, *fw;
	if (strchr(argv[1], '/') || strchr(argv[2], '/')) {
		printf("函数 %s 非法的参数\n", argv[0]);
		return -1;
	}
	snprintf(buf, sizeof (buf), "wwwtmp/nav.%s.%s", argv[1], argv[2]);
	t1 = file_time(buf);
	t2 = file_time(NAVFILE);
	if (t1 > t2)
		return 0;
	for (i = 0; check_type[i].str != NULL; i++) {
		if (strcmp(argv[1], check_type[i].str))
			continue;
		if (process_nav_line(INIT, check_type[i].id, argv[2], &bcount)) {
			printf("函数 %s 非法的参数 %s\n", argv[0], argv[2]);
			return -1;
		} else
			break;
	}
	if (NULL == check_type[i].str) {
		printf("函数 %s 非法的参数 %s\n", argv[0], argv[1]);
		return -1;
	}
	fw = fopen(buf, "w");
	if (NULL == fw) {
		printf("函数 %s 不能打开临时文件写 \n", argv[0]);
		return -2;
	}
	flock(fileno(fw), LOCK_EX);
	fr = fopen(NAVFILE, "r");
	if (NULL == fr) {
		printf("函数 %s 不能打开数据文件读 \n", argv[0]);
		fclose(fw);
		return -3;
	}
	boards = alloca(bcount * sizeof (int));
	bzero(boards, bcount * sizeof (int));
	while (NULL != fgets(buf, sizeof (buf), fr))
		if (process_nav_line(DOWORK, 0, buf, fw))
			break;
	fclose(fw);
	fclose(fr);
	return 0;
}

static void
free_c_files(struct content_files *s)
{
	int i;
	for (i = 0; i < s->nums; i++) {
		mmapfile(NULL, &(s->cf[i].mf));
		free(s->cf[i].id);
		free(s->cf[i].path);
	}
#if defined(ENABLE_GHT_HASH) && defined(ENABLE_FASTCGI)
	if (s->p_table) {
		ght_finalize(s->p_table);
		s->p_table = NULL;
	}
#endif
}

static void
load_c_files(struct content_files *s)
{
	FILE *fp;
	time_t t;
	char buf[512];
	t = file_time(s->config);
	if (s->mtime == t)
		return;
	fp = fopen(s->config, "r");
	if (NULL == fp)
		return;
	free_c_files(s);
	if (!(s->cf = realloc(s->cf, sizeof (struct content_file) * (s->max)))) {
		errlog("no enough memory");
		exit(10);
	}
	s->nums = 0;
	while (NULL != fgets(buf, sizeof (buf), fp) && s->nums < s->max) {
		s->cf[s->nums].id = strdup(strtok(buf, " \n\r\t"));
		if (!s->cf[s->nums].id) {
			errlog("no enough memory");
			exit(10);
		}
		s->cf[s->nums].path = strdup(strtok(NULL, " \n\t\t"));
		if (!s->cf[s->nums].path) {
			errlog("no enough memory");
			exit(10);
		}
		bzero(&(s->cf[s->nums].mf), sizeof (struct mmapfile));
		s->nums++;
	}
	if (!(s->cf = realloc(s->cf, sizeof (struct content_file) * (s->nums)))) {
		errlog("memory error...");
		exit(11);
	}
	fclose(fp);
}

static struct content_file *
get_c_file(struct content_files *s, char *id)
{
	int i;
	load_c_files(s);
#if defined(ENABLE_GHT_HASH) && defined(ENABLE_FASTCGI)
	if (s->nums > USE_HASH_LIMIT) {
		if (s->p_table == NULL) {
			s->p_table = ght_create(s->max, NULL, 0);
			for (i = 0; i < s->nums; i++) {
				ght_insert(s->p_table, &(s->cf[i]),
					   strlen(s->cf[i].id), s->cf[i].id);
			}
		}
		return ght_get(s->p_table, strlen(id), id);
	}
#endif
	for (i = 0; i < s->nums; i++)
		if (!strcmp(s->cf[i].id, id))
			return &(s->cf[i]);
	return NULL;
}

static char *
ytml_cur_css(int argc, char *argv[])
{
	check_argc(1);
	printf("<link rel=stylesheet type=text/css href='%s'>\n",
	       currstyle->cssfile);
	return SUCCESS;
}

static char *
ytml_board_class(int argc, char *argv[])
{
	int i;
	check_argc(2);
	c = get_class(argv[1]);
	if (!c) {
		printf("函数 %s 错误的参数 %s\n!", argv[0], argv[1]);
		return FAULT;
	}

	printf
	    ("<table border=0 width=100%%><tr><td bgcolor=%s class=f2><center><b>分类讨论区</b></td><td><table width=100%% cellspacing=0 border=0 bgcolor=%s><tr align=center>\n",
	     currstyle->colortb1, currstyle->colortb1);
	for (i = 0; i < (c->gcount + 1) / 2; i++) {
		printf("<td><nobr><a href=boa?c=%s&sec=%d>%s</a>", c->cname, i,
		       c->group[i].gname);
		if (i < (c->gcount + 1) / 2 - 1)
			printf("</td><td>|</td>");
	}
	printf("</tr><tr align=center>");
	for (; i < c->gcount; i++) {
		printf("<td><nobr><a href=boa?c=%s&sec=%d>%s</a>", c->cname, i,
		       c->group[i].gname);
		if (i < c->gcount - 1)
			printf("</td><td>|</td>");
	}
	printf("</tr></table></td></tr></table>");
	return SUCCESS;
}

static char *
ytml_show_excellent(int argc, char *argv[])
{
	check_argc(3);
	if (generatenav(argv))
		return FAULT;
	printf
	    ("<table><tr><td bgcolor=%s class=blk><font color=%s>★</font>&nbsp;近日精彩话题推荐 &nbsp;(<a href=bbsshownav?a1=%s&a2=%s class=blk>查看全部</a>)</td></tr><tr><td>",
	     currstyle->colortb2, currstyle->colorstar, argv[1], argv[2]);
	shownavpart(0, "");	//argv);
	printf("</td></tr></table>");
	return SUCCESS;
}

int
show_content(char *id)
{
	struct content_file *tmp;
	tmp = get_c_file(&cfs, id);
	if (NULL == tmp)
		return -1;
	if (mmapfile(tmp->path, &(tmp->mf))) {
		printf("mmap 错误,联系系统维护!");
		return 0;
	}
	fwrite(tmp->mf.ptr, tmp->mf.size, 1, stdout);
	return 0;
}

static char *
ytml_show_content(int argc, char *argv[])
{
	check_argc(4);
	if (strcmp(argv[2], ""))
		printf
		    ("<table width=100%%><tr><td bgcolor=%s class=%s><font color=%s>★</font>&nbsp;%s</td></tr><tr><td>",
		     currstyle->colortb2, argv[3], currstyle->colorstar,
		     argv[2]);
	if (show_content(argv[1])) {
		printf("%s 函数 不支持对 id %s 的调用", argv[0], argv[1]);
		return FAULT;
	}
	if (strcmp(argv[2], ""))
		printf("</td></tr></table>");
	return SUCCESS;
}

static int
generatehot()
{
	FILE *fp, *fw;
	int i, r;
	char buf[256], name[256];
	int board[HOT_MAX], t1, t2;
	struct boardmem *x;
	snprintf(buf, sizeof (buf), "wwwtmp/hot.%s", c->cname);
	t1 = file_time("0Announce/bbslist/board2");
	t2 = file_time(buf);
	if (t2 > t1)
		return 0;
	fw = fopen(buf, "w");
	if (NULL == fw)
		return -1;
	flock(fileno(fw), LOCK_EX);
	fp = fopen("0Announce/bbslist/board2", "r");
	if (fp == NULL) {
		fclose(fw);
		return -2;
	}
	r = 0;
	for (i = 0; i < c->bcount && r < HOT_MAX; i++) {
		if (cl->bdata[i].advice) {
			for (t1 = 0; t1 < r; t1++)
				if (board[t1] == cl->bdata[i].bno)
					break;
			if (t1 == r)
				board[r++] = cl->bdata[i].bno;
		}
	}
	i = r;
	while (fgets(buf, sizeof (buf), fp) && r < HOT_MAX) {
		if (sscanf(buf, "%*s %*s %s %*s", name) != 1)
			continue;
		x = findboard(c, name, &t2);
		if (!x)
			continue;
		t2 = x - &(shm_bcache->bcache[0]);
		for (t1 = 0; t1 < i; t1++)
			if (t2 == board[t1])
				break;
		if (t1 < i)
			continue;
		board[r++] = t2;
	}
	for (i = 0; i < r; i++) {
		if (i)
			fprintf(fw, "%s", " &nbsp;");
		x = &(shm_bcache->bcache[board[i]]);
		fprintf(fw, "<a href=tdoc?B=%s class=pur><u>%s</u></a>",
			x->header.filename, x->header.title);
	}
	fclose(fp);
	fclose(fw);
	return 0;
}

int
print_small_file(char *fname)
{
	char buf[256];
	FILE *fp;
	fp = fopen(fname, "r");
	if (!fp)
		return -1;
	while (fgets(buf, sizeof (buf), fp))
		printf("%s", buf);
	fclose(fp);
	return 0;
}

char *
ytml_show_hot_board(int argc, char *argv[])
{
	char buf[256];
	check_argc(2);
	c = get_class(argv[1]);
	if (NULL == c) {
		printf("%s 函数 非法的参数 %s\n", argv[0], argv[1]);
		return FAULT;
	}
	if (generatehot()) {
		printf("%s 函数 内部错误\n", argv[0]);
		return FAULT;
	}
	printf
	    ("<table width=588 border=1><tr><td bgcolor=%s width=55 align=center>热门讨论区推荐</td><td>",
	     currstyle->colortb1);
	snprintf(buf, sizeof (buf), "wwwtmp/hot.%s", c->cname);
	print_small_file(buf);
	printf("</td></tr></table>");
	return SUCCESS;
}

typedef char string[60];
string *filterstr;
int nfilterstr;

static int
initfilter(char *filename)
{
	FILE *fp;
	char *ptr, *p;
	int n;
	fp = fopen(filename, "r");
	if (!fp)
		return 0;
	for (n = 0; n < LM_MAXFILTER; n++) {
		ptr = filterstr[n];
		if (fgets(filterstr[n], 60, fp) == NULL)
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
			n--;
	}
	fclose(fp);
	return n;
}

static int
need_filter(char *str)
{
	int i;
	if (!nfilterstr)
		return 0;
	for (i = 0; i < nfilterstr; i++) {
		if (strstr(str, filterstr[i]))
			return 1;
	}
	return 0;
}

static int
readlastmark(char *board, char *title, int *thread)
{
	char buf[200], *ptr;
	FILE *fp;
	int found = 0;
	snprintf(buf, sizeof (buf), "wwwtmp/lastmark/%s", board);
	if (!file_exist(buf))
		goto END;
	if ((fp = fopen(buf, "r")) == NULL)
		goto END;
	while (fgets(buf, 200, fp)) {
		if (need_filter(buf))
			continue;
		ptr = strchr(buf, '\t');
		if (ptr == NULL)
			break;
		ptr++;
		*thread = atoi(ptr);
		ptr = strchr(ptr, '\t');
		if (ptr == NULL)
			break;
		ptr++;
		strsncpy(title, ptr, STRLEN);
		ptr = strchr(title, '\n');
		if (ptr)
			*ptr = 0;
		found = 1;
	}
	fclose(fp);
      END:
	return found;
}

static void
makerand(int w[], int n, int max)
{
	int i, j, l;
	for (i = 0; i < n; i++) {
		l = random() % max;
		for (j = 0; j < i; j++)
			if (w[j] == l)
				break;
		if (i > j) {
			i--;
			continue;
		}
		w[i] = l;
	}
}

static void
print_lm_line(FILE * fw, struct boardmem *x, char *title, int thread)
{
	fprintf(fw,
		"<tr><td valign=top>·</td><td><a href='tfind?B=%s&th=%d&T=%s'>",
		x->header.filename, thread, encode_url(title));
	if (!strncmp(title, "[转载] ", 7) && strlen(title) > 20)
		title += 7;
	if (strlen(title) > 45)
		title[45] = 0;
	fprintf(fw,
		"%s</a> &lt;<a href='tdoc?B=%s' class=blk>%s</a>&gt;</td></tr>\n",
		void1(titlestr(title)), x->header.filename, x->header.title);
}

static int
generate_last_mark()
{
	int i, j, k, t;
	int r[LM_MAXLRAND];
	char title[STRLEN];
	int thread;
	struct boardmem *x, *x1;
	struct c_mark lastmark;
	FILE *fp;

	i = file_time("wwwtmp/lastmark");
	snprintf(title, sizeof (title), "wwwtmp/lastmark.%s", c->cname);
	j = file_time(title);
	if (j > i)
		return 0;
	filterstr = alloca(LM_MAXFILTER * sizeof (string));
	if (!filterstr)
		return -1;
	fp = fopen(title, "w");
	if (NULL == fp)
		return -1;
	flock(fileno(fp), LOCK_EX);
	nfilterstr = initfilter("etc/filtertitle");
	bzero(&lastmark, sizeof (lastmark));
	for (i = 0; i < c->gcount; i++) {
		for (j = 0; j < c->group[i].bcount; j++) {
			x =
			    &(shm_bcache->bcache
			      [cl->bdata[c->group[i].start + j].bno]);
			if (hideboard(x->header.filename))
				continue;
			for (k = 0; k < lastmark.gmark[i].n; k++) {
				x1 =
				    &(shm_bcache->bcache
				      [lastmark.gmark[i].bmark[k].bno]);
				if (x->score > x1->score)
					break;
			}
			if (k == LM_MAXL)
				continue;
			if (!readlastmark(x->header.filename, title, &thread))
				continue;
			if (lastmark.gmark[i].n == LM_MAXL)
				lastmark.gmark[i].n--;
			t = k;
			for (k = lastmark.gmark[i].n; k > t; k--)
				lastmark.gmark[i].bmark[k] =
				    lastmark.gmark[i].bmark[k - 1];
			lastmark.gmark[i].bmark[t].bno =
			    x - (struct boardmem *) (&shm_bcache->bcache[0]);
			snprintf(lastmark.gmark[i].bmark[t].title, STRLEN, "%s",
				 title);
			lastmark.gmark[i].bmark[t].thread = thread;
			lastmark.gmark[i].n++;
		}
	}
	t = c->gcount - 1;
	if (!strcmp(c->cname, "all"))
		t -= 2;
	fprintf(fp, "<table><tr><td valign=top>\n");
	for (i = t; i >= 0; i--) {
		if (t - i == (t + 1) / 2)
			fprintf(fp, "</td><td>&nbsp;</td><td valign=top>\n");
		fprintf
		    (fp,
		     "<table cellpadding=2 cellspacing=0 border=0 width=100%%><tr class=tb2_blk><td width=15>"
		     "<font class=star>★</font></td><td><a href=boa?c=%s&sec=%d class=blk>%s</td></tr>\n",
		     c->cname, i, c->group[i].gname);

		if (!lastmark.gmark[i].n)
			fprintf(fp, "%s", "<tr><td>&nbsp;</td></tr>\n");
		if (lastmark.gmark[i].n <= LM_MAXLPRINT) {
			for (j = 0; j < lastmark.gmark[i].n; j++)
				print_lm_line(fp,
					      shm_bcache->bcache +
					      lastmark.gmark[i].bmark[j].bno,
					      lastmark.gmark[i].bmark[j].title,
					      lastmark.gmark[i].
					      bmark[j].thread);
			fprintf(fp, "</table>\n");
			continue;
		}
		for (j = 0; j < LM_MAXLNRAND; j++)
			print_lm_line(fp,
				      shm_bcache->bcache +
				      lastmark.gmark[i].bmark[j].bno,
				      lastmark.gmark[i].bmark[j].title,
				      lastmark.gmark[i].bmark[j].thread);
		makerand(r, LM_MAXLRAND, lastmark.gmark[i].n - LM_MAXLNRAND);
		for (k = 0; k < LM_MAXLRAND; k++) {
			j = r[k] + LM_MAXLNRAND;
			print_lm_line(fp,
				      shm_bcache->bcache +
				      lastmark.gmark[i].bmark[j].bno,
				      lastmark.gmark[i].bmark[j].title,
				      lastmark.gmark[i].bmark[j].thread);
		}
		fprintf(fp, "</table>\n");
	}
	fprintf(fp, "</td></tr></table>\n");
	fclose(fp);
	return 0;
}

static char *
ytml_show_last_mark(int argc, char *argv[])
{
	time_t t;
	char buf[256];
	check_argc(2);
	c = get_class(argv[1]);
	if (!c) {
		printf("%s 函数 错误的参数 %s\n", argv[0], argv[1]);
		return FAULT;
	}
	if (generate_last_mark()) {
		printf("%s 函数 内部处理错误\n", argv[0]);
		return FAULT;
	}
	snprintf(buf, sizeof (buf), "wwwtmp/lastmark.%s", c->cname);
	printf
	    ("<table border=0><tr><td width=25%%></td><td align=center width=50%%>"
	     "<FONT class=f3><b>--== 讨论区导读 ==--</b></FONT></td>");
	t = file_time(buf);
	printf("<td width=25%% align=right>更新时间: %s</td></tr></table>",
	       ctime(&t) + 4);
	ytml_show_hot_board(argc, argv);
	print_small_file(buf);
	return SUCCESS;
}

int
generate_last_b(struct boardmem *x)
{
#define LASTB_UPDATEP 1800
#define LASTB_DAYS 14
#define LASTB_MAX 10
	char buf1[256], buf2[256];
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *fh;
	FILE *fp = NULL;
	int fc, t1, t2, i = 0, start, ct = 0;
	snprintf(buf1, sizeof (buf2), "wwwtmp/lastb.%s", x->header.filename);
	t1 = file_time(buf1);
	snprintf(buf2, sizeof (buf2), "boards/%s/.DIR", x->header.filename);
	t2 = file_time(buf2);
	if (t1 && t1 + LASTB_UPDATEP > t2)
		return 0;
	MMAP_TRY {
		if (mmapfile(buf2, &mf)) {
			MMAP_RETURN(-1);
		}
		fp = fopen(buf1, "w");
		if (NULL == fp) {
			mmapfile(NULL, &mf);
			MMAP_RETURN(-2);
		}
		flock(fileno(fp), LOCK_EX);
		fc = mf.size / sizeof (struct fileheader);
		start = now_t - LASTB_DAYS * 86400;
		for (fh = (struct fileheader *) (mf.ptr) + fc - 1;
		     fh >= (struct fileheader *) mf.ptr && ct < LASTB_MAX; fh--) {
			if (fh->filetime >= start) {
				if ((fh->accessed & FH_MARKED)
				    && (fh->accessed & FH_DIGEST)) {
					ct++;
					fprintf(fp,
						"<li><b><a href=con?B=%s&F=%s&N=%d&T=%d>%s</a><br></b></li>\n",
						x->header.filename,
						fh2fname(fh),
						fh -
						(struct fileheader *) mf.ptr +
						1, feditmark(*fh), void1(titlestr(fh->title)));
				}
			} else
				i++;
			if (i > 10)
				break;
		}
	}
	MMAP_CATCH {
	}
	MMAP_END {
		if (fp)
			fclose(fp);
		mmapfile(NULL, &mf);
	}
	return 0;
}

static char *
ytml_show_last_b(int argc, char *argv[])
{
	char buf[256];
	struct boardmem *b;
	check_argc(2);
	b = getbcache(argv[1]);
	if (!b || !has_read_perm(&currentuser, b->header.filename)) {
		printf("%s 函数 错误的参数 %s\n", argv[0], argv[1]);
		return FAULT;
	}
	if (generate_last_b(b)) {
		printf("%s 函数 内部处理错误\n", argv[0]);
		return FAULT;
	}
	snprintf(buf, sizeof (buf), "wwwtmp/lastb.%s", b->header.filename);
	printf
	    ("<table><tr><td bgcolor=%s class=blk><font color=%s>★</font>&nbsp;%s</td></tr></table>\n",
	     currstyle->colortb2, currstyle->colorstar, b->header.title);
	print_small_file(buf);
	return SUCCESS;
}

static char *
ytml_show_board_mge(int argc, char *argv[])
{
	struct boardmem *b;
	check_argc(2);
	b = getbcache(argv[1]);
	if (!b || !has_read_perm(&currentuser, b->header.filename)) {
		printf("%s 函数 错误的参数 %s\n", argv[0], argv[1]);
		return FAULT;
	}
	printf
	    ("<table><tr><td bgcolor=%s class=blk><font color=%s>★</font>&nbsp;<a href=tdoc?b=%s>%s</a></td></tr></table>\n",
	     currstyle->colortb2, currstyle->colorstar, b->header.filename,
	     b->header.title);
	printf("<table>\n");
	printlastmark(b->header.filename);
	printf("</table>");
	return SUCCESS;
}

static char *
ytml_test(int argc, char *argv[])
{
	int i = 0;
	printf("ytml处理过程");
	printf(" %s 参数序列", argv[0]);
	for (i = 1; i < argc; i++)
		printf(" (%s)", argv[i]);
	printf("\n");
	return SUCCESS;
}

struct func_applet funcs[] = {
	{ytml_cur_css, {"cur_css", NULL}},
	{ytml_board_class, {"board_class", NULL}},
	{ytml_show_excellent, {"show_excellent", NULL}},
	{ytml_show_content, {"show_content", NULL}},
	{ytml_show_hot_board, {"sbow_hot_board", NULL}},
	{ytml_show_last_mark, {"show_last_mark", NULL}},
	{ytml_show_last_b, {"show_last_b", NULL}},
	{ytml_show_board_mge, {"show_board_mge", NULL}},
	{ytml_test, {"debug", "test", NULL}},
	{NULL}
};
