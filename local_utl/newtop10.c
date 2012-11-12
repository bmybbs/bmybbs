#include "bbs.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <ght_hash_table.h>
#include "www.h"
#define TOPN 2
#define TOPFN "TOPN"
#define FILTERTITLE "etc/.filtertitle_new"

#define AREA_TOP_CNT	5	// 显示每个区的前TOP_AREA_CNT大热门话题
//#define AREA_CNT		16	// 共有AREA_CNT个区
#define LEN(Array)		(sizeof(Array)/sizeof(Array[0]))		
#define AREA_DIR		"etc/Area_Dir"	// 每个区的热门话题文件的存放目录

struct data_s {
	ght_hash_table_t *user_hash;
	struct boardtop bt;
};

int now_t;

struct boardtop *topten = NULL;
struct boardtop *ctopten = NULL;

char area[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'G', 'N', 'H', 'A', 'C'}; // 各区列表
struct boardtop **topten_area = NULL;
struct boardtop **ctopten_area = NULL;

int allflag = 0;
struct mmapfile filtermf = { ptr:NULL, size:0 };

int
cmpbt(struct boardtop *a, struct boardtop *b)
{
	return b->unum - a->unum;
}

int
filtertitle(char *tofilter)
{
	if(filtermf.ptr==NULL)
		return 0;
	return filter_string(tofilter, &filtermf);
}
 
int
trytoinsert(char *board, struct boardtop *bt)
{
	memcpy(topten + 10, bt, sizeof (struct boardtop));
	qsort(topten, 11, sizeof (struct boardtop), cmpbt);
	if (!filtertitle(bt->title) && !filtertitle(board)) {
		memcpy(ctopten + 10, bt, sizeof (struct boardtop));
		qsort(ctopten, 11, sizeof (struct boardtop), cmpbt);
	}
	return 0;
}

struct boardtop *
getTopTenBt(struct boardtop ** basebt, struct boardheader *bh)
{
	int i;
	for (i = 0; i < LEN(area); i++)
	{
		if (area[i] == bh->secnumber1)
		{
			return basebt[i];
		}
	}

	return NULL; 
}

int
trytoinsert_area(struct boardheader *bh, struct boardtop *bt)
{
	struct boardtop * bt1= getTopTenBt(topten_area, bh);
	struct boardtop * bt2= getTopTenBt(ctopten_area, bh);
	
	if (bt1 == NULL)
	{
		printf("bt1 is NULL!\n");
		exit(0);
	}
	if (bt2 == NULL)
	{
		printf("bt2 is NULL!\n");
		exit(0);
	}

	memcpy(bt1 + AREA_TOP_CNT, bt, sizeof (struct boardtop));
	qsort(bt1, AREA_TOP_CNT + 1, sizeof (struct boardtop), cmpbt);
	if (!filtertitle(bt->title) && !filtertitle(bh->filename)) {
		memcpy(bt2 + AREA_TOP_CNT, bt, sizeof (struct boardtop));
		qsort(bt2, AREA_TOP_CNT + 1, sizeof (struct boardtop), cmpbt);
	}
	return 0;
}

