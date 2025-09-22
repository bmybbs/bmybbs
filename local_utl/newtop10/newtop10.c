#include "bbs.h"

#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "3rd/uthash.h"
#include "www.h"
#define TOPN 2
#define TOPFN "TOPN"
#define FILTERTITLE "etc/.filtertitle_new"

#define AREA_TOP_CNT	5	// ��ʾÿ������ǰTOP_AREA_CNT�����Ż���
//#define AREA_CNT		16	// ����AREA_CNT����
#define LEN(Array)		(sizeof(Array)/sizeof(Array[0]))
#define AREA_DIR		"etc/Area_Dir"	// ÿ���������Ż����ļ��Ĵ��Ŀ¼

struct user_kv {
	char *key; // malloc ������
	int num;   // ���ִ���
	UT_hash_handle hh;
};

struct data_s {
	struct user_kv *user_hash;
	struct boardtop bt;
};

struct thread_kv {
	int thread;          // key
	struct data_s *data; // value
	UT_hash_handle hh;
};

time_t now_t;

struct boardtop *topten = NULL;
struct boardtop *ctopten = NULL;

char area[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'G', 'N', 'H', 'A', 'C'}; // �����б�
struct boardtop **topten_area = NULL;
struct boardtop **ctopten_area = NULL;

const char * TDSTYLE = "<style type=\"text/css\">.td-overflow { width: 236px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;}</style>";
const char * BDSTYLE = "<style type=\"text/css\">.bd-overflow { width: 500px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;}</style>";

int allflag = 0;
struct mmapfile filtermf = { .ptr = NULL, .size = 0 };

extern int postfile(char *filename, char *owner, char *nboard, char *posttitle);

int
cmpbt(const void *aa, const void *bb)
{
	struct boardtop *a = (struct boardtop *)aa;
	struct boardtop *b = (struct boardtop *)bb;
	return b->unum - a->unum;
}

