/*
   bftpd
   Copyright (C) 1999-2000 Max-Wilhelm Bruker
   yftpd, YTHT fptd, a ftpd designed for Bulletin Board System.
   Copyright (C) 2001 ecnegrevid, ecnegrevid.bbs@ytht.net 

   This program is is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 of the
   License as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef HAVE_WAIT_H
# include <wait.h>
#else
# ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
#endif

#include "mystring.h"
#include "logging.h"
#include "yftpdutmp.h"
#include "options.h"
#include "login.h"
#include "pathop.h"
#include "../include/bbs.h"

struct sockaddr_in name;
int listensocket, sock;
struct sockaddr_in remotename;
char *remotehostname;
int alarm_type = 0;
int flow_semid;

void
print_file(int number, char *filename)
{
	FILE *phile;
	char foo[256];
	phile = fopen(filename, "r");
	if (phile) {
		while (fgets(foo, sizeof (foo), phile)) {
			foo[strlen(foo) - 1] = '\0';
			prints("%i-%s", number, foo);
		}
		fclose(phile);
	}
}

void
end_child()
{
	yftpd_log("Quitting.\n");
	yftpdutmp_end();
	log_end();
	close(sock);
}

void
handler_sigchld(int sig)
{
	/* Get the child's return code so that the zombie dies */
	while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0) ;
}

void
handler_sigterm(int signum)
{
	exit(0);		/* Force normal termination so that end_child() is called */
}

void
handler_sigalrm(int signum)
{
	if (alarm_type) {
		close(alarm_type);
		yftpd_log
		    ("Kicked from the server due to data connection timeout.\n");
		prints
		    ("421 Kicked from the server due to data connection timeout.");
		exit(0);
	} else {
		yftpd_log
		    ("Kicked from the server due to control connection timeout.\n");
		prints
		    ("421 Kicked from the server due to control connection timeout.");
		exit(0);
	}
}

void
init_everything()
{
	struct rlimit rlim;
	getrlimit(RLIMIT_CORE, &rlim);
	rlim.rlim_max = rlim.rlim_cur = 0;
	if (setrlimit(RLIMIT_CORE, &rlim))
		exit(-1);
	log_init();
	yftpdutmp_init();
}

void
do_savequota(int n)
{
	if (n) {
		quota->changed = 0;
		memcpy(quotaf, quota, sizeof (*quota));
		return;
	}
	while (1) {
		sleep(10 * 60);
		if (quota->changed) {
			quota->changed = 0;
			memcpy(quotaf, quota, sizeof (*quota));
		}
	}
}

void
do_sem()
{
	struct sembuf sop0, sop1;
	union semun arg;
	int fd = open(PATH_FLOWLOCK, O_CREAT | O_RDONLY, 0660);
	struct UTMPFILE *shm_utmp;
      AGAIN:
	shm_utmp =
	    (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY,
					    sizeof (struct UTMPFILE));
	if (!shm_utmp) {
		sleep(10);
		goto AGAIN;
	}
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
		exit(0);
	arg.val = 0;
	if (semctl(flow_semid, 0, SETVAL, arg) < 0)
		exit(0);
	sop0.sem_num = 0;
	sop0.sem_op = 0;
	sop0.sem_flg = 0;
	sop1.sem_num = 0;
	sop1.sem_flg = 0;
	while (1) {
		if (shm_utmp->activeuser < 500) {
			sop1.sem_op = XFER_TICKETUNIT * 4;
		} else if (shm_utmp->activeuser < 1000) {
			sop1.sem_op = XFER_TICKETUNIT * 3;
		} else if (shm_utmp->activeuser < 1500) {
			sop1.sem_op = XFER_TICKETUNIT * 2;
		} else
			sop1.sem_op = XFER_TICKETUNIT;
		semop(flow_semid, &sop0, 1);
		semop(flow_semid, &sop1, 1);
		usleep(XFER_TICKETSLEEP);
	}
}

