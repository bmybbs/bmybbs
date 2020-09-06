#include "../include/bbs.h"
#include "sys/socket.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdarg.h"

#include "bshm.h"

#define PORT	10111


/* iconf 的格式: 转信站地址	对方转信版面	本地转入版面 */

char *iconf[]={
	"bbs.nju.edu.cn",
	"sesa.nju.edu.cn",
	"bbs.hhu.edu.cn",
	"bbs.ustb.edu.cn",
	"bbs.sjtu.edu.cn",
	"bbs.whnet.edu.cn",
	"bbs.pku.edu.cn",
	//"fb2000.dhs.org",
	//"bbs.feeling.dhs.org",
	"sbbs.seu.edu.cn",
	"bbs.swjtu.edu.cn",
	NULL
};


int main(int argn, char **argv) {
	int i;
	FILE *fp;
	if(argn>1)
		get_mail(argv[1]);
	else for(i=0; iconf[i]!=NULL; i++) {
		get_mail(iconf[i]);
	}
}

int get_mail(char *host) {
	int fd;
	int i;
	FILE *fp;
	struct sockaddr_in xs;
	struct hostent *he;
	char buf[200];
	buf[199]=0;
//	file: 本地文件名, buf: 临时变量, brk: 分隔符.

	bzero((char*) &xs, sizeof(xs));
	xs.sin_family=AF_INET;
	if((he=gethostbyname(host))!=NULL)
		bcopy(he->h_addr, (char*) &xs.sin_addr, he->h_length);
	else
		xs.sin_addr.s_addr=inet_addr(host);
	xs.sin_port=htons(PORT);
	fd=socket(AF_INET, SOCK_STREAM, 0);
	if(connect(fd, (struct sockaddr*) &xs, sizeof(xs))<0) {
		close(fd);
		printf("%s: 没有连上\n", host);
		return;
	}
	fp=fdopen(fd,"r+");
	if(fgets(buf, 199, fp)==0) return;
	if(strlen(buf)<10) {
		fclose(fp);
		return;
	}
	fprintf(fp,"select * from oboard_ctl where dt < 0\n");
	fflush(stdout);
	printf("-------%s-------\n", host);
	while(fgets(buf, 200, fp)!=0)
		printf("%s", buf);
	fclose(fp);
}