int
_topn(struct boardheader *bh)
{
	//从 bh 里面找到TOPN个thread
	int size = sizeof (struct fileheader), total;
	int i, j, k, *usernum;
	struct mmapfile mf;
	struct fileheader *ptr;
	ght_hash_table_t *p_table = NULL;
	ght_iterator_t iterator, iterator1;
	struct data_s *data;
	char owner[14];
	int start, tocount;
	struct boardtop *bt, *bt1;
	char topnfilename[80], tmpfn[80];
	FILE *fp;
	char DOTDIR[80];
	char path[80];
	char *real_title;

	if (bh->level!=0) //特殊版面
		return 0;

	if (bh->clubnum != 0)
		if (!(bh->flag & CLUBTYPE_FLAG))
			return 0;
	
	// 以下修改是为了让某些版面无法上10大，modified by interma 2005.11.24
	char buf[256];
	FILE *fp2;
	fp2 = fopen(MY_BBS_HOME "/etc/top10forbid", "r");
	if (fp2 != 0) 
	{
	while(fgets(buf, 256, fp2) != NULL)
	{
		buf[strlen(buf) - 1] = 0;
		if (strcmp(bh->filename, buf)==0)
		{
			return 0;
		}
	}
	fclose(fp2);
	}

	sprintf(DOTDIR, "boards/%s/.DIR", bh->filename);
	sprintf(path, "boards/%s", bh->filename);
	sprintf(topnfilename, "%s/%s", path, TOPFN);
	unlink(topnfilename);
	MMAP_TRY {
	if (mmapfile(DOTDIR, &mf) == -1) {
		errlog("open .DIR error, %s\n", DOTDIR);
		MMAP_RETURN(0);
	}
	if (mf.size == 0) {	//空版面
		mmapfile(NULL, &mf);
		MMAP_RETURN(0);
	}
	total = mf.size / size - 1;	//共 0 - total 条目
	start = Search_Bin(mf.ptr, now_t - 86400, 0, total);
	if (start < 0) {
		start = -(start + 1);
	}
	if (start > total) {	//没有新文章
		mmapfile(NULL, &mf);
		MMAP_RETURN(0);
	}
	tocount = total - start + 1;
	bt = malloc(sizeof (struct boardtop) * tocount);
	if (bt == NULL) {
		errlog("malloc failed");
		exit(-1);
	}
	p_table = ght_create(tocount, NULL, GHT_HEURISTICS_MOVE_TO_FRONT);
	for (i = start; i <= total; i++) {
		ptr =
		    (struct fileheader *) (mf.ptr +
					   i * sizeof (struct fileheader));
		if ((data = ght_get(p_table, sizeof (int), &(ptr->thread))) ==
		    NULL) {
			if ((data = malloc(sizeof (struct data_s))) == NULL) {
				errlog("malloc failed");
				exit(-1);
			}
			if ((usernum = malloc(sizeof (int))) == NULL) {
				errlog("malloc failed");
				exit(-1);
			}
			real_title = ptr->title;
			if (!strncmp(real_title, "Re: ", 4))
				real_title += 4;
			strncpy(data->bt.title, real_title, 60);
			data->user_hash = NULL;
			data->user_hash =
			    ght_create(tocount, NULL,
				       GHT_HEURISTICS_MOVE_TO_FRONT);
			data->bt.unum = 1;
			(data->bt.lasttime) = ptr->filetime;
			data->bt.thread = ptr->thread;
			strncpy(data->bt.board, bh->filename, 24);
			strncpy(owner, fh2realauthor(ptr), 14);
			strncpy(data->bt.firstowner, fh2owner(ptr), 14);
			*usernum = 0;
			ght_insert(data->user_hash, usernum,
				   sizeof (char) * strlen(owner), owner);
			ght_insert(p_table, data, sizeof (int), &(ptr->thread));
		} else {
			strncpy(owner, fh2realauthor(ptr), 14);
			if (
			    (usernum =
			     ght_get(data->user_hash,
				     sizeof (char) * strlen(owner),
				     owner)) == NULL) {
				(data->bt.unum)++;
				(data->bt.lasttime) = ptr->filetime;
				if ((usernum = malloc(sizeof (int))) == NULL) {
					errlog("malloc failed");
					exit(-1);
				}
				ght_insert(data->user_hash, usernum,
					   sizeof (char) * strlen(owner),
					   owner);
			} else {
				(*usernum)++;
			}
		}
	}
	}
	MMAP_CATCH {
		mmapfile(NULL, &mf);
		MMAP_RETURN(0);
	}
	MMAP_END mmapfile(NULL, &mf);
	i = 0;
	bt1 = bt;
	for (data = ght_first(p_table, &iterator); data;
	     data = ght_next(p_table, &iterator)) {
		memcpy(bt1, &(data->bt), sizeof (struct boardtop));
		bt1++;
		i++;
		for (usernum = ght_first(data->user_hash, &iterator1); usernum;
		     usernum = ght_next(data->user_hash, &iterator1)) {
			free(usernum);
		}
		ght_finalize(data->user_hash);
		data->user_hash = NULL;
		free(data);
	}
	ght_finalize(p_table);
	p_table = NULL;
	qsort(bt, i, sizeof (struct boardtop), cmpbt);
	sprintf(tmpfn, "%s/topntmp", path);
	fp = fopen(tmpfn, "w");
	if (!fp) {
		errlog("touch %s error", tmpfn);
		exit(1);
	}
	for (j = 0, k = 0, bt1 = bt; j < TOPN && j < i; j++, bt1++) {
		if (allflag)
		{
			trytoinsert_area(bh, bt1);
			trytoinsert(bh->filename, bt1);
		}
		if (bt1->unum < 15)
			continue;
		if (k == 0) {
			//fprintf(fp, "<table width=80%%>");
			fprintf(fp, "<table width=80%% align=\"center\" > ");
		}
		k++;
		fprintf(fp, "<tr><td><font color=red>HOT</font></td><td><a href=tfind?B=%s&th=%d&T=%s>%s", bh->filename, bt1->thread, encode_url(bt1->title), nohtml(bt1->title));
		fprintf(fp, "</a></td><td>[讨论人数:%d] </td></tr>\n ", bt1->unum);
	}
	if (k)
		fprintf(fp, "</table>");
	fclose(fp);
	if (k)
		rename(tmpfn, topnfilename);
	free(bt);
	return 0;
}

