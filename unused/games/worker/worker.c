/* 
   	program name: worker.c
   	Chinese name: ÍÆÏä×Ó
   	this program is a GNU free software 
   	first written by period.bbs@smth.org 
   	             and cityhunter.bbs@smth.org, Nov 11, 1998(?)
   	rewitten by zhch.bbs@bbs.nju.edu.cn, Nov 27, 2000

   	Ê¹ÓÃ·½·¨, °Ñworker·ÅÔÚbinÄ¿Â¼, map.dat·ÅÔÚbbs homeÄ¿Â¼,
	È»ºóÔÚbbsÖÐÓÃsystemµ÷ÓÃ:
	(Èç¹ûmkcfraw³ö´í, ÆÁ±Îµôtelnet_init, µ«ÕâÑù¾ÍÖ»ÄÜÔÚbbsÖÐµ÷ÓÃÁË)

	cc worker.c -o worker
	cp worker ~/bin
	cp map.dat ~
*/

#include "stdio.h"
#include "termios.h"

char genbuf[256];

#define KEY_UP 		'A'
#define KEY_DOWN	'B'
#define KEY_RIGHT	'C'
#define KEY_LEFT	'D'

#define move(y, x) printf("[%d;%dH", y+1, x*2+1)
#define clear() printf("[H[J")
#define pressanykey() inkey()
#define fatal(s) printf("Fatal error: %s!\r\n", s); quit()
#define refresh() fflush(stdout)
 
int map_data[100][20][30];
int map_now[20][30];
int map_total=1;

int stage;
int now_y, now_x;

struct history {
	int max;
	char y0[10000];
	char x0[10000];
	char y1[10000];
	char x1[10000];
} my_history;

struct termios __t, __t0;
int ttymode=0;

int telnet_init() {
	if (tcgetattr(0, &__t0)>=0) {
		ttymode=1;
		cfmakeraw(&__t);
		tcsetattr(0, TCSANOW, &__t);
	}
}

int quit() {
	if(ttymode) tcsetattr(0, TCSANOW, &__t0);
	exit(0);
}

int _inkey() {
	if(read(0, genbuf, 1)<=0) quit();
	return genbuf[0];
}


int inkey() {
	int c;
	c=_inkey();
	if(c==3 || c==4) quit();
	if(c==127) c=8;
	if(c!=27) return c;
	c=_inkey();
	c=_inkey();
	if(c>='A' && c<='D') return c;
	return 0;
}

char *map_char(int n) {
	if(n&8) return "¡ö";
	if(n&4) return "¡õ";
	if(n&2) return "¡Ñ";
	if(n&1) return "¡¤";
	return "  "; 
}

int map_init() {
	FILE *fp;
	int map_y=0, map_x=0;
	fp=fopen("/home/bbs/etc/worker/map.dat", "r");
	if(fp==NULL) return 0;
	map_x=0;
	while(1) {
		bzero(genbuf, 80);
		if(fgets(genbuf, 80, fp)<=0) break;
		if(genbuf[0]=='-') {
			map_y=0;
			map_total++;
		}
		for(map_x=0; map_x<30; map_x++) {
                        if(!strncmp(genbuf+map_x*2, map_char(1), 2)) map_data[map_total-1][map_y][map_x]=1;
                        if(!strncmp(genbuf+map_x*2, map_char(2), 2)) map_data[map_total-1][map_y][map_x]=2;
                        if(!strncmp(genbuf+map_x*2, map_char(4), 2)) map_data[map_total-1][map_y][map_x]=4;
                        if(!strncmp(genbuf+map_x*2, map_char(8), 2)) map_data[map_total-1][map_y][map_x]=8;
		}
		if(map_y<20) map_y++;
	}
	fclose(fp);
	return 1;
}


int map_show() {
	int m, n, c;
	char *s;
	clear();
	for(n=0; n<20; n++) {
		for(m=0; m<30; m++) printf("%2.2s", map_char(map_now[n][m]));
		printf("\r\n");
	}
	move(20, 0);
	printf("[44m                                                                        [m");
	move(19, 0);
	printf("¹¦ÄÜ¼ü  [1;32m¡ü[m [1;32m¡ý [m[1;32m¡û[m [1;32m¡ú[mÒÆ¶¯  ' ' [1;32m^C[m [1;32m^D[mÍË³ö  [1;32mBackSpace[m»ÚÆå  [1;32m^L[mË¢ÐÂ  [1;32mTAB[mÖØ¿ª\r\n");
	move(now_y, now_x);
	refresh();
	
}

int map_show_pos(int y, int x) {
	int c=map_now[y][x];
	move(y, x);
	if(c==5) 
		printf("[1;32m%2.2s[m", map_char(c));
        else
		printf("%2.2s", map_char(c));
}

int check_if_win() {
	int m, n;
	for(n=0; n<20; n++)
	for(m=0; m<30; m++)
		if(map_now[n][m]==1 || map_now[n][m]==3) return 0;
	return 1;
}

