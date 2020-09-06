//Copyright: ylsdd
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define BUFSIZE (8*1024)
#define NUARRAY 200

struct Room {
	char title[30];
	char name[13];
	int type;
	int nuser;
	int owner; //第一个进来的id
	int seat[8]; //8 players at most
	char *data;
} room[100];

struct userinfo {
        char userid[20];
        int perm;
};

struct bbsstatus {
	int room;
};

struct mybuffer {
        int size;
        int n;
        char *data;
};

struct mymsg {
        short len;
        unsigned char cmd;
        char data[1];
};

struct mymsgqueue {
        struct mymsgqueue *next;
        struct mymsg m;
};

struct waiting {
        int prev;
        int next;
        int head;
        int tail;
} warray[NUARRAY];

struct userarray {
        int fd;
        char host[17];
        int haslogin;
        int lasttime;
        struct userinfo uinfo;
        struct bbsstatus status;
        struct mybuffer inbuf;
        int outlen, outn, ninq;
        struct mymsgqueue *outq, *tail;
} uarray[NUARRAY];

int nuser=0, sock;

fd_set fdsr0, fdsw0, fdse0, fdsr, fdsw, fdse;

void addmsgqueue(int i, struct mymsgqueue *mq)
{
        if(uarray[i].fd<0) {
                free(mq);
                return;
        }
        uarray[i].ninq++;
        mq->next=NULL;
        if(uarray[i].outq==NULL) {
                uarray[i].outq=mq;
                uarray[i].tail=mq;
                mq->next=NULL;
                uarray[i].outlen=mq->m.len;
                mq->m.len=htons(mq->m.len);
                uarray[i].outn=0;
                FD_SET(uarray[i].fd, &fdsw0);
                return;
        }
        uarray[i].tail->next=mq;
        uarray[i].tail=mq;
}

struct mymsgqueue *mallocmq(int len)
{
        return (struct mymsgqueue*)
                malloc(sizeof(struct mymsgqueue)-sizeof(struct mymsg)+len);
}

struct mymsgqueue *simplemq(unsigned char cmd)
{
        struct mymsgqueue *mq=mallocmq(3);
        mq->m.cmd=cmd;
        mq->m.len=3;
        return mq;
}

struct mymsgqueue *strmq(unsigned char cmd, char *str)
{
        struct mymsgqueue *mq;
        int l=strlen(str);
        mq=mallocmq(4+l);
        mq->m.cmd=cmd;
        strcpy(mq->m.data, str);
        mq->m.len=4+l;
        return mq;
}

int cmd_login(int i, struct mymsg *m);
int cmd_logout(int i, struct mymsg *m);
int cmd_lsroom(int i, struct mymsg *m);
int cmd_lsuser(int i, struct mymsg *m);
int cmd_intoroom(int i, struct mymsg *m);
int cmd_chat(int i, struct mymsg *m);
int cmd_chess(int i, struct mymsg *m);
int cmd_type(int i, struct mymsg *m);
int cmd_seat(int i, struct mymsg *m);
int cmd_kick(int i, struct mymsg *m);
int cmd_setowner(int i, struct mymsg *m);
int (*ecmdlist[256])() = {
        cmd_login,
	cmd_logout,
	cmd_lsroom,
	cmd_lsuser,
	cmd_intoroom,
	cmd_chat,
	cmd_chess,
	cmd_type,
	cmd_seat,
	cmd_kick,
	cmd_setowner,
};

void addtowaitqueue(int i, int j)
{
        if(warray[i].head==-1) {
                warray[i].head=j;
                warray[i].tail=j;
                warray[j].prev=i;
                warray[j].next=i;
                return;
        }
        warray[j].prev=warray[i].tail;
        warray[j].next=i;
        warray[warray[i].tail].next=j;
        warray[i].tail=j;
}

//将i从等候队列中去掉
void removewaitqueue(int i)
{
        if(warray[i].prev==-1)
                return;
        if(warray[warray[i].prev].head==i)
                warray[warray[i].prev].head=warray[i].next;
        else
                warray[warray[i].prev].next=warray[i].next;
        if(warray[warray[i].next].tail==i)
                warray[warray[i].next].tail=warray[i].prev;
        else
                warray[warray[i].next].prev=warray[i].prev;
}

int wakeupone(int i)
{
        int j;
        if(warray[i].head==-1)
                return -1;
        j=warray[i].head;
        if(warray[j].next==-1)
                warray[i].tail=-1;
        else
                warray[warray[j].next].prev=i;
        warray[i].head=warray[j].next;
        warray[j].prev=-1;
        warray[j].next=-1;
        return j;
}

void trydealinbuf(int i);

//清空i的等候队列
void cleanupwaitqueue(int i)
{
        while(warray[i].head!=-1)
                trydealinbuf(wakeupone(i));
}
                
void initarray()
{
        int i;
        for(i=0;i<NUARRAY;i++)
                uarray[i].fd=-1;
	for(i=0;i<100;i++) {
		room[i].nuser=0;
	}
	room[0].nuser=1;
	strcpy(room[0].title,"今天你糊涂了吗?");
	strcpy(room[0].name,"ytht");
	room[0].type=0;
	room[0].owner=-1;
	room[0].data=NULL;
}

int bindport(int port)
{
        int s, val;
        struct linger ld;
        struct sockaddr_in sin;
        
        s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(s<0) return -1;
        
        val = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));
        ld.l_onoff = ld.l_linger = 0;
        setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
        if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
                close(s);
                return -1;
        }
        
        if(listen(s, 5) < 0) {
                close(s);
                return -1;
        }
        return s;
}


int newuser()
{
        struct sockaddr_in sin;
        int value, newfd, i;
        newfd=accept(sock, (struct sockaddr *)&sin, &value);
        if(newfd<0) return -1;
        if(fcntl(newfd, F_SETFL, O_NONBLOCK|fcntl(newfd, F_GETFL))<0) {
                close(newfd);
                return -1;
        }
        for(i=0;i<NUARRAY;i++)
                if(uarray[i].fd<0) break;
        if(i==NUARRAY) {
                close(newfd);
                return -1;
        }
        if((uarray[i].inbuf.data=malloc(BUFSIZE))==NULL) {
                close(newfd);
                return -1;
        }
        uarray[i].inbuf.size=BUFSIZE;
        uarray[i].inbuf.n=0;
        
        uarray[i].fd=newfd;
        uarray[i].haslogin=0;
        uarray[i].lasttime=time(NULL);
        uarray[i].ninq=0;
        uarray[i].outq=NULL;

        warray[i].prev=warray[i].next=-1;
        warray[i].head=warray[i].tail=-1;
        
        FD_SET(newfd, &fdsr0);
        nuser++;
        if(nuser==NUARRAY)
                FD_CLR(sock, &fdsr0);
                
        return 0;
}       

