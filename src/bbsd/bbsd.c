/*
    Pirate Bulletin Board System
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
#include <malloc.h>

#define	QLEN		5
#define	PID_FILE	"reclog/bbs.pid"
#define	LOG_FILE	"reclog/bbs.log"

#define LOAD_LIMIT
static int myports[] = { BBS_PORT, 3456 /*, 3001, 3002, 3003 */  };
static int big5ports[] = { BBS_BIG5_PORT /* , 3456, 3001, 3002, 3003 */  };
int big5 = 0;
int runtest = 0;

static int mport;

int max_load = 20;
int csock;			/* socket for Master and Child */

int killer();
int checkaddr(struct in6_addr addr, int csock);//ipv6

void
cat(filename, msg)
char *filename, *msg;
{
	FILE *fp;

	if ((fp = fopen(filename, "a")) != NULL) {
		fputs(msg, fp);
		fclose(fp);
	}
}

int is4map6addr(char *s){
	return !strncasecmp(s,"::ffff:",7);
}

char *getv4addr(char *fromhost){
	char *addr;
	addr=rindex(fromhost,':');
	return ++addr;
}

void
prints(char *format, ...)
{
	va_list args;
	char buf[512];

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	write(0, buf, strlen(buf));
}

void
get_load(load)
double load[];
{
#ifdef LINUX
	FILE *fp;
	fp = fopen("/proc/loadavg", "r");
	if (!fp)
		load[0] = load[1] = load[2] = 0;
	else {
		float av[3];
		fscanf(fp, "%g %g %g", av, av + 1, av + 2);
		fclose(fp);
		load[0] = av[0];
		load[1] = av[1];
		load[2] = av[2];
	}
#else
#ifdef BSD44
	getloadavg(load, 3);
#else
	struct statstime rs;
	rstat("localhost", &rs);
	load[0] = rs.avenrun[0] / (double) (1 << 8);
	load[1] = rs.avenrun[1] / (double) (1 << 8);
	load[2] = rs.avenrun[2] / (double) (1 << 8);
#endif
#endif
}

//#ifndef CAN_EXEC
static void
telnet_init()
{
	static char svr[] = {
		IAC, DO, TELOPT_TTYPE,
		IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
		IAC, WILL, TELOPT_ECHO,
		IAC, WILL, TELOPT_SGA
	};

	send(0, svr, sizeof (svr), 0);
}

//#endif

static void
start_daemon(inetd, port)
int inetd;
int port;
{
	int n;
	struct linger ld;
	struct sockaddr_in6 sin;	//ipv6
	struct rlimit rl;
	char buf[80], data[80];
	time_t val;
	int portcount, big5portcount;
	time_t now;

	portcount = sizeof (myports) / sizeof (int);
	big5portcount = sizeof (big5ports) / sizeof (int);

	chdir(MY_BBS_HOME);
	umask(007);

	rl.rlim_cur = 20 * 1024 * 1024;
	rl.rlim_max = 40 * 1024 * 1024;
	setrlimit(RLIMIT_CORE, &rl);

	close(1);
	close(2);

	now = time(0);

	if (inetd) {
		setgid(BBSGID);
		setuid(BBSUID);
		mport = port;

		big5 = port;
		sprintf(data, "%d\tinetd -i\n", getpid());
		cat(PID_FILE, data);

		return;
	}

	sprintf(buf, "bbsd start at %s", ctime(&now));
	cat(PID_FILE, buf);

	close(0);

	if (fork())
		exit(0);

	setsid();

	if (fork())
		exit(0);

	if (fork()) {
		setgid(BBSGID);
		setuid(BBSUID);
		killer();
		exit(0);
	}
	//ipv6
	sin.sin6_family = AF_INET6;
	sin.sin6_addr = in6addr_any;

	if (port <= 0) {
		n = portcount + big5portcount - 1;
		while (n) {
			if (fork() == 0)
				break;
			sleep(1);
			n--;
		}
		if (n < portcount)
			port = myports[n];
		else {
			big5 = 1;
			port = big5ports[n - portcount];
		}
	}

	if (port == 3456)
		runtest = 1;
	else
		runtest = 0;

	//ipv6
	n = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	val = 1;
	setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (val));
	ld.l_onoff = ld.l_linger = 0;
	setsockopt(n, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof (ld));

	mport = port;
	sin.sin6_port = htons(port);
	if ((bind(n, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	    || (listen(n, QLEN) < 0))
		exit(1);

	setgid(BBSGID);
	setuid(BBSUID);

	sprintf(data, "%d\t\t%d\n", getpid(), port);
	cat(PID_FILE, data);
}

static void
reaper()
{
	while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0) ;
}

