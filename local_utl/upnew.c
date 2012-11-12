#include "bbs.h"
int
main()
{
	void *shmptr;
	struct UTMPFILE *utmpshm;
	struct tm *now;
	int shmid;
	time_t nowtime;
	if ((shmid = shmget(UTMP_SHMKEY, 0, 0)) == -1)
		exit(1);
	shmptr = (void *) shmat(shmid, NULL, 0);
	utmpshm = shmptr;
	nowtime = time(0);
	now = localtime(&nowtime);
	printf("%d %d\n", now->tm_hour, utmpshm->activeuser);
	shmdt(shmptr);
	return 1;
}