int doexitroom(int i);
int doenterroom(int i, int r, char *name);
int broadcastroom(int r, int cmd, char *str);

int removeuser(i)
{
        struct mymsgqueue *q0, *q1;
        if(uarray[i].fd<0) return -1;
        FD_CLR(uarray[i].fd, &fdsr0);
        FD_CLR(uarray[i].fd, &fdsw0);
        FD_CLR(uarray[i].fd, &fdse0);
        close(uarray[i].fd);
        free(uarray[i].inbuf.data);
        q0=uarray[i].outq;
        while(q0!=NULL) {
                q1=q0->next;
                free(q0);
                q0=q1;
        }
        removewaitqueue(i); //remove应该在clean之前
        cleanupwaitqueue(i);
        nuser--;
        FD_SET(sock, &fdsr0);
        
	if(uarray[i].haslogin) 
		doexitroom(i);
	uarray[i].fd=-1;
	return 0;
}

int dealinbuf(int i)
{
        short len;
        char tmp[BUFSIZE];
        struct mymsg *m=(struct mymsg*)tmp;
        if(uarray[i].inbuf.n<3)
                return -1;
        len=ntohs(*(unsigned int*)uarray[i].inbuf.data);
        if(len<3||len>BUFSIZE) {
                removeuser(i);
                return -1;
        }
        if(len>uarray[i].inbuf.n)
                return -1;
        memcpy(m, uarray[i].inbuf.data, len);
        m->len=len;
        uarray[i].inbuf.n-=len;
        memmove(uarray[i].inbuf.data, uarray[i].inbuf.data+len, uarray[i].inbuf.n);
#ifdef DEBUG
	printf("cmd=%d\n", (int)m->cmd);
	if(m->len>3) printf("data=%s\n", m->data);
#endif
	if(ecmdlist[m->cmd]!=NULL)
                return (ecmdlist[m->cmd])(i, m);
	return 0;
}

void trydealinbuf(int i)
{
        if(warray[i].prev>=0)
                return;
        if(uarray[i].fd<0)
                return;
        while(warray[i].prev<0&&dealinbuf(i)>=0);
        if(uarray[i].fd<0)
                return;
        if(uarray[i].inbuf.n==uarray[i].inbuf.size)
                FD_CLR(uarray[i].fd, &fdsr0);
        else
                FD_SET(uarray[i].fd, &fdsr0);
}

int outputuser(int i)
{
        int retv;
        if(uarray[i].fd<0)
                return -1;
        while(uarray[i].ninq>0) {
                retv=write(uarray[i].fd, &uarray[i].outq->m+uarray[i].outn,
                                uarray[i].outlen-uarray[i].outn);
                if(retv<0) {
                        if(errno==EINTR)
                                continue;
                        if(errno==EAGAIN)
                                break;
                        removeuser(i);
                        return -1;
                }
                uarray[i].outn+=retv;
                if(uarray[i].outn==uarray[i].outlen) {
                        struct mymsgqueue *q=uarray[i].outq->next;
                        uarray[i].ninq--;
                        free(uarray[i].outq);
                        uarray[i].outq=q;
                        if(q==NULL) break;
                        uarray[i].outn=0;
                        uarray[i].outlen=q->m.len;
                        q->m.len=htons(q->m.len);
                }
        }
        
        if(uarray[i].ninq<10)
                cleanupwaitqueue(i);

        if(uarray[i].ninq==0)
                FD_CLR(uarray[i].fd, &fdsw0);
        return 0;
}

int inputuser(int i)
{
        int retv;
        if(uarray[i].fd<0)
                return -1;
        retv=read(uarray[i].fd, uarray[i].inbuf.data+uarray[i].inbuf.n,
                        uarray[i].inbuf.size-uarray[i].inbuf.n);
        if(retv<0) {
                if(errno!=EINTR&&errno!=EAGAIN)
                        removeuser(i);
                return -1;
        }
        if(retv==0) {
                removeuser(i);
                return -1;
        }
        uarray[i].inbuf.n+=retv;

        trydealinbuf(i);
        
        if(uarray[i].fd<0)
                return -1;
        if(uarray[i].inbuf.n==uarray[i].inbuf.size)
                FD_CLR(uarray[i].fd, &fdsr0);
        return 0;
}

int main(int argn, char **argv)
{
        int port=0, retv, i;
        struct timeval tv;
        //成为守护进程
        //if(startdeamon()<0) return;
        //初始化用户组
        signal(SIGPIPE, SIG_IGN);
	initarray();
        //绑定到端口
        if(argn>=2&&isdigit(argv[1][0]))
                port=atoi(argv[1]);
        if(port<=0)
                port=3333;
        if((sock=bindport(port))<0) exit(0);
        
        FD_ZERO(&fdsr0);
        FD_ZERO(&fdsw0);
        FD_ZERO(&fdse0);
        FD_SET(sock, &fdsr0);
        tv.tv_sec=30;
        tv.tv_usec=0;

        while(1) {
                fdsr=fdsr0;
                fdsw=fdsw0;
                fdse=fdse0;
                retv=select(3*NUARRAY, &fdsr, &fdsw, &fdse, &tv);
                if(retv<0) {
                        if(errno==EINTR) continue;
                        else {
                                char str[100];
                                sprintf(str, "ebbs: select: %s", strerror(errno));
                                //logebbs(str);
                                exit(0);
                        }
                }
                if(retv==0) {
                        //doreaper();
                        tv.tv_sec=30;
                        tv.tv_usec=0;
                        continue;
                }
                if(FD_ISSET(sock, &fdsr))
                        newuser();
                for(i=0;i<NUARRAY;i++) {
                        if(uarray[i].fd<0) continue;
                        if(FD_ISSET(uarray[i].fd, &fdse)) {
                                removeuser(i);
                                continue;
                        }
                        if(FD_ISSET(uarray[i].fd, &fdsw)) {
#ifdef DEBUG
				printf("output user %d", i);
#endif
				outputuser(i);
			}
                        if(FD_ISSET(uarray[i].fd, &fdsr))
                                inputuser(i);
                }
        }
}