int
top_all_dir()
{
	return new_apply_record(".BOARDS", sizeof (struct boardheader),
				_topn, NULL);
}

int
html_topten(int mode, char *file)
{	//modify by mintbaggio  20040829 for new www
	struct boardtop *bt;
	int j;
	FILE *fp;
	if (mode)
		bt = ctopten;
	else
		bt = topten;
	fp = fopen("wwwtmp/topten.tmp", "w");
	if (!fp) {
		errlog("topten write error");
		exit(1);
	}
	fprintf(fp, "<body><center><div class=rhead>%s --<span class=h11> 今日十大热门话题</span></div>\n<hr>\n",
		MY_BBS_NAME);
	fprintf(fp, "<table border=1>\n");
	fprintf
	    (fp,
	     "<tr><td>名次</td><td>讨论区</td><td>标题</td><td>人数</td></tr>\n");
	for (j = 0; j < 10 && bt->unum != 0; j++, bt++) {
		fprintf
		    (fp,
		     "<tr><td>第 %d 名</td><td><a href=tdoc?board=%s>%s</a></td><td><a href='bbstfind?board=%s&th=%d&title=%s'>%42.42s</a></td><td>%d</td></tr>\n",
		     j + 1, bt->board, bt->board, bt->board, bt->thread,
		     encode_url(bt->title), void1(nohtml(bt->title)),bt->unum);
	}
	fprintf(fp, "</table><center></body>");
	fclose(fp);
	rename("wwwtmp/topten.tmp", file);

	// 生成各区热门文章的文件, added by interma, 2006-09-11
	struct boardtop **bt_area;
	if (mode)
		bt_area = ctopten_area;
	else
		bt_area = topten_area;

	int i;
	for (i = 0; i < LEN(area); i++)
	{
		bt = bt_area[i];
		char path[256];
		sprintf(path, AREA_DIR "/%c", area[i]);
		fp = fopen(path, "w");
		fprintf(fp, "<table width=90%>");
		for (j = 0; j < AREA_TOP_CNT && bt->unum != 0; j++, bt++) 
		{
			fprintf(fp, "<tr><td width=120px>[<a href=tdoc?board=%s>%s</a>]</td><td><a href='bbstfind?board=%s&th=%d&title=%s'>%42.42s</a></td><td width=20px>(%d)</td></tr>", 
				bt->board, bt->board, bt->board, bt->thread, encode_url(bt->title), void1(nohtml(bt->title)),bt->unum);
		}
		fprintf(fp, "</table>");
		fclose(fp);
	}

	return 0;
}

int
index_topten(int mode, char *file)
{
        struct boardtop *bt;
        int j;
        FILE *fp;
        if (mode)
                bt = ctopten;
        else
                bt = topten;
        fp = fopen("wwwtmp/indextopten.tmp", "w");
        if (!fp) {
                errlog("topten write error");
                exit(1);
        }
        for (j = 0; j < 10 && bt->unum != 0; j++, bt++) {
                fprintf
                    (fp,
		     "<tr><td><span class=\"smalltext\">%d</span></td><td><a href='bbstfind?board=%s&th=%d&title=%s'>%42.42s</a></td></tr>\n",	
                     j + 1, bt->board, bt->thread,
                     encode_url(bt->title), void1(nohtml(bt->title)));
        }
        fclose(fp);
        rename("wwwtmp/indextopten.tmp", file);
        return 0;
}

