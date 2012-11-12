#include "bbs.h"
#define PASSWDFILE "PASS123"
main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	int size1 = sizeof (rec);

	if ((fd1 = open(PASSWDFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		if (strcmp(rec.userid, "forpass") == 0) {
			printf("代号         : %s\n", rec.userid);
			printf("昵称         : %s\n", rec.username);
			printf("上站次数     : %d 次\n", rec.numlogins);
			printf("文章数目     : %d\n", rec.numposts);
			printf("pass	   : %s\n", rec.passwd);
			break;
		}
	}
	close(fd1);
}
