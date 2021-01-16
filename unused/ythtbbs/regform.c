#include <string.h>
#include <unistd.h>
#include "ythtbbs/ythtbbs.h"
int
getregforms(char *filename, int num, const char *userid)
{
	int lockfd, t, numline = 0, count = 0;
	FILE *fpr, *fpw;
	char buf[256];
	lockfd =
	    openlockfile(MY_BBS_HOME "/.lock_new_register", O_RDONLY, LOCK_EX);
	if (lockfd <= 0)
		return -1;
	mkdir(SCANREGDIR, 0770);
	if (file_exist(GETREGFILE))
		goto ERROR1;
	rename(NEWREGFILE, GETREGFILE);
	fpr = fopen(GETREGFILE, "r");
	if (fpr == NULL)
		goto ERROR1;
	strcpy(filename, SCANREGDIR);
	sprintf(buf, "R.%%d.%s", userid);
	t = trycreatefile(filename, buf, time(NULL), 5);
	if (t < 0)
		goto ERROR2;
	fpw = fopen(filename, "w");
	if (fpw == NULL)
		goto ERROR2;
	while (fgets(buf, sizeof (buf), fpr)) {
		if (buf[0] == '-') {
			if (!numline)
				continue;
			count++;
			if (count >= num)
				break;
			fputs(buf, fpw);
			numline = 0;
			continue;
		}
		numline++;
		fputs(buf, fpw);
	}
	fclose(fpw);
	fpw = fopen(NEWREGFILE, "a");
	if (fpw == NULL)
		goto ERROR2;
	while (fgets(buf, sizeof (buf), fpr)) {
		fputs(buf, fpw);
	}
	fputs("----\n", fpw);
	fclose(fpw);
	fclose(fpr);
	unlink(GETREGFILE);
	close(lockfd);
	return count;
      ERROR2:
	fclose(fpr);
      ERROR1:
	close(lockfd);
	return -1;
}
