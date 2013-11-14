#include <sys/ipc.h>
#include <sys/msg.h>
#include "ythtbbs.h"
#include <time.h>
#include <stdlib.h>
#include <iconv.h>
int pu = 0;

void
getrandomint(unsigned int *s)
{
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, 4);
	close(fd);
#else
	srandom(getpid() - 19751016);
	*s=random();
#endif
}

void
getrandomstr(unsigned char *s)
{
	int i;
#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, 30);
	close(fd);
	for (i = 0; i < 30; i++)
		s[i] = 65 + s[i] % 26;
#else
	time_t now_t;
	now_t = time(NULL);
	srandom(now_t - 19751016);
	for (i = 0; i < 30; i++)
		s[i] = 65 + random() % 26;
#endif
	s[30] = 0;
}

void getrandomstr_r(unsigned char *s, size_t len)
{
	int i, fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, s, len);
	close(fd);
	for(i=0; i<len; ++i) {
		s[i] = s[i]%26 + 'A';
	}
	s[len-1] = 0;
}

int
init_newtracelogmsq()
{
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

void
newtrace(s)
char *s;
{
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

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
	iconv_close(cd);
	return 0;
}

//UNICODE码转为GB2312码
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312码转为UNICODE码
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

int is_utf_special_byte(unsigned char c){
	unsigned special_byte = 0X02;	//binary 00000010
	if(c>>6==special_byte)
		return 1;
	else
		return 0;
}
//判断是否为UNICODE编码
int is_utf(char * inbuf, size_t inlen)
{
    unsigned one_byte 	= 0X00; 	//binary 00000000
    unsigned two_byte 	= 0X06; 	//binary 00000110
    unsigned three_byte = 0X0E; 	//binary 00001110
    unsigned four_byte 	= 0X1E; 	//binary 00011110
    unsigned five_byte 	= 0X3E; 	//binary 00111110
    unsigned six_byte 	= 0X7E; 	//binary 01111110

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
    		if(is_utf_special_byte(m)
    				&& is_utf_special_byte(n)){
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
