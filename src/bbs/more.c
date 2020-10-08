/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include <sys/mman.h>
#include "bbs.h"
#include "smth_screen.h"
#include "term.h"
#include "io.h"
#include "stuff.h"
#include "main.h"
#include "help.h"
#include "bcache.h"
#include "xyz.h"
#include "goodbye.h"
#include "bbs_global_vars.h"

#define MAXnettyLN            7    /* lines of  activity board  */
#define ACBOARD_BUFSIZE     255    /* max. length of each line for activity board  */
#define ACBOARD_MAXLINE     400    /* max. lines of  activity board  160 */

static int stuffmode = 0;
time_t calltime = 0;
extern int isattached;

struct ACSHM {
	char line[ACBOARD_MAXLINE][ACBOARD_BUFSIZE];
	int movielines;
	time_t update;
};

struct ACSHM *movieshm;
int nnline = 0, xxxline = 0;

struct MemMoreLines {
	char *ptr;
	int size;
	char *line[100];
	char ty[100];		/* 0: 普通, 有回车; 1: 普通, 无回车; 2: 引文, 有回车; 3: 引文, 无回车 */
	int len[100];
	int s[100];
	int start;		/* this->line[start%100]是检索的行号最小的行，行号为 start */
	int num;		/* 共检索了row到row+num-1这么多行 */
	int curr_line;		/* 当前游标位置 */
	char *curr;		/* 当前游标的行 */
	char currty;
	int currlen;
	int total;
};

static void netty_more(void);
static int measure_line(char *p0, int size, int *l, int *s, char oldty, char *ty);
static void init_MemMoreLines(struct MemMoreLines *l, char *ptr, int size);
static int next_MemMoreLines(struct MemMoreLines *l);
static int seek_MemMoreLines(struct MemMoreLines *l, int n);
static int mmap_show(char *fn, int row, int numlines);
static int mmap_more(char *fn, int quit, char *keystr, char *title);
static void mem_printline(char *ptr, int len, char *fn, char ty,struct MemMoreLines *l);	/*clearboy 2005.1,增加tenet下面图片链接显示*/
static void mem_show(char *ptr, int size, int row, int numlines, char *fn);
static void mem_printbotline(int l1, int l2, int total, int read, int size);
static int mem_more(char *ptr, int size, int quit, char *keystr, char *fn, char *title);

int
NNread_init()
{
	FILE *fffd;
	char *ptr;
	char buf[ACBOARD_BUFSIZE];
	struct stat st;
	time_t ftime, now;

	now = time(0);
	if (stat("etc/movie", &st) < 0) {
		return 0;
	}
	ftime = st.st_mtime;
	if (movieshm == NULL) {
		movieshm =
		    (void *) attach_shm(ACBOARD_SHMKEY, sizeof (*movieshm));
	}
	if (abs(now - movieshm->update) < 12 * 60 * 60
	    && ftime < movieshm->update) {
		return 1;
	}
	if ((fffd = fopen("etc/movie", "r")) == NULL) {
		return 0;
	}
	nnline = 0;
	xxxline = 0;
	if (!DEFINE(DEF_ACBOARD, currentuser)) {
		nnline = 1;
		xxxline = 1;
		return 1;
	}
	while ((xxxline < ACBOARD_MAXLINE) &&
	       (fgets(buf, ACBOARD_BUFSIZE, fffd) != NULL)) {
		ptr = movieshm->line[xxxline];
		memcpy(ptr, buf, sizeof (buf));
		xxxline++;
	}
	sprintf(buf, "%79.79s\n", " ");
	movieshm->movielines = xxxline;
	while (xxxline % MAXnettyLN != 0) {
		ptr = movieshm->line[xxxline];
		memcpy(ptr, buf, sizeof (buf));
		xxxline++;
	}
	movieshm->movielines = xxxline;
	movieshm->update = time(0);
	sprintf(buf, "system reload movie %d", xxxline);
	newtrace(buf);
	fclose(fffd);
	return 1;
}

void
setcalltime()
{
	char ans[6];
	int ttt;

	move(1, 0);
	clrtoeol();
	getdata(1, 0, "几分钟后要系统提醒你: ", ans, 3, DOECHO, YEA);
	if (!isdigit(ans[0]))
		return;
	ttt = atoi(ans);
	if (ttt <= 0)
		return;
	calltime = time(0) + ttt * 60;
}

