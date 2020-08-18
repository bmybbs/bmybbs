#ifndef __SECTREE_H
#define __SECTREE_H
#define MAXSUBSEC 20
#define MAXSECM 10
struct sectree {
	const struct sectree *parent;
	char basestr[20];
	char seccodes[MAXSUBSEC];
	char introstr[MAXSUBSEC + 1];
	const struct sectree *subsec[MAXSUBSEC];
	char title[30];
	char des[30];
	int nsubsec;
};

struct secmanager {
	char secstr[20];
	char secm[MAXSECM][20];
	int n;
};

extern const struct sectree sectree;
const struct sectree *getsectree(const char *str);
int gensecm(const char *txtfile);
struct secmanager *getsecm(const char *str);
int issecm(const char *str, const char *userid);
int issecm_strict(const char *str, const char *userid);
#endif
