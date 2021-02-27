/*  五子棋程式   Programmed by Birdman     */
/*  140.116.102.125 连珠哇哈哈小站         */
/*  成大电机88级                           */

#include "bbs.h"
#include <sys/socket.h>
#include "smth_screen.h"
#include "stuff.h"
#include "io.h"
#include "xyz.h"
#include "main.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

#define black 1
#define white 2
#define FDATA "five"
#define b_lines 24
#define LCECHO (2)
#define cuser currentuser
#define setutmpmode(a) modify_user_mode( a )

struct fivechess {
	int winner;
	int hand, tdeadf, tlivef, livethree, threefour;
	int playboard[15][15];
};

extern int RMSG;

static void Box(int x, int y, int x1, int y1);
static void InitScreen(void);
static void user_guide(void);
static void haha(int what);
static void win(struct fivechess *fc, int who);
static void quit(void);
static void calvalue(struct fivechess *fc, int x1, int y1, int x2, int y2,
		int x3, int y3, int x4, int y4, int x5, int y5);
static void callfour(struct fivechess *fc, int x1, int y1, int x2, int y2,
		int x3, int y3, int x4, int y4, int x5, int y5, int x6,
		int y6);
static void bandhand(struct fivechess *fc, int style);
static void five_chat(char *msg, int init);
static void press(void);

static void
Box(int x, int y, int x1, int y1)
{
	char *lt = "┌", *rt = "┐", *hor = "─", *ver = "│", *lb = "└", *rb = "┘";
	int i;

	move(x, y);
	outs(lt);
	for (i = y + 2; i <= y1 - 2; i += 2)
		outs(hor);
	outs(rt);
	for (i = x + 1; i <= x1 - 1; i++) {
		move(i, y);
		outs(ver);
		move(i, y1);
		outs(ver);
	}
	move(x1, y);
	outs(lb);
	for (i = y + 2; i <= y1 - 2; i += 2)
		outs(hor);
	outs(rb);
}

static void
InitScreen()
{
	int i;

	for (i = 0; i < 16; i++) {
		move(i, 0);
		clrtoeol();
	}
	move(0, 0);
	outs(   "┌┬┬┬┬┬┬┬┬┬┬┬┬┬┐15\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤14\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤13\n"
			"├┼┼＋┼┼┼┼┼┼┼＋┼┼┤12\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤11\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤10\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤9\n"
			"├┼┼┼┼┼┼＋┼┼┼┼┼┼┤8\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤7\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤6\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤5\n"
			"├┼┼＋┼┼┼┼┼┼┼＋┼┼┤4\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤3\n"
			"├┼┼┼┼┼┼┼┼┼┼┼┼┼┤2\n"
			"└┴┴┴┴┴┴┴┴┴┴┴┴┴┘1" "A B C D E F G H I J K L M N O");

	user_guide();
	move(0, 33);
	outs("\033[35;43m◆五子棋对战◆\033[30;42m  程式:成大电机88级 Birdman  \033[m");
}

static void
user_guide()
{
	move(4, 64);
	outs("切换:   Tab键");
	move(5, 64);
	outs("移动:  方向键");
	move(6, 64);
	outs("      H,J,K,L");
	move(7, 64);
	outs("下子:  空格键");
	move(8, 64);
	outs("重开:  N 或者");
	move(9, 64);
	outs("       Ctrl+N");
	move(10, 64);
	outs("退出:  Q 或者");
	move(11, 64);
	outs("       Ctrl+C");
	move(12, 64);
	outs("  黑先有禁手");
	Box(3, 62, 13, 78);
	move(3, 64);
	outs("用法");
}

static void
haha(int what)
{
	char *logo[3] = { " 活三喽! ", "哈哈活四!", " 小心冲四! " };

	move(15, 64);
	if (what >= 3)
		outs("            ");
	else
		outs(logo[what]);
}

