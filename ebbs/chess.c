//Copyright: ylsdd
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include "keys.h"
struct termios tsave;
int place=0, inputlen=0;
int chesstype=0;
char inputline[80];
char roomname[20];
char roomtitle[30];
int yy=1, xx=1, hh=6;

void scan_mode()
{
	struct termios tbuf;
	tcgetattr(0,&tbuf);
	tsave=tbuf;
	tbuf.c_lflag&=~(ECHO|ICANON|ISIG);
	tbuf.c_cc[VMIN]=tbuf.c_cc[VTIME] =0;
	tcsetattr(0,TCSANOW,&tbuf);
}

void restore_exit(int i)
{
	tcsetattr(0,TCSANOW,&tsave);
	exit(i);
}

int sfd;
char inbuf[64*1024];
int ninbuf=0;

struct mymsg {
	short len;
	unsigned char cmd;
	char data[1];
};

struct mymsg *mallocmymsg(int len)
{
	return (struct mymsg *)malloc(len);
}


int readtobuf()
{
	int retv=read(sfd, inbuf+ninbuf, sizeof(inbuf)-ninbuf);
	if(retv<=0)
		restore_exit(0);
	ninbuf+=retv;
	return 0;
}

struct mymsg *parsemsg()
{
	struct mymsg *mp;
	int len=ntohs(*(unsigned short*)inbuf);
	if(len>ninbuf) return NULL;
	mp=mallocmymsg(len);
	memcpy(mp, inbuf, len);
	mp->len=len;
	ninbuf-=len;
	memmove(inbuf, inbuf+len, ninbuf);
#ifdef DEBUG
	move(25, 1);
	clrtoeol();
	printf("%d new msg! cmd=%d data=%s\n", (int)time(NULL), (int)mp->cmd, mp->data);
#endif
	return mp;
}

int sendmymsg(struct mymsg *mp)
{
	int len=mp->len;
	mp->len=htons(mp->len);
#ifdef DEBUG
	move(26,1);
	clrtoeol();
	printf("%d sending msg, cmd=%d, data=%s\n", (int)time(NULL), (int)mp->cmd, mp->data);
#endif
	if(write(sfd, mp, len)<0) {
		free(mp);
		return -1;
	}
	free(mp);
	return 0;
}

struct mymsg *simplemsg(unsigned char cmd)
{
	struct mymsg *msg;
	msg=mallocmymsg(3);
	msg->len=3;
	msg->cmd=cmd;
	return msg;
}

struct mymsg *strmsg(unsigned char cmd, char *str)
{
	int len;
	struct mymsg *msg;
	len=strlen(str);
	msg=mallocmymsg(4+len);
	msg->len=4+len;
	msg->cmd=cmd;
	memcpy(msg->data, str, len+1);
	return msg;
}

int getconnect(char *servername, int port)
{
	struct sockaddr_in sin ;
	struct hostent *h2;
	int fd;
	if((h2 = gethostbyname(servername))==NULL)
		sin.sin_addr.s_addr = inet_addr(servername);
	else
		memcpy(&sin.sin_addr.s_addr, h2->h_addr, h2->h_length);
	sin.sin_family=AF_INET;
	sin.sin_port = htons(port);
	fd=socket(PF_INET,SOCK_STREAM,0);
	if(fd<0) return -1;
	if(connect(fd, (struct sockaddr *)&sin, sizeof(sin))) {
		close(fd);
		return -1;
	}
	fcntl(fd, F_SETFL, O_NONBLOCK|fcntl(fd, F_GETFL));
	return fd;
}

int maninput(char ch);

int main(int argn, char**argv)
{
	struct mymsg *mp;
	int retv;
	fd_set fdsr, fdse;
	char ch;

	if(fork()==0) {
		close(0);
		close(1);
		close(2);
		execl("/home/bbs/bin/chessd","chessd", NULL);
		exit(0);
	}
	scan_mode();
	printf("\033[2J");
	signal(SIGPIPE, restore_exit);
	
	sfd=getconnect("ytht.net", 3333);
	if(sfd<0) restore_exit(0);

	if(argn>1) sendmymsg(strmsg(0,argv[1]));
	else sendmymsg(strmsg(0,"guest"));
	
	while(1) {
		if(place) move(yy,xx);
		else move(24,inputlen+1+6);
		fflush(stdout);
		FD_ZERO(&fdsr);
		FD_SET(0,&fdsr);
		FD_SET(sfd,&fdsr);
		fdse=fdsr;
		retv=select(sfd+1,&fdsr,NULL,&fdse,NULL);
		if(retv<=0) restore_exit(0);
		if(FD_ISSET(0,&fdse)||FD_ISSET(sfd, &fdse))
			restore_exit(0);
		if(FD_ISSET(sfd,&fdsr)) {
			readtobuf();
			fflush(stdout);
			while((mp=parsemsg())!=NULL)
				dispatchmsg(mp);
		}
		if(FD_ISSET(0,&fdsr)) {
			if(read(0,&ch,1)<=0) restore_exit(0);
			maninput(ch);
		}
	}
}

