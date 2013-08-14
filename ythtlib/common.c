#include "ythtlib.h"
void
_errlog(char *fmt, ...)
{
	FILE *fp;
	int proc;
	char buf[1024], timestr[16], progname[256], *thetime;
	time_t dtime;
	int i = 0, k;
	pid_t pid;
	va_list ap;

	pid = getpid();
	snprintf(buf, sizeof (buf), "/proc/%d/cmdline", pid);
	if ((proc = open(buf, O_RDONLY)) >= 0) {
		i = read(proc, progname, sizeof (progname));
		close(proc);
	}
	if (i > 0) {
		for (k = 0; k < i; k++)
			if (!progname[k])
				progname[k] = ' ';
		snprintf(buf, i + 1, "%s", progname);
		strcpy(buf + i, "|");
		i++;
	} else
		i = 0;
	va_start(ap, fmt);
	vsnprintf(buf + i, 1000 - i, fmt, ap);
	va_end(ap);
	buf[1000] = 0;

	time(&dtime);
	thetime = ctime(&dtime);
	strncpy(timestr, &(thetime[4]), 15);
	timestr[15] = '\0';
	fp = fopen(ERRLOG, "a");

	if (fp != NULL) {
		fprintf(fp, "%s %d %s\n", timestr, pid, buf);
		fclose(fp);
	}
}

int
mystrtok(char *buf, int c, char *tmp[], int max)
{
	int i;
	char *a;
	for (i = 0; i < max - 1; i++) {
		a = index(buf, c);
		if (a != NULL) {
			strncpy(tmp[i], buf, a - buf);
			tmp[i][a - buf] = 0;
			buf = a + 1;
			continue;
		}
		break;
	}
	strcpy(tmp[i], buf);
	return i + 1;
}