/*Add by SmallPig*/
int
countln(fname)
char *fname;
{
	FILE *fp;
	char tmp[256];
	int count = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		return 0;

	while (fgets(tmp, sizeof (tmp), fp) != NULL)
		count++;
	fclose(fp);
	return count;
}

						      /* below added by netty  *//*Rewrite by SmallPig */
static void
netty_more()
{
	char buf[256];
	int ne_row = 1;
	int x, y;
	time_t thetime = time(0);

	if (!DEFINE(DEF_ACBOARD, currentuser)) {
		update_endline();
		return;
	}

	nnline = ((thetime / 5) % (movieshm->movielines / MAXnettyLN)) * MAXnettyLN;
	getyx(&y, &x);
	update_endline();
	move(3, 0);
	while ((nnline < movieshm->movielines) /*&&DEFINE(DEF_ACBOARD) */ ) {
		move(1 + ne_row, 0);//by bjgyt
		clrtoeol();
		strcpy(buf, movieshm->line[nnline]);
		showstuff(buf);
		nnline = nnline + 1;
		ne_row = ne_row + 1;
		if (nnline == movieshm->movielines) {
			nnline = 0;
			break;
		}
		if (ne_row > MAXnettyLN) {
			break;
		}
	}
	move(y, x);
}

static void printacbar() {
	struct boardmem *bp;
	int x, y;
	getyx(&y, &x);

	bp = getbcache(DEFAULTBOARD);
	if (bp == NULL)
		return;
	move(2, 0);
	prints("\033[1;31m□——————————————┤\033[37m活  动  看  版\033[31m├——————————————□ \033[m\n");
	move(3 + MAXnettyLN, 0);
	if (bp->header.flag & VOTE_FLAG)
		prints("\033[1;31m□———————————┤\033[37m系统投票中 [ Config->Vote ] \033[31m├——————————□ \033[m\n");
	else
		prints("\033[1;31m□—————————————————————————————————————□ \033[m\n");

	move(y, x);
	refresh();
}

void
check_calltime()
{
	int line;

	if (calltime != 0 && time(0) >= calltime) {
		if (uinfo.mode == TALK)
			line = t_lines / 2 - 1;
		else
			line = 0;
		saveline(line, 0, NULL);	/* restore line */
		bell();
		bell();
		bell();
		move(line, 0);
		clrtoeol();
		prints("\033[1;44;32mBBS 系统通告: \033[37m%-65s\033[m",
		       "系统闹钟 铃～～～～～～");
		igetkey();
		move(line, 0);
		clrtoeol();
		saveline(line, 1, NULL);
		calltime = 0;
	}

}

void
R_monitor()
{

	if (!DEFINE(DEF_ACBOARD, currentuser) && !DEFINE(DEF_ENDLINE, currentuser))
		return;

	if (uinfo.mode != MMENU)
		return;
	alarm(0);
	signal(SIGALRM, (void *) R_monitor);
	netty_more();
	//printacbar(); by bjgyt
	if (!DEFINE(DEF_ACBOARD, currentuser))
		alarm(60);
	else
		alarm(10);
}