int clr_initscr();
int gen_initscr();
int five_initscr();
int five_manchar(int ch);
int five_manmsg(struct mymsg*mp);
int cchess_initscr();
int cchess_manchar(int ch);
int cchess_manmsg(struct mymsg*mp);
int go_initscr();
int go_manchar(int ch);
int go_manmsg(struct mymsg*mp);

struct {
	int (*initscr)();
	int (*manchar)(int);
	int (*manmsg)(struct mymsg*);
} chessop[4]={
	{gen_initscr, NULL, NULL},
	{five_initscr, five_manchar, five_manmsg},
	{cchess_initscr, cchess_manchar, cchess_manmsg},
	{go_initscr, go_manchar, go_manmsg}
};

int dispatchmsg(struct mymsg *mp)
{
	switch(mp->cmd) {
		case 0:
			break;
		case 1:
			restore_exit(0);
		case 5:
		case 'C':
			addchatline(mp->data);
			break;
		case 2:
			addchatline("\033[33m现在的房间有:\033[m\033[0m");
			addchatline(mp->data);
			break;
		case 3:
			addchatline("\033[33m本房间的用户有:\033[m\033[0m");
			addchatline(mp->data);
			break;
		case 6:
			if(chessop[chesstype].manmsg!=NULL)
				chessop[chesstype].manmsg(mp);
			break;
		case 4:
			{
				char *ptr, str[80];
				sscanf(mp->data, "%s", roomname);
				ptr=strchr(mp->data,' ');
				if(ptr==NULL)
					sprintf(roomtitle,"今天你糊涂了吗?");
				else
					strcpy(roomtitle, ptr+1);
				sprintf(str,"\033[33m你进入了 \033[32m%s \033[33m房间\033[m\033[0m", roomname);
				addchatline(str);
			}
			break;
		case 7:
			chesstype=atoi(mp->data);
			//if(chesstype!=0) chesstype=1;
			chessop[chesstype].initscr();
			break;
	}
		
	free(mp);
	return 0;
}

int addbotchar(char ch);
int KEY_ESC_arg;
int transkey(char ch)
{
	static int mode=0, last=0;

	if(mode==0) {
		if( ch == KEY_ESC ) mode = 1;
		else { 
			if((ch=='\n'&&last=='\r')||(ch=='\r'&&last=='\n')) {
				last=0;
				return 0;
			}
			if(ch=='\n'||ch=='\r') {
				last=ch;
				return '\n';
			}
			return ch;
		}
	} else if(mode==1) {
		if( ch == '[' || ch == 'O' )  mode = 2;
		else if( ch == '1' || ch == '4' )  mode = 3;
		else { mode=0; KEY_ESC_arg=ch; return KEY_ESC; }
	} else if(mode==2) {
		mode=0;
		if( ch >= 'A' && ch <= 'D' )
			return KEY_UP + (ch - 'A');
		else if( ch >= '1' && ch <= '6' )  mode = 3;
		else return ch;
	} else if( mode == 3 ) {
		mode=0;
		if( ch == '~' )
			return KEY_HOME + (last - '1');
		else  return ch;
	}
	last = ch;
	return 0;
}

int maninput(char ch)
{
	int key;
	
	key=transkey(ch);
	if(key==0) return 0;

	if(key=='\t') {
		place=!place;
		move(24-1-hh,1);
		if(place) printf("\033[45m∧\033[m\033[0m");
		else printf("\033[45m∨\033[m\033[0m");
		return 0;
	}
	if(place==0) {
		if(key=='\n') {
			analysisline(inputline);
			inputlen=0;
			inputline[0]=0;
			clrbotline();
			return 0;
		}
		if(isprint2(key)&&inputlen<80) {
			addbotchar(key);
			inputline[inputlen++]=key;
			inputline[inputlen]=0;
			return 0;
		}
		if(key==8&&inputlen>0) {
			delbotchar();
			inputline[inputlen--]=0;
			return 0;
		}
		return 0;
	}
	if(chessop[chesstype].manchar!=NULL)
		chessop[chesstype].manchar(key);
	return 0;
}

int addchatline(char *str)
{
	static int currline=0;
	static char lines[20][90];
	int i, j;
	if(str!=NULL) {
		strncpy(lines[currline],str,79);
		lines[currline++][79]=0;
	}
	if(currline==20) {
		for(i=1;i<20;i++) {
			strcpy(lines[i-1],lines[i]);
		}
		currline--;
	}
	i=(currline>hh)?(currline-hh):0;
	for(j=0;i<currline;i++,j++) {
		move(24-hh+j,1);
		clrtoeol();
		printf("%s", lines[i]);
	}
	for(;j<hh;j++) {
		move(24-hh+j,1);
		clrtoeol();
	}
	return 0;
}