int find_y_x(int *y, int *x) {
	int m, n;
	for(n=0; n<20; n++)
	for(m=0; m<30; m++)
		if(map_now[n][m]&2) {
			*x=m;
			*y=n;
			return 1;
		};
	return 0;
}

map_move(int y0, int x0, int y1, int x1) {
	int b0, f0, b1, f1;
	b0=map_now[y0][x0]&1;
	f0=map_now[y0][x0]&6;
	b1=map_now[y1][x1]&1;
	map_now[y1][x1]=f0 | b1;
	map_now[y0][x0]=b0;
	map_show_pos(y0, x0);
	map_show_pos(y1, x1);
	move(20, 0);
	printf("[0;44m[[1;33mÍÆÏä×Ó v1127[0;44m]    [µÚ [1;33m%d[0;44m ¹Ø] [µÚ [1;33m%d[0;44m ²½] [From (%d,%d) to (%d,%d)] [m", stage, my_history.max, y0, x0, y1, x1);
}

main() {
	int c, m, n, i;
	int dx, dy;
	telnet_init();
	if(map_init()==0) printf("map.dat error\n");
	stage=0;
	clear();
	printf("»¶Ó­¹âÁÙ[1;32mÍÆÏä×Ó[mÓÎÏ·¡£\r\n");
	printf("¹æÔòºÜ¼òµ¥£¬Ö»Ðè°ÑËùÓÐµÄ'¡õ'¶¼ÍÆµ½'¡¤'ÉÏÃæÈ¥(»á±ä³ÉÂÌÉ«)¾Í¹ý¹ØÁË¡£\r\n");
	printf("µ«ÍæÆðÀ´ÄÑ¶È¿ÉÊÇÏàµ±´óµÄ£¬²»ÒªÇáÊÓà¸¡£\r\n");
	refresh();
	pressanykey();
	clear();
	printf("ÇëÓÃ·½Ïò¼üÑ¡¹Ø, »Ø³µ¼üÈ·ÈÏ: 0 ");
	move(0, 14);
	refresh();
	stage=0;
	while(1) {
		c=inkey();
		if((c==KEY_LEFT||c==KEY_UP) && stage>0) stage--;
		if((c==KEY_RIGHT||c==KEY_DOWN) && stage<map_total-1) stage++;
		if(c==10 || c==13) break;
		if(c==3 || c==4 || c==32) quit();
		move(0, 14);
		printf("%d  ", stage);
		move(0, 14);
		refresh();
	}
start:
	if(stage<0 || stage>map_total-1) stage=0;
	clear();
	printf("ÍÆÏä×Ó: µÚ [1;32m%d[m ¹Ø", stage);
	move(20, 0);
	refresh();
	sleep(1);
start2:
	for(n=0; n<20; n++)
	for(m=0; m<30; m++)
		map_now[n][m]=map_data[stage][n][m];
        if(!find_y_x(&now_y, &now_x)) printf("stage error\n");
	map_show();
	bzero(&my_history, sizeof(my_history));
	while(1) {
		c=inkey();
		if(my_history.max>=1999) {
			move(21, 0);
			printf("ÄãÓÃÁË2000²½»¹Ã»ÓÐ³É¹¦! GAME OVER.");
			quit();
		}
		dx=0;
		dy=0;
		if(c==8 && my_history.max>0) {
			my_history.max--;
			i=my_history.max;
			map_move(my_history.y1[i], my_history.x1[i], my_history.y0[i], my_history.x0[i]);
			find_y_x(&now_y, &now_x);
		 	move(now_y, now_x);
			refresh();
			continue;
		}

		if(c==' ') quit();
		if(c=='') map_show();
		if(c==9) goto start2;
		
		if(c==KEY_UP) dy=-1;
		if(c==KEY_DOWN) dy=1;
		if(c==KEY_LEFT) dx=-1;
		if(c==KEY_RIGHT) dx=1;

		if(dx==0 && dy==0) continue;

		if(map_now[now_y+dy][now_x+dx]&4)
		if(map_now[now_y+dy*2][now_x+dx*2]<2) {
			map_move(now_y+dy, now_x+dx, now_y+dy*2, now_x+dx*2);
			i=my_history.max;
			my_history.y0[i]=now_y+dy;
			my_history.x0[i]=now_x+dx;
			my_history.y1[i]=now_y+dy*2;
			my_history.x1[i]=now_x+dx*2;
			my_history.max++;
		}
		if(map_now[now_y+dy][now_x+dx]<2) {
			map_move(now_y, now_x, now_y+dy, now_x+dx);
			i=my_history.max;
			my_history.y0[i]=now_y;
			my_history.x0[i]=now_x;
			my_history.y1[i]=now_y+dy;
			my_history.x1[i]=now_x+dx;
			my_history.max++;
		}
		if(check_if_win()) break;
		find_y_x(&now_y, &now_x);
		move(now_y, now_x);
		refresh();
	}
	move(19, 0);
	printf("×£ºØÄã, Äã³É¹¦ÁË£¡[K");
	refresh();
	sleep(3);
	stage++;
	goto start;
}