static void
main_term()
{
	exit(0);
}

static void
main_signals()
{
	struct sigaction act;

/* act.sa_mask = 0; *//* Thor.981105: 标准用法 */
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	act.sa_handler = reaper;
	sigaction(SIGCHLD, &act, NULL);

	act.sa_handler = main_term;
	sigaction(SIGTERM, &act, NULL);

	act.sa_handler = SIG_IGN;
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);
	/*sigblock(sigmask(SIGPIPE)); */
}

#ifdef LINUX
struct proc_entry {
	const char *path;
	const char *name;
	const char *fmt;
};

#define SENSOR_P "/proc/sys/dev/sensors/lm87-i2c-0-2e/"

struct proc_entry my_proc[5] = {
	{SENSOR_P "temp1", "CPU1温度", "%*s %*s %20s"},
	{SENSOR_P "temp2", "CPU2温度", "%*s %*s %20s"},
	{SENSOR_P "fan1", "FAN1转速", "%*s %20s"},
	{SENSOR_P "fan2", "FAN2转速", "%*s %20s"},
	{0, 0, 0}
};

void
show_proc_info(void)
{
	int i;
	FILE *fp;
	char buf[256];
	prints("\033[1;32m健康状态\033[36m");
	for (i = 0; my_proc[i].path; i++) {
		fp = fopen(my_proc[i].path, "r");
		if (NULL == fp)
			continue;
		if (1 == fscanf(fp, my_proc[i].fmt, buf))
			prints(" %s [\033[33m%s\033[36m]", my_proc[i].name,
			       buf);
		fclose(fp);
	}
	prints("\033[m\n\r");
}

void
show_bandwidth_info(void)
{
	FILE *fp;
	char buf[256];
	fp = fopen("bbstmpfs/dynamic/ubar.txt", "r");
	if (NULL == fp)
		return;
	prints("\033[1;32m网络流量监测");
	if (fgets(buf, 256, fp))
		prints("%s", buf);
	fclose(fp);
	prints("\033[m\n\r");
}
#endif

int
bbs_main(hid)
char *hid;
{
	char buf[256];
	char bbs_prog_path[256];
	char bbstest_prog_path[256];
/* load control for BBS */
#ifdef LOAD_LIMIT
	{
		double cpu_load[3];
		int load;

		get_load(cpu_load);
		load = cpu_load[0];
		if (big5)
			prints
			    ("\033[1;36mBBS 程 \033[33m(1,5,15)\033[36m だ牧キА璽颤だ\033[33m %.2f, %.2f, %.2f \033[36m(ヘ玡 = %d).\033[0m\n\r\n\r",
			     cpu_load[0], cpu_load[1], cpu_load[2], max_load);
		else
			prints
			    ("\033[1;36mBBS 最近 \033[33m(1,5,15)\033[36m 分钟的平均负荷分别为\033[33m %.2f, %.2f, %.2f \033[36m(目前上限 = %d).\033[0m\n\r",
			     cpu_load[0], cpu_load[1], cpu_load[2], max_load);

		if (load < 0 || load > max_load) {
			if (big5)
				prints
				    ("╆簆,ヘ玡╰参璽颤筁, 叫祔ㄓ\n\r");
			else
				prints
				    ("很抱歉,目前系统负荷过重, 请稍后再来\n\r");
			close(csock);
			exit(-1);
		}
	}
#endif				/* LOAD_LIMIT */
	{
		FILE *fp;

		if ((fp = fopen("NOLOGIN", "r")) != NULL
		    && (!runtest || access("CANTEST", F_OK))) {
			while (fgets(buf, 256, fp) != NULL)
				prints(buf);
			fclose(fp);
			close(csock);
			exit(-1);
		}
	}

#ifdef BBSRF_CHROOT
	sprintf(bbs_prog_path, "/bin/bbs", MY_BBS_HOME);
	sprintf(bbstest_prog_path, "/bin/bbstest", MY_BBS_HOME);
	if (chroot(BBSHOME) != 0) {
		prints("Cannot chroot, exit!\r\n");
		exit(-1);
	}
#else
	sprintf(bbs_prog_path, "%s/bin/bbs", MY_BBS_HOME);
	sprintf(bbstest_prog_path, "%s/bin/bbstest", MY_BBS_HOME);
#endif

	if (checkbansite(hid)) {
		if (big5)
			prints("セヘ玡ぃ舧ㄓ %s 砐拜!\r\n", hid);
		else
			prints("本站目前不欢迎来自 %s 访问!\r\n", hid);
		shutdown(csock, 2);
		close(csock);
		exit(-1);
	}

	hid[40] = '\0';

	if (big5) {
		if (!runtest)
			execl(bbs_prog_path, "bbs", "e", hid, NULL);	/*调用BBS */
		else
			execl(bbstest_prog_path, "bbstest", "e", hid, NULL);	/*调用BBS */
	} else {
#ifdef LINUX
		//show_proc_info();  by bjgyt
		//show_bandwidth_info();
		prints("\033[0m\n\r");
//		if(!runtest)
//			sleep(2);
#endif
		if (!runtest)
			execl(bbs_prog_path, "bbs", "d", hid, NULL);	/*调用BBS */
		else {
#if 1
			execl(bbstest_prog_path, "bbstest", "d", hid, NULL);	/*调用BBS */
#else
			execl("/usr/bin/memusage", "memusage", "-u", "-d", "bbstestmemusage.dat",
				MY_BBS_HOME "/bin/bbstest", "d", hid, NULL);
#endif
		}
	}
	write(0, "execl failed\r\n", 12);
	exit(-1);
}