int move(int y, int x)
{
	printf("\033[%d;%dH", y,x);
	return 0;
}

int clrtoeol()
{
	printf("\033[K");
	return 0;
}

int clrbotline()
{
	move(24,1);
	clrtoeol();
	printf("输入: ");
	return 0;
}

int addbotchar(char ch)
{
	move(24,inputlen+1+6);
	printf("%c", ch);
	return 0;
}

int delbotchar()
{
	move(24, inputlen+6);
	printf(" ");
	move(24, inputlen+6);
	return 0;
}

int clr_initscr()
{
	int i;
	for(i=1;i<24-hh;i++) {
		move(i,1);
		clrtoeol();
	}
	move(24-1-hh,1);
	printf("\033[1;44;32m   房间: \033[33m%-10.10s \033[32m话题: \033[33m%-40.40s   \033[m\033[0m",
		       	roomname, roomtitle);
	move(24-1-hh,1);
	if(place) printf("\033[45m∧\033[m\033[0m");
	else printf("\033[45m∨\033[m\033[0m");
	move(24,1);
	printf("输入: ");
	return 0;
}

int gen_initscr()
{
	hh=14;
	clr_initscr();
	move(1,1);
	printf("一塌糊涂棋牌室(开发中)\r\n"
			"\t/j name\t进入name房间(name为自己选定的名字);\r\n"
			"\t/t num\t选择状态(ytht房间不可以选择状态, 其它房间先进者掌权);\r\n"
			"\t\t/t 0 普通; /t 1 五子棋; /t 2 象棋; /t 3 围棋;\r\n"
			"\t/w\t列出房间里的人;\t\t/r\t列出所有房间;\r\n"
			"\tTAB\t在游戏区域和聊天区域间切换;\r\n"
			"\t/b\t退出;\r\n"
			"\t/h\t求助");
	addchatline(NULL);
	return 0;
}

struct {
	int x, y;
	int n;
	int q;
	char history[250][2];
} fivedata;
       
int five_initscr()
{
	hh=6;
	clr_initscr();
	move(1,1);
	printf(		"\033[0;30;43m┌┬┬┬┬┬┬┬┬┬┬┬┬┬┐\033[m15\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m14\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m13\r\n"
			"\033[0;30;43m├┼┼＋┼┼┼┼┼┼┼＋┼┼┤\033[m12\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m11\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m10\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m9\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼＋┼┼┼┼┼┼┤\033[m8\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m7\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m6\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m5\r\n"
			"\033[0;30;43m├┼┼＋┼┼┼┼┼┼┼＋┼┼┤\033[m4\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m3\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m2\r\n"
			"\033[0;30;43m└┴┴┴┴┴┴┴┴┴┴┴┴┴┘\033[m1\r\n"
			"A B C D E F G H I J K L M N O");
	fivedata.x=8;
	fivedata.y=8;
	fivedata.q=0;
	move(1, 35);
	printf("五子棋----ytht.net");
	move(2, 35);
	printf("B/W选择座位, 方向键");
	move(3, 35);
	printf("移动, 空格键落子");
	move(4, 35);
	printf("(\033[1;33mB\033[m\033[0m)○黑方: 空座位0");
	move(5, 35);
	printf("(\033[1;33mW\033[m\033[0m)●白方: 空座位1");
	move(7, 35);
	printf("(\033[1;33mN\033[m\033[0m)重新开局");
	move(8, 35);
	printf("(\033[1;33mF\033[m\033[0m)认输");
	yy=8;
	xx=16;
	addchatline(NULL);
	fivedata.n=0;
	return 0;
}

int five_manchar(int ch)
{
	char str[30];
	if(fivedata.q) {
		if(!strchr("yYnN",ch))
			return 0;
		move(10, 35);
		clrtoeol();
		fivedata.q=0;
		if(ch=='y'||ch=='Y') 
			sendmymsg(strmsg(6,"y"));
		else if(ch=='n'||ch=='N')
			sendmymsg(strmsg(6,"n"));
		return 0;
	}

	switch(ch) {
		case 'u': case KEY_UP:
			if(fivedata.y>1)
				fivedata.y--;
			break;
		case 'd': case KEY_DOWN:
			if(fivedata.y<15)
				fivedata.y++;
			break;
		case 'l': case KEY_LEFT:
			if(fivedata.x>1)
				fivedata.x--;
			break;
		case 'r': case KEY_RIGHT:
			if(fivedata.x<15)
				fivedata.x++;
			break;
		case ' ':
			sprintf(str,"%d %d", fivedata.y, fivedata.x);
			sendmymsg(strmsg(6,str));
			break;
		case 'b': case 'B':
			sendmymsg(strmsg(8,"0"));
			break;
		case 'w': case 'W':
			sendmymsg(strmsg(8,"1"));
			break;
		case 'f': case 'F':
			sendmymsg(strmsg(6,"F"));
			break;
		case 'N': case 'n':
			sendmymsg(strmsg(6,"N"));
			break;
	
	}
	yy=fivedata.y;
	xx=fivedata.x*2;
	return 0;
}