int checkaddr(struct in_addr addr, int csock);
int
main()
{
	char str[MAXCMD + 1];
	int i, port;
	struct sockaddr_in myaddr;
	umask(027);
	if (fork())
		exit(0);
	setsid();
	if (fork())
		return 0;
	flow_semid = semget(YFTP_SEMKEY, 1, IPC_CREAT | S_IRWXU);
	if (flow_semid < 0)
		exit(0);
	if (my_quota_init() < 0)
		exit(0);
	if (fork()) {
		signal(SIGCHLD, SIG_IGN);
		do_sem();
	}
	if (fork()) {
		signal(SIGCHLD, SIG_IGN);
		do_savequota(0);
	}

	chdir(MY_FTP_ROOT);
	for (i = 3; i <= getdtablesize(); i++)
		close(i);
	//for(i=1; i<=NSIG; i++) signal(i, SIG_IGN);
	signal(SIGCHLD, handler_sigchld);
	listensocket = socket(AF_INET, SOCK_STREAM, 0);
	i = 1;
#ifdef SO_REUSEADDR
	setsockopt(listensocket, SOL_SOCKET, SO_REUSEADDR, (void *) &i,
		   sizeof (i));
#endif
#ifdef SO_REUSEPORT
	setsockopt(listensocket, SOL_SOCKET, SO_REUSEPORT, (void *) &i,
		   sizeof (i));
#endif
	memset((void *) &myaddr, 0, sizeof (myaddr));
	port = MY_FTP_PORT;
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	//inet_addr(config_getoption("BIND_TO_ADDR"));
	if (bind
	    (listensocket, (struct sockaddr *) &myaddr, sizeof (myaddr)) < 0) {
		fprintf(stderr, "Bind failed: %s\n", strerror(errno));
		exit(1);
	}
	if (listen(listensocket, 5)) {
		fprintf(stderr, "Listen failed: %s\n", strerror(errno));
		exit(1);
	}
	setuid(BBSUID);
	setgid(BBSGID);
	i = sizeof (remotename);
	while (
	       (sock =
		accept(listensocket, (struct sockaddr *) &remotename, &i))) {
		if (checkaddr(remotename.sin_addr, sock) < 0) {
			close(sock);
			continue;
		}
		if (!fork()) {
			close(listensocket);
			dup2(sock, 0);
			dup2(sock, 1);
			dup2(sock, 2);
			if (sock > 2)
				close(sock);
			break;
		}
		close(sock);
		sleep(2);
	}
	i = 1;
	setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (void *) &i, sizeof (i));
	setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (void *) &i, sizeof (i));
	init_everything();
	atexit(end_child);
	signal(SIGTERM, handler_sigterm);
	signal(SIGALRM, handler_sigalrm);
	alarm(CONTROL_TIMEOUT);
	remotehostname = strdup(inet_ntoa(remotename.sin_addr));
	if (checkbansite(remotehostname)) {
		prints("421  抱歉您的IP被封禁了, 如有疑问请联系 SYSOP.\r\n");
		exit(0);
	}
	yftpd_log("Incoming connection from %s.\n", remotehostname);
	i = sizeof (name);
	getsockname(fileno(stdin), (struct sockaddr *) &name, &i);
	print_file(220, PATH_MOTD_GLOBAL);
	/* Parse hello message */
	strcpy(str, HELLO_STRING);
	replace(str, "%v", VERSION);
	if (strstr(str, "%h")) {
		replace(str, "%h", (char *) inet_ntoa(name.sin_addr));
	}
	replace(str, "%i", (char *) inet_ntoa(name.sin_addr));
	prints("220 %s", str);
	/* Read lines from client and execute appropiate commands */
	while (fgets(str, sizeof (str), stdin)) {
		alarm(CONTROL_TIMEOUT);
		str[strlen(str) - 2] = 0;
		//yftpd_log("Processing command: %s\n", str);
		parsecmd(str);
		fflush(stderr);
	}
	return 0;
}

//checkaddr(...), add by ylsdd, 对抗上站机
#define NADDRCHECK 500
struct {
	struct in_addr addr;
	time_t t;
	float x;
	int n;
} addrcheck[NADDRCHECK];

int
checkaddr(struct in_addr addr, int csock)
{
	int i, j;
	static int fd = -1, lll = 1;
	char str[150];
	time_t timenow, ttemp;

	if (fd < 0) {
		fd = open(PATH_SECLOG, O_CREAT | O_WRONLY | O_APPEND, 0664);
		if (fd >= 0)
			fcntl(fd, F_SETFD, 1);
	}
	if (lll == 1) {
		bzero(addrcheck, sizeof (addrcheck));
		lll = 0;
	}

	time(&timenow);
	for (i = 0; i < NADDRCHECK; i++) {
		if (addrcheck[i].t == 0)
			continue;
		if (timenow - addrcheck[i].t > 60 * 11
		    || timenow < addrcheck[i].t) {
			if (addrcheck[i].x > 100 && addrcheck[i].n > 15) {
				sprintf(str, "remove\t%s\t%d\t%s",
					inet_ntoa(addrcheck[i].addr),
					addrcheck[i].n, ctime(&timenow));
				write(fd, str, strlen(str));
			}
			addrcheck[i].t = 0;
			continue;
		}
		if (memcmp(&addrcheck[i].addr, &addr, sizeof (addr)) == 0) {
			if (addrcheck[i].x <= 100 || addrcheck[i].n <= 15) {
				j = 0;
				addrcheck[i].x = addrcheck[i].x / (((unsigned)

								    (timenow -
								     addrcheck
								     [i].t) +
								    15) / 20.) +
				    30;
			} else
				j = 1;
			addrcheck[i].n++;

			if (addrcheck[i].x > 100 && addrcheck[i].n > 15) {
				if (j == 0 && fd >= 0)
					if (fork() == 0) {
						sprintf(str, "add\t%s\t%d\t%s",
							inet_ntoa(addr),
							addrcheck[i].n,
							ctime(&timenow));
						write(fd, str, strlen(str));
						write(csock,
						      "421 对不起, 连接将封闭 11 分钟。请不要不断连接冲击本站\n",
						      strlen
						      ("421 对不起, 连接将封闭 11 分钟。请不要不断连接冲击本站\n"));
						sleep(5);
						exit(0);
					}
				return -1;
			}
			addrcheck[i].t = timenow;
			return 0;
		}
	}
	//如果在addrcheck中没有该地址, 则加入.
	for (i = 0, j = -1, ttemp = timenow + 1; i < NADDRCHECK; i++) {
		if (addrcheck[i].t < ttemp
		    && (addrcheck[i].x <= 100 || addrcheck[i].n <= 15)) {
			ttemp = addrcheck[i].t;
			j = i;
		}
	}
	if (j == -1)
		for (i = 0, j = 0, ttemp = timenow + 1; i < NADDRCHECK; i++) {
			if (addrcheck[i].t < ttemp) {
				ttemp = addrcheck[i].t;
				j = i;
			}
		}
	if (addrcheck[j].x > 100 && addrcheck[j].n > 15) {
		sprintf(str, "remove\t%s\t%d\t%s",
			inet_ntoa(addrcheck[i].addr), addrcheck[j].n,
			ctime(&timenow));
		write(fd, str, strlen(str));
	}
	addrcheck[j].addr = addr;
	addrcheck[j].t = timenow;
	addrcheck[j].x = 0;
	addrcheck[j].n = 1;
	return 0;
}
