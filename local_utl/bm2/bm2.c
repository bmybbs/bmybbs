#include "bbs.h"
#define PASSWDFILE MY_BBS_HOME"/.PASSWDS"

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	int fd1;
	struct userec rec;
	struct stat info;
	char buf[100];
	int size1 = sizeof (rec);
	time_t visit;

	if ((fd1 = open(PASSWDFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		visit = 0;
		if (rec.userlevel & PERM_BOARDS) {
			printf("%s ", rec.userid);
			snprintf(buf, sizeof buf, MY_BBS_HOME "/tmp/%s.anpath", rec.userid);
			if (stat(buf, &info) != -1)
				if (visit < info.st_mtime)
					visit = info.st_mtime;
			snprintf(buf, sizeof buf, MY_BBS_HOME "/tmp/%s.copypaste",
				rec.userid);
			if (stat(buf, &info) != -1)
				if (visit < info.st_mtime)
					visit = info.st_mtime;
			if (visit == 0)
				printf("0 ´ÓÎ´\n");
			else
				printf("%d %s", visit, ctime(&visit));
		}
	}
	close(fd1);
}