static void
win(struct fivechess *fc, int who)
{
	move(12, 35);
	outs("\033[47m\033[31m┌————┐\033[m");
	move(13, 35);
	if (who == black)
		outs("\033[47m\033[31m│  \033[30;42m黑胜\033[m\033[47m \033[31m │\033[m");
	else
		outs("\033[47m\033[31m│  \033[30;42m白胜\033[m\033[47m \033[31m │\033[m");
	move(14, 35);
	outs("\033[47m\033[31m└————┘\033[m");
	fc->winner = who;
	press();
}

static void
quit(void)
{
	move(12, 35);
	outs("\033[47m\033[31m┌———————┐\033[m");
	move(13, 35);
	outs("\033[47m\033[31m│  \033[30;42m对方退出了\033[m\033[47m \033[31m │\033[m");
	move(14, 35);
	outs("\033[47m\033[31m└———————┘\033[m");
	bell();
	press();
}

static void
calvalue(struct fivechess *fc, int x1, int y1,
		int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5)
{
	int n_black, n_white, empty, i, j;

	n_black = n_white = empty = 0;

	if (x1 < 0 || x2 < 0 || x3 < 0 || x4 < 0 || x5 < 0 ||
			x1 > 14 || x2 > 14 || x3 > 14 || x4 > 14 || x5 > 14)
		return;
	if (fc->winner != 0)
		return;
	if (fc->playboard[x2][y2] == 0 || fc->playboard[x3][y3] == 0
			|| fc->playboard[x4][y4] == 0)
		empty = 1;	/*check 10111型死四 */

	if (fc->playboard[x1][y1] == black)
		n_black += 1;
	if (fc->playboard[x1][y1] == white)
		n_white += 1;
	if (fc->playboard[x2][y2] == black)
		n_black += 1;
	if (fc->playboard[x2][y2] == white)
		n_white += 1;
	if (fc->playboard[x3][y3] == black)
		n_black += 1;
	if (fc->playboard[x3][y3] == white)
		n_white += 1;
	if (fc->playboard[x4][y4] == black)
		n_black += 1;
	if (fc->playboard[x4][y4] == white)
		n_white += 1;
	if (fc->playboard[x5][y5] == black)
		n_black += 1;
	if (fc->playboard[x5][y5] == white)
		n_white += 1;

	if (fc->playboard[x1][y1] == 0 && fc->playboard[x5][y5] == 0) {
		if (n_white == 3 || n_black == 3)
			haha(0);

		if (n_black == 3)
			fc->livethree += 1;
	}

	if ((n_white == 4 || n_black == 4) && (empty == 1)) {
		fc->tdeadf += 1;
		fc->tlivef += 1;
		haha(2);
		return;
	}

	if (n_black == 5) {	/*再扫连六 */
		fc->tlivef = -1;
		fc->tdeadf = 0;
		fc->livethree = 0;
		for (i = 0; i <= 14; i++)	/*四纵向 */
			for (j = 0; j <= 9; j++)
				callfour(fc, i, j, i, j + 1, i, j + 2, i, j + 3,
						i, j + 4, i, j + 5);
		for (i = 0; i <= 9; i++)	/*四横向 */
			for (j = 0; j <= 14; j++)
				callfour(fc, i, j, i + 1, j, i + 2, j, i + 3, j,
						i + 4, j, i + 5, j);
		for (i = 0; i <= 9; i++)	/*四斜右下 */
			for (j = 0; j <= 9; j++) {
				callfour(fc, i, j, i + 1, j + 1, i + 2, j + 2,
						i + 3, j + 3, i + 4, j + 4, i + 5,
						j + 5);
				/*四斜左下 */
				callfour(fc, i, j + 5, i + 1, j + 4, i + 2,
						j + 3, i + 3, j + 2, i + 4, j + 1,
						i + 5, j);
			}
		if (fc->winner == 0)
			win(fc, black);
	}
	if (n_white == 5)
		win(fc, white);
	return;
}

