// by zhch
#include "../include/bbs.h"
#include "sys/socket.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdarg.h"

#include "bshm.h"

#define PORT	10111
char *servernow = "";

/* iconf 的格式: 转信站地址	对方转信版面	本地转入版面 */

char *iconf[] = {
	//"bbs.nju.edu.cn               BBSDev          test",
	//"bbs.nju.edu.cn               test            deleted",
	//"bbs.nju.edu.cn		BBSDev			BBSDev",
	//"bbs.nju.edu.cn               LinuxUnix               Linux",
	//"bbs.nju.edu.cn         Abroad               AdvancedEdu",
	"sesa.nju.edu.cn	BBSDev			BBSDev",
	//"sesa.nju.edu.cn      Linux                   Linux",
	//"bbs.hhu.edu.cn               Linux                   Linux",
	"bbs.hhu.edu.cn         NewBBSDev               BBSDev",
	"bbs.hhu.edu.cn         InstallBBS              BBSDev",
	//"bbs.ustb.edu.cn      UnixLinux               Linux",
	"bbs.ustb.edu.cn	BBSDev			BBSDev",
	//"bbs.ustb.edu.cn      gowest                  AdvancedEdu",
	//"bbs.sjtu.edu.cn        installbbs              BBSDev",
	"bbs.whnet.edu.cn       BBSDev                  BBSDev",
	//"bbs.pku.edu.cn               BBSDev                  BBSDev",
	"bbs.pku.edu.cn		Mud_builder		Mud_Builder",
	"bbs.pku.edu.cn		MUD			Mud",
	"bbs.pku.edu.cn		Mechanics		mechanics",
	//"fb2000.dhs.org         BBSdev                  BBSDev",
	//"fb2000.dhs.org         zBBSdev                 BBSDev",
	"bbs.feeling.dhs.org    InstallBBS		BBSDev",
	//"bbs.feeling.dhs.org    Linux                 Linux",
	//"sbbs.seu.edu.cn      Linux                   Linux",
	"sbbs.seu.edu.cn	BBSDev			BBSDev",
	//"bbs.swjtu.edu.cn     Unix_Linux              Linux",
	//"bbs.swjtu.edu.cn     BBSinst_dev             BBSDev",
	"bbs.swjtu.edu.cn	Programming		programming",
	//"bbs.swjtu.edu.cn     JAVA                    JAVA",
	//"bbs.whu.edu.cn               BBSDev                  BBSDev",
	"bbs.wuhee.edu.cn	BBS_Admin		BBSDev",
	"bbs.neu.edu.cn		BBSDev			BBSdev",
	"dhxy.dhs.org		Article			dahua",
	NULL
};

/* 转信黑名单: 不转入这些人的文章 */

char *black_user[] = {
	"deliver",
	"anonymous",
	"guest",
	"SYSOP",
	"",
	" ",
	NULL
};

int
valid_user(char *user)
{
	int i;
	char s2[80], *p;
	strcpy(s2, user);
	p = strchr(s2, '.');
	if (p)
		p[0] = 0;
	for (i = 0; black_user[i] != NULL && i < 999; i++)
		if (!strcasecmp(s2, black_user[i]))
			return 0;
	return 1;
}

int
do_log(char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	char cmd[256];
	time_t t = time(0);
	va_start(ap, fmt);
	vsnprintf(cmd, 255, fmt, ap);
	va_end(ap);
	fp = fopen("bbsinnd.log", "a");
	fprintf(fp, "%24.24s %-24s %s\n", ctime(&t), servernow, cmd);
	fclose(fp);
}

int last;

int
main()
{
	int i;
	FILE *fp;
	if (fork())
		exit(0);
	chdir(MY_BBS_HOME);
	for (i = 0; i <= getdtablesize(); i++)
		close(i);
	for (i = 1; i <= NSIG; i++)
		signal(i, SIG_IGN);
	signal(SIGALRM, SIG_DFL);
	last = time(0) - 86400;	//首次运行的缺省值: 1天以内的文章。
	fp = fopen("etc/bbsinnd.last", "r");
	if (fp) {
		fscanf(fp, "%d", &last);
		fclose(fp);
	}
	while (1) {
		int r;
		char host[80], oboard[80], iboard[80];
		for (i = 0; iconf[i] != 0; i++) {
			r = sscanf(iconf[i], "%s %s %s", host, oboard, iboard);
			if (r < 3)
				continue;
			if (fork() > 0) {
				alarm(900);
				get_mail(host, oboard, iboard);
				exit(0);
			}
			sleep(3);
		}
		last = time(0);
		fp = fopen("etc/bbsinnd.last", "w");
		fprintf(fp, "%d\n", last);
		fclose(fp);
		sleep(1000);
	}
}

