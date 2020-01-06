#ifndef __OPTIONS_H
#define __OPTIONS_H
#include <sys/ipc.h>
#include <sys/sem.h>

#include "bbs.h"
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;		/* value for SETVAL */
	struct semid_ds *buf;	/* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array;	/* array for GETALL, SETALL */
	struct seminfo *__buf;	/* buffer for IPC_INFO */
};
#endif
#define MY_FTP_PORT 2121
#define MY_FTP_HOME MY_BBS_HOME "/ftphome"
#define MY_FTP_ROOT MY_FTP_HOME "/root"
#define PATH_STATUSLOG "/dev/null"
#define PATH_ADMINISTRATOR MY_FTP_HOME "/ftp_adm"
#define PATH_YFTPDUTMP MY_FTP_HOME "/yftpdutmp"
#define PATH_YFTPQUOTA MY_FTP_HOME "/quota"
#define PATH_UTMPLOCK MY_FTP_HOME "/utmplock"
#define PATH_MOTD_GLOBAL MY_FTP_HOME "/ftpmotd"
#define PATH_LOGFILE MY_FTP_HOME "/log"
#define PATH_FLOWLOCK MY_FTP_HOME "/flowlock"
#define PATH_DENY_LOGIN MY_FTP_HOME "/deny_login"
#define PATH_SECLOG MY_FTP_HOME "/seclog"
#define MAX_FTPUSER 60
#define CONTROL_TIMEOUT 300
#define DATA_TIMEOUT 300
#define XFER_BUFSIZE 4096
#define XFER_TICKETSLEEP 200000	//usec
#define XFER_TICKETUNIT 4	//maxspeed=UNIT*1E6/SLEEP*BUFSIZE
#define YFTP_SEMKEY 1111
#define YFTP_SHMKEY 1111
#define ALLOW_FXP 0
#define HELLO_STRING "yftpd %v at %i ready."
//#define PASSIVE_PORTS "10000,12000-12100,13000"
#define PASSIVE_PORTS "0"
#define QUIT_MSG "See you later..."
#define ENABLE_SITE 1

#endif
