#include "bbs.h"

extern int cmpbnames(char *bname, struct boardheader *brec);

int
report(err)
char *err;
{
	return printf(err);
}

int main(int argc, char *argv[])
{
	if(argc!=3) {
		printf("Error usage!\n"
				"Please use this command:\n"
				"\033[1;32mchangeboardname\033[m \033[4mboard name\033[m \033[4mnew chinese name(in utf8)\033[m\n");
		return -1;
	}

	struct boardheader fh, newfh;
	int pos = new_search_record(BOARDS, &fh, sizeof(fh), (void *)cmpbnames, argv[1]);

	if(!pos) {
		printf("\033[1;31mwrong board name, exit!\033[m\n");
		return -1;
	}

	char *newtitle_gbk = (char *)malloc(strlen(argv[2])*2);
	memset(newtitle_gbk, 0, strlen(argv[2])*2);
	u2g(argv[2], strlen(argv[2]), newtitle_gbk, strlen(argv[2])*2);

	memset(fh.title, 0, sizeof(fh.title));
	strncpy(fh.title, newtitle_gbk, 24);

	substitute_record(BOARDS, &fh, sizeof(fh), pos);
	printf("board %s at position %d updated successfully!\n", argv[1], pos);

	return 0;
}