int cmd_login(int i, struct mymsg *m)
{
	int n;
	if(uarray[i].haslogin)
		return 0;
	if(m->len<6) {
		removeuser(i);
		return 0;
	}
	m->data[m->len-4]=0;
	if(strlen(m->data)>=14) {
		removeuser(i);
		return 0;
	}
	for(n=0;n<NUARRAY;n++) {
		if(uarray[n].fd<0||n==i||!uarray[n].haslogin) continue;
#ifdef DEBUG
		printf("%d userid %s\n", n, uarray[n].uinfo.userid);
#endif
		if(!strcasecmp(m->data, uarray[n].uinfo.userid)) {
			removeuser(i);
			return 0;
		}
	}
	uarray[i].haslogin=1;
	strcpy(uarray[i].uinfo.userid, m->data);
#ifdef DEBUG
	printf("new userid %s\n", m->data);
#endif
	doenterroom(i,0, "ytht");
	return 0;
}
int cmd_logout(int i, struct mymsg *m)
{
	removeuser(i);
	return 0;
}
int cmd_lsroom(int i, struct mymsg *m)
{
	int n, k;
	char str[80];
	if(!uarray[i].haslogin)
		return 0;
#ifdef DEBUG
	printf("here\n");
#endif
	str[0]=0;
	for(n=0,k=0;n<100;n++) {
		if(room[n].nuser==0) continue;
		if(n==uarray[i].status.room)
			strcat(str, "*");
		else
			strcat(str, " ");
		strcat(str,room[n].name);
		k++;
		if(k==5) {
			addmsgqueue(i, strmq(m->cmd,str));
			str[0]=0;
			k=0;
			continue;
		}
	}
	if(k) addmsgqueue(i, strmq(m->cmd,str));
	return 0;
}
int cmd_lsuser(int i, struct mymsg *m)
{
	int n, k;
	char str[80];
	if(!uarray[i].haslogin)
		return 0;
	str[0]=0;
	for(n=0,k=0;n<NUARRAY;n++) {
		if(uarray[n].fd<0||uarray[n].status.room!=uarray[i].status.room)
			continue;
		if(room[uarray[n].status.room].owner==n)
			strcat(str, "*");
		else
			strcat(str, " ");
		strcat(str, uarray[n].uinfo.userid);
		k++;
		if(k==5) {
			addmsgqueue(i, strmq(m->cmd,str));
			str[0]=0;
			k=0;
			continue;
		}
	}
	if(k) addmsgqueue(i, strmq(m->cmd, str));
	return 0;
}
int cmd_intoroom(int i, struct mymsg *m)
{
	char str[80];
	int n;

	if(!uarray[i].haslogin)
		return 0;

	m->data[((m->len-4)>=20)?19:(m->len-4)]=0;
	if(sscanf(m->data, "%s", str)!=1||strlen(str)>12||strlen(str)<1) {
		addmsgqueue(i,strmq('C', "系统: 请给出正确的聊天室名称(最多12个字节)"));
		return 0;
	}
	for(n=0;n<100;n++) {
		if(room[n].nuser==0)
			continue;
		if(!strcasecmp(room[n].name, str))
			break;
	}
	if(n==100) for(n=0;n<100;n++) {
		if(room[n].nuser==0)
			break;
	}
	if(n==100) return 0;
	if(n==uarray[i].status.room) {
		sprintf(str, "系统: 你现在就在 %s 房间", room[n].name);
		addmsgqueue(i,strmq('C', str));
		return 0;
	}
	doexitroom(i);
	doenterroom(i,n, str);
	return 0;
}

int five_init(int r);
int five_go(int i, int r, char *data);
int five_restore(int i, int r);
int five_seatchanged(int r);
int cchess_init(int r);
int cchess_go(int i, int r, char *data);
int cchess_restore(int i, int r);
int cchess_seatchanged(int r);
int go_init(int r);
int go_go(int i, int r, char *data);
int go_restore(int i, int r);
int go_seatchanged(int r);

struct {
	char name[20];
	int max;
	int (*init)(int r);
	int (*go)(int i, int r, char *data); //走子
	int (*restore)(int i, int r); //用户中途进入查看
	int (*seatchanged)(int r);
} chesslist[5]={
	{"闲聊", 0, NULL, NULL, NULL, NULL},
	{"五子棋", 2, five_init, five_go, five_restore, five_seatchanged},
	{"中国象棋", 2, cchess_init, cchess_go, cchess_restore, cchess_seatchanged},
	{"围棋", 2, go_init, go_go, go_restore, go_seatchanged},
};


int doexitroom(int i)
{
	int n, r, k;
	char str[50];
	r=uarray[i].status.room;
	room[r].nuser--;
	if(room[r].nuser==0&&room[r].data!=NULL) {
		free(room[r].data);
		return 0;
	}
	if(room[r].owner==i&&room[r].nuser>0) {
		room[r].owner=-1;
		for(n=1;n<NUARRAY;n++) {
			k=(n+i)%NUARRAY;
			if(uarray[k].status.room==r) {
				room[r].owner=k;
				break;
			}
		}
	}
	for(n=0;n<8;n++) {
		if(room[r].seat[n]!=i)
			continue;
		room[r].seat[n]=-1;
		if(chesslist[room[r].type].seatchanged!=NULL)
			chesslist[room[r].type].seatchanged(r);
	}
	sprintf(str,"%s慢慢离开了", uarray[i].uinfo.userid);
	for(n=0;n<NUARRAY;n++) {
		if(uarray[n].fd<0||uarray[n].haslogin==0
				||uarray[n].status.room!=uarray[i].status.room
				||n==i)
			continue;
		addmsgqueue(n, strmq('C', str));
	}
	return 0;
}

