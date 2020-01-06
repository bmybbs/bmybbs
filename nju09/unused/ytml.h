#ifndef _YTML_H
#define _YTML_H
#define ARGC_MAX 10
#define CONTENT_FILE_MAX 256
#define USE_HASH_LIMIT 10
#define C_FILES_LIST "etc/c_files_list"
typedef char *(*function) (int argc, char *argv[]);

struct func_applet {
	function func;
	char *(name[5]);
	double utime;
	double stime;
	int count;
};

struct content_file {
	struct mmapfile mf;
	char *id;
	char *path;
};

struct content_files {
	struct content_file *cf;
	char *config;
	time_t mtime;
	int nums;
	int max;
#if defined(ENABLE_GHT_HASH) && defined(ENABLE_FASTCGI)
	ght_hash_table_t *p_table;
#endif
};

struct a2i {
	const char *str;
	const int id;
};

extern struct func_applet funcs[];

#define SUCCESS "success"
#define FAULT	"error"

#define check_argc(i) \
	if(argc!=(i)) { \
		printf("函数 %s 参数个数不对,应该有%d个!",argv[0],i-1); \
		return FAULT; \
	} else {;}

#define NAV_MAXOB 3
#define NAV_MAX 90
#define NAV_DAY 3.8
#define HOT_MAX 16
#define LM_MAXL 10
#define LM_MAXLPRINT 7
#define LM_MAXLRAND 3
#define LM_MAXLNRAND (LM_MAXLPRINT -LM_MAXLRAND)
#define LM_MAXFILTER 100

struct b_mark {
	int bno;
	char title[STRLEN];
	int thread;
};
struct g_mark {
	struct b_mark bmark[LM_MAXL];
	int n;
};
struct c_mark {
	struct g_mark gmark[MAXGROUP];
};
#endif