int
main(argc, argv)
int argc;
char *argv[];
{
	socklen_t value;
	fd_set fds;
	struct sockaddr_in6 sin;	//ipv6
	char hid[INET6_ADDRSTRLEN];	//ipv6

	main_signals();
	start_daemon(argc > 2, atoi(argv[argc - 1]));
	char cp[INET6_ADDRSTRLEN];
//  main_signals();

	if (argc <= 2)
		for (;;) {
			FD_ZERO(&fds);
			FD_SET(0, &fds);
			if (select(1, &fds, NULL, NULL, NULL) < 0)
				continue;
/*
    value = 1;
    if (select(1, (fd_set *) & value, NULL, NULL, NULL) < 0)
      continue;
*/
			value = sizeof (sin);
			csock = accept(0, (struct sockaddr *) &sin, &value);
			if (csock < 0) {
				reaper();
				continue;
			}
//add by ylsdd, 对抗上站机
			if (checkaddr(sin.sin6_addr, csock) < 0) {	//ipv6
				close(csock);
				continue;
			}

			if (fork()) {
				close(csock);

#ifdef LOAD_LIMIT
				{
					double cpu_load[3];
					int load;
					get_load(cpu_load);
					load = cpu_load[0];
					if (load < 0 || load > max_load)
						sleep(5);	/* sleep for heavy load */
				}
#endif
				continue;
			}

			if (csock) {
				dup2(csock, 0);
				close(csock);
			}
			break;
	} else {
		int sinlen = sizeof (sin);	//ipv6
		getpeername(0, (struct sockaddr *) &sin, (void *) &sinlen);
	}
#ifdef GETHOST
/*
	whee =
	    gethostbyaddr((char *) &sin.sin_addr.s_addr,
			  sizeof (struct in_addr), AF_INET);
	if ((whee) && (whee->h_name[0])) {
		strncpy(hid, whee->h_name, 17);
		hid[40] = 0;
	} else
*/
#endif
	{		//ipv6

	   inet_ntop(AF_INET6, (struct in6_addr *)&(sin.sin6_addr), cp, INET6_ADDRSTRLEN);

		if(is4map6addr(cp))
		{
				strcpy(hid,getv4addr(cp));
		}
		else
				strncpy(hid,cp,INET6_ADDRSTRLEN);
		/*
		char *host = (char *) inet_ntoa(sin.sin_addr);
		strncpy(hid, host, 17);
		hid[40] = 0;*/
	}

//#ifndef CAN_EXEC
	telnet_init();
//#endif
	bbs_main(hid);
	return 0;
}

//checkaddr(...), add by ylsdd, 对抗上站机
#define NADDRCHECK 500
struct {
	//ipv6
	struct in6_addr addr;
	time_t t;
	float x;
	int n;
} addrcheck[NADDRCHECK];

