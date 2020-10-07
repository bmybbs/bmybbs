#include <time.h>
#include <stdlib.h>
#include <iconv.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "ythtbbs/ythtbbs.h"

int
init_newtracelogmsq() {
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(BBSLOG_MSQKEY, IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &buf);
	buf.msg_qbytes = 50 * 1024;
	msgctl(msqid, IPC_SET, &buf);
	return msqid;
}

void newtrace(char *s) {
	static int disable = 0;
	static int msqid = -1;
	time_t dtime;
	char buf[512];
	char timestr[16];
	char *ptr;
	struct tm *n;
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;
	if (disable)
		return;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(timestr, "%02d:%02d:%02d", n->tm_hour, n->tm_min, n->tm_sec);
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype),
		 "%s %s\n", timestr, s);
	ptr = msg->mtext;
	while ((ptr = strchr(ptr, '\n'))) {
		if (!ptr[1])
			break;
		*ptr = '*';
	}
	msg->mtype = 1;
	if (msqid < 0) {
		msqid = init_newtracelogmsq();
		if (msqid < 0) {
			disable = 1;
			return;
		}
	}
	msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR);
	return;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
	iconv_t cd;
	size_t rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf,0,outlen);

	rc = iconv(cd,pin,&inlen,pout,&outlen);
	iconv_close(cd);

	return (rc == (size_t) -1) ? -1 : 0;
}

//UNICODE码转为GBK码
int u2g(char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
	return code_convert("utf-8","gbk",inbuf,inlen,outbuf,outlen);
}
//GBK码转为UNICODE码
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
	return code_convert("gbk","utf-8",inbuf,inlen,outbuf,outlen);
}

int is_utf_special_byte(unsigned char c){
	unsigned special_byte = 0X02;	//binary 00000010
	if(c>>6==special_byte)
		return 1;
	else
		return 0;
}

//判断是否为UNICODE编码
int is_utf(char * inbuf, size_t inlen) {
	unsigned one_byte   = 0X00; 	//binary 00000000
	unsigned two_byte   = 0X06; 	//binary 00000110
	unsigned three_byte = 0X0E; 	//binary 00001110
	unsigned four_byte  = 0X1E; 	//binary 00011110
	unsigned five_byte  = 0X3E; 	//binary 00111110
	unsigned six_byte   = 0X7E; 	//binary 01111110

	unsigned int i;
	unsigned int c;

	unsigned char k = 0;
	unsigned char m = 0;
	unsigned char n = 0;
	unsigned char p = 0;
	unsigned char q = 0;

	int utf_yes = 0;

	for (i=0;i<inlen;){
		c=(unsigned char)inbuf[i];
		if(c>>7==one_byte){
			i++;
			continue;
		} else if(c>>5==two_byte){
			k = (unsigned char)inbuf[i+1];
			if(is_utf_special_byte(k)){
				return 1;
			}
		} else if(c>>4==three_byte){
			m = (unsigned char)inbuf[i+1];
			n = (unsigned char)inbuf[i+2];
			if(is_utf_special_byte(m) && is_utf_special_byte(n)){
				return 1;
			}
		} else if(c>>3==four_byte){
			k = (unsigned char)inbuf[i+1];
			m = (unsigned char)inbuf[i+2];
			n = (unsigned char)inbuf[i+3];
			if(is_utf_special_byte(k)
					&& is_utf_special_byte(m)
					&& is_utf_special_byte(n)){
				return 1;
			}
		} else if(c>>2 == five_byte){
			k = (unsigned char)inbuf[i+1];
			m = (unsigned char)inbuf[i+2];
			n = (unsigned char)inbuf[i+3];
			p = (unsigned char)inbuf[i+4];
			if(is_utf_special_byte(k)
					&& is_utf_special_byte(m)
					&& is_utf_special_byte(n)
					&& is_utf_special_byte(p)){
				return 1;
			}
		} else if(c>>1==six_byte){
			k = (unsigned char)inbuf[i+1];
			m = (unsigned char)inbuf[i+2];
			n = (unsigned char)inbuf[i+3];
			p = (unsigned char)inbuf[i+4];
			q = (unsigned char)inbuf[i+5];
			if ( is_utf_special_byte(k)
					&& is_utf_special_byte(m)
					&& is_utf_special_byte(n)
					&& is_utf_special_byte(p)
					&& is_utf_special_byte(q) ) {
				return 1;
			}
		}
	}
	return 0;
}

// add by IronBlood@bmy 20120107
static char *get_login_pic_link (char *picname, char *linkback) {
	FILE *fp;
	char link[256];
	memset(link, '\0',sizeof(link));
	char linkfile[256];
	sprintf(linkfile, MY_BBS_HOME "/loglinks/%s", picname);
	if (!(fp = fopen ( linkfile,"r")))
		return "BMY/home?B=XJTUnews";
	if (!fgets (link,sizeof (link),fp))
		return "BMY/home?B=XJTUnews";
	fclose (fp);
	if (link[strlen(link) - 1] == '\n')
		link[strlen(link) - 1] = '\0';
	return strcpy(linkback, link);
}

// added by IronBlood@11.09.05
// 修正函数返回值，记得释放 by IronBlood@2014.10.22
char *get_no_more_than_four_login_pics() {
	FILE *fp;
	if(!(fp = fopen(MY_BBS_HOME "/logpics","r")))
		return "cai.jpg";

	char pics[256];
	const char *pics_dir ="bmyMainPic/using/";
	char pics_list[4096];
	char file[16][256];
	int file_line=0;
	char link[256];
	memset(pics_list, '\0', sizeof(pics_list));

	// 读取文件
	while(fgets(pics,sizeof(pics),fp)!=NULL)
	{
		char *tmp=file[file_line];
		if (pics[strlen(pics) - 1] == '\n')
			pics[strlen(pics) - 1] = 0;
		strcpy(tmp,pics);
		++file_line;
	}
	// 释放句柄
	fclose(fp);

	int i=0;

	while( (i != file_line - 1) && i !=4) // 不超过总图片个数、不超过最大上限
	{
		srand(time(NULL)+rand()%100); // 加种子
		int randnum = 1 + rand()%file_line; // 生成随机数
		char *tmp = file[randnum];

		if( strstr(pics_list,tmp)==NULL ) //不包含图片字符串，才执行下面的操作
		{
			get_login_pic_link(tmp,link);
			if(i>0)
				strcat(pics_list, ";;");
			strcat(pics_list, pics_dir);
			strcat(pics_list, tmp);
			strcat(pics_list, ";");
			strcat(pics_list, link);
			++i;
		}
	}

	return strdup(pics_list);
}

int
badstr(char *s)
{
	int i;
	unsigned char *t = (unsigned char *)s;
	for (i = 0; s[i]; i++)
		if (t[i] != 9 && (t[i] < 32 || t[i] == 255))
			return 1;
	return 0;
}

