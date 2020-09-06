#ifndef __LOGGING_H
#define __LOGGING_H

extern FILE *logfile;

void log_init();

void yftpd_log(char *format, ...);
void log_end();

#endif
