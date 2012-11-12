#include "bbs.h"

#ifdef CRAY
/*
 * getnpty()
 *
 * Return the number of pty's configured into the system.
 */
int
getnpty(void)
{
#ifdef _SC_CRAY_NPTY
	int numptys;
	if ((numptys = sysconf(_SC_CRAY_NPTY)) != -1)
		return numptys;
	else
#endif				/* _SC_CRAY_NPTY */
		return 128;
}
#endif				/* CRAY */

#ifdef CONF_HAVE_OPENPTY
int
getpty(char *line)
{
	int m, s, t;
	if (openpty(&m, &s, line, NULL, NULL))
		return -1;
	close(s);
	return m;
}
#else
int
getpty(char *line)
{
	int p;
	time_t now;
#ifndef CRAY
	char *p1, *p2;
	int i, j;

	now = time(0);
	sprintf(line, "/dev/ptyXX");
	p1 = &line[8];
	p2 = &line[9];

	for (i = 0; i < 16; i++) {
		struct stat stb;

		*p1 = "pqrstuvwxyzabcde"[(now + i) % 16];
		*p2 = '0';
		if (stat(line, &stb) < 0)
			continue;
		for (j = 0; j < 16; j++) {
			*p2 = "0123456789abcdef"[j];
			p = open(line, 2);
			if (p > 0) {
				line[5] = 't';
				return (p);
			}
		}
	}
#else				/* CRAY */
	int npty;
	int highpty;
	struct stat sb;

	highpty = getnpty();
	now = time(0);
	for (npty = 0; npty <= highpty; npty++) {

		(void) sprintf(myline, "/dev/pty/%03d",
			       (npty + now) % (highpty + 1));
		p = open(myline, 2);
		if (p < 0)
			continue;
		(void) sprintf(line, "/dev/ttyp%03d",
			       (npty + now) % (highpty + 1));
		/*
		 * Here are some shenanigans to make sure that there
		 * are no listeners lurking on the line.
		 */
		if (stat(line, &sb) < 0) {
			(void) close(p);
			continue;
		}
		if (sb.st_uid || sb.st_gid || sb.st_mode != 0600) {
			chown(line, 0, 0);
			chmod(line, 0600);
			(void) close(p);
			p = open(myline, 2);
			if (p < 0)
				continue;
		}
		/*
		 * Now it should be safe...check for accessability.
		 */
		if (access(line, 6) == 0)
			return (p);
		else {
			/* no tty side to pty so skip it */
			(void) close(p);
		}
	}
#endif				/* CRAY */
	return (-1);
}
#endif

int
closepty(int pty)
{
	close(pty);
}

void
wlogin(const char *line, const char *name, const char *host)
{
	struct utmp ut;
	struct stat buf;
	int fd;

	if ((fd = open(_PATH_UTMP, O_WRONLY | O_APPEND, 0)) < 0)
		return;
	if (fstat(fd, &buf) == 0) {
		ut.ut_pid = getpid();
		ut.ut_type = (name[0] != '\0') ? USER_PROCESS : DEAD_PROCESS;
		strncpy(ut.ut_id, &line[3], 2);
		strncpy(ut.ut_line, line, sizeof (ut.ut_line));
		strncpy(ut.ut_name, name, sizeof (ut.ut_name));
		strncpy(ut.ut_host, host, sizeof (ut.ut_host));
		time(&ut.ut_time);
		if (write(fd, &ut, sizeof (struct utmp)) !=
		    sizeof (struct utmp))
			ftruncate(fd, buf.st_size);
	}
	close(fd);
}

int
wlogout(const char *line)
{
	int fd, rval;
	struct utmp ut;

	if ((fd = open(_PATH_UTMP, O_RDWR, 0)) < 0)
		return (0);
	rval = 0;
	while (read(fd, &ut, sizeof (ut)) == sizeof (ut)) {
		if (!ut.ut_name[0] || strncmp(ut.ut_line, line, UT_LINESIZE))
			continue;
		ut.ut_type = DEAD_PROCESS;
		bzero(ut.ut_name, UT_NAMESIZE);
		bzero(ut.ut_host, UT_HOSTSIZE);
		(void) time(&ut.ut_time);
		(void) lseek(fd, -(off_t) sizeof (ut), L_INCR);
		(void) write(fd, &ut, sizeof (ut));
		rval = 1;
	}
	(void) close(fd);
	return (rval);
}
