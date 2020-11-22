#include <string.h>
#include <unistd.h>
#include "ythtbbs/ythtbbs.h"

#define BINSECMFILE MY_BBS_HOME "/etc/secmlist.data"
const struct sectree *
getsectree(const char *s)
{
	const struct sectree *sec;
	char *ptr;
	const char *str;
	sec = &sectree;
	str = s;
	while (*str && (ptr = strchr(sec->seccodes, *str))) {
		sec = sec->subsec[ptr - sec->seccodes];
		str++;
	}
	return sec;
}

int
gensecm(const char *txtfile)
{
	FILE *fp;
	char buf[256], *ptr, *p0;
	int fd, i, count = 0;
	struct secmanager secm;
	if ((fp = fopen(txtfile, "rt")) == NULL)
		return -1;
	if ((fd = open(BINSECMFILE, O_WRONLY | O_CREAT, 0660)) < 0) {
		fclose(fp);
		return -1;
	}
	while (fgets(buf, sizeof (buf), fp)) {
		if (buf[0] == '#')
			continue;
		p0 = buf;
		ptr = strsep(&p0, "\t\r\n ");
		if (!ptr)
			continue;
		bzero(&secm, sizeof (secm));
		ytht_strsncpy(secm.secstr, ptr, sizeof(secm.secstr));
		for (i = 0; i < MAXSECM; i++) {
			ptr = strsep(&p0, "\t\r\n ");
			if (ptr == NULL)
				break;
			ytht_strsncpy(secm.secm[i], ptr, sizeof(secm.secm));
		}
		if (i == 0)
			continue;
		secm.n = i;
		write(fd, &secm, sizeof (secm));
		count++;
	}
	bzero(&secm, sizeof (secm));
	do {
		write(fd, &secm, sizeof (secm));
		count++;
	} while (count < 20);
	close(fd);
	fclose(fp);
	return 0;
}

struct secmanager *
getsecm(const char *str)
{
	static struct mmapfile mf = { ptr:NULL };
	struct secmanager *secmlist;
	int n, i;
	if (!mf.ptr) {
		if (mmapfile(BINSECMFILE, &mf) < 0)
			return NULL;
	}
	secmlist = (struct secmanager *) mf.ptr;
	n = mf.size / sizeof (struct secmanager);
	for (i = 0; i < n; i++) {
		if (!secmlist[i].secstr[0])
			return NULL;
		if (!strcmp(str, secmlist[i].secstr))
			return &secmlist[i];
	}
	return NULL;
}

int
issecm_strict(const char *str, const char *userid)
{
	struct secmanager *secm;
	int i;
	if ((secm = getsecm(str)) == NULL)
		return 0;
	for (i = 0; i < secm->n; i++) {
		if (!strcasecmp(userid, secm->secm[i]))
			return 1;
	}
	return 0;
}

int
issecm(const char *str, const char *userid)
{
	char tocheck[10];
	strcpy(tocheck, str);
	while (tocheck[0] != 0) {
		if (issecm_strict(tocheck, userid))
			return 1;
		tocheck[strlen(tocheck) - 1] = 0;
	}
	return 0;
}