int
get_mail(char *host, char *oboard, char *iboard)
{
	int fd;
	FILE *fp0;
	int i;
	struct sockaddr_in xs;
	struct hostent *he;
	char dir[80], file[80], buf[256], brk[80];
//      file: 本地文件名, buf: 临时变量, brk: 分隔符.

	servernow = host;

	do_log("gethostbyname %s", host);
	bzero((char *) &xs, sizeof (xs));
	xs.sin_family = AF_INET;
	if ((he = gethostbyname(host)) != NULL)
		bcopy(he->h_addr, (char *) &xs.sin_addr, he->h_length);
	else
		xs.sin_addr.s_addr = inet_addr(host);
	xs.sin_port = htons(PORT);
	do_log("connecting %s", host);
	fd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
	if (connect(fd, (struct sockaddr *) &xs, sizeof (xs)) < 0) {
		fd_set fds;
		struct timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(fd + 1, NULL, &fds, NULL, &timeout) <= 0) {
			do_log("can't connect to %s", host);
			close(fd);
			return;
		}
		if (connect(fd, (struct sockaddr *) &xs, sizeof (xs)) < 0) {
			do_log("can't connect to %s", host);
			close(fd);
			return;
		}
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
	do_log("connected %s", host);
	fp0 = fdopen(fd, "r+");
	if (fgets(brk, 80, fp0) == 0)
		goto E;
	if (strlen(brk) < 10)
		goto E;
	fprintf(fp0, "select * from %s where dt < %d\n", oboard,
		time(0) - last);
	fflush(fp0);
	do_log("SEND request %s %d", oboard, time(0) - last);
	while (1) {
		FILE *fp;
		int t;
		struct fileheader x;
		char owner[256];
		bzero(&x, sizeof (x));
		do_log("reading...");
		if (fgets(x.title, sizeof (x.title), fp0) == 0)
			break;
		if (fgets(owner, sizeof (owner), fp0) == 0)
			break;
		fh_setowner(&x, owner, 0);
		check_str(x.title);
		check_str(x.owner);
		removetailspace(x.title);
		do_log("%s", x.title);
		sprintf(file, "boards/%s/", iboard);
		t = trycreatefile(file, "M.%d.A", time(NULL), 100);
		x.filetime = t;
		do_log(file);
		do_log(fh2fname(&x));
		fp = fopen(file, "w");
		while (1) {
			if (fgets(buf, 255, fp0) == 0)
				break;
			if (!strcmp(buf, brk))
				break;
			fprintf(fp, "%s", buf);
		}
		fclose(fp);
		if (!valid_user(x.owner)) {
			unlink(file);
			do_log("bad user: %s", x.owner);
			continue;
		}
		fh_find_thread(&x, iboard);
		sprintf(dir, "boards/%s/.DIR", iboard);
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT, 0660);
		write(fd, &x, sizeof (x));
		close(fd);
		do_log("updatelastpost %s", iboard);
		updatelastpost(iboard);
	};
      E:fclose(fp0);
	close(fd);
	do_log("done");
}

int
f_exist(char *file)
{
	struct stat buf;
	if (stat(file, &buf) == -1)
		return 0;
	return 1;
}

int
check_str(unsigned char *s)
{
	int i;
	for (i = 0; i < strlen(s); i++)
		if (s[i] < 32 || s[i] == 255)
			s[i] = 0;
}

int
removetailspace(char *str)
{
	int i = strlen(str) - 1;
	for (; i > 0; i--) {
		if (!strchr("\t \r\n", str[i]))
			break;
		str[i] = 0;
	}
}