int doenterroom(int i, int r, char *name)
{
	int n;
	char str[80];
	if(room[r].nuser==0) {
		strcpy(room[r].name, name);
		strcpy(room[r].title, "今天你糊涂了吗?");
		room[r].type=0;
		room[r].data=NULL;
		room[r].owner=i;
		for(n=0;n<8;n++)
			room[r].seat[n]=-1;
	}
	room[r].nuser++;
	uarray[i].status.room=r;
	
	sprintf(str,"%s进来了", uarray[i].uinfo.userid);
	for(n=0;n<NUARRAY;n++) {
		if(uarray[n].fd<0||uarray[n].haslogin==0
				||uarray[n].status.room!=r
				||n==i)
			continue;
		addmsgqueue(n, strmq('C', str));
	}
	sprintf(str, "%s %s", room[r].name, room[r].title);
	addmsgqueue(i, strmq(4, str));
	sprintf(str,"%d",room[r].type);
	addmsgqueue(i, strmq(7,str));
	if(chesslist[room[r].type].restore!=NULL)
		chesslist[room[r].type].restore(i,r);
	//sprintf(str,"你进入了%s房间\n", room[r].name);
	//addmsgqueue(i, strmq('C',str));
	return 0;
}

int cmd_chat(int i, struct mymsg *m)
{
	int  r;
	char str[100];
	if(!uarray[i].haslogin)
		return 0;
	r=uarray[i].status.room;
	if(m->len>=70)
		m->data[66]=0;
	sprintf(str, "%s: %s", uarray[i].uinfo.userid, m->data);
	broadcastroom(r,'C',str);
	return 0;
}

int cmd_chess(int i, struct mymsg *m)
{
	if(!uarray[i].haslogin)
		return 0;
#ifdef DEBUG
	printf("cmd_chess\n");
#endif
	if(chesslist[room[uarray[i].status.room].type].go!=NULL)
		chesslist[room[uarray[i].status.room].type].go(i,uarray[i].status.room,m->data);
	return 0;
}

int cmd_type(int i, struct mymsg *m)
{
	char str[80];
	int n, type, r=uarray[i].status.room;
	if(!uarray[i].haslogin)
		return 0;
	m->data[m->len-3-1]=0;
	if(sscanf(m->data,"%d", &type)!=1)
		return 0;
	if(type<0||type>=4) {
		addmsgqueue(i, strmq('C',"系统: 错误的房间类型类型"));
		return 0;
	}
	if(r==0) {
		addmsgqueue(i, strmq('C', "系统: 大厅的房间类型类型不能改变,"
				       " 请先\"/j\"开房间或建房间"));
		return 0;
	}
	if(room[r].type!=0&&room[r].owner!=i) {
#ifdef DEBUG
		sprintf(str, "type=%d, owner=%d", room[r].type, room[r].owner);
		addmsgqueue(i, strmq('C', str));
#endif
		addmsgqueue(i, strmq('C', "系统: 对不起, 你不是这个房间的老大"));
		return 0;
	}

	if(type==room[r].type) {
		addmsgqueue(i, strmq('C', "系统: 这个房间本来就是这个类型啊"));
		return 0;
	}
	if(room[r].data!=NULL) {
		free(room[r].data);
		room[r].data=NULL;
	}
	for(n=0;n<8;n++)
		room[r].seat[n]=-1;
	room[r].type=type;
	if((chesslist[type].init)!=NULL)
		chesslist[type].init(r);
	sprintf(str,"%d",room[r].type);
	broadcastroom(r,7,str);
	sprintf(str,"系统: 房间被 %s 设置为 %s 室", uarray[i].uinfo.userid, chesslist[type].name);
	broadcastroom(r,'C',str);
	return 0;
}

int cmd_seat(int i, struct mymsg *m)
{
	int r, s=atoi(m->data);
	char str[30];
	if(!uarray[i].haslogin)
		return 0;
	r=uarray[i].status.room;
	if(s<0||s>=chesslist[room[r].type].max
			||(room[r].seat[s]!=-1&&
				room[r].seat[s]!=i))
		return 0;
	if(room[r].seat[s]!=i) {
		room[r].seat[s]=i;
		sprintf(str, "您在 %d 号座位就座",s );
	} else {
		room[r].seat[s]=-1;
		sprintf(str, "您离开了 %d 号座位", s);
	}
	if(chesslist[room[r].type].seatchanged!=NULL)
		chesslist[room[r].type].seatchanged(r);
	addmsgqueue(i, strmq('C',str));
	return 0;
}

int cmd_kick(int i, struct mymsg *m)
{
	int r, n;
	char str[80], who[40];
	if(!uarray[i].haslogin)
		return 0;
	r=uarray[i].status.room;
	if(room[r].owner!=i) {
		addmsgqueue(i, strmq('C', "系统: 对不起, 你不是这个房间的管理员"));
		return 0;
	}
	m->data[((m->len-4)>39)?39:(m->len-4)]=0;
	if(sscanf(m->data, "%s", who)!=1||strlen(who)>=19) {
		addmsgqueue(i, strmq('C', "系统: 请给出正确的id"));
		return 0;
	}
	who[19]=0;
	for(n=0;n<NUARRAY;n++) {
		if(uarray[n].fd<0||uarray[n].status.room!=r) continue;
		if(!strcmp(uarray[n].uinfo.userid, who))
			break;
	}
	if(n==NUARRAY) {
		sprintf(str, "系统: 这个房间没有叫 %s 的呀", who);
		addmsgqueue(i, strmq('C',str));
		return 0;
	}
	if(uarray[i].status.room==0)
		removeuser(n);
	else {
		doexitroom(n);
		doenterroom(n, 0, "ytht");
	}
	return 0;
}

int cmd_setowner(int i, struct mymsg *m)
{
	int r, n;
	char str[80], who[40];
	if(!uarray[i].haslogin)
		return 0;
	r=uarray[i].status.room;
	if(room[r].owner!=i) {
		addmsgqueue(i, strmq('C', "系统: 对不起, 你不是这个房间的管理员"));
		return 0;
	}
	m->data[((m->len-4)>39)?39:(m->len-4)]=0;
	if(sscanf(m->data, "%s", who)!=1||strlen(who)>=19) {
		addmsgqueue(i, strmq('C', "系统: 请给出正确的id"));
		return 0;
	}
	who[19]=0;
	for(n=0;n<NUARRAY;n++) {
		if(uarray[n].fd<0||uarray[n].status.room!=r) continue;
		if(!strcmp(uarray[n].uinfo.userid, who))
			break;
	}
	if(n==NUARRAY) {
		sprintf(str, "系统: 这个房间没有叫 %s 的呀", who);
		addmsgqueue(i, strmq('C',str));
		return 0;
	}
	if(n==i) {
		addmsgqueue(i, strmq('C',"系统: 你本来就是这个房间的管理员啊"));
		return 0;
	}
	room[r].owner=n;
	sprintf(str,"系统: %s 将管理权限交给了 %s",
			uarray[i].uinfo.userid, uarray[n].uinfo.userid);
	broadcastroom(r, 'C', str);
	return 0;
}
	
	
struct fivedata {
	char matrix[15][15];
	int who;
	int n;
	int fi;
	char q[2];
	char history[250][2];
};