static int
measure_line(char *p0, int size, int *l, int *s, char oldty, char *ty)
{
	int i, w, in_esc = 0, db = 0, lastspace = 0, asciiart = 0;
	char *p = p0;
	if (size == 0)
		return -1;
	for (i = 0, w = 0; i < size; i++, p++) {
		if (*p == '\n') {
			*l = i;
			*s = i + 1;
			break;
		}
		if (asciiart) {
			continue;
		} else if (*p == '\t') {
			db = 0;
			w = (w + 8) / 8 * 8;
			lastspace = i;
		} else if (*p == '\033') {
			db = 0;
			in_esc = 1;
			lastspace = i - 1;
		} else if (in_esc) {
			if (strchr("suHMfL@PABCDJK", *p) != NULL) {
				asciiart = 1;
				continue;
			}
			if (strchr("[0123456789;,", *p) == NULL)
				in_esc = 0;
		} else if (isprint2(*p)) {
			if (w > t_columns - 1) {
				if (db) {
					*l = i - 1;
					*s = i - 1;
				} else if (isblank(*p)) {
					*l = i;
					*s = i;
				} else if (lastspace > 0) {
					*l = lastspace + 1;
					*s = lastspace + 1;
				} else {
					*l = i;
					*s = i;
				}
				//while(*l>0&&isblank(p0[*l-1]))
				//      (*l)--;
				while (*s < size && isblank(p0[*s]))
					(*s)++;
				if (*s < size && p0[*s] == '\n')
					(*s)++;
				break;
			}

			if (!db) {
				if ((unsigned char) *p >= 128)
					db = 1;
				else if (isblank(*p))
					lastspace = i;
			} else {
				db = 0;
				lastspace = i;
			}
			w++;
		}
	}
	if (i >= size) {
		*l = size;
		*s = size;
	}
	if (*s > 0 && p0[*s - 1] == '\n') {
		switch (oldty) {
		case 1:
			*ty = 0;
			break;
		case 3:
			*ty = 2;
			break;
		default:
			if (*l < 2 || strncmp(p0, ": ", 2))
				*ty = 0;
			else
				*ty = 2;
		}
	} else {
		switch (oldty) {
		case 1:
			*ty = 1;
			break;
		case 3:
			*ty = 3;
			break;
		default:
			if (*l < 2 || strncmp(p0, ": ", 2))
				*ty = 1;
			else
				*ty = 3;
		}
	}
	if (*s == size)
		return 0;
	if (oldty % 2 == 0 && *ty == 0 && size > 10
	    && !strncmp(p0, "begin 644 ", 10)) {
		char *pe = p0 + size;
		for (p = p0; p < pe;) {
			if (!(p = memchr(p, '\n', pe - p)))
				break;
			p++;
			if (pe - p > 3 && !strncmp(p, "end", 3)) {
				if ((p = memchr(p, '\n', pe - p))) {
					p++;
					break;
				}
			}
		}
		if (p == NULL)
			*s = size;
		else
			*s = p - p0;
		*ty = 100;
	}
	if (oldty % 2 == 0 && *ty == 0 && size > 19
	    && !strncmp(p0, "beginbinaryattach ", 18)
	    && size - *s >= 5 && p0[*s] == 0) {
		unsigned int len;
		p = p0 + *s + 1;
		len = ntohl(*(unsigned int *) p);
		if (len > size - 5)
			len = size - 5;
		*s += 5 + len;
		*ty = 102;
		if (size - *s >= 1 && p0[*s] == '\n')
			(*s)++;
	}
	if ((oldty == 100 || oldty == 102) && (*ty != 100 && *ty != 102)) {
		*l = 0;
		*s = 0;
		*ty = 104;
	}
	return 0;
}

int effectiveline;		//有效行数, 只计算前面的部分, 头部不含, 空行不含, 签名档不含, 引言不含

static void
init_MemMoreLines(struct MemMoreLines *l, char *ptr, int size)
{
	int i, s, u;
	char *p0, oldty = 0;
	l->ptr = ptr;
	l->size = size;
	l->start = 0;
	l->num = 0;
	l->total = 0;
	effectiveline = 0;
	for (i = 0, p0 = ptr, s = size; i < 25 && s > 0; i++) {
		u = (l->start + l->num) % 100;
		l->line[u] = p0;
		if (measure_line(p0, s, &l->len[u], &l->s[u], oldty, &l->ty[u])
		    < 0) {
			break;
		}
		oldty = l->ty[u];
		s -= l->s[u];
		p0 = l->line[u] + l->s[u];
		l->num++;
		if (effectiveline >= 0) {
			if (l->len[u] >= 2 && strncmp(l->line[u], "--", 2) == 0)
				effectiveline = -effectiveline;
			else if (l->num > 3 && l->len[u] >= 2 && l->ty[u] < 2)
				effectiveline++;
		}
	}
	if (effectiveline < 0)
		effectiveline = -effectiveline;
	if (s == 0)
		l->total = l->num;
	l->curr_line = 0;
	l->curr = l->line[0];
	l->currlen = l->len[0];
	l->currty = l->ty[0];
}