int five_addhistory(int y, int x)
{
	if(fivedata.n>=250) {
		int n;
		for(n=20;n<250;n++) {
			fivedata.history[n-20][0]=fivedata.history[n][0];
			fivedata.history[n-20][1]=fivedata.history[n][1];
		}
		fivedata.n-=20;
	}
	fivedata.history[fivedata.n][0]=y;
	fivedata.history[fivedata.n][1]=x;
	fivedata.n++;
}

int five_drawhistory()
{
	int i,n;
	char str[30];
	if(fivedata.n>=15) n=fivedata.n-15;
	else n=0;
	for(i=0;n<fivedata.n&&i<15;i++,n++) {
		move(i+1, 58);
		printf("%4d. %c: %c %2d", n+1, (n%2)?'W':'B',
				'A'-1+fivedata.history[n][1],
				15+1-fivedata.history[n][0]);
	}
}


int five_manmsg(struct mymsg *mp)
{
	int who, y, x;
	if(sscanf(mp->data, "g%d%d%d", &who, &y, &x)==3) {
		move(y, x*2-1);
		if(!who) printf("\033[0;30;43m●\033[m");
		else printf("\033[0;37;43m●\033[m");
		move(6, 35);
		printf("该%s方走", who?"○":"●");
		five_addhistory(y,x);
		five_drawhistory();
		return 0;
	}
	if(mp->data[0]=='s') {
		char s0[20], s1[20];
		sscanf(mp->data+1, "%s%s%d", s0, s1, &who);
		move(4, 35);
		printf("(\033[1;33mB\033[m\033[0m)○黑方: %-14.14s", s0);
		move(5, 35);
		printf("(\033[1;33mW\033[m\033[0m)●白方: %-14.14s", s1);
		move(6, 35);
		printf("该%s方走", who?"●":"○");
		return 0;
	}
	if(mp->data[0]=='q') {
		move(10, 35);
		printf("%s", mp->data+1);
		fivedata.q=1;
		return 0;
	}
	if(mp->data[0]=='n') {
		addchatline("哦, 对方说不");
		move(10, 35);
		clrtoeol();
		fivedata.q=0;
		return 0;
	}
	if(mp->data[0]=='N') {
		five_initscr();
		return 0;
	}
	if(mp->data[0]=='r') {
		char (*matrix)[15][15];
		matrix=(char(*)[15][15])(mp->data+1);
		for(y=1;y<=15;y++) for(x=1;x<=15;x++) {
			if(!(*matrix)[y-1][x-1]) continue;
			move(y, 2*x-1);
			if((*matrix)[y-1][x-1]==1)
				printf("\033[0;30;43m●\033[m");
			else
				printf("\033[0;37;43m●\033[0m");
		}
	}
	if(mp->data[0]=='R') {
		memcpy(fivedata.history, mp->data+1, sizeof(fivedata.history));
		for(fivedata.n=0;fivedata.n<1000;fivedata.n++) {
			if(fivedata.history[fivedata.n][0]==0)
				break;
		}
		five_drawhistory();
	}

	return 0;
}

int analysisline(char *str)
{
	if(!str[0]) return 0;
	if(str[0]=='/') {
		if(str[1]=='w')
			return sendmymsg(simplemsg(3));
		if(str[1]=='j')
			return sendmymsg(strmsg(4,str+2));
		if(str[1]=='r')
			return sendmymsg(simplemsg(2));
		if(str[1]=='e')
			return sendmymsg(strmsg(4,"ytht"));
		if(str[1]=='b')
			return sendmymsg(simplemsg(1));
		if(str[1]=='t') {
			char typestr[20];
			if(strlen(str+2)>19)
				str[2+19]=0;
			if(sscanf(str+2, "%s", typestr)==1) {
				if(isdigit(typestr[0]))
					return sendmymsg(strmsg(7,str+2));
				if(!strcasecmp(typestr,"five"))
					return sendmymsg(strmsg(7,"1"));
				if(!strcasecmp(typestr,"cchess"))
					return sendmymsg(strmsg(7,"2"));
				if(!strcasecmp(typestr,"go"))
					return sendmymsg(strmsg(7,"3"));
			}
			return addchatline("五子棋 /t five  象棋 /t cchess  围棋 /t go");
		}
		if(str[1]=='s')
			return sendmymsg(strmsg(8,str+2));
		if(str[1]=='/')
			return addchatline("很抱歉现在还不支持这些动作");
		if(str[1]=='k')
			return sendmymsg(strmsg(9,str+2));
		if(str[1]=='o')
			return sendmymsg(strmsg(10,str+2));
		if(str[1]=='h') {
			addchatline("/w 列出用户 /j 进入房间 /r 列出房间 /e 离开房间");
			addchatline("/b 退出 /t 选择游戏 /s 就座 /h 求助");
			addchatline("(管理员) /k 踢某人出去 /o 将管理权交给某人");
			return 0;
		}
	}
	return sendmymsg(strmsg(5,str));
}