static void
callfour(struct fivechess *fc, int x1, int y1, int x2, int y2, int x3, int y3,
		int x4, int y4, int x5, int y5, int x6, int y6)
{
	int n_black, n_white, dead;

	n_black = n_white = dead = 0;

	if (x1 < 0 || x2 < 0 || x3 < 0 || x4 < 0 || x5 < 0 || x6 < 0 ||
			x1 > 14 || x2 > 14 || x3 > 14 || x4 > 14 || x5 > 14 || x6 > 14)
		return;

	if (fc->winner != 0)
		return;

	if ((fc->playboard[x1][y1] != 0 && fc->playboard[x6][y6] == 0) ||
			(fc->playboard[x1][y1] == 0 && fc->playboard[x6][y6] != 0))
		dead = 1;	/* for checking  冲四 */

	if (fc->playboard[x2][y2] == black)
		n_black += 1;
	if (fc->playboard[x2][y2] == white)
		n_white += 1;
	if (fc->playboard[x3][y3] == black)
		n_black += 1;
	if (fc->playboard[x3][y3] == white)
		n_white += 1;
	if (fc->playboard[x4][y4] == black)
		n_black += 1;
	if (fc->playboard[x4][y4] == white)
		n_white += 1;
	if (fc->playboard[x5][y5] == black)
		n_black += 1;
	if (fc->playboard[x5][y5] == white)
		n_white += 1;

	if (fc->playboard[x1][y1] == 0 && fc->playboard[x6][y6] == 0 &&
			(fc->playboard[x3][y3] == 0 || fc->playboard[x4][y4] == 0)) {
		if (n_black == 3 || n_white == 3)
			haha(0);
		if (n_black == 3)
			fc->livethree += 1;
	}

	if (n_black == 4) {
		if (fc->playboard[x1][y1] == black && fc->playboard[x6][y6] == black)
			bandhand(fc, 6);
		if (fc->playboard[x1][y1] != 0 && fc->playboard[x6][y6] != 0)
			return;

		if (dead) {
/* add by satan Mar 19, 1999 start*/
			if ((fc->playboard[x1][y1] == 0 && fc->playboard[x5][y5] == 0)
					|| (fc->playboard[x2][y2] == 0 && fc->playboard[x6][y6] == 0))
				fc->livethree -= 1;
/* add by satan Mar 19, 1999 end*/

			haha(2);
			fc->tdeadf += 1;
			fc->tlivef += 1;	/*黑死四啦 */
			fc->threefour = 0;
			return;
		}

		fc->threefour = black;
		fc->tlivef += 1;	/*活四也算双四 */
	}
	if (n_white == 4) {
		if (fc->playboard[x1][y1] != 0 && fc->playboard[x6][y6] != 0)
			return;
		if (dead) {
			haha(2);
			fc->tdeadf += 1;
			fc->threefour = 0;
			return;
		}

		fc->threefour = white;
		fc->tlivef += 1;

	}
	if (fc->playboard[x1][y1] == black)
		n_black += 1;	/*check 连子 */
	if (fc->playboard[x6][y6] == black)
		n_black += 1;

	if (n_black == 5
			&& (fc->playboard[x3][y3] == 0 || fc->playboard[x4][y4] == 0
			|| fc->playboard[x5][y5] == 0 || fc->playboard[x2][y2] == 0))
		fc->tlivef -= 1;	/* 六缺一型, 不算冲四 */

	if (n_black >= 6)
		bandhand(fc, 6);
	return;
}

static void
bandhand(struct fivechess *fc, int style)
{
	if (style == 3) {
		move(12, 35);
		outs("\033[47m\033[31m┌黑败——————┐\033[m");
		move(13, 35);
		outs("\033[47m\033[31m│  \033[37;41m黑禁手双活三\033[m\033[47m  \033[31m│\033[m");
		move(14, 35);
		outs("\033[47m\033[31m└————————┘\033[m");
	} else if (style == 4) {
		move(12, 35);
		outs("\033[47m\033[31m┌黑败——————┐\033[m");
		move(13, 35);
		outs("\033[47m\033[31m│  \033[37;41m黑禁手双  四\033[m\033[47m  \033[31m│\033[m");
		move(14, 35);
		outs("\033[47m\033[31m└————————┘\033[m");
	} else {
		move(12, 35);
		outs("\033[47m\033[31m┌黑败——————┐\033[m");
		move(13, 35);
		outs("\033[47m\033[31m│  \033[37;41m黑禁手连六子\033[m\033[47m  \033[31m│\033[m");
		move(14, 35);
		outs("\033[47m\033[31m└————————┘\033[m");
	}

	fc->winner = white;
	press();
	return;
}

