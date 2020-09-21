#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include "config.h"
#include "ytht/fileop.h"
#include "ytht/uudecode.h"
#include "ythtbbs/binaryattach.h"

int
attachuu2bin(char *filename)	//将老格式的uuencode附件变成bin格式的附件
{
	FILE *fr, *fw;
	char tofile[PATH_MAX + 1], *attachfile, *ptr, buf[512], tmpattach[512];
	int hast = 0;

	tofile[PATH_MAX] = 0;
	if (strlen(filename) > PATH_MAX - 1)
		return -1;
	strncpy(tofile, filename, PATH_MAX - 1);
	strcat(tofile, "b");
	fr = fopen(filename, "r");
	if (NULL == fr)
		return -2;
	fw = fopen(tofile, "w");
	if (NULL == fw) {
		fclose(fr);
		return -3;
	}
	while (fgets(buf, sizeof (buf), fr) != NULL) {
		if (!strncmp(buf, "begin 644 ", 10)) {
			attachfile = buf + 10;
			ptr = strchr(attachfile, '\r');
			if (ptr)
				*ptr = 0;
			ptr = strchr(attachfile, '\n');
			if (ptr)
				*ptr = 0;
			strcpy(tmpattach, PATHUSERATTACH "/lepton/");
			strcat(tmpattach, attachfile);
			if (uudecode(fr, tmpattach)) {
				printf("error: %s\n", filename);
				continue;
			}
			fclose(fw);
			appendbinaryattach(tofile, "lepton", attachfile);
			hast = 1;
			fw = fopen(tofile, "a");
			if (NULL == fw) {
				fclose(fr);
				unlink(tofile);
				return -4;
			}
		} else if (!strncmp(buf, "beginbinaryattach ", 18)) {
			char ch;
			fread(&ch, 1, 1, fr);
			if (ch != 0) {
				ungetc(ch, fr);
				fprintf(fw, "%s", buf);
				continue;
			}
			fclose(fr);
			fclose(fw);
			unlink(tofile);
			return -5;
		} else
			fprintf(fw, "%s", buf);
	}
	if (hast) {
		fclose(fr);
		fclose(fw);
		copyfile(tofile, filename);
		unlink(tofile);
		return -6;
	} else {
		fclose(fr);
		fclose(fw);
		unlink(tofile);
		return 0;
	}
}

int
dotran(const char *file, const struct stat *sb, int flag, struct FTW *unused)
{
	int ret;
	char *ptr;
	if (FTW_F != flag)
		return 0;
	ptr = basename(file);
	if (*ptr == '.')
		return 0;
	ret = attachuu2bin(file);
	if (ret && ret!=-5)
		printf("%d %s\n", ret, file);
	return 0;

}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("error!no enough argments!\n");
		printf("usage: %s <dir>\n", argv[0]);
		exit(1);
	}
	printf("finished %d\n", nftw(argv[1], dotran, 30, FTW_PHYS));

	return 0;
}