static int
next_MemMoreLines(struct MemMoreLines *l)
{
	int n;
	char *p0;

	if (l->curr_line + 1 >= l->start + l->num) {
		char oldty;
		n = (l->start + l->num - 1) % 100;
		if (l->ptr + l->size == (l->line[n] + l->s[n])) {
			return -1;
		}
		if (l->num == 100) {
			l->start++;
			l->num--;
		}
		oldty = l->ty[n];
		p0 = l->line[n] + l->s[n];
		n = (l->start + l->num) % 100;
		l->line[n] = p0;
		measure_line(p0, l->size - (p0 - l->ptr), &l->len[n], &l->s[n],
			     oldty, &l->ty[n]);
		l->num++;
		if (l->size - (p0 - l->ptr) == l->s[n]) {
			l->total = l->start + l->num;
		}
	}
	l->curr_line++;
	l->curr = l->line[l->curr_line % 100];
	l->currlen = l->len[l->curr_line % 100];
	l->currty = l->ty[l->curr_line % 100];
	return l->curr_line;
}

static int
seek_MemMoreLines(struct MemMoreLines *l, int n)
{
	int i;
	if (n < 0) {
		seek_MemMoreLines(l, 0);
		return -1;
	}
	if (n < l->start) {
		i = l->total;
		init_MemMoreLines(l, l->ptr, l->size);
		l->total = i;
	}
	if (n < l->start + l->num) {
		l->curr_line = n;
		l->curr = l->line[l->curr_line % 100];
		l->currlen = l->len[l->curr_line % 100];
		l->currty = l->ty[l->curr_line % 100];
		return l->curr_line;
	}
	while (l->curr_line != n)
		if (next_MemMoreLines(l) < 0)
			return -1;
	return l->curr_line;
}