int
checkaddr(struct in6_addr addr, int csock)
{
	int i, j;
	static int fd = -1, lll = 1;
	char str[150];
	time_t timenow, ttemp;
	char str_addr[INET6_ADDRSTRLEN];

	if (fd < 0) {
		fd = open("/tmp/attacklog", O_CREAT | O_WRONLY | O_APPEND,
			  0664);
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
		if (timenow - addrcheck[i].t > 60 * 5
		    || timenow < addrcheck[i].t) {
			if (addrcheck[i].x > 100 && addrcheck[i].n > 7) {
				/*
				sprintf(str, "remove\t%s\t%d\t%s",
					inet_ntoa(addrcheck[i].addr),
					addrcheck[i].n, ctime(&timenow));
				*/
				//ipv6
				if(inet_ntop(PF_INET6,(const void *)&addrcheck[i].addr, str_addr, INET6_ADDRSTRLEN) != NULL) {
					sprintf(str, "remove\t%s\t%d\t%s", str_addr, addrcheck[i].n, ctime(&timenow));
				}
				write(fd, str, strlen(str));
			}
			addrcheck[i].t = 0;
			continue;
		}
		if (memcmp(&addrcheck[i].addr, &addr, sizeof (addr)) == 0) {
			if (addrcheck[i].x <= 100 || addrcheck[i].n <= 7) {
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

			if (addrcheck[i].x > 100 && addrcheck[i].n > 7) {
				if (j == 0 && fd >= 0)
					if (fork() == 0) {
						/*
						sprintf(str, "add\t%s\t%d\t%s",
							inet_ntoa(addr),
							addrcheck[i].n,
							ctime(&timenow));
						*/
					//ipv6
					if (inet_ntop(PF_INET6,(const void *)&addrcheck[i].addr, str_addr, sizeof(struct in6_addr)) != NULL) {
						sprintf(str, "add\t%s\t%d\t%s", str_addr, addrcheck[i].n, ctime(&timenow));
					}

					  write(fd, str, strlen(str));
						write(csock,
						      "对不起, 连接将封闭5分钟。请不要不断连接冲击本站\n",
						      strlen
						      ("对不起, 连接将封闭5分钟。请不要不断连接冲击本站\n"));
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
		    && (addrcheck[i].x <= 100 || addrcheck[i].n <= 7)) {
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
	if (addrcheck[j].x > 100 && addrcheck[j].n > 7) {
		//sprintf(str, "remove\t%s\t%d\t%s", inet_ntoa(addrcheck[i].addr),
		//	addrcheck[j].n, ctime(&timenow));
				//ipv6
				if (inet_ntop(PF_INET6,(const void *)&addrcheck[i].addr, str_addr, sizeof(struct in6_addr)) != NULL) {
					sprintf(str, "remove\t%s\t%d\t%s", str_addr, addrcheck[i].n, ctime(&timenow));
				}
		write(fd, str, strlen(str));
	}
	addrcheck[j].addr = addr;
	addrcheck[j].t = timenow;
	addrcheck[j].x = 0;
	addrcheck[j].n = 1;
	return 0;
}

int
killer()
{
	int shmid, i, j, k, pid, fd;
	struct UTMPFILE *utmpshm;
	exit(1);
	fd = open(".killerlock", O_RDONLY | O_CREAT, 0600);
	if (fd < 0)
		return -1;
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
		return -1;
	shmid = shmget(UTMP_SHMKEY, sizeof (struct UTMPFILE), 0);
	if (shmid < 0)
		return -1;
	utmpshm = (struct UTMPFILE *) shmat(shmid, NULL, 0);
	if (utmpshm == (struct UTMPFILE *) -1)
		return -1;
	while (1) {
		for (i = 0, j = 0, k = 0; i < USHM_SIZE; i++) {
			if (utmpshm->uinfo[i].active != 1)
				continue;
			if ((pid = utmpshm->uinfo[i].pid) <= 0)
				continue;
			kill(pid, SIGTTOU);
			k++;
			if (k % ((USHM_SIZE / 18 == 0) ? 1 : (USHM_SIZE / 18)))
				continue;
			j++;
			sleep(1);
		}
		sleep((j > 20) ? 0 : 20 - j);
	}
}
