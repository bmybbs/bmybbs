#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include "main.h"
#include <string.h>
#include <errno.h>

#include "logging.h"
#include "options.h"
#include "commands.h"
#include "mystring.h"

FILE *logfile = NULL;

void
log_init()
{
	if (!(logfile = fopen(PATH_LOGFILE, "a"))) {
		prints("421-Could not open log file.\r\n"
		       "421 Server disabled for security reasons.");
		exit(1);
	}
}

void
yftpd_log(char *format, ...)
{
	va_list val;
	char buffer[1024], timestr[40];
	time_t t;
	va_start(val, format);
	vsnprintf(buffer, sizeof (buffer), format, val);
	va_end(val);
	if (logfile) {
		fseek(logfile, 0, SEEK_END);
		time(&t);
		strcpy(timestr, (char *) ctime(&t));
		timestr[strlen(timestr) - 1] = '\0';
		fprintf(logfile, "%s yftpd[%i]: %s", timestr,
			(int) getpid(), buffer);
		fflush(logfile);
	}
}

void
log_end()
{
	if (logfile) {
		fclose(logfile);
		logfile = NULL;
	}
}