int five_init(int r)
{
	struct fivedata *f;
	f=malloc(sizeof(struct fivedata));
	room[r].data=(void*)f;
	bzero(f, sizeof(struct fivedata));
	return 0;
}

int five_restart(int r)
{
	bzero(room[r].data, sizeof(struct fivedata));
	broadcastroom(r,6,"N");
	five_seatchanged(r);
	return 0;
}


int five_addhistory(struct fivedata *f, int y, int x)
{
	if(f->n>=250) {
		memmove(f->history, (char*)f->history+20, 250*2-20);
		f->n-=10;
	}
	f->history[f->n][0]=y;
	f->history[f->n][1]=x;
	f->n++;
	return 0;
}

int five_back(int r)
{
	return 0;
}

int numstr(int k[], int nk, int s[], int ns)
{
	int i, j;
	for(i=0;i<=nk-ns;i++) {
		for(j=0;j<ns;j++) {
			if(k[i+j]!=s[j]) break;
		}
		if(j==ns) return 1;
	}
	return 0;
}

int five_found;
int five_calc(int k[9])
{
	int six[6]={1,1,1,1,1,1};
	int five[5]={1,1,1,1,1};
	int four[5][5]={{0,1,1,1,1}, {1, 0, 1, 1, 1},
		{1, 1, 0, 1, 1}, {1, 1, 1, 0, 1}, {1,1,1,1,0}};
	int three[4][6]={{0,0,1,1,1,0},{0,1,1,1,0,0},
			  {0,1,0,1,1,0},{0,1,1,0,1,0}};
	int i;
	five_found=1;
	if(numstr(k,9,six,6)) return 6;
	if(numstr(k,9,five,5)) return 5;
	for(i=0;i<5;i++)
		if(numstr(k,9,four[i],5))
			return 4;
	for(i=0;i<4;i++)
		if(numstr(k,9,three[i],6))
			return 3;
	five_found=0;
	return 0;
}

int five_check(struct fivedata *f, int y, int x)
{
	int i, j, n, num[7]={0,0,0,0,0,0,0};
	int k[9];
	int xx, yy, retv;
	char str[60];
	j=f->matrix[y-1][x-1];
	for(i=0;i<9;i++) {
		yy=y-1-4+i;
		if(yy<0||yy>=15) k[i]=2;
		else if(!f->matrix[yy][x-1]) k[i]=0;
		else if(f->matrix[yy][x-1]==j) k[i]=1;
		else k[i]=2;
	}
	retv=five_calc(k);
	num[retv]++;
	for(i=0;i<9;i++) {
		xx=x-1-4+i;
		if(xx<0||xx>=15) k[i]=2;
		else if(!f->matrix[y-1][xx]) k[i]=0;
		else if(f->matrix[y-1][xx]==j) k[i]=1;
		else k[i]=2;
	}
	retv=five_calc(k);
	num[retv]++;
	for(i=0;i<9;i++) {
		xx=x-1-4+i;
		yy=y-1-4+i;
		if(xx<0||xx>=15||yy<0||yy>=15) k[i]=2;
		else if(!f->matrix[yy][xx]) k[i]=0;
		else if(f->matrix[yy][xx]==j) k[i]=1;
		else k[i]=2;
	}
	retv=five_calc(k);
	num[retv]++;
	for(i=0;i<9;i++) {
		xx=x-1-4+i;
		yy=y-1+4-i;
		if(xx<0||xx>=15||yy<0||yy>=15) k[i]=2;
		else if(!f->matrix[yy][xx]) k[i]=0;
		else if(f->matrix[yy][xx]==j) k[i]=1;
		else k[i]=2;
	}
	retv=five_calc(k);
	num[retv]++;
	if(num[6])
		return 1; //白胜
	if(num[5])
		return j-1;
	if(num[4]==2&&j==1)
		return 1; //白胜
	if(num[4]==0&&num[3]==2&&j==1)
		return 1; //白胜
	return -1;
	
}

int five_go(int i, int r, char *data)
{
	struct fivedata *f=(struct fivedata *)room[r].data;
	int x, y, retv;
	char str[30];
	if(i!=room[r].seat[0]&&i!=room[r].seat[1])
		return 0;
	if(room[r].seat[0]<0) f->q[0]=0;
	if(room[r].seat[1]<0) f->q[1]=0;
	if(data[0]=='y'||data[0]=='n') {
		int who=(i==room[r].seat[0])?0:1;
		if(room[r].seat[0]==room[r].seat[1]) {
			if(f->q[who]==0) who=!who;
			else f->q[!who]=0;
		}
		switch(f->q[who]) {
			case 0: //没有问问题哦
				return 0;
			case 1: //是否开新局
				f->q[who]=0;
				if(data[0]=='n') {
					if(room[r].seat[!who]>=0) {
						addmsgqueue(room[r].seat[!who], strmq(6, "n"));
						f->q[!who]=0;
					}
					return 0;
				}
				if(f->q[!who]==0) {
					five_restart(r); //双方都同意了, 重新开局
					broadcastroom(r,'C',"重新开局");
				} else
					addmsgqueue(i, strmq('C', "正在征询对方意见"));
				return 0;
			case 2: //是否同意悔棋
				f->q[who]=0;
				if(data[0]=='n') {
					addmsgqueue(room[r].seat[!who], strmq(6, "n"));
					return 0;
				}
				do {
					five_back(r);
				}while(f->who!=!who); //退到该你走的那一步
				return 0;
		}
	}
	if(f->q[0]||f->q[1])
		return 0;
	
	//要求重新开局
	if(data[0]=='N') {
		int who=(i==room[r].seat[0])?0:1;
		f->q[!who]=1;
		addmsgqueue(i, strmq('C', "正在征求对方意见"));
		addmsgqueue(room[r].seat[!who], strmq(6,"q对方提出重新开局, 是否同意?(y/n)"));
		return 0;
	}
	if(f->fi) return 0;
	//认输
	if(data[0]=='F') {
		int who=(i==room[r].seat[0])?0:1;
		if(f->n==0) return 0;
		f->q[0]=1;
		f->q[1]=1;
		f->fi=1;
		addmsgqueue(room[r].seat[who], strmq(6,"q你认输了, 重新开局?(y/n)"));
		addmsgqueue(room[r].seat[!who], strmq(6,"q对方认输了, 重新开局?(y/n)"));
		sprintf(str,"%s 认输了", uarray[i].uinfo.userid);
		broadcastroom(r, 'C', str);
		return 0;
	}
	
	if(room[r].seat[f->who]!=i)
		return 0;
	if(sscanf(data, "%d %d", &y, &x)!=2)
		return 0;
	if(y<=0||y>15||x<=0||x>15)
		return 0;
	if(f->matrix[y-1][x-1]!=0)
		return 0;
	sprintf(str, "g%d %d %d", f->who, y, x);
	f->matrix[y-1][x-1]=f->who+1;
	f->who=!f->who;
	broadcastroom(r,6,str);
	five_addhistory(f,y,x);
	if((retv=five_check(f, y, x))>=0) {
		sprintf(str,"%s方赢了", (retv==0)?"黑":"白");
		broadcastroom(r, 'C', str);
		f->fi=1;
		f->q[0]=1;
		f->q[1]=1;
		addmsgqueue(room[r].seat[!retv], strmq(6,"q你输了, 重新开局?(y/n)"));
		addmsgqueue(room[r].seat[retv], strmq(6,"q你赢了, 重新开局?(y/n)"));
	}
	return 0;
}

