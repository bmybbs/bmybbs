#include "bbs.h"
#include "bbsstatlog.h"

struct BCACHE *shm_bcache;
struct UCACHE *shm_ucache;

struct bbsstatlogitem item;
int
shm_init()
{
	ythtbbs_cache_utmp_resolve();
	shm_bcache = (struct BCACHE *) get_old_shm(BCACHE_SHMKEY, sizeof (struct BCACHE));
	if (shm_bcache == NULL)
		return -1;
	shm_ucache = (struct UCACHE *) get_old_shm(UCACHE_SHMKEY, sizeof (struct UCACHE));
	if (shm_ucache == NULL)
		return -1;
	return 0;
}

void
bonlinesync()
{
	int i, numboards;
	struct user_info *uentp;
	numboards = shm_bcache->number;
	for (i = 0; i < numboards; i++)
		shm_bcache->bcache[i].inboard = 0;
	for (i = 0; i < USHM_SIZE; i++) {
		uentp = ythtbbs_cache_utmp_get_by_idx(i);
		if (uentp->active && uentp->pid && uentp->curboard)
			shm_bcache->bcache[uentp->curboard - 1].inboard++;
	}
}

void
get_load(load)
float load[];
{
	FILE *fp;
	fp = fopen("/proc/loadavg", "r");
	if (!fp)
		load[0] = load[1] = load[2] = 0;
	else {
		float av[3];
		fscanf(fp, "%f %f %f", av, av + 1, av + 2);
		fclose(fp);
		load[0] = av[0];
		load[1] = av[1];
		load[2] = av[2];
	}
}

int
get_netflow()
{
	FILE *fp;
	float fMbit_s = 0;
	char *ptr, buf[256];
	fp = fopen(MY_BBS_HOME "/bbstmpfs/dynamic/ubar.txt", "r");
	if (!fp)
		return 0;
	if (!fgets(buf, sizeof (buf), fp))
		goto ERR;
	ptr = strchr(buf, '.');
	if (!ptr)
		goto ERR;
	while (ptr > buf && isdigit(*(ptr - 1)))
		ptr--;
	fMbit_s = atof(ptr);
      ERR:
	fclose(fp);
	return fMbit_s * 1024;
}

int
main()
{
	int i, fd;
	struct user_info *p;
	struct tm *ptm;

	if (shm_init() < 0)
		return 0;
	item.time = time(NULL);
	get_load(item.load);
	item.netflow = get_netflow();
	item.naccount = ythtbbs_cache_UserTable_get_number();
	for (i = 0; i < USHM_SIZE; i++) {
		p = ythtbbs_cache_utmp_get_by_idx(i);
		if (!p->active)
			continue;
		item.nonline++;
		if (p->pid == 1 && strcmp(p->userid, "guest"))
			item.nwww++;
		else if (p->pid == 1)
			item.nwwwguest++;
		else
			item.ntelnet++;
		if (!strncmp("162.105", p->from, 7)) {
			item.n162105++;
			if (p->pid != 1)
				item.n162105telnet++;
		}
	}
	ptm = localtime(&item.time);
	if (ptm->tm_hour == 0 && ptm->tm_min < 6)
		ythtbbs_cache_utmp_set_maxtoday(item.nonline);
	fd = open(BBSSTATELOGFILE, O_WRONLY | O_CREAT, 0600);
	if (fd >= 0) {
		lseek(fd, ((ptm->tm_mday * 24 + ptm->tm_hour) * 10 + ptm->tm_min / 6) * sizeof (item), SEEK_SET);
		write(fd, &item, sizeof (item));
		close(fd);
	}
	bonlinesync();
}

