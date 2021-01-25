#include "bbs.h"
#include "config.h"
#include "nbstat.h"

char *timeperiod;

extern void bm_init();
extern void bu_init();
extern void bs_init();
extern void bbslists_init();

int
boardnoread(struct boardheader *fptr)
{

	return (((fptr->level != 0)
		&& !(fptr->level & PERM_NOZAP || fptr->level & PERM_POSTMASK))
		|| ((fptr->clubnum != 0) && (!(fptr->flag & CLUBTYPE_FLAG))));
}

void
usage(char *progname)
{
	printf("Usage:\n%s action-string days [endday]\n", progname);
	printf("actions:'m' board manager stat\n");
	printf("\t'u' board usage stat\n");
	printf("\t's' calculate board score, must be the unique action\n");
	printf("\t'b' bbslists stat, runs every hour, day must be 1\n");
	exit(1);
}

struct f_link {
	void (*f) ();
	struct f_link *next;
};

struct s_a {
	const char *action;
	struct f_link *fl;
};

struct s_a af_table[] = {
	{"post", NULL},
	{"use", NULL},
	{"import", NULL},
	{"additem", NULL},
	{"moveitem", NULL},
	{"paste", NULL},
	{"undigest", NULL},
	{"digest", NULL},
	{"mark", NULL},
	{"unmark", NULL},
	{"ranged", NULL},
	{"del", NULL},
	{"deny", NULL},
	{"sametitle", NULL},
	{"select", NULL},
	{"enter", NULL},
	{"drop", NULL},
	{"exitbbs", NULL},
	{"newaccount", NULL},
	{"kill", NULL},
	{"reload", NULL},
	{"kick", NULL},
	{"changetitle", NULL},
	{"edit", NULL},
	{"crosspost", NULL},
	{"talk", NULL},
	{"five", NULL},
	{"undel", NULL},
	{"rangedmail", NULL},
	{"bbsfind", NULL},
	{"finddf", NULL},
	{"mail", NULL},
	{"netmail", NULL},
	{"thread", NULL},
	{"exec", NULL},
	{"sendgoodwish", NULL},
	{"check1984", NULL},
	{0}
};

struct f_link *exit_fl = NULL;

struct f_link *
register_fun(struct f_link *fhead, void (*f) ())
{
	struct f_link *nfl;
	nfl = (struct f_link *) malloc(sizeof (struct f_link));
	if (nfl == NULL)
		return NULL;
	nfl->f = f;
	nfl->next = fhead;
	return nfl;
}

int
register_log(const char *action, void (*f) ())
{
	struct s_a *as;
	for (as = af_table; as->action != NULL; as++)
		if (!strcmp(as->action, action)) {
			as->fl = register_fun(as->fl, f);
			if (as->fl == NULL)
				return -1;
			return 0;
		}
	return -2;
}

int
register_stat(struct action_f *mod, void (*mod_exit) ())
{
	int ret;
	while (mod->action != NULL) {
		if ((ret = register_log(mod->action, mod->f))) {
			errlog("Can't register %s %d", mod->action, ret);
			return ret;
		}
		mod++;
	}
	if ((exit_fl = register_fun(exit_fl, mod_exit)) == NULL) {
		errlog("Can't regiester out %d", ret);
		return -3;
	}
	return 0;
}

void
parse(int day, char *buf)
{
	char user[30], time[30], action[30], other[512];
	char *tmp[4] = { time, user, action, other };
	int i;
	struct s_a *as;
	i = ytht_strtok(buf, ' ', tmp, 4);
	if (i < 3) {
		errlog("Error input %d %s", day, buf);
		return;
	}
	for (as = af_table; as->action != NULL; as++)
		if (!strcmp(as->action, action)) {
			struct f_link *flist = as->fl;
			while (flist != NULL) {
				(*(flist->f)) (day, time, user, other);
				flist = flist->next;
			}
			return;
		}
	errlog("Unknown action %s", action);
}

int
main(int argc, char *argv[])
{
	time_t now;
	int i, days, end;
	FILE *fp;
	struct tm *n;
	char file[256], buf[512];
	char target[10];
	if (argc <= 2)
		usage(argv[0]);
	strncpy(target, argv[1], 10);
	target[sizeof(target) - 1] = 0;
	if (strchr(target, 's') || strchr(target, 'b')) {
		if (strlen(target) > 1)
			usage(argv[0]);
	} else if (!strchr(target, 'u') && !strchr(target, 'm'))
		usage(argv[0]);
	days = atoi(argv[2]);
	if (argc > 3)
		end = atoi(argv[3]);
	else
		end = 1;
	if (days <= 0 || end <= 0)
		usage(argv[0]);
	if (strchr(target, 'b')) {
		if (days != 1)
			usage(argv[0]);
		else
			end = 0;
	}

	ythtbbs_cache_Board_resolve();

	if (strchr(target, 'm'))
		bm_init();
	if (strchr(target, 'u'))
		bu_init();
	if (strchr(target, 's'))
		bs_init();
	if (strchr(target, 'b'))
		bbslists_init();
	now = time(NULL) - end * 86400;
	for (i = 0; i < days; i++) {
		n = localtime(&now);
		sprintf(file, MY_BBS_HOME "/newtrace/%d-%02d-%02d.log", 1900 + n->tm_year,
			1 + n->tm_mon, n->tm_mday);
		fp = fopen(file, "r");
		if (fp == NULL) {
			errlog("Can not open file %s", file);
			now -= 86400;
			continue;
		}
		while (fgets(buf, 500, fp) != NULL) {
			parse(i, buf);
		}
		fclose(fp);
		now -= 86400;
	}
	timeperiod = malloc(STRLEN);
	if (timeperiod == NULL) {
		errlog("Can't malloc timeperiod");
		exit(-1);
	}
	now += 86400;
	n = localtime(&now);
	sprintf(timeperiod, "%d-%02d-%02d", 1900 + n->tm_year, 1 + n->tm_mon,
		n->tm_mday);
	now += 86400 * (days - 1);
	n = localtime(&now);
	sprintf(timeperiod + 10, " µ½ %d-%02d-%02d", 1900 + n->tm_year,
		1 + n->tm_mon, n->tm_mday);

	while (exit_fl != NULL) {
		(*(exit_fl->f)) ();
		exit_fl = exit_fl->next;
	}

	return 0;
}

