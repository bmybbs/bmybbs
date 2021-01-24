// NJU bbsnet, preview version, zhch@dii.nju.edu.cn, 2000.3.23 //

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "ytht/strlib.h"

char host1[100][40], host2[100][40], ip[100][40];
int port[100], counts = 0;
char str[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-_=+\\|[{};:'\"<>,./?";

char datafile[80] = "etc/bbsnet.ini";
char telnet[80] = "bin/telnet";
char userid[80] = "unknown.";

static void init_data(void);
static void sh(int n);
static void show_all(void);
static void locate(int n);
static int getch(void);
static void main_loop(void);
static void bbsnet(int n);
static void save_tty(void);
static void init_tty(void);
static void reset_tty(void);
static void syslog(char *s);

static void
init_data()
{
	FILE *fp;
	char t[256], *t1, *t2, *t3, *t4;
	fp = fopen(datafile, "r");
	if (fp == NULL)
		return;
	while (fgets(t, 255, fp) && (counts >= 0 && (unsigned) counts < strlen(str))) {
		t1 = strtok(t, " \t");
		t2 = strtok(NULL, " \t\n");
		t3 = strtok(NULL, " \t\n");
		t4 = strtok(NULL, " \t\n");
		if (t1 == NULL || t1[0] == '#' || t2 == NULL || t3 == NULL)
			continue;
		strncpy(host1[counts], t1, 16);
		strncpy(host2[counts], t2, 36);
		strncpy(ip[counts], t3, 36);
		port[counts] = t4 ? atoi(t4) : 23;
		counts++;
	}
	fclose(fp);
}

static void
sh(int n)
{
	static int oldn = -1;
	if (n >= counts)
		return;
	if (oldn >= 0) {
		locate(oldn);
		printf("\033[1;32m %c.\033[m%s", str[oldn], host2[oldn]);
	}
	oldn = n;
	locate(n);
	printf("[%c]\033[1;42m%s\033[m", str[n], host2[n]);
	printf("\033[22;3H\033[1;37m单位: \033[1;33m%s                   \033[22;32H\033[1;37m 站名: \033[1;33m%s              \r\n",
			host1[n], host2[n]);
	printf("\033[1;37m\033[23;3H连往: \033[1;33m%s %d                   \033[1;1H",
			ip[n], port[n]);
}

static void
show_all()
{
	int n;
	printf("\033[H\033[2J\033[m");
	printf("□□□□□□□□□□\033[1;37m   现在要连到哪里呢？（Ctrl+C退出） \033[m□□□□□□□□□□□□\r\n");
	for (n = 1; n < 23; n++)
		printf("□                                                                            □\r\n");
	printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□");
	printf("\033[21;3H----------------------------------------------------------------------------");
	for (n = 0; n < counts; n++) {
		locate(n);
		printf("\033[1;32m %c.\033[m%s", str[n], host2[n]);
	}
}

static void
locate(int n)
{
	int x, y;
	if (n >= counts)
		return;
	y = n % 19 + 2;
	x = n / 19 * 16 + 3;
	printf("\033[%d;%dH", y, x);
}

static int
getch()
{
	int c, d, e;
	static int lastc = 0;
	fflush(stdout);
	c = getchar();
	if (c == 10 && lastc == 13)
		c = getchar();
	lastc = c;
	if (c != 27)
		return c;
	d = getchar();
	e = getchar();
	if (d == 27)
		return 27;
	if (e == 'A')
		return 257;
	if (e == 'B')
		return 258;
	if (e == 'C')
		return 259;
	if (e == 'D')
		return 260;
	return 0;
}

static void
main_loop()
{
	int p = 0;
	int c, n;
L:
	show_all();
	sh(p);
	fflush(stdout);
	while (1) {
		c = getch();
		if (c == 3 || c == 4 || c == 27 || c < 0)
			break;
		if (c == 257 && p > 0)
			p--;
		if (c == 258 && p < counts - 1)
			p++;
		if (c == 259 && p < counts - 19)
			p += 19;
		if (c == 260 && p >= 19)
			p -= 19;
		if (c == 13 || c == 10) {
			bbsnet(p);
			goto L;
		}
		for (n = 0; n < counts; n++)
			if (str[n] == c)
				p = n;
		sh(p);
		fflush(stdout);
	}
}

static void
bbsnet(int n)
{
	char pp[180];
	if (n >= counts)
		return;
	printf("\033[H\033[2J\033[1;32mo 连往: %s (%s %d)\r\n", host2[n], ip[n], port[n]);
	printf("%s\r\n\r\n\033[m", "o 连不上时请稍候，20 秒后将自动退出");
	syslog(ip[n]);
	sprintf(pp, "%d", port[n]);
	fflush(stdout);
	reset_tty();
	execl(telnet, "telnet", ip[n], pp, NULL);
}

static struct termios t1, t2;

static void
save_tty()
{
	if (ttyname(1))
		tcgetattr(1, &t1);
}

static void
init_tty()
{
	if (ttyname(1)) {
		//cfmakeraw(&t2);
		t2.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
				| INLCR | IGNCR | ICRNL | IXON);
		t2.c_oflag &= ~OPOST;
		t2.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		t2.c_cflag &= ~(CSIZE | PARENB);
		t2.c_cflag |= CS8;
		tcsetattr(1, TCSANOW, &t2);
	}
}

static void
reset_tty()
{
	if (ttyname(1))
		tcsetattr(1, TCSANOW, &t1);
}

int
main(int n, char *cmd[])
{
	save_tty();
	init_tty();
	if (n >= 2)
		ytht_strsncpy(datafile, cmd[1], sizeof(datafile));
	if (n >= 3)
		ytht_strsncpy(telnet, cmd[2], sizeof(telnet));
	if (n >= 4)
		ytht_strsncpy(userid, cmd[3], sizeof(userid));
	init_data();
	main_loop();
	printf("\033[m");
	reset_tty();
	return 0;
}

static void
syslog(char *s)
{
	char timestr[16], *thetime;
	time_t dtime;
	FILE *fp;
	fp = fopen("bbsnet.log", "a");
	time(&dtime);
	thetime = (char *) ctime(&dtime);
	strncpy(timestr, &(thetime[4]), 15);
	timestr[15] = '\0';
	fprintf(fp, "%s %s %s\n", userid, timestr, s);
	fclose(fp);
}
