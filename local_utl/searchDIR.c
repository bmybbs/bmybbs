#include "bbs.h"
#include "ythtbbs.h"

char *
encode_url(unsigned char *s)
{
	int i, j;
	static char buf[512];
	char a[3];
	j = 0;
	for (i = 0; s[i]; i++) {
		switch (s[i]) {
		case '\'':
		case '&':
		case '%':
		case '?':
		case '=':
		case '#':
		case '+':
		case ' ':
		case ',':
		case '\"':
			buf[j++] = '%';
			sprintf(a, "%02X", s[i]);
			buf[j++] = a[0];
			buf[j++] = a[1];
			break;
		default:
			buf[j++] = s[i];
		}
	}
	buf[j] = 0;
	return buf;
}

int
countdb(char *str)
{
	int db = 0;
	unsigned char *ptr = str;
	while (*ptr) {
		if (db)
			db = 0;
		else if (*ptr >= 128)
			db = 1;
		*ptr++;
	}
	return db;
}

int
main(int argn, char **argv)
{
	int retv, total;
	int filetime;
	char fn[20];
	char path[100];
	struct fileheader fh;
	struct mmapfile mf = { ptr:NULL };
	if (argn != 3 || strlen(argv[1]) > 20)
		return 0;
	strncpy(fn, argv[2], 20);
	fn[19] = 0;
	filetime = atoi(fn + 2);
	if (filetime == 0)
		return 0;
	sprintf(path, MY_BBS_HOME "/boards/%s/.DIR", argv[1]);

	MMAP_TRY {
		if (mmapfile(path, &mf) == -1)
			MMAP_RETURN(0);
		total = mf.size / sizeof (struct fileheader) - 1;
		retv = Search_Bin(mf.ptr, filetime, 0, total);
		if (retv < 0)
			return 0;
		memcpy(&fh,
		       (struct fileheader *) (mf.ptr +
					      retv *
					      sizeof (struct fileheader)),
		       sizeof (struct fileheader));
	}

	MMAP_CATCH {
		MMAP_RETURN(0);
	}

	MMAP_END mmapfile(NULL, &mf);

	if (fh.sizebyte && fh.sizebyte < 20)
		return 0;
	if (countdb(fh.title)) {
		fh.title[strlen(fh.title) - 1] = 0;
	}
	printf("%s,%d,%s,%s\n", fh2owner(&fh), fh.thread, encode_url(fh.title),
	       fh.title);
	return 0;
}