int
telnet_topten(int mode, char *file)
{
	struct boardtop *bt;
	int j;
	FILE *fp, *fp2;
	char buf[40];
	char *p;
	if (mode)
		bt = ctopten;
	else
		bt = topten;
	fp = fopen("etc/topten.tmp", "w");
	fp2 = fopen("etc/dayf_index", "w");	// 建立这个文件是为了实现telnet下边直接看10大，modified by interma@BMY 2005.6.25
	
	if (!fp) {
		errlog("topten write error");
		exit(1);
	}

	fprintf(fp,
		"                \033[1;34m-----\033[37m=====\033[41m 本日十大热门话题 \033[40m=====\033[34m-----\033[0m\n\n");
	for (j = 0; j < 10 && bt->unum != 0; j++, bt++) {
		strcpy(buf, ctime(&(bt->lasttime)));
		buf[20] = '\0';
		p = buf + 4;
		fprintf(fp,
			"\033[1;37m第\033[1;31m%3d\033[37m 名 \033[37m信区 : \033[33m%-16s\033[37m【\033[32m%s\033[37m】\033[36m%4d\033[37m 人\n     \033[37m标题 : \033[1;44;37m%-60.60s\033[40m\n",
			j + 1, bt->board, p, bt->unum,bt->title);

		fprintf(fp2, "%s\n%s\n", bt->board, bt->title);	
	}
	fclose(fp);
	fclose(fp2);
	
	rename("etc/topten.tmp", file);
	return 0;
}

int
main(int argc, char **argv)
{
	struct boardheader bh;
	char *name;
	while (1) {
		int c;
		c = getopt(argc, argv, "ah");
		if (c == -1)
			break;
		switch (c) {
		case 'a':
			allflag = 1;
			break;
		case 'h':
			printf
			    ("%s [-a|boardname]\n  do board top %d article\n",
			     argv[0], TOPN);
			return 0;
		case '?':
			printf
			    ("%s:Unknown argument.\nTry `%s -h' for more information.\n",
			     argv[0], argv[0]);
			return 0;
		}
	}
	chdir(MY_BBS_HOME);
	now_t = time(NULL);
	if (optind < argc) {
		name = argv[optind++];
		if (optind < argc) {
			printf
			    ("%s:Too many arguments.\nTry `%s -h' for more information.\n",
			     argv[0], argv[0]);
			return 0;
		}
		strncpy(bh.filename, name, STRLEN);
		_topn(&bh);
	}
	if (allflag) {
		topten = calloc(11, sizeof (struct boardtop));
		if (NULL == topten) {
			errlog("malloc failed");
			exit(1);
		}
		ctopten = calloc(11, sizeof (struct boardtop));
		if (NULL == ctopten) {
			errlog("malloc failed");
			exit(1);
		}
		
		int area_cnt = LEN(area);
		topten_area = calloc(area_cnt, sizeof (struct boardtop *));
		ctopten_area = calloc(area_cnt, sizeof (struct boardtop *));
		int i;
		for (i = 0; i < area_cnt; i++)
		{
			topten_area[i] = calloc(AREA_TOP_CNT + 1, sizeof (struct boardtop));
			ctopten_area[i] = calloc(AREA_TOP_CNT + 1, sizeof (struct boardtop));
		}

		if (!file_exist(AREA_DIR))
		{
			mode_t mt = S_ISUID | S_IRWXU | S_IRGRP | S_IROTH;
			if (mkdir(AREA_DIR, mt) == -1)
			{
				errlog("create " AREA_DIR " failed");
				exit(1);
			}
		}

		if (file_exist(FILTERTITLE)) {
			if (mmapfile(FILTERTITLE, &filtermf) == -1) {
				errlog("mmap failed");
				exit(1);
			}
		}

		top_all_dir();
		telnet_topten(0, "etc/posts/day");
		telnet_topten(1, "etc/dayf");
		html_topten(0, "wwwtmp/topten");
		html_topten(1, "wwwtmp/ctopten");
		index_topten(0, "wwwtmp/indextopten");
        index_topten(1, "wwwtmp/cindextopten");
		//要exit了，我就不free了，呵呵
	}
	return 0;
}