#define c_jiang 1
#define c_shi   2
#define c_xiang 3
#define c_ju    4
#define c_ma    5
#define c_pao   6
#define c_zu    7

char cchess_map[11][20]={
	"┏┯┯┯┯┯┯┯┓j",
	"┠┼┼┼×┼┼┼┨i",
	"┠┼┼┼┼┼┼┼┨h",
	"┠┼┼┼┼┼┼┼┨g",
	"┣┷┷┷┷┷┷┷┫f",
	"┣┯┯┯┯┯┯┯┫e",
	"┠┼┼┼┼┼┼┼┨d",
	"┠┼┼┼┼┼┼┼┨c",
	"┠┼┼┼×┼┼┼┨b",
	"┗┷┷┷┷┷┷┷┛a",
	"１２３４５６７８９"
};
char cchess_str0[8][3]={"","帅","仕","相","车","马","炮","兵"};
char cchess_str1[8][3]={"","将","士","象","车","马","炮","卒"};


struct {
	int x, y;
	int x0, y0;
	char matrix[10][9];
	int n;
	int q;
	char player[2][20];
	char history[1000][5];
	char h2[1000][5];
} cchessdata, cchessdata0={
	1,1,0,0,
	{{c_ju, c_ma, c_xiang, c_shi, c_jiang, c_shi, c_xiang, c_ma, c_ju},
	{0,},
	{0, c_pao, 0, 0, 0, 0, 0, c_pao, 0},
	{c_zu, 0, c_zu, 0, c_zu, 0, c_zu, 0, c_zu},
	{0,},
	{0,},
	{-c_zu, 0, -c_zu, 0, -c_zu, 0, -c_zu, 0, -c_zu},
	{0, -c_pao, 0, 0, 0, 0, 0, -c_pao, 0},
	{0,},
	{-c_ju, -c_ma, -c_xiang, -c_shi, -c_jiang, -c_shi, -c_xiang, -c_ma, -c_ju}},
	0,
	0,
	{"",""},
};
       
int cchess_initscr()
{
	int i;
	hh=11;
	clr_initscr();
	cchessdata=cchessdata0;
	for(i=0;i<11;i++) {
		move(i+1,1);
		printf("\033[37m");
		printf(cchess_map[i]);
	}
	draw_cchess();
	move(1, 22);
	printf("中国象棋----ytht.net");
	move(2, 22);
	printf("R/G选择座位, 方向键移动");
	move(3, 22);
	printf("空格键选子和落子");
	move(4, 22);
	printf("(\033[1;33mR\033[m\033[0m)红方: 空座位0");
	move(5, 22);
	printf("(\033[1;33mG\033[m\033[0m)绿方: 空座位1");
	move(7, 22);
	printf("(\033[1;33mN\033[m\033[0m)重新开局");
	move(8, 22);
	printf("(\033[1;33mF\033[m\033[0m)认输");
	yy=1;
	xx=2;
	addchatline(NULL);
	return 0;
}

