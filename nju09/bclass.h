#ifndef _BCLASS_H
#define _BCLASS_H
#define N_LEN 20
#define MAXCLASS 10
#define MAXGROUP 20
#define MAXB	1000
#define CLASSES_CONFIG "etc/classes"

struct oneboard {
	unsigned short bno;
	unsigned short advice:1, nav_day:5, nav_max:5, unuse:5;
};

struct onegroup {
	char gname[N_LEN];
	unsigned short start;
	unsigned short bcount;
};

struct oneclass {
	char cname[N_LEN];
	struct onegroup group[MAXGROUP];
	unsigned short gcount;
	unsigned short bcount;
};

struct classes {
	struct oneclass class[MAXCLASS];
	int ccount;
	time_t uptime;
	struct oneboard bdata[MAXB];
};

extern struct classes *cl;
#endif