int five_restore(int i, int r)
{
	char str[50];
	struct fivedata *f=(struct fivedata*)room[r].data;
	struct mymsgqueue *mq=mallocmq(4+sizeof(f->matrix));
#ifdef DEBUG
	printf("sending five restore msg\n");
#endif
	mq->m.cmd=6;
	mq->m.len=4+sizeof(f->matrix);
	mq->m.data[0]='r';
	memcpy(mq->m.data+1,f->matrix,sizeof(f->matrix));
	addmsgqueue(i,mq);
	
	if(f->n<250) f->history[f->n][0]=0;
	mq=mallocmq(4+sizeof(f->history));
	mq->m.cmd=6;
	mq->m.len=4+sizeof(f->history);
	mq->m.data[0]='R';
	memcpy(mq->m.data+1, f->history, sizeof(f->history));
	addmsgqueue(i,mq);
	
	sprintf(str, "s%s %s %d",
		(room[r].seat[0]!=-1)?uarray[room[r].seat[0]].uinfo.userid:"空座位0",
		(room[r].seat[1]!=-1)?uarray[room[r].seat[1]].uinfo.userid:"空座位1",
		f->who);
	addmsgqueue(i,strmq(6,str));
	return 0;
}

int five_seatchanged(int r)
{
	char str[50];
	struct fivedata *f=(struct fivedata *)room[r].data;
	str[0]=0;
#ifdef DEBUG
	printf("five_seatchanged");
#endif
	sprintf(str, "s%s %s %d",
			(room[r].seat[0]!=-1)?uarray[room[r].seat[0]].uinfo.userid:"空座位0",
			(room[r].seat[1]!=-1)?uarray[room[r].seat[1]].uinfo.userid:"空座位1",
			f->who);
	broadcastroom(r,6,str);
	return 0;
}

int broadcastroom(int r, int cmd, char *str)
{
	int n;
	for(n=0;n<NUARRAY;n++) {
		if(uarray[n].fd<=0||!uarray[n].haslogin||uarray[n].status.room!=r) continue;
		addmsgqueue(n,strmq(cmd,str));
	}
	return 0;
}


#define c_jiang 1
#define c_shi	2
#define c_xiang	3
#define c_ju	4
#define c_ma	5
#define c_pao	6
#define c_zu	7

struct cchessdata {
	char matrix[10][9];
	int who;
	int n, fi;
	char q[2];
	char history[1000][5];
	
};

struct cchessdata cchess_ff={ {
	{c_ju, c_ma, c_xiang, c_shi, c_jiang, c_shi, c_xiang, c_ma, c_ju},
	{0,},
	{0, c_pao, 0, 0, 0, 0, 0, c_pao, 0},
	{c_zu, 0, c_zu, 0, c_zu, 0, c_zu, 0, c_zu},
	{0,},
	{0,},
	{-c_zu, 0, -c_zu, 0, -c_zu, 0, -c_zu, 0, -c_zu},
	{0, -c_pao, 0, 0, 0, 0, 0, -c_pao, 0},
	{0,},
	{-c_ju, -c_ma, -c_xiang, -c_shi, -c_jiang, -c_shi, -c_xiang, -c_ma, -c_ju}
}, 0,0,0, {0, 0},};
	
int cchess_addhistory(struct cchessdata *c, int y0, int x0, int y, int x, int z)
{
	if(c->n>=1000) {
		memmove(c->history, (char*)c->history+20, 1000*2-20);
		c->n-=10;
	}
	c->history[c->n][0]=y0;
	c->history[c->n][1]=x0;
	c->history[c->n][2]=y;
	c->history[c->n][3]=x;
	c->history[c->n][4]=z;
	c->n++;
	return 0;
}

int cchess_init(int r)
{
	struct cchessdata *f;
	f=malloc(sizeof(struct cchessdata));
	room[r].data=(void*)f;
	*f=cchess_ff;
	return 0;
}

int cchess_restart(int r)
{
	struct cchessdata *f;
	f=(struct cchessdata *)room[r].data;
	*f=cchess_ff;
	broadcastroom(r,6,"N");
	cchess_seatchanged(r);
	return 0;
}

#define min(x,y)   ((x>y)?y:x)
#define max(x,y)   ((x>y)?x:y)