int cchess_manchar(int ch)
{
	char str[30];
	if(cchessdata.q) {
		if(!strchr("yYnN",ch))
			return 0;
		move(10, 22);
		clrtoeol();
		cchessdata.q=0;
		if(ch=='y'||ch=='Y') 
			sendmymsg(strmsg(6,"y"));
		else if(ch=='n'||ch=='N')
			sendmymsg(strmsg(6,"n"));
		return 0;
	}
	switch(ch) {
		case KEY_UP:
			if(cchessdata.y>1)
				cchessdata.y--;
			break;
		case KEY_DOWN:
			if(cchessdata.y<10)
				cchessdata.y++;
			break;
		case KEY_LEFT:
			if(cchessdata.x>1)
				cchessdata.x--;
			break;
		case KEY_RIGHT:
			if(cchessdata.x<9)
				cchessdata.x++;
			break;
		case ' ':
			if(!cchessdata.x0) {
				if(!cchessdata.matrix[cchessdata.y-1][cchessdata.x-1])
					break;
				cchessdata.x0=cchessdata.x;
				cchessdata.y0=cchessdata.y;
				highlight_cchess(cchessdata.y0, cchessdata.x0);
				break;
			}
			one_cchess(cchessdata.y0, cchessdata.x0);
			if(cchessdata.x0==cchessdata.x&&cchessdata.y0==cchessdata.y) {
				cchessdata.x0=0;
				break;
			}
			sprintf(str,"%d %d %d %d",
					cchessdata.y0, cchessdata.x0,
					cchessdata.y, cchessdata.x);
			sendmymsg(strmsg(6,str));
			cchessdata.x0=0;
			break;
		case 'r': case 'R':
			sendmymsg(strmsg(8,"0"));
			break;
		case 'g': case 'G':
			sendmymsg(strmsg(8,"1"));
			break;
		case 'f': case 'F':
			sendmymsg(strmsg(6,"F"));
			break;
		case 'N': case 'n':
			sendmymsg(strmsg(6,"N"));
			break;
	}
	yy=cchessdata.y;
	xx=cchessdata.x*2;
	return 0;
}

char c_num[]="零一二三四五六七八九";
char d_num[]="０１２３４５６７８９";

char *cchess_historystr(int n, char h[5])
{
	static char str[30];
	int startx, endx, verb, distance;
	       	
	sprintf(str,"\033[m(%3d) \033[1;%dm%s\033[0m %c%d-%c%d \033[m",
			n, (n%2)?31:32,
			(h[4]>0)?cchess_str0[abs(h[4])]:cchess_str1[abs(h[4])],
			(char)('a'+10-h[0]), (int)h[1], (char)('a'+10-h[2]), (int)h[3]);
	return str;
}

int cchess_trans(char h[5], char h2[5])
{
	int startx, endx, verb, distance;
	char hh[5];       	
	startx=hh[1];
	endx=hh[3];
	switch(hh[4]) {
		case c_ju:
		case c_pao:
		case c_jiang:
		case c_zu:
			if(hh[2]>hh[0]) {
				verb=1;
				distance=hh[2]-hh[0];
			} else if(hh[2]<hh[0]) {
				verb=-1;
				distance=hh[0]-hh[2];
			} else {
				verb=0;
				distance=endx;
			}
			break;
		default:
			if(hh[2]>hh[0]) verb=1;
			else verb=-1;
			distance=endx;
	}
	//...
}

int cchess_addhistory(int y0, int x0, int y, int x, int z)
{
	if(cchessdata.n>=1000) {
		int n;
		for(n=20;n<1000;n++) {
			cchessdata.history[n-20][0]=cchessdata.history[n][0];
			cchessdata.history[n-20][1]=cchessdata.history[n][1];
			cchessdata.history[n-20][2]=cchessdata.history[n][2];
			cchessdata.history[n-20][3]=cchessdata.history[n][3];
			cchessdata.history[n-20][4]=cchessdata.history[n][4];
		}
		cchessdata.n-=20;
	}
	cchessdata.history[cchessdata.n][0]=y0;
	cchessdata.history[cchessdata.n][1]=x0;
	cchessdata.history[cchessdata.n][2]=y;
	cchessdata.history[cchessdata.n][3]=x;
	cchessdata.history[cchessdata.n][4]=z;
	cchessdata.n++;
}

int cchess_drawhistory()
{
	int i,n;
	if(cchessdata.n>=20) n=cchessdata.n-20;
	else n=0;
	for(i=0; i<10; i++, n++) {
		move(i+1, 45);
		if(n<cchessdata.n) {
			printf("%s", cchess_historystr(n+1, cchessdata.history[n]));
		} else printf("                ");
		move(i+1, 64);
		if(n+10<cchessdata.n) {
			printf("%s", cchess_historystr(n+10+1, cchessdata.history[n+10]));
		} else printf("                ");
	}	
}


