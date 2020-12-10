#include "bbs.h"
#include "bmy/convcode.h"
#include <stdio.h>

int postfile(char *filename, char *owner, char *nboard, char *posttitle);

int main(int argc, char **argv)
{
	//postfile file user boardname title
	char title[128];
	if (argc != 5 && argc!=6 ) {
		printf("usage: ./postfile file author board title [codechange]\n");
		printf("which means post \"file\" to \"board\", with the \"title\" and \"author\"\n");
		return -1;
		return 1;
	}
	if (!strcmp(argv[5], "1")) {
		u2g(argv[4], strlen(argv[4]), title, 128);
	}
	int ret = postfile(argv[1], argv[2], argv[3], title);
	return ret;
}
