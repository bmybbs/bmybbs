#include "ythtbbs.h"
int fix_title(struct fileheader *fh, int *fd) {
	struct fileheader fw;
	char *ptr;
	FILE *fp;
	char buf[1024];
	char title[60];
	memcpy(&fw, fh, sizeof(struct fileheader));
	fp=fopen(fh2fname(fh), "r");
	if (!fp)
		return 0;
	strncpy(title, fh->title,60);
	ptr=strstr(title, " - ");
	if (ptr)
		*ptr=0;
	while (fgets(buf, 1024, fp)) {
		if (!strncmp(buf, "Бъ  Ьт", 6)) {
			strncpy(title, buf+8, 60);
			break;
		}
	}
	fclose(fp);
	strncpy(fw.title, title,60);
	ptr=strchr(fw.title, '\n');
	if (ptr)
		*ptr=0;
	write(*fd, &fw, sizeof(struct fileheader));
	return 0;
}

main (){
	int fd;
	fd=open("tmpfile", O_CREAT | O_TRUNC | O_WRONLY, 0700);
 	new_apply_record(".DIR", sizeof (struct fileheader), fix_title, &fd);
	close(fd);
}