void
five_pk(fd, first)
int fd;
int first;
{
	struct fivechess fc;

	int quitf, cx, ch, cy, datac, fdone, x;
	char genbuf[100], data[90], xy_po[5], genbuf1[20];
	int i, j, fway, banf, idone;
	int player, px, py;
	int chess[250][2] = { {0, 0} };
	char abcd[15] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G',
		'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O'
	};
	char save_page_requestor[40];
/*
 *      增加聊天功能. Added by satan. 99.04.02
 */

#define START    17
#define END      21
#define PROMPT   23
#undef MAX
#define MAX      (END - START)
#define BSIZE    60

	char chatbuf[80], *cbuf;
	int ptr = 0, chating = 0;

	setutmpmode(FIVE);	/*用户状态设置 */
	clear();
	InitScreen();
	five_chat(NULL, 1);

	cbuf = chatbuf + 19;
	chatbuf[0] = '\0';
	chatbuf[79] = '\0';
	cbuf[0] = '\0';
	sprintf(chatbuf + 1, "%-16s: ", cuser.username);

	add_io(fd, 0);

begin:
	for (i = 0; i <= 14; i++)
		for (j = 0; j <= 14; j++)
			fc.playboard[i][j] = 0;

	fc.hand = 1;
	fc.winner = 0;
	quitf = 0;
	px = 14;
	py = 7;
	fway = 1;
	banf = 1;
	idone = 0;
	x = 0;

	sprintf(genbuf, "%s (%s)", cuser.userid, cuser.username);

	if (first) {
		move(1, 33);
		prints("黑●先手 %s  ", genbuf);
		move(2, 33);
		prints("白○后手 %s  ", save_page_requestor);
	} else {
		move(1, 33);
		prints("白○后手 %s  ", genbuf);
		move(2, 33);
		prints("黑●先手 %s  ", save_page_requestor);
	}

	move(15, 35);
	if (first)
		outs("★等待对方下子★");
	else
		outs("◆现在该自己下◆");
	move(7, 14);
	outs("●");
	player = white;
	fc.playboard[7][7] = black;
	chess[1][0] = 14;	/*纪录所下位址 */
	chess[1][1] = 7;
	move(4, 35);
	outs("第 1手 ●H 8");

	if (!first) {		/*超怪! */
		move(7, 14);
		fdone = 1;
	} else
		fdone = 0;	/*对手完成 */

	while (1) {
		ch = igetkey();

		if (ch == I_OTHERDATA) {
			datac = recv(fd, data, sizeof (data), 0);
			if (datac <= 0) {
				move(17, 30);
				outs("\033[47m\033[31;47m 对方投降了...@_@ \033[m");
				break;
			}
			if (data[0] == '\0') {
				data[((unsigned /* safe */) datac < sizeof(data)) ? (unsigned) datac : (sizeof(data) - 1)] = 0;
				five_chat(data + 1, 0);
				if (chating)
					move(PROMPT, ptr + 6);
				else
					move(py, px);
				continue;
			} else if (data[0] == '\1') {
				bell();
				RMSG = YEA;
				saveline(PROMPT, 0, NULL);
				sprintf(genbuf,
					"%s 说: 重来一盘好吗? (Y/N)[Y]:",
					save_page_requestor);
				getdata(PROMPT, 0, genbuf, genbuf1, 2, LCECHO,
					YEA);
				RMSG = NA;
				if (genbuf1[0] == 'n' || genbuf1[0] == 'N') {
					saveline(PROMPT, 1, NULL);
					send(fd, "\3", 1, 0);
					continue;
				} else {
					saveline(PROMPT, 1, NULL);
					InitScreen();
					first = 0;
					send(fd, "\2", 1, 0);
					goto begin;
				}
			} else if (data[0] == '\2') {
				bell();
				saveline(PROMPT, 0, NULL);
				move(PROMPT, 0);
				clrtoeol();
				prints("%s 接受了你的请求 :-)", save_page_requestor);
				refresh();
				sleep(1);
				saveline(PROMPT, 1, NULL);
				InitScreen();
				first = 1;
				goto begin;
			} else if (data[0] == '\3') {
				bell();
				saveline(PROMPT, 0, NULL);
				move(PROMPT, 0);
				clrtoeol();
				prints("%s 拒绝了你的请求 :-(", save_page_requestor);
				refresh();
				sleep(1);
				saveline(PROMPT, 1, NULL);
				if (chating)
					move(PROMPT, ptr + 6);
				else
					move(py, px);
				continue;
			} else if (data[0] == '\xff') {
				move(PROMPT, 0);
				quit();
				break;
			}
			i = atoi(data);
			cx = i / 1000;	/*解译data成棋盘资料 */
			cy = (i % 1000) / 10;
			fdone = i % 10;
			fc.hand += 1;

			if (fc.hand % 2 == 0)
				move(((fc.hand - 1) % 20) / 2 + 4, 48);
			else
				move(((fc.hand - 1) % 19) / 2 + 4, 35);

			prints("第%2d手 %s%c%2d", fc.hand,
					(player == black) ? "●" : "○", abcd[cx / 2],
					15 - cy);

			move(cy, cx);
			x = cx / 2;
			fc.playboard[x][cy] = player;
			if (player == black) {
				outs("●");
				player = white;
			} else {
				outs("○");
				player = black;
			}
			move(cy, cx);
			bell();
			move(15, 35);
			outs("◆现在该自己下◆");
			haha(5);

			fc.tdeadf = fc.tlivef = fc.livethree = fc.threefour = 0;
			for (j = 0; j <= 10; j++)
				calvalue(&fc, cx / 2, j, cx / 2, j + 1, cx / 2,
						j + 2, cx / 2, j + 3, cx / 2, j + 4);
			for (i = 0; i <= 10; i++)	/*横向 */
				calvalue(&fc, i, cy, i + 1, cy, i + 2, cy,
						i + 3, cy, i + 4, cy);
			for (i = -4; i <= 0; i++)	/*斜右下 */
				calvalue(&fc, cx / 2 + i, cy + i,
						cx / 2 + i + 1, cy + i + 1,
						cx / 2 + i + 2, cy + i + 2,
						cx / 2 + i + 3, cy + i + 3,
						cx / 2 + i + 4, cy + i + 4);
			for (i = -4; i <= 0; i++)	/*斜左下 */
				calvalue(&fc, cx / 2 - i, cy + i,
						cx / 2 - i - 1, cy + i + 1,
						cx / 2 - i - 2, cy + i + 2,
						cx / 2 - i - 3, cy + i + 3,
						cx / 2 - i - 4, cy + i + 4);

			for (j = 0; j <= 9; j++)
				callfour(&fc, cx / 2, j, cx / 2, j + 1, cx / 2,
						j + 2, cx / 2, j + 3, cx / 2, j + 4,
						cx / 2, j + 5);
			for (i = 0; i <= 9; i++)	/*四横向 */
				callfour(&fc, i, cy, i + 1, cy, i + 2, cy,
						i + 3, cy, i + 4, cy, i + 5, cy);
			for (i = -5; i <= 0; i++) {	/*四斜右下 */
				callfour(&fc, cx / 2 + i, cy + i,
						cx / 2 + i + 1, cy + i + 1,
						cx / 2 + i + 2, cy + i + 2,
						cx / 2 + i + 3, cy + i + 3,
						cx / 2 + i + 4, cy + i + 4,
						cx / 2 + i + 5, cy + i + 5);
				/*四斜左下 */
				callfour(&fc, cx / 2 - i, cy + i,
						cx / 2 - i - 1, cy + i + 1,
						cx / 2 - i - 2, cy + i + 2,
						cx / 2 - i - 3, cy + i + 3,
						cx / 2 - i - 4, cy + i + 4,
						cx / 2 - i - 5, cy + i + 5);
			}

			py = cy;
			px = cx;
			if (fc.tlivef >= 2 && fc.winner == 0)
				bandhand(&fc, 4);
			if (fc.livethree >= 2 && fc.tlivef == 0)
				bandhand(&fc, 3);
			if (fc.threefour == black)
				haha(1);
			else if (fc.threefour == white)
				haha(1);
			if (chating) {
				sleep(1);
				move(PROMPT, ptr + 6);
			} else
				move(py, px);
			if (fc.winner) {
				InitScreen();
				goto begin;
			}
		} else {
			if (ch == Ctrl('X')) {
				quitf = 1;
			} else if (ch == Ctrl('C') || ((ch == 'Q' || ch == 'q') && !chating)) {
				RMSG = YEA;
				saveline(PROMPT, 0, NULL);
				getdata(PROMPT, 0, "您确定要离开吗? (Y/N)?[N] ",
					genbuf1, 2, LCECHO, YEA);
				if (genbuf1[0] == 'Y' || genbuf1[0] == 'y')
					quitf = 1;
				else
					quitf = 0;
				saveline(PROMPT, 1, NULL);
				RMSG = NA;
			} else if (ch == Ctrl('N') || ((ch == 'N' || ch == 'n') && !chating)) {
				saveline(PROMPT, 0, NULL);
				RMSG = YEA;
				getdata(PROMPT, 0,
					"您确定要重新开始吗? (Y/N)?[N] ",
					genbuf1, 2, LCECHO, YEA);
				if (genbuf1[0] == 'Y' || genbuf1[0] == 'y') {
					send(fd, "\1", 1, 0);
					move(PROMPT, 0);
					bell();
					clrtoeol();
					move(PROMPT, 0);
					outs("已经已经替您发出请求了");
					refresh();
					sleep(1);
				}
				RMSG = NA;
				saveline(PROMPT, 1, NULL);
				if (chating)
					move(PROMPT, ptr + 6);
				else
					move(py, px);
				continue;
			} else if (ch == '\t') {
				if (chating) {
					chating = 0;
					move(py, px);
				} else {
					chating = 1;
					move(PROMPT, 6 + ptr);
				}
				continue;
			} else if (ch == '\0')
				continue;
			else if (chating) {
				if (ch == '\n' || ch == '\r') {
					if (!cbuf[0])
						continue;
					ptr = 0;
					five_chat(chatbuf + 1, 0);
					send(fd, chatbuf, strlen(chatbuf + 1) + 2, 0);
					cbuf[0] = '\0';
					move(PROMPT, 6);
					clrtoeol();
				} else if (ch == KEY_LEFT) {
					if (ptr)
						ptr--;
				} else if (ch == KEY_RIGHT) {
					if (cbuf[ptr])
						ptr++;
				} else if (ch == Ctrl('H') || ch == '\177') {
					if (ptr) {
						ptr--;
						memcpy(&cbuf[ptr], &cbuf[ptr + 1], BSIZE - ptr);
						move(PROMPT, ptr + 6);
						clrtoeol();
						prints(&cbuf[ptr]);
					}
				} else if (ch == KEY_DEL) {
					if (cbuf[ptr]) {
						memcpy(&cbuf[ptr], &cbuf[ptr + 1], BSIZE - ptr);
						clrtoeol();
						prints(&cbuf[ptr]);
					}
				} else if (ch == Ctrl('A')) {
					ptr = 0;
				} else if (ch == Ctrl('E')) {
					while (cbuf[++ptr]) ;
				} else if (ch == Ctrl('K')) {
					ptr = 0;
					cbuf[ptr] = '\0';
					move(PROMPT, ptr + 6);
					clrtoeol();
				} else if (ch == Ctrl('U')) {
					memmove(cbuf, &cbuf[ptr],
						BSIZE - ptr + 1);
					ptr = 0;
					move(PROMPT, ptr + 6);
					clrtoeol();
					prints(cbuf);
				} else if (ch == Ctrl('W')) {
					if (ptr) {
						int optr;

						optr = ptr;
						ptr--;
						do {
							if (cbuf[ptr] != ' ')
								break;
						} while (--ptr);
						do {
							if (cbuf[ptr] == ' ') {
								if (cbuf[ptr + 1] != ' ')
									ptr++;
								break;
							}
						} while (--ptr);
						memcpy(&cbuf[ptr], &cbuf[optr], BSIZE - optr + 1);
						move(PROMPT, ptr + 6);
						clrtoeol();
						prints(&cbuf[ptr]);
					}
				} else if (isprint2(ch)) {
					if (ptr == BSIZE)
						continue;
					if (!cbuf[ptr]) {
						cbuf[ptr] = ch;
						move(PROMPT, 6 + ptr);
						outc(ch);
						cbuf[++ptr] = 0;
					} else {
						memmove(&cbuf[ptr + 1], &cbuf[ptr], BSIZE - ptr + 1);
						cbuf[ptr] = ch;
						move(PROMPT, 6 + ptr);
						prints(&cbuf[ptr]);
						ptr++;
					}
				}
				move(PROMPT, 6 + ptr);
				continue;
			}
		}

		if (fdone == 1 && !chating && ch != I_OTHERDATA) {	/*换我 */

			move(py, px);
			switch (ch) {

			case KEY_DOWN:
			case 'j':
			case 'J':
				py = py + 1;
				if (py > 14)
					py = 0;
				break;

			case KEY_UP:
			case 'k':
			case 'K':
				py = py - 1;
				if (py < 0)
					py = 14;
				break;

			case KEY_LEFT:
			case 'h':
			case 'H':
				px = px - 1;
				if (px < 0)
					px = 28;
				break;

			case KEY_RIGHT:
			case 'l':
			case 'L':
				px = px + 1;
				if (px > 28) {
					px = 0;
					px = px - 1;
				}	/*会跳格咧 */
				break;
			case ' ':
				if (banf == 1)
					break;

				if ((px % 2) == 1)
					px = px - 1;	/*解决netterm不合问题 */
				move(py, px);
				fc.hand += 1;
				fc.playboard[x][py] = player;
				if (player == black) {
					outs("●");
					player = white;
				} else {
					outs("○");
					player = black;
				}
				chess[fc.hand][0] = px;
				chess[fc.hand][1] = py;
				if (fc.hand % 2 == 0)
					move(((fc.hand - 1) % 20) / 2 + 4, 48);
				else
					move(((fc.hand - 1) % 19) / 2 + 4, 35);

				prints("第%2d手 %s%c%2d", fc.hand,
						(fc.hand % 2 == 1) ? "●" : "○",
						abcd[px / 2], 15 - py);
				idone = 1;
				move(py, px);
				break;
			default:
				break;
			}
			move(py, px);
			x = px / 2;
			if (fc.playboard[x][py] != 0)
				banf = 1;
			else
				banf = 0;

			if (idone == 1) {
				xy_po[0] = px / 10 + '0';
				xy_po[1] = px % 10 + '0';
				xy_po[2] = py / 10 + '0';
				xy_po[3] = py % 10 + '0';
				fdone = 0;
				xy_po[4] = '1';
				if (send(fd, xy_po, sizeof (xy_po), 0) == -1)
					break;

				move(15, 35);
				outs("★等待对方下子★");
				haha(5);

				fc.tdeadf = fc.tlivef = fc.livethree = fc.threefour = 0;
				for (j = 0; j <= 10; j++)
					calvalue(&fc, px / 2, j, px / 2, j + 1,
							px / 2, j + 2, px / 2, j + 3,
							px / 2, j + 4);
				for (i = 0; i <= 10; i++)	/*横向 */
					calvalue(&fc, i, py, i + 1, py, i + 2,
							py, i + 3, py, i + 4, py);
				for (i = -4; i <= 0; i++)	/*斜右下 */
					calvalue(&fc, px / 2 + i, py + i,
							px / 2 + i + 1, py + i + 1,
							px / 2 + i + 2, py + i + 2,
							px / 2 + i + 3, py + i + 3,
							px / 2 + i + 4, py + i + 4);
				for (i = -4; i <= 0; i++)	/*斜左下 */
					calvalue(&fc, px / 2 - i, py + i,
							px / 2 - i - 1, py + i + 1,
							px / 2 - i - 2, py + i + 2,
							px / 2 - i - 3, py + i + 3,
							px / 2 - i - 4, py + i + 4);

				for (j = 0; j <= 9; j++)
					callfour(&fc, px / 2, j, px / 2, j + 1,
							px / 2, j + 2, px / 2, j + 3,
							px / 2, j + 4, px / 2, j + 5);
				for (i = 0; i <= 9; i++)	/*四横向 */
					callfour(&fc, i, py, i + 1, py, i + 2,
							py, i + 3, py, i + 4, py,
							i + 5, py);
				for (i = -5; i <= 0; i++) {	/*四斜右下 */
					callfour(&fc, px / 2 + i, py + i,
							px / 2 + i + 1, py + i + 1,
							px / 2 + i + 2, py + i + 2,
							px / 2 + i + 3, py + i + 3,
							px / 2 + i + 4, py + i + 4,
							px / 2 + i + 5, py + i + 5);
					/*四斜左下 */
					callfour(&fc, px / 2 - i, py + i,
							px / 2 - i - 1, py + i + 1,
							px / 2 - i - 2, py + i + 2,
							px / 2 - i - 3, py + i + 3,
							px / 2 - i - 4, py + i + 4,
							px / 2 - i - 5, py + i + 5);
				}

				if (fc.tlivef >= 2 && fc.winner == 0)
					bandhand(&fc, 4);
				if (fc.livethree >= 2 && fc.tlivef == 0)
					bandhand(&fc, 3);
				if (fc.threefour == black)
					haha(1);
				else if (fc.threefour == white)
					haha(1);

			}
			idone = 0;
		}
		if (quitf) {
			genbuf1[0] = '\xff';
			send(fd, genbuf1, 1, 0);
			press();
			break;
		}
		if (fc.winner) {
			InitScreen();
			goto begin;
		}
	}

	add_io(0, 0);
	close(fd);
	return;
}

