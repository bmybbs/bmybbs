#include "bbslib.h"

struct classes *cl = NULL;

static void
parse_1arg(char *buf, struct oneboard *b)
{
	char *add;
	add = strchr(buf, ':');
	if (NULL == add) {
		if (!strcmp(buf, "advice"))
			b->advice = 1;
		return;
	}
	*(add++) = 0;
	if (!strcmp(buf, "nav_max"))
		b->nav_max = atoi(add);
	else if (!strcmp(buf, "nav_day"))
		b->nav_day = atoi(add);
	return;
}

static int
parse_boardarg(char *buf, struct oneboard *b)
{
	char *arg[3];
	struct boardmem *x;
	int i;
	for (i = 0; i < 3; i++) {
		arg[i] = strchr(buf, ' ');
		if (NULL == arg[i])
			break;
		*(arg[i]++) = 0;
	}
	x = getbcache(buf);
	if (!x)
		return -1;
	b->bno = x - (struct boardmem *) shm_bcache;
	for (i = 0; arg[i]; i++)
		parse_1arg(arg[i], b);
	return 0;
}

static void
load_default_class(int *bnum)
{
	int i, j;
	struct boardmem *x;
#if ( SECNUM > MAXGROUP )
	can not compile this, change SECNUM or MAXGROUP
#endif
	 strcpy(cl->class[0].cname, "all");
	for (i = 0; i < SECNUM; i++) {
		snprintf(cl->class[0].group[i].gname, N_LEN, "%s",
			 secname[i][0]);
		cl->class[0].group[i].start = *bnum;
		for (j = 0; j < MAXBOARD && j < shm_bcache->number; j++) {
			x = &(shm_bcache->bcache[j]);
			if (x->header.filename[0] <= 32
			    || x->header.filename[0] > 'z')
				continue;
			if (seccodes[i] != x->header.secnumber1
			    && seccodes[i] != x->header.secnumber2)
				continue;
			cl->bdata[*bnum].bno = j;
			//special code for PKU and triangle
			if (!strcmp(x->header.filename, "triangle")) {
				cl->bdata[*bnum].nav_max = 5;
				cl->bdata[*bnum].nav_day = 2;
			}
			if (!strcmp(x->header.filename, "PKU"))
				cl->bdata[*bnum].advice = 1;
			//end
			(*bnum)++;
			cl->class[0].group[i].bcount++;
			cl->class[0].bcount++;
		}
	}
	cl->class[0].gcount = SECNUM;
	cl->ccount = 1;
}

int
load_classes()
{
	FILE *fp;
	char buf[256], *ptr;
	int nc, ng, bnum;
	if (NULL == cl)
		cl = calloc(1, sizeof (struct classes));
	if (NULL == cl)
		return -1;
	if (cl->uptime > file_time(CLASSES_CONFIG)
	    && cl->uptime > shm_bcache->uptime)
		return 0;
	bzero(cl, sizeof (struct classes));
	bnum = 0;
	load_default_class(&bnum);
	fp = fopen(CLASSES_CONFIG, "r");
	if (NULL == fp)
		return 0;
	nc = 0;
	ng = SECNUM - 1;
	while (fgets(buf, sizeof (buf), fp)) {
		ptr = strchr(buf, '\n');
		if (ptr)
			*ptr = 0;
		if (!strncmp(buf, "##", 2)) {
			if (cl->ccount + 1 > MAXCLASS)
				continue;
			nc = cl->ccount;
			ng = -1;
			snprintf(cl->class[nc].cname, N_LEN, "%s", buf + 2);
			cl->ccount++;
		} else if (buf[0] == '#') {
			if (nc < 0)
				continue;
			if (cl->class[nc].gcount + 1 > MAXGROUP)
				continue;
			ng = cl->class[nc].gcount;
			snprintf(cl->class[nc].group[ng].gname, N_LEN, "%s",
				 buf + 1);
			cl->class[nc].group[ng].start = bnum;
			cl->class[nc].gcount++;
		}
		if (nc < 0 || ng < 0)
			continue;
		if (bnum + 1 > MAXB)
			break;
		if (!parse_boardarg(buf, &(cl->bdata[bnum]))) {
			cl->class[nc].group[ng].bcount++;
			cl->class[nc].bcount++;
			bnum++;
		}
	}
	fclose(fp);
	cl->uptime = time(NULL);
	return 0;
}

struct oneclass *
get_class(char *class)
{
	int i;
	if (load_classes())
		return NULL;
	for (i = 0; i < cl->ccount; i++)
		if (!strcmp(cl->class[i].cname, class))
			return &(cl->class[i]);
	return NULL;
}
