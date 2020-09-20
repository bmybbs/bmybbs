#include "ythtbbs.h"
struct action_f {
	const char *action;
	void (*f) (int, char *, char *, char *);
};
extern char *timeperiod;
extern struct BCACHE *brdshm;
extern struct boardmem *bcache;
int register_stat(struct action_f *mod, void (*mod_exit) (void));
//void errlog (char *fmt, ...);
int boardnoread(struct boardheader *);
