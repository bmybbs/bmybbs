#include "bbs.h"
#include "ythtlib.h"

static struct UTMPFILE *shm_utmp;

static int
myatoi(unsigned char *a)
{
	int i = 0;
	while (*a)
		i = i * 26 + (*(a++) - 'A');
	return i;
}

int
main(int argc, char *argv[])
{
	int i;
	struct user_info *uin;
	if (argc != 4) {
		return -1;
	}
	shm_utmp =
	    (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY,
					    sizeof (struct UTMPFILE));
	if (shm_utmp == NULL) {
		printf("test\n");
		return -2;
	}
	i = myatoi(argv[1]);
	if (i < 0 || i > USHM_SIZE) {
		printf("test\n");
		return -3;
	}
	uin = &(shm_utmp->uinfo[i]);
	if (uin->active && !strcmp(uin->sessionid, argv[2])
	    && !strcmp(uin->from, argv[3])) {
		printf("%s\n", uin->userid);
		return 0;
	}
	printf("test\n");
	return -4;
}