int can_cchess(char matrix[10][9], int i0, int j0, int i, int j)
{
	int n, m, z=abs(matrix[i0][j0]);
	switch(z) {
		case c_jiang:
			if(j0==j&&abs(matrix[i][j])==c_jiang) {
				m=0;
				for(n=min(i0,i)+1;n<max(i0,i);n++)
					if(matrix[n][j])
						m++;
				if(m==0) return 0;
				return -1;
			}
			if((i>2&&i<7)||(j<3||j>5))
				return -1;
			if(abs(i-i0)+abs(j-j0)>1)
				return -1;
			return 0;
		case c_shi:
			if((i>2&&i<7)||(j<3||j>5))
				return -1;
			if(abs(i-i0)==1&&abs(j-j0)==1)
				return 0;
			return -1;
		case c_xiang:
			if((i<5&&i0>=5)||(i>=5&&i0<5))
				return -1;
			if(abs(i-i0)!=2||abs(j-j0)!=2)
				return -1;
			if(matrix[(i+i0)/2][(j+j0)/2]!=0)
				return -1;
			return 0;
		case c_ju:
			if(i==i0) {
				for(n=min(j0,j)+1;n<max(j0,j);n++)
					if(matrix[i][n])
						return -1;
				return 0;
			}
			if(j==j0) {
				for(n=min(i0,i)+1;n<max(i0,i);n++)
					if(matrix[n][j])
						return -1;
				return 0;
			}
			return -1;
		case c_ma:
			if(abs(i-i0)==2&&abs(j-j0)==1) {
				if(matrix[(i+i0)/2][j0])
					return -1;
				return 0;
			}
			if(abs(i-i0)==1&&abs(j-j0)==2) {
				if(matrix[i0][(j+j0)/2])
					return -1;
				return 0;
			}
			return -1;
		case c_pao:
			m=0;
			if(i!=i0&&j!=j0)
				return -1;
			if(i==i0) {
				for(n=min(j0,j)+1;n<max(j0,j);n++)
					if(matrix[i][n])
						m++;
			} else if(j==j0) {
				for(n=min(i0,i)+1;n<max(i0,i);n++)
					if(matrix[n][j])
						m++;
			}
			if(m==1&&matrix[i][j]) return 0;
			if(m==0&&!matrix[i][j]) return 0;
			return -1;
		case c_zu:
			if((abs(i-i0)+abs(j-j0))!=1)
				return -1;
			if(matrix[i0][j0]>0) {
				if(i0<5&&(i-i0)<=0)
					return -1;
				if(i-i0<0)
					return -1;
				return 0;
			}
			if(matrix[i0][j0]<0) {
				if(i0>4&&(i-i0)>=0)
					return -1;
				if(i-i0>0)
					return -1;
				return 0;
			}
	}
	return 0;
}

int cchess_back(int r)
{
	return 0;
}

int cchess_go(int i, int r, char *data)
{
	int x0, y0, x, y, fi=0;
	struct cchessdata *f=(struct cchessdata *)room[r].data;
	char str[40];
	if(i!=room[r].seat[0]&&i!=room[r].seat[1])
		return 0;
	if(room[r].seat[0]<0) f->q[0]=0;
	if(room[r].seat[1]<0) f->q[1]=0;
	if(data[0]=='y'||data[0]=='n') {
		int who=(i==room[r].seat[0])?0:1;
		if(room[r].seat[0]==room[r].seat[1])
			f->q[!who]=0;
		switch(f->q[who]) {
			case 0: //没有问问题哦
				return 0;
			case 1: //是否开新局
				f->q[who]=0;
				if(data[0]=='n') {
					if(room[r].seat[!who]>=0) {
						addmsgqueue(room[r].seat[!who], strmq(6, "n"));
						f->q[!who]=0;
					}
					return 0;
				}
				if(f->q[!who]==0) {
					cchess_restart(r); //双方都同意了, 重新开局
					broadcastroom(r,'C',"重新开局");
				} else
					addmsgqueue(i, strmq('C', "正在征询对方意见"));
				return 0;
			case 2: //是否同意悔棋
				f->q[who]=0;
				if(data[0]=='n') {
					addmsgqueue(room[r].seat[!who], strmq(6, "n"));
					return 0;
				}
				do {
					cchess_back(r);
				}while(f->who!=!who); //退到该你走的那一步
				return 0;
		}
	}
	if(f->q[0]||f->q[1])
		return 0;
	
	//要求重新开局
	if(data[0]=='N') {
		int who=(i==room[r].seat[0])?0:1;
		f->q[!who]=1;
		addmsgqueue(i, strmq('C', "正在征求对方意见"));
		addmsgqueue(room[r].seat[!who], strmq(6,"q对方提出重新开局, 是否同意?(y/n)"));
		return 0;
	}
	if(f->fi) return 0;
	//认输
	if(data[0]=='F') {
		int who=(i==room[r].seat[0])?0:1;
		f->q[0]=1;
		f->q[1]=1;
		f->fi=1;
		addmsgqueue(room[r].seat[who], strmq(6,"q你认输了, 重新开局?(y/n)"));
		addmsgqueue(room[r].seat[!who], strmq(6,"q对方认输了, 重新开局?(y/n)"));
		sprintf(str,"%s 认输了", uarray[i].uinfo.userid);
		broadcastroom(r, 'C', str);
		return 0;
	}
	
	if(room[r].seat[f->who]!=i)
		return 0;
	if(sscanf(data, "%d %d %d %d", &y0, &x0, &y, &x)!=4)
		return 0;
	if(y<=0||y>10||x<=0||x>9||y0<=0||y0>10||x0<=0||x0>9)
		return 0;
	if(y0==y&&x0==x)
		return 0;
	if(!f->who&&(f->matrix[y0-1][x0-1]<=0||f->matrix[y-1][x-1]>0))
		return 0;
	if(f->who&&(f->matrix[y0-1][x0-1]>=0||f->matrix[y-1][x-1]<0))
		return 0;
	if(can_cchess(f->matrix, y0-1, x0-1, y-1, x-1)==-1)
		return 0;
	sprintf(str, "g%d %d %d %d %d", f->who, y0, x0, y, x);
	broadcastroom(r,6,str);
	cchess_addhistory(f, y0, x0, y, x, f->matrix[y0-1][x0-1]);
	if(abs(f->matrix[y-1][x-1])==c_jiang) {
		sprintf(str,"%s方输了", f->who?"红":"绿");
		broadcastroom(r, 'C', str);
		f->fi=1;
		f->q[0]=1;
		f->q[1]=1;
		addmsgqueue(room[r].seat[!f->who], strmq(6,"q你输了, 重新开局?(y/n)"));
		addmsgqueue(room[r].seat[f->who], strmq(6,"q对方输了, 重新开局?(y/n)"));
	}	
	f->matrix[y-1][x-1]=f->matrix[y0-1][x0-1];
	f->matrix[y0-1][x0-1]=0;
	f->who=!f->who;
	return 0;
}