static void
five_chat(char *msg, int init)
{
	char prompt[] = "===>";
	char chat[] = "聊天: ";
	static char win[MAX][80];
	static int curr, p, i;

	if (init) {
		for (i = 0; i < MAX; i++)
			win[i][0] = '\0';
		curr = START;
		p = 0;
		move(START - 1, 0);
		for (i = 0; i < 80; i++)
			outc('-');
		move(END + 1, 0);
		for (i = 0; i < 80; i++)
			outc('-');
		move(curr, 0);
		clrtoeol();
		prints(prompt);
		move(PROMPT, 0);
		prints(chat);
		return;
	}

	if (msg) {
		ytht_strsncpy(win[p], msg, sizeof(win[p]));
		move(curr, 0);
		clrtoeol();
		prints(win[p]);
		p++;
		if (p == MAX)
			p = 0;
		curr++;
		if (curr > END) {
			for (i = START; i < END; i++) {
				move(i, 0);
				clrtoeol();
				prints(win[(p + MAX + (i - START)) % MAX]);
			}
			curr = END;
		}
		move(curr, 0);
		clrtoeol();
		prints(prompt);
	}
}

static void
press(void)
{
	int c;
	extern int showansi;
	int tmpansi;

	tmpansi = showansi;
	showansi = 1;
	saveline(t_lines - 1, 0, NULL);
	move(t_lines - 1, 0);
	clrtoeol();
	prints("\033[37;40m\033[0m                               \033[33m按任意键继续 ...\033[37;40m\033[0m");
	c = egetch();
	move(t_lines - 1, 0);
	saveline(t_lines - 1, 1, NULL);
	showansi = tmpansi;
}
