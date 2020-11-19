#include "bbs.h"
int
main()
{
	void *shmptr;
	struct tm *now;
	int shmid;
	time_t nowtime;
	ythtbbs_cache_utmp_resolve();
	nowtime = time(0);
	now = localtime(&nowtime);
	printf("%d %d\n", now->tm_hour, ythtbbs_cache_utmp_get_activeuser());
	return 1;
}