int cchess_manmsg(struct mymsg *mp)
{
	int who, y0, x0, y, x;
	if(sscanf(mp->data, "g%d%d%d%d%d", &who, &y0, &x0, &y, &x)==5) {
		cchessdata.matrix[y-1][x-1]=cchessdata.matrix[y0-1][x0-1];
		cchessdata.matrix[y0-1][x0-1]=0;
		one_cchess(y,x);
		one_cchess(y0,x0);
		cchess_addhistory(y0, x0, y, x, cchessdata.matrix[y-1][x-1]);
		cchess_drawhistory();
		move(6, 22);
		printf("该%s方走", who?"红":"绿");
		return 0;
	}
	if(mp->data[0]=='q') {
		move(10, 22);
		printf("%s", mp->data+1);
		cchessdata.q=1;
		return 0;
	}
	if(mp->data[0]=='n') {
		addchatline("哦, 对方说不");
		move(10, 22);
		clrtoeol();
		cchessdata.q=0;
		return 0;
	}
	if(mp->data[0]=='N') {
		cchess_initscr();
		return 0;
	}
	if(mp->data[0]=='s') {
		char s0[20], s1[20];
		sscanf(mp->data+1, "%s%s%d", s0, s1, &who);
		move(4, 22);
		printf("(\033[1;33mR\033[m\033[0m)红方: %-14.14s", s0);
		move(5, 22);
		printf("(\033[1;33mG\033[m\033[0m)绿方: %-14.14s", s1);
		move(6, 22);
		printf("该%s方走", who?"绿":"红");
		return 0;
	}
	if(mp->data[0]=='r') {
		memcpy(cchessdata.matrix,mp->data+1,sizeof(cchessdata.matrix));
		draw_cchess();
		return 0;
	}
	if(mp->data[0]=='R') {
		memcpy(cchessdata.history, mp->data+1, sizeof(cchessdata.history));
		for(cchessdata.n=0;cchessdata.n<1000;cchessdata.n++) {
			if(cchessdata.history[cchessdata.n][0]==0)
				break;
		}
		cchess_drawhistory();
		return 0;
	}
	return 0;
}

int draw_cchess()
{
	int y, x;
	for(y=1;y<=10;y++) for(x=1;x<=9;x++)
		one_cchess(y,x);      
}

int one_cchess(int y, int x)
{
	if(!cchessdata.matrix[y-1][x-1]) {
		move(y, 2*x-1);
		printf("\033[37m%2.2s\033[m\033[0m", cchess_map[y-1]+x*2-2);
		return;
	}
	move(y, 2*x-1);
	if(cchessdata.matrix[y-1][x-1]>0) {
		printf("\033[1;31m%s\033[m\033[0m",
				cchess_str0[(unsigned int)cchessdata.matrix[y-1][x-1]]);
	} else {
		printf("\033[1;32m%s\033[m\033[0m",
				cchess_str1[(unsigned int)-cchessdata.matrix[y-1][x-1]]);
	}
}

int highlight_cchess(int y, int x)
{
	if(!cchessdata.matrix[y-1][x-1]) {
		move(y, 2*x-1);
		printf("\033[37m%2.2s\033[m\033[0m", cchess_map[y-1]+x*2-2);
		return;
	}
	move(y, 2*x-1);
	if(cchessdata.matrix[y-1][x-1]>0) {
		printf("\033[1;31;44m%s\033[m\033[0m",
				cchess_str0[(unsigned int)cchessdata.matrix[y-1][x-1]]);
	} else {
		printf("\033[1;32;44m%s\033[m\033[0m",
				cchess_str1[(unsigned int)-cchessdata.matrix[y-1][x-1]]);
	}
}


struct {
	int x, y;
	char matrix[19][19];
	int n;
	char history[1000][2];
} godata;
       
int go_initscr()
{
	hh=2;
	clr_initscr();
	move(1,1);
	printf(		"\033[0;30;43m┌┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┐\033[m19\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m18\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m17\r\n"
			"\033[0;30;43m├┼┼＋┼┼┼┼┼＋┼┼┼┼┼＋┼┼┤\033[m16\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m15\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m14\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m13\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m12\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m11\r\n"
			"\033[0;30;43m├┼┼＋┼┼┼┼┼＋┼┼┼┼┼＋┼┼┤\033[m10\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m9\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m8\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m7\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m6\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m5\r\n"
			"\033[0;30;43m├┼┼＋┼┼┼┼┼＋┼┼┼┼┼＋┼┼┤\033[m4\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m3\r\n"
			"\033[0;30;43m├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\033[m2\r\n"
			"\033[0;30;43m└┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┘\033[m1\r\n"
			"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳ");
	godata.x=8;
	godata.y=8;
	move(1, 43);
	printf("围棋----ytht.net");
	move(2, 43);
	printf("B/W选择座位, 方向键移动, 空格键落子");
	move(3, 43);
	printf("(\033[1;33mB\033[m\033[0m)●黑方: 空座位0");
	move(4, 43);
	printf("(\033[1;33mW\033[m\033[0m)○白方: 空座位1");
	yy=8;
	xx=16;
	addchatline(NULL);
	bzero(godata.matrix, sizeof(godata.matrix));
	godata.n=0;
	return 0;
}