static int
mmap_show(char *fn, int row, int numlines)
{
      struct mmapfile mf = { ptr:NULL };
	MMAP_TRY {
		if (mmapfile(fn, &mf) < 0)
			MMAP_RETURN(-1);
		mem_show(mf.ptr, mf.size, row, numlines, fn);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

static int
mmap_more(char *fn, int quit, char *keystr, char *title)
{
      struct mmapfile mf = { ptr:NULL };
	int retv = 0;
	MMAP_TRY {
		if (mmapfile(fn, &mf) < 0)
			MMAP_RETURN(-1);
		retv = mem_more(mf.ptr, mf.size, quit, keystr, fn, title);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return retv;
}

static void
mem_printline(char *ptr, int len, char *fn, char ty,struct MemMoreLines *l)
{
	if (stuffmode) {
		char buf[256];
		memcpy(buf, ptr, (len >= 256) ? 255 : len);
		buf[(len >= 256) ? 255 : len] = 0;
		showstuff(buf);
		prints("\n");
		return;
	}
	if (!strncmp(ptr, "□ 引用", 7) || !strncmp(ptr, "==>", 3)
	    || !strncmp(ptr, "【 在", 5) || !strncmp(ptr, "※ 引述", 7)) {
		outns("\033[1;33m", 7);
		outns(ptr, len);
		outns("\033[m\n", 4);
		return;
	} else if (ty == 100 || ty == 102) {
		char attachname[41], *p;
		isattached = 1;
		ytht_strsncpy(attachname, ptr + (ty == 100 ? 10 : 18), 40);
		p = strchr(attachname, '\n');
		if (p != NULL)
			*p = 0;
		p = strrchr(attachname, '.');
		if (p != NULL)
		{
			int nPos=0;
			int last_line;
			last_line = l->curr_line;
			if(l!=NULL)
				{
				 nPos=l->curr- l->ptr+20+strlen(attachname);
				}
/* clearboy 2005.1
修改目的：增加tenet下面图片链接显示
修改方法：给mem_printline函数，增加一个接口，以获得图片链接的地址。
原理说明：

图片链接地址的生成规律如下：
http://站名/版面名称/文件名称/随机数/包含后缀的文件名

其中：
1、随机数的生成规律为：
随机数= l->curr- l->ptr + 20 + strlen(图片文件名)
2、包含后缀的文件名，实际上并没有实际的意义，只要后缀相同，文件名可以任意:)

原理解释：

这种修改的方法，是直接把图片的链接地址显示出来了。
麻烦的是，链接地址上面的那个随机数。

没有文档，我根据观察，总结的随机数的规律似乎是：
随机数= l->curr- l->ptr；
但是，这个值恰好少20+，不知道原因是什么，所以我就在代码里面加了
+20 + strlen(图片文件名)

对于中文的文件名，fterm现在不支持。好在现在服务器端这个版本的atthttpd不要求文件名必须是上传时候的那个。。。
只根据文件名后缀判断一下文件类型。因此，为了避免因为用户上载的文件名的不同引起的麻烦，返回的图片链接地址，
并没有用原始的文件名。

*/
/*			if((attachname[0] > ' ' && attachname[0] < 'z' && strlen(attachname) < 20))
			{
			prints
                            ("\033[m附图: %s 链接:\nhttp://%s:8080/%s%s/%d/%s\033[0m\n",
                             	attachname,MY_BBS_DOMAIN,currboard,strrchr(fn, '/'),nPos,attachname);
			}
			else
*/			{
/*			prints
			    ("\033[m附图: %s 链接:\n\033[1;4mhttp://%s:8080/%s%s/%d/%d%s\033[0m\n",
				attachname,MY_BBS_IP, currboard, strrchr(fn,'/'), nPos, l->curr_line - 4, strrchr(attachname, '.'));
*/
/*修改by Clearboy@BMY 2005.2.27
1. 去掉原来的文件名,不分行显示;
2. 更改了文件名提取的方法,用"."来提取，以便在推荐的帖子中图片仍然能正常显示；*/
			if (!strcasecmp(p, ".bmp") || !strcasecmp(p, ".jpg") || !strcasecmp(p, ".gif") || !strcasecmp(p, ".jpeg") ||!strcasecmp(p, ".png"))
				prints("\033[m附图: \033[1;4mhttp://%s/attach/%s/M%s/%d/%d%s\033[0m\n",
				MY_BBS_DOMAIN, currboard, strchr(fn,'.'), nPos, l->curr_line - 4, strrchr(attachname, '.'));
			else
				prints("\033[m附件: \033[1;4mhttp://%s/attach/%s/M%s/%d/%d%s\033[0m\n",
				MY_BBS_DOMAIN, currboard, strchr(fn,'.'), nPos, l->curr_line - 4, strrchr(attachname, '.'));
			}

		}
		return;
	} else if (ty == 104) {
		char *q;
		char temp_sessionid[10];
		int type;
		type = 0;
		if (!strncmp(fn, "boar", 4)) {
			if (!strncmp(fn + 7, ".1984", 5))
				type = 4;
			else if (!strncmp(fn + 7, ".back", 5))
				type = 5;
			else
				type = 1;
		} else if (!strncmp(fn, "0Ann", 4))
			type = 2;
		else if (!strncmp(fn, "mail", 4))
			type = 3;
		get_temp_sessionid(temp_sessionid);
		q = strrchr(fn, '/') + 1;
		switch (type) {
		case 1:
			if (digestmode == YEA)
				prints("http://%s/" SMAGIC "%s/gcon?B=%s&F=%s",
						MY_BBS_DOMAIN, temp_sessionid, currboard, q);
			else
				prints("http://%s/" SMAGIC "%s/con?B=%s&F=%s",
						MY_BBS_DOMAIN, temp_sessionid, currboard, q);
			break;
		case 2:
			if (0)
				prints("http://%s/" SMAGIC "%s/anc?path=%s",
						MY_BBS_DOMAIN, temp_sessionid, q);
			break;
		case 3:
			prints("http://%s/" SMAGIC "%s/bbsmailcon?file=%s",
					MY_BBS_DOMAIN, temp_sessionid, q);
			break;
		case 4:
			prints("http://%s/" SMAGIC "%s/c1?T=%d&F=%s",
					MY_BBS_DOMAIN, temp_sessionid, type, fn + 13);
			break;
		case 5:
			prints("http://%s/" SMAGIC "%s/c1?T=%d&F=%s",
					MY_BBS_DOMAIN, temp_sessionid, type, fn + 20);
			break;
		default:
			break;
		}
		prints("\n");
		return;
	} else if (ty >= 2) {
		outns("\033[36m", 5);
		outns(ptr, len);
		outns("\033[m\n", 4);
		return;

	}
	//else outns("\033[37m",5);
	outns(ptr, len);
	//outns("\033[m\n", 4);
	outns("\n", 1);
}

static void
mem_show(char *ptr, int size, int row, int numlines, char *fn)
{
	extern int t_lines;
	struct MemMoreLines l;
	int i, curr_line;
	if (size <= 0)
		return;
	init_MemMoreLines(&l, ptr, size);
	move(row, 0);
	clrtobot();
	prints("\033[m");
	curr_line = l.curr_line;
	for (i = 0; i < t_lines - 1 - row && i < numlines; i++) {
		mem_printline(l.curr, l.currlen, fn, l.currty,&l);
		if (next_MemMoreLines(&l) < 0)
			break;
	}
}

static void
mem_printbotline(int l1, int l2, int total, int read, int size)
{
	extern int t_lines;
	static int n = 0;
	char *(s[4]) = {
	"结束 ← q | ↑↓ PgUp PgDn 移动",
		    "s 开头 | e 末尾 | b f 前后翻页",
		    "g 跳到某行 | ? / 上下搜索字符串",
		    "l n 上篇下篇 | R 回文 | E 评价"};
	n++;
	if (uinfo.mode == READING)
		n %= 4;
	else
		n %= 3;
	move(t_lines - 1, 0);
	prints
	    ("\033[1;44;32m%s (%d%%) 第(%d-%d)行 \033[33m| %s | h 辅助说明\033[m",
	     (read >= size) ? "看到末尾啦" : "下面还有喔",
	     total ? (100 * l2 / total) : (100 * read / size), l1, l2, s[n]);
}

static int
mem_more(char *ptr, int size, int quit, char *keystr, char *fn, char *title)
{
	extern int t_lines;
	struct MemMoreLines l;
	static char searchstr[30] = "";
	char buf[256];

	int i, ch = 0, curr_line, last_line, change;

	if (size <= 0)
		return 0;
	init_MemMoreLines(&l, ptr, size);

	prints("\033[m");
	while (1) {
		move(0, 0);
		clear();
		curr_line = l.curr_line;
		for (i = 0;;) {
			mem_printline(l.curr, l.currlen, fn, l.currty,&l);
			i++;
			if (i >= t_lines - 1)
				break;
			if (next_MemMoreLines(&l) < 0)
				break;
		}
		last_line = l.curr_line;
		if (l.total && l.total <= t_lines - 1)
			return 0;
		if (l.line[last_line % 100] - ptr + l.s[last_line % 100] == size
		    && (ch == KEY_RIGHT || ch == KEY_PGDN || ch == ' '
			|| ch == Ctrl('f'))) {
			move(t_lines - 1, 0);
			clrtobot();
			return 0;
		}
		change = 0;
		while (change == 0) {
			mem_printbotline(curr_line + 1, last_line + 1, l.total,
					 l.line[last_line % 100] - ptr +
					 l.s[last_line % 100], size);
			 l.line[l.curr_line% 100] - ptr + l.s[l.curr_line % 100],
			ch = egetch();
			move(t_lines - 1, 0);
			clrtoeol();
			switch (ch) {
			case KEY_UP:
			case 'u':
				change = -1;
				break;
			case KEY_DOWN:
			case 'd':
			case 'j':
			case '\n':
				change = 1;
				break;
			case 'b':
			case Ctrl('b'):
			case KEY_PGUP:
				change = -t_lines + 2;
				break;
			case ' ':
			case 'f':
			case Ctrl('f'):
			case KEY_PGDN:
			case KEY_RIGHT:
				if (!l.total)
					seek_MemMoreLines(&l,
							  last_line + t_lines);
				change = t_lines - 2;
				if (l.total && last_line < l.total
				    && curr_line + change + t_lines - 1 >
				    l.total)
					change =
					    l.total - curr_line - t_lines + 1 ;
				break;
			case 's':
				change = -curr_line;
				break;
			case 'e':
				if (!l.total) {
					while (next_MemMoreLines(&l) >= 0) ;
					curr_line = l.curr_line;
				} else
					curr_line = l.total - 1;
				change = -t_lines + 2;
				break;
			case 'g':
				getdata(t_lines - 1, 0, "跳转到的行号:", buf, 9,
					DOECHO, YEA);
				if (isdigit(buf[0])) {
					change = atoi(buf) - curr_line;
				}
				break;
			case '/':
			case '?':
				getdata(t_lines - 1, 0,
					ch ==
					'/' ? "向下查找字符串:" :
					"向上查找字符串:", searchstr, 29,
					DOECHO, NA);
				if (strlen(searchstr) > 0) {
					int i = curr_line;
					while (1) {
						if (ch == '/')
							i++;
						else
							i--;
						if (seek_MemMoreLines(&l, i) <
						    0)
							break;
						memcpy(buf, l.curr,
						       (l.currlen >=
							256) ? 255 : l.currlen);
						buf[(l.currlen >= 256) ? 255 :
						    l.currlen] = 0;
						if (strcasestr(buf, searchstr)
						    != NULL) {
							change = i - curr_line;
							break;
						}
					}
					if (change == 0) {
						move(t_lines - 1, 0);
						prints("没有找到呀...");
						continue;
					}
				}
				break;
			case KEY_LEFT:
			case 'q':
				return 0;
			case 'n':
				return KEY_DOWN;
			case 'l':
				return KEY_UP;
			case 'h':
				show_help("help/memmorehelp");
				curr_line += t_lines - 1;
				change = 1 - t_lines;
				break;
			case Ctrl('Y'):
				if (title) {
					zsend_file(fn, title);
					curr_line += t_lines - 1;
					change = 1 - t_lines;
				}
				break;
			default:
				if (keystr != NULL
				    && strchr(keystr, ch) != NULL)
					return ch;
			}
			if (change < 0 && curr_line == 0) {
				if (quit)
					return KEY_UP;
				change = 0;
			}
			if (change == 1) {
				if (seek_MemMoreLines
				    (&l, curr_line + t_lines - 1) >= 0) {
					curr_line++;
					last_line++;
					scroll();
					move(t_lines - 2, 0);
					clrtoeol();
					mem_printline(l.curr, l.currlen, fn,l.currty,&l);
					if ((ch == KEY_PGDN || ch == ' '
					     || ch == Ctrl('f')
					     || ch == KEY_RIGHT
					     || ch == KEY_DOWN || ch == 'j'
					     || ch == '\n')
					    && l.line[last_line % 100] - ptr +
					    l.s[last_line % 100] == size) {
						move(t_lines - 1, 0);
						clrtoeol();
						return 0;
					}
				} else
					return 0;
				change = 0;
			}
			if (change == -1) {
				if (seek_MemMoreLines(&l, curr_line - 1) >= 0) {
					curr_line--;
					last_line--;
					rscroll();
					move(0, 0);
					mem_printline(l.curr, l.currlen, fn,l.currty,&l);
				}
				change = 0;
			}
		}
		seek_MemMoreLines(&l, curr_line + change);
	}
}

int ansimore(char *filename, int promptend) {
	int ch;

	clear();
	ch = mmap_more(filename, 1, "RrEexp", NULL);
	if (promptend)
		pressanykey();
	move(t_lines - 1, 0);
	prints("\033[m\033[m");
	return ch;
}

int
ansimore_withzmodem(filename, promptend, title)
char *filename;
int promptend;
char *title;
{
	int ch;

	clear();
	ch = mmap_more(filename, 1, "RrEexp", title);
	if (promptend)
		pressanykey();
	move(t_lines - 1, 0);
	prints("\x1b[m\x1b[m");
	return ch;
}

int
ansimore2(filename, promptend, row, numlines)
char *filename;
int promptend;
int row;
int numlines;
{
	int ch;
	if (numlines)
		ch = mmap_show(filename, row, numlines);
	else
		ch = mmap_more(filename, 1, NULL, NULL);
	if (promptend)
		pressanykey();
	return ch;
}

static int ansimorestuff(char *filename, int promptend) {
	int retv;
	stuffmode = 1;
	retv = ansimore(filename, promptend);
	stuffmode = 0;
	return retv;
}

int
ansimore2stuff(filename, promptend, row, numlines)
char *filename;
int promptend;
int row;
int numlines;
{
	int retv;
	stuffmode = 1;
	retv = ansimore2(filename, promptend, row, numlines);
	stuffmode = 0;
	return retv;
}