int
filtertitle(char *tofilter)
{
	if(filtermf.ptr==NULL)
		return 0;
	return ytht_smth_filter_string(tofilter, &filtermf);
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
	size_t i;
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
_topn(void *bh_void, void * fargs)
{
	struct boardheader *bh = (struct boardheader *) bh_void;
	//�� bh �����ҵ�TOPN��thread
	int size = sizeof (struct fileheader), total;
	int i, j, k, *usernum;
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *ptr;
	struct thread_kv *p_table = NULL;
	struct thread_kv *tk, *tmp_tk;
	struct data_s *data;
	char owner[14];
	int start, tocount;
	struct boardtop *bt, *bt1;
	char topnfilename[160], tmpfn[160];
	FILE *fp;
	char DOTDIR[80];
	char path[80];
	char *real_title;

	if (bh->level!=0) //�������
		return 0;

	if (bh->clubnum != 0)
		if (!(bh->flag & CLUBTYPE_FLAG))
			return 0;

	// �����޸���Ϊ����ĳЩ�����޷���10��modified by interma 2005.11.24
	char buf[256];
	FILE *fp2;
	fp2 = fopen(MY_BBS_HOME "/etc/top10forbid", "r");
	if (fp2 != NULL) {
		while(fgets(buf, 256, fp2) != NULL) {
			buf[strlen(buf) - 1] = 0;
			if (strcmp(bh->filename, buf)==0) {
				fclose(fp2);
				return 0;
			}
		}
		fclose(fp2);
	}

	sprintf(DOTDIR, "boards/%s/.DIR", bh->filename);
	sprintf(path, "boards/%s", bh->filename);
	snprintf(topnfilename, sizeof(topnfilename), "%s/%s", path, TOPFN);
	unlink(topnfilename);
	MMAP_TRY {
		if (mmapfile(DOTDIR, &mf) == -1) {
			errlog("open .DIR error, %s\n", DOTDIR);
			MMAP_RETURN(0);
		}
		if (mf.size == 0) {	//�հ���
			mmapfile(NULL, &mf);
			MMAP_RETURN(0);
		}
		total = mf.size / size - 1;	//�� 0 - total ��Ŀ
		start = Search_Bin(mf.ptr, now_t - 86400, 0, total);
		if (start < 0) {
			start = -(start + 1);
		}
		if (start > total) {	//û��������
			mmapfile(NULL, &mf);
			MMAP_RETURN(0);
		}
		tocount = total - start + 1;
		bt = malloc(sizeof (struct boardtop) * tocount);
		if (bt == NULL) {
			errlog("malloc failed");
			exit(-1);
		}
		for (i = start; i <= total; i++) {
			ptr = (struct fileheader *) (mf.ptr + i * sizeof (struct fileheader));
			if(ptr->accessed & FH_ISWATER)	// ˮ������������
				continue;

			HASH_FIND(hh, p_table, &(ptr->thread), sizeof(ptr->thread), tk);
			data = tk ? tk->data : NULL;
			if (data == NULL) {
				if(ptr->thread != ptr->filetime) { // ���ⲻ���ڣ����Ҹ�ƪ���� id �����ⲻ��ͬ����Ϊ����
					// ���� ptr->thread ��Ѱ����
					int th_num = Search_Bin(mf.ptr, ptr->thread, 0, total);
					if (th_num < 0) {
						th_num = -(th_num+1);
					}
					if (th_num > total) {
						th_num = total;
					}
					struct fileheader * th_ptr = (struct fileheader*) (mf.ptr + th_num * sizeof(struct fileheader));

					// ���ԭ���Ѿ�ɾ��
					if (th_ptr->filetime != ptr->thread)
						continue;

					// ���ԭ������ע y
					if (th_ptr->accessed & FH_ISWATER)
						continue;
				}

				if ((data = malloc(sizeof (struct data_s))) == NULL) {
					errlog("malloc failed");
					exit(-1);
				}
				real_title = ptr->title;
				if (!strncmp(real_title, "Re: ", 4))
					real_title += 4;
				strncpy(data->bt.title, real_title, 60);
				data->user_hash = NULL;
				data->bt.unum = 1;
				(data->bt.lasttime) = ptr->filetime;
				data->bt.thread = ptr->thread;
				strncpy(data->bt.board, bh->filename, 24);
				strncpy(owner, fh2realauthor(ptr), 14);
				owner[sizeof(owner) - 1] = 0;
				strncpy(data->bt.firstowner, fh2owner(ptr), 14);
				*usernum = 0;
				{
					struct user_kv *uk = malloc(sizeof(*uk));
					if (!uk) {
						errlog("malloc failed");
						exit(-1);
					}
					uk->key = strdup(owner);
					if (!uk->key) {
						errlog("malloc failed");
						exit(-1);
					}
					uk->num = 0;
					HASH_ADD_KEYPTR(hh, data->user_hash, uk->key, strlen(uk->key), uk);
				}
				tk = malloc(sizeof(*tk));
				if (!tk) {
					errlog("malloc failed");
					exit(-1);
				}
				tk->thread = ptr->thread;
				tk->data = data;
				HASH_ADD(hh, p_table, thread, sizeof(tk->thread), tk);
			} else {
				strncpy(owner, fh2realauthor(ptr), 14);
				struct user_kv *uk = NULL;
				HASH_FIND(hh, data->user_hash, owner, strlen(owner), uk);
				if (uk == NULL) {
					(data->bt.unum)++;
					(data->bt.lasttime) = ptr->filetime;
					uk = malloc(sizeof(*uk));
					if (!uk) {
						errlog("malloc failed");
						exit(-1);
					}
					uk->key = strdup(owner);
					if (!uk->key) {
						errlog("malloc failed");
						exit(-1);
					}
					uk->num = 0;
					HASH_ADD_KEYPTR(hh, data->user_hash, uk->key, strlen(uk->key), uk);
				} else {
					uk->num++;
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
	HASH_ITER(hh, p_table, tk, tmp_tk) {
		memcpy(bt1, &(data->bt), sizeof (struct boardtop));
		bt1++;
		i++;
		{
			struct user_kv *uk, *tmp_uk;
			HASH_ITER(hh, data->user_hash, uk, tmp_uk) {
				HASH_DEL(data->user_hash, uk);
				free(uk->key);
				free(uk);
			}
			data->user_hash = NULL;
		}
		free(data);
		HASH_DEL(p_table, tk);
		free(tk);
	}
	p_table = NULL;
	qsort(bt, i, sizeof (struct boardtop), cmpbt);
	snprintf(tmpfn, sizeof(tmpfn), "%s/topntmp", path);
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
			fprintf(fp, "<table width='80%%' align=\"center\" > ");
		}
		k++;
		fprintf(fp, "<tr><td><font color='red'>HOT</font></td><td><a href='tfind?B=%s&amp;th=%ld'>%s", bh->filename, bt1->thread, nohtml(bt1->title));
		fprintf(fp, "</a></td><td>[��������:%d] </td></tr>\n ", bt1->unum);
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
	return new_apply_record(".BOARDS", sizeof (struct boardheader), _topn, NULL);
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
	fprintf(fp, "%s<body><center><div class='rhead'>%s --<span class='h11'> ����ʮ�����Ż���</span></div>\n<hr>\n",
		TDSTYLE, MY_BBS_NAME);
	fprintf(fp, "<table border='1'>\n");
	fprintf(fp, "<tr><td>����</td><td>������</td><td>����</td><td>����</td></tr>\n");
	for (j = 0; j < 10 && bt->unum != 0; j++, bt++) {
		fprintf(fp, "<tr><td>�� %d ��</td><td><a href='tdoc?board=%s'>%s</a></td><td><div class='td-overflow'><a href='tfind?board=%s&amp;th=%ld' title='%s'>%s</a></div></td><td>%d</td></tr>\n", j + 1, bt->board, bt->board, bt->board, bt->thread, void1(nohtml(bt->title)), void1(nohtml(bt->title)),bt->unum);
	}
	fprintf(fp, "</table></center></body>");
	fclose(fp);
	rename("wwwtmp/topten.tmp", file);

	// ���ɸ����������µ��ļ�, added by interma, 2006-09-11
	struct boardtop **bt_area;
	if (mode)
		bt_area = ctopten_area;
	else
		bt_area = topten_area;

	size_t i;
	for (i = 0; i < LEN(area); i++)
	{
		bt = bt_area[i];
		char path[256];
		snprintf(path, sizeof path, AREA_DIR "/%c", area[i]);
		fp = fopen(path, "w");
		fprintf(fp, "%s<table width='90%%'>", BDSTYLE);
		for (j = 0; j < AREA_TOP_CNT && bt->unum != 0; j++, bt++)
		{
			fprintf(fp, "<tr><td width='120px'>[<a href='tdoc?board=%s'>%s</a>]</td><td><div class='bd-overflow'><a href='tfind?board=%s&amp;th=%ld' title='%s'>%s</a></div></td><td width='20px'>(%d)</td></tr>",
				bt->board, bt->board, bt->board, bt->thread, void1(nohtml(bt->title)), void1(nohtml(bt->title)),bt->unum);
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
	fprintf(fp, "%s", TDSTYLE);
	for (j = 0; j < 10 && bt->unum != 0; j++, bt++) {
		fprintf(fp, "<tr><td><span class=\"smalltext\">%d</span></td><td><div class='td-overflow'><a href='tfind?board=%s&th=%ld' title='%s'>%s</a></div></td></tr>\n", j + 1, bt->board, bt->thread, void1(nohtml(bt->title)), void1(nohtml(bt->title)));
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
	fp2 = fopen("etc/dayf_index", "w");	// ��������ļ���Ϊ��ʵ��telnet�±�ֱ�ӿ�10��modified by interma@BMY 2005.6.25

	if (!fp) {
		errlog("topten write error");
		exit(1);
	}

	fprintf(fp,
		"                \033[1;34m-----\033[37m=====\033[41m ����ʮ�����Ż��� \033[40m=====\033[34m-----\033[0m\n\n");
	for (j = 0; j < 10 && bt->unum != 0; j++, bt++) {
		strcpy(buf, ctime(&(bt->lasttime)));
		buf[20] = '\0';
		p = buf + 4;
		fprintf(fp,
			"\033[1;37m��\033[1;31m%3d\033[37m �� \033[37m���� : \033[33m%-16s\033[37m��\033[32m%s\033[37m��\033[36m%4d\033[37m ��\n     \033[37m���� : \033[1;44;37m%-60.60s\033[40m\n",
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
			printf("%s [-a|boardname]\n  do board top %d article\n", argv[0], TOPN);
			return 0;
		case '?':
			printf("%s:Unknown argument.\nTry `%s -h' for more information.\n", argv[0], argv[0]);
			return 0;
		}
	}
	chdir(MY_BBS_HOME);
	now_t = time(NULL);
	if (optind < argc) {
		name = argv[optind++];
		if (optind < argc) {
			printf("%s:Too many arguments.\nTry `%s -h' for more information.\n", argv[0], argv[0]);
			return 0;
		}
		ytht_strsncpy(bh.filename, name, sizeof(bh.filename));
		_topn(&bh, NULL);
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
		time_t tt=time(0);
		char title[128];
		strcpy(title,  asctime(localtime(&tt)));
		char* p=strchr(title, '\n');
		*p='\0';
		postfile("/home/bbs/etc/posts/day", "XJTU-XANET", "TopTen", title);
		//Ҫexit�ˣ��ҾͲ�free�ˣ��Ǻ�
	}
	return 0;
}

