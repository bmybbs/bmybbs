#include "bbs.h"
#define numboards 296 
int *zapbuf;
int now;
int save_zapbuf(char *userid,int rs) {
	char filename[200];
	int fd, size;
	sprintf(filename, "home/%c/%s/.newlastread", mytoupper(userid[0]), userid);
	if (rs == 0) {
		remove(filename);
		return 0;
	}
        if ((fd = open(filename, O_WRONLY | O_CREAT, 0600)) != -1) {
                size = numboards * sizeof (int);
                write(fd, zapbuf, rs);
		ftruncate(fd, rs);
                close(fd);
        }
	return 0;
}
int
fixzapbuf(char *userid)
{
	char filename[200];
	char bakfn[200];
	int fd,i, size, rs=0;

	if (userid[0] == 0)
		return 0;
	bzero(zapbuf, MAXBOARD * sizeof(int));
	sprintf(filename, "home/%c/%s/.lastread", mytoupper(userid[0]),
		userid);
	if ((fd = open(filename, O_RDONLY, 0600)) != -1) {
		printf("trans %s\n", userid);
		for (i = 0; i < MAXBOARD; i++)
			zapbuf[i] = 1;
		size = numboards * sizeof (int);
		rs = read(fd, zapbuf, size);
		close(fd);
		for (i = 0; i < MAXBOARD; i++)
			zapbuf[i] = ((zapbuf[i] == 0) ? now : 0);	//反一下，记下什么时候z掉的，而不是什么时候z回来的
		sprintf(bakfn, "home/%c/%s/.lastread.transbak", mytoupper(userid[0]), userid);
		rename(filename, bakfn);
	}
	else {
        sprintf(filename, "home/%c/%s/.newlastread", mytoupper(userid[0]), userid);
        if ((fd = open(filename, O_RDONLY, 0600)) != -1) {
                size = numboards * sizeof (int);
                rs=read(fd, zapbuf, size);
                close(fd);
        }
	}
	save_zapbuf(userid,rs);	
	return 0;
}

int
_fixzapbuf(struct userec *user) {
	return fixzapbuf(user->userid);
}

main()
{
	zapbuf = malloc(MAXBOARD * sizeof (int));
	chdir(MY_BBS_HOME);
	now = time(NULL);
	new_apply_record(".PASSWDS", sizeof (struct userec), _fixzapbuf, NULL);
}