int go_manchar(int ch)
{
	char str[30];
	switch(ch) {
		case 'u': case KEY_UP:
			if(godata.y>1)
				godata.y--;
			break;
		case 'd': case KEY_DOWN:
			if(godata.y<19)
				godata.y++;
			break;
		case 'l': case KEY_LEFT:
			if(godata.x>1)
				godata.x--;
			break;
		case 'r': case KEY_RIGHT:
			if(godata.x<19)
				godata.x++;
			break;
		case ' ':
			sprintf(str,"%d %d", godata.y, godata.x);
			sendmymsg(strmsg(6,str));
			break;
		case 'b': case 'B':
			sendmymsg(strmsg(8,"0"));
			break;
		case 'w': case 'W':
			sendmymsg(strmsg(8,"1"));
	}
	yy=godata.y;
	xx=godata.x*2;
	return 0;
}

int go_addhistory(int y, int x)
{
	if(godata.n>=1000) {
		int n;
		for(n=20;n<1000;n++) {
			godata.history[n-20][0]=godata.history[n][0];
			godata.history[n-20][1]=godata.history[n][1];
		}
		godata.n-=20;
	}
	godata.history[godata.n][0]=y;
	godata.history[godata.n][1]=x;
	godata.n++;
}

int go_drawhistory()
{
	int i,n;
	char str[30];
	if(godata.n>=15) n=godata.n-15;
	else n=0;
	for(i=0;n<godata.n&&i<15;i++,n++) {
		move(i+6, 43);
		printf("%4d. %c: %c %2d", n+1, (n%2)?'W':'B',
				'A'-1+godata.history[n][1],
				19+1-godata.history[n][0]);
	}
}

int go_manmsg(struct mymsg *mp)
{
	int who, y, x;
	if(sscanf(mp->data, "g%d%d%d", &who, &y, &x)==3) {
		move(y, x*2-1);
		if(!who) printf("\033[0;30;43m●\033[m");
		else printf("\033[0;37;43m●\033[m");
		godata.matrix[y-1][x-1]=who+1;
		check_godata(!who);
		check_godata(who);
		move(5, 43);
		printf("该%s方走", who?"○":"●");
		go_addhistory(y,x);
		go_drawhistory();
		return 0;
	}
	if(mp->data[0]=='s') {
		char s0[20], s1[20];
		sscanf(mp->data+1, "%s%s%d", s0, s1, &who);
		move(3, 43);
		clrtoeol();
		printf("(\033[1;33mB\033[m\033[0m)○黑方: %s", s0);
		move(4, 43);
		clrtoeol();
		printf("(\033[1;33mW\033[m\033[0m)●白方: %s", s1);
		move(5, 43);
		printf("该%s方走", who?"●":"○");
		return 0;
	}
	if(mp->data[0]=='r') {
		memcpy(godata.matrix,mp->data+1,sizeof(godata.matrix));
		for(y=1;y<=19;y++) for(x=1;x<=19;x++) {
			if(!godata.matrix[y-1][x-1]) continue;
			move(y, 2*x-1);
			if(godata.matrix[y-1][x-1]==1)
				printf("\033[0;30;43m●\033[m");
			else
				printf("\033[0;37;43m●\033[m");
		}
	}
	if(mp->data[0]=='R') {
		memcpy(godata.history, mp->data+1, sizeof(godata.history));
		for(godata.n=0;godata.n<1000;godata.n++) {
			if(godata.history[godata.n][0]==0)
				break;
		}
		go_drawhistory();
	}
	return 0;
}

int check_godata(int who)
{
	int i, j, c=1;
	char matrix[19][19];
	memcpy(matrix, godata.matrix, sizeof(matrix));
	while(c) {
		c=0;
		for(i=0;i<19;i++) for(j=0;j<19;j++) {
			if(matrix[i][j]==who+1) {
				if((i-1>=0&&matrix[i-1][j]==0)||
				   (i+1<19&&matrix[i+1][j]==0)||
				   (j-1>=0&&matrix[i][j-1]==0)||
				   (j+1<19&&matrix[i][j+1]==0)) {
					matrix[i][j]=0;
					c++;
				}
			}
		}
	}

	for(i=0;i<19;i++) for(j=0;j<19;j++) {
		if(matrix[i][j]==who+1) {
			undo_go(i+1,j+1);
			godata.matrix[i][j]=0;
		}
	}
}

int undo_go(int y, int x)
{
	move(y,x*2-1);
	printf("\033[30;43m");
	if(y==1&&x==1)
		printf("┌");
	else if(y==1&&x==19)
		printf("┐");
	else if(y==19&&x==1)
		printf("└");
	else if(y==19&&x==19)
		printf("┘");
	else if(y==1)
		printf("┬");
	else if(y==19)
		printf("┴");
	else if(x==1)
		printf("├");
	else if(x==19)
		printf("┤");
	else if((y==4||y==10||y==16)&&(x==4||x==10||x==16))
		printf("＋");
	else
		printf("┼");
	printf("\033[m");
}