int cchess_restore(int i, int r)
{
	char str[50];
	struct cchessdata *f=(struct cchessdata*)room[r].data;
	struct mymsgqueue *mq=mallocmq(4+sizeof(f->matrix));
	mq->m.cmd=6;
	mq->m.len=4+sizeof(f->matrix);
	mq->m.data[0]='r';
	memcpy(mq->m.data+1,f->matrix,sizeof(f->matrix));
	addmsgqueue(i,mq);

	if(f->n<1000) f->history[f->n][0]=0;
	mq=mallocmq(4+sizeof(f->history));
	mq->m.cmd=6;
	mq->m.len=4+sizeof(f->history);
	mq->m.data[0]='R';
	memcpy(mq->m.data+1, f->history, sizeof(f->history));
	addmsgqueue(i,mq);
	
	sprintf(str, "s%s %s %d",
		(room[r].seat[0]!=-1)?uarray[room[r].seat[0]].uinfo.userid:"空座位0",
		(room[r].seat[1]!=-1)?uarray[room[r].seat[1]].uinfo.userid:"空座位1",
		f->who);
	addmsgqueue(i,strmq(6,str));
	return 0;
}

int cchess_seatchanged(int r)
{
	char str[50];
	struct cchessdata *f=(struct cchessdata *)room[r].data;
	str[0]=0;
	sprintf(str, "s%s %s %d",
			(room[r].seat[0]!=-1)?uarray[room[r].seat[0]].uinfo.userid:"空座位0",
			(room[r].seat[1]!=-1)?uarray[room[r].seat[1]].uinfo.userid:"空座位1",
			f->who);
	broadcastroom(r,6,str);
	return 0;
}

struct godata {
	char matrix[19][19];
	char history[1500][2];
	int n;
	int uy, ux;
	int who;
};

int go_init(int r)
{
	struct godata *f;
	f=malloc(sizeof(struct godata));
	room[r].data=(void*)f;
	bzero(f, sizeof(struct godata));
	return 0;
}

int check_godata(char m[19][19], int who, int *y, int *x);

int go_addhistory(struct godata *g, int y, int x)
{
	if(g->n>=1000) {
		memmove(g->history, (char*)g->history+20, 1000*2-20);
		g->n-=10;
	}
	g->history[g->n][0]=y;
	g->history[g->n][1]=x;
	g->n++;
	return 0;
}

int single(char m[19][19], int y, int x, int nw)
{
	int i,j,k;
	i=y-1;
	j=x-1;
	k=nw+1;
	if((i==18||m[i+1][j]==k)&&
	   (i==0||m[i-1][j]==k)&&
	   (j==18||m[i][j+1]==k)&&
	   (j==0||m[i][j-1]==k))
		return 1;
	return 0;
}

int go_go(int i, int r, char *data)
{
	struct godata *f=(struct godata *)room[r].data;
	int x, y, c, si;
	char str[30];
	if(room[r].seat[f->who]!=i)
		return 0;
	if(sscanf(data, "%d %d", &y, &x)!=2)
		return 0;
	if(y<=0||y>19||x<=0||x>19)
		return 0;
	if(f->matrix[y-1][x-1]!=0)
		return 0;
	if(f->uy==y&&f->ux==x) {
		addmsgqueue(i,strmq('C',"系统: 这个位置现在不能落子呢, 换个地方走走吧"));
		return 0;
	}
	f->matrix[y-1][x-1]=f->who+1;
	si=single(f->matrix, y, x, !f->who); //是不是一个无气单子
	c=check_godata(f->matrix, !f->who, &f->uy, &f->ux); //拔对方的子
	if(c==0&&si) {
		f->matrix[y-1][x-1]=0;
		addmsgqueue(i, strmq('C', "系统: 这是毫无价值的走法, 换个地方走走吧"));
		return 0;
	}
	if(c!=1||!si) f->uy=0;
	sprintf(str, "g%d %d %d", f->who, y, x);
	broadcastroom(r,6,str);
	check_godata(f->matrix, f->who, &c, &c); //拔自己的子
	f->who=!f->who; //倒手
	go_addhistory(f, y, x);
	return 0;
}

int check_godata(char m[19][19], int who, int *y, int *x)
{
	int i, j, c=1;
	char matrix[19][19];
	memcpy(matrix, m, sizeof(matrix));
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
	for(i=0;i<19;i++) for(j=0;j<19;j++)
		if(matrix[i][j]==who+1) {
			m[i][j]=0;
			*y=i+1;
			*x=j+1;
			c++;
		}
	return c;
}

int go_restore(int i, int r)
{
	char str[50];
	struct godata *f=(struct godata*)room[r].data;
	struct mymsgqueue *mq=mallocmq(4+sizeof(f->matrix));
	mq->m.cmd=6;
	mq->m.len=4+sizeof(f->matrix);
	mq->m.data[0]='r';
	memcpy(mq->m.data+1,f->matrix,sizeof(f->matrix));
	addmsgqueue(i,mq);

	if(f->n<1000) f->history[f->n][0]=0;
	mq=mallocmq(4+sizeof(f->history));
	mq->m.cmd=6;
	mq->m.len=4+sizeof(f->history);
	mq->m.data[0]='R';
	memcpy(mq->m.data+1, f->history, sizeof(f->history));
	addmsgqueue(i,mq);
	sprintf(str, "s%s %s %d",
		(room[r].seat[0]!=-1)?uarray[room[r].seat[0]].uinfo.userid:"空座位0",
		(room[r].seat[1]!=-1)?uarray[room[r].seat[1]].uinfo.userid:"空座位1",
		f->who);
	addmsgqueue(i,strmq(6,str));
	return 0;
}

int go_seatchanged(int r)
{
	char str[50];
	struct godata *f=(struct godata *)room[r].data;
	str[0]=0;
	sprintf(str, "s%s %s %d",
			(room[r].seat[0]!=-1)?uarray[room[r].seat[0]].uinfo.userid:"空座位0",
			(room[r].seat[1]!=-1)?uarray[room[r].seat[1]].uinfo.userid:"空座位1",
			f->who);
	broadcastroom(r,6,str);
	return 0;
}


