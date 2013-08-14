#include "bbslib.h"
static char *flist[][2] = {
	{"newYC", "wwwtmp/longupdate.secY"},
	{NULL, NULL}
};

int
bbsshowfile_main()
{
	char *filename;
	int i;
	html_header(1);
	filename = getparm("F");
	for (i = 0; flist[i][0]; i++) {
		if (!strcmp(filename, flist[i][0]))
			break;
	}
	if (!flist[i][0])
		http_fatal("File not found");
	filename = flist[i][1];
	showfile(filename);
	printf("</body></html>");
	return 0;
}
