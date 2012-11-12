/*-------------------------------------------------------*/
/* sec_hand.c   ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : ernie.bbs@bbs.ee.nthu.edu.tw                 */
/* target : A second hand market                         */
/* create : 00/04/03                                     */
/* update :                                              */
/*-------------------------------------------------------*/
#include "bbs.h"
#include "sec_hand.h"

#undef HAVE_RESTRICTION		// restrict posting according to currentuser.vmail
#ifdef  HAVE_RESTRICTION
#define MSG_RES  "对不起，本功能只限北大同学使用，您的ip地址不在其内"
#endif
#define MSG_2ND  "\033[0;34;46m 跳蚤市场 \033[31;47m (Q)\033[30m离开  \033[31m(H)\033[30m操作说明  \033[0m"

#define SPACE  "                                                  "
SDL *sdl_head;
SDL *top, *bot;
SDL *curr_slot, *curr_grp, *last_grp, *curr_item;
char f_size;

extern char quote_file[];
//------------------------------------------------------------------------
#define NEXT   1
#define PREV   0
static SDL *
ptr_move(x, dir, n)
SDL *x;
int dir, n;
{
	int i = 0;
	switch (dir) {
	case NEXT:
		for (i = 1; i <= n; i++) {
			if (x->next)
				x = x->next;
			else
				return x;
		}
		break;
	case PREV:
		for (i = 1; i <= n; i++) {
			if (x->prev)
				x = x->prev;
			else
				return x;
		}
		break;
	}
	return x;
}

static SDL *
seek_curr_grp()
{
	SDL *x;
	if (curr_slot->slot.prop & PROP_IS_GROUP)
		return curr_slot;
	x = curr_slot;
	while (x->prev && (x->prev)->slot.prop & PROP_IS_ITEM)
		x = x->prev;
	return x->prev;
}

static void
set_frame(x)
SDL *x;
{
	bot = top = x;
	bot = ptr_move(bot, NEXT, f_size - 1);
	return;
}

static void
clrhalf()
{
	move(f_size + 2, 0);
	clrtobot();
	outs(MSG_2ND);
	return;
}

static void
draw_frame()
{
	clear();
	outs(" \033[0;36m─\033[1;33;46m STAR跳蚤市场 \033[0;36m────────\
──────────────────────\033[0m");
	move(f_size + 1, 0);
	outs(" \033[0;36m────────────────────────────\
──────────\033[0m");
	return;
}

static void
more_it(title, fname)
char *title, *fname;
{
	int i, j;
	char buf[256];
	FILE *fp;
	clrhalf();
	move(f_size + 3, 0);
	clrtobot();
	prints("  \033[1;37;46m %s \033[0m\n\n", title);
	if ((fp = fopen(fname, "r"))) {
		i = 0;
		j = t_lines - f_size - 4;
		while (fgets(buf, sizeof (buf), fp) && (++i <= j))
			outs(buf);
		fclose(fp);
	}
	curr_item = NULL;
	return;
}

static void
do_draw(x, row, is_bar)
SDL *x;
int row, is_bar;
{
	SLOT *s;
	char buf[256];
	move(row, 0);
	clrtoeol();
	s = &(x->slot);
	if (s->prop & PROP_IS_GROUP) {
		sprintf(buf, "%s%s %s (%d) \033[0m",
			s->reply ?
			((s->prop & PROP_G_OPEN) ? "[-]" : "[+]") : "   ",
			is_bar ? "\033[1;37;41m>" : " ", s->title, s->reply);
	} else {
		sprintf(buf, "    %s%s%s (%d) %s%s",
			x->next ?
			(((x->next)->slot.prop & PROP_IS_ITEM) ? "├" : "└")
			: ("└"),
			is_bar ? "\033[1;37;41m>" : "\033[0;37;41m ",
			s->date, s->reply, s->title, SPACE);
		sprintf(&buf[60], " %12s \033[0m", s->userid);
	}
	outs(buf);
	clrtoeol();
	return;
}

#define BAR_YES   1
#define BAR_NO   0
#define DRAW_BAR_UP  -1
#define DRAW_ALL  0
#define DRAW_BAR_DOWN  1
#define DRAW_TO_BOT  2
static void
draw(where, how)
SDL *where;
int how;
{
	int row, flag;
	SDL *x;
	if (!f_size)
		return;
	x = top;
	row = 1;
	flag = 0;
	while (x) {
		if (x == curr_slot) {
			do_draw(x, row, BAR_YES);
			flag = 1;
		} else if ((how == DRAW_ALL) ||
			   ((how == DRAW_TO_BOT) && (flag == 1)) ||
			   ((how == DRAW_BAR_UP)
			    && (x == curr_slot->next))
			   || ((how == DRAW_BAR_DOWN)
			       && (x == curr_slot->prev))) {
			do_draw(x, row, BAR_NO);
		}
		row++;
		if (x == bot)
			break;
		else
			x = x->next;
	}
	while (row <= f_size) {
		move(row++, 0);
		clrtoeol();
	}
	return;
}

static int
sdl_add(fd, where)
int fd;
SDL **where;
{
	SDL *x;
	x = (SDL *) malloc(sizeof (SDL));
	if (read(fd, &(x->slot), sizeof (SLOT)) != sizeof (SLOT)) {
		free(x);
		return 0;
	}
	if (x->slot.prop & PROP_IS_CANCEL) {
		free(x);
		return 1;
	}
	x->prev = x->next = NULL;
	if (*where) {
		SDL *tmp;
		tmp = (*where)->next;
		(*where)->next = x;
		x->prev = *where;
		if (tmp) {
			tmp->prev = x;
			x->next = tmp;
		}
	} else {
		sdl_head = x;
	}
	*where = x;
	return 1;
}

static void
read_in_rec(fname, where)
char *fname;
SDL *where;
{
	int fd;
	SDL *x;
	fd = open(fname, O_RDONLY);
	if (fd > 0) {
		x = where;
		while (sdl_add(fd, &x)) ;
		close(fd);
	}
	return;
}

static void
open_grp(grp)
SDL *grp;
{
	char fname[80];
	if ((grp->slot.prop & PROP_G_OPEN) || (grp->slot.prop & PROP_IS_CANCEL))
		return;
	sprintf(fname, "2nd/%s/%s", grp->slot.fn, FN_ITEM);
	read_in_rec(fname, grp);
	grp->slot.prop |= PROP_G_OPEN;
	return;
}

static SDL *
close_grp(grp)
SDL *grp;
{
	SDL *this, *next;
	if (!(grp->slot.prop & PROP_G_OPEN))
		return grp;
	this = grp->next;
	while (this && (this->slot.prop & PROP_IS_ITEM)) {
		next = this->next;
		free(this);
		this = next;
	}
	grp->next = this;
	if (this)		// "this" could be NULL,
		this->prev = grp;	// while grp is the last group
	grp->slot.prop ^= PROP_G_OPEN;
	return grp;
}

static void
sdl_new()			// modified from mail.c  :poup--------------------------------------------
{
	SDL *list, *next;
	list = sdl_head;
	while (list) {
		next = list->next;
		free(list);
		list = next;
	}
	sdl_head = NULL;
}

static void
func_i(a, b)
SLOT *a, *b;
{
	a->prop |= PROP_I_CANCEL;
	sprintf(a->title, "<< 本布告经 %s 删除 >>", currentuser.userid);
	b->prop |= PROP_I_CANCEL;
	sprintf(b->title, "<< 本布告经 %s 删除 >>", currentuser.userid);
	return;
}

static void
func_g(a, b)
SLOT *a, *b;
{
	a->prop |= PROP_G_CANCEL;
	sprintf(a->title, "<< 本群组经 %s 删除 >>", currentuser.userid);
	b->prop |= PROP_G_CANCEL;
	sprintf(b->title, "<< 本群组经 %s 删除 >>", currentuser.userid);
	return;
}

static void
func_inc(a, b)
SLOT *a, *b;
{
	a->reply++, b->reply++;
	return;
}

static void
func_dec(a, b)
SLOT *a, *b;
{
	a->reply--, b->reply--;
	return;
}

static int
slot_change(fpath, s, func_ptr)
char *fpath;
SLOT *s;
void *
func_ptr();
{
	int fd, pos;
	SLOT slot;
	if ((fd = open(fpath, O_RDONLY)) < 0)
		return 0;
	pos = 1;
	while (read(fd, &slot, sizeof (SLOT)) == sizeof (SLOT)) {
		if (s->chrono == slot.chrono)	// hope there's no 2 same chronos
		{		// of groups
			func_ptr(s, &slot);
			close(fd);
			substitute_record(fpath, &slot, sizeof (SLOT), pos);
			return 1;
			break;
		}
		pos++;
	}
	close(fd);
	return 0;
}

static int
sdl_del()
{
	char fpath[80];
	SDL *x;
	if (curr_slot->slot.prop & PROP_IS_CANCEL)
		return 0;
	if ((!(currentuser.userlevel & PERM_SYSOP)) &&
	    strcmp(curr_slot->slot.userid, currentuser.userid))
		return 0;
	sprintf(fpath, " 确定删除：%s？[N] ", curr_slot->slot.title);
	if (askyn(fpath, NA, YEA) == NA)
		return 0;
	x = curr_slot;
	if (x->slot.prop & PROP_IS_ITEM) {
		sprintf(fpath, "2nd/%s/%s", curr_grp->slot.fn, FN_ITEM);
		slot_change(fpath, &x->slot, func_i);
		slot_change("2nd/" FN_GRP, &curr_grp->slot, func_dec);
	} else if (x->slot.prop & PROP_IS_GROUP) {
		if (askyn
		    (" 您将删除的是整个分类群组，真的要继续？[N] ", NA,
		     YEA) == NA)
			return 0;
		sprintf(genbuf, "删除二手市场分类 %s", x->slot.title);
		securityreport(genbuf, genbuf);
		sprintf(fpath, "2nd/%s", FN_GRP);
		slot_change(fpath, &x->slot, func_g);
		close_grp(x);
	}
	return 1;
}

static void
short_stamp(str, chrono)
char *str;
time_t *chrono;
{
	struct tm *ptime;
	ptime = localtime(chrono);
	sprintf(str, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
}

static void
more_item()
{
	int i, j;
	char fname[80], buf[256];
	SLOT *s;
	FILE *fp;
	if (!curr_item)
		return;
	s = &(curr_item->slot);
	sprintf(fname, "2nd/%s/%s", curr_grp->slot.fn, s->fn);
	sprintf(buf, "  \033[1;37;46m %s \033[0m %s  %s\n\n",
		curr_grp->slot.title, s->title,
		(s->prop & PROP_I_WANT) ? "\033[1;5;31m(WANTED)\033[0m" : "");
	clrhalf();
	move(f_size + 3, 0);
	outs(buf);
	if ((fp = fopen(fname, "r"))) {
		i = 0;
		j = t_lines - f_size - 7;
		while (fgets(buf, sizeof (buf), fp) && ++i <= j)
			outs(buf);
		fclose(fp);
	}
	move(t_lines - 4, 0);
	sprintf(buf, "  ◆ 希望价格    ：%s\n", s->price);
	outs(buf);
	sprintf(buf, "  ◆ 请回信给我  ：%s\n", s->userid);
	outs(buf);
	sprintf(buf, "  ◆ 其他联络方式：%s\n", s->contact);
	outs(buf);
	return;
}

#ifdef HAVE_RESTRICTION
static int
can_post()
{
	int i;
	char *table[3] = { "162.105", "166.111", NULL };
	i = 0;
	while (table[i]) {
		if (str_str(currentuser.lasthost, table[i]))
			return 1;
		i++;
	}
	return 0;
}
#endif
static int
new_item()
{
	struct stat xx;
	int fp, count = 0;
	char ans[3];
	char log[1024];
	// return -1: cancel posting, redraw frame
	//         0: do nothing, frame remains untouched
	//         1: creat a new group, redraw frame
	//         2: creat a new item, redraw frame
	if (currentuser.userlevel & PERM_SYSOP)
		getdata(t_lines - 1, 0,
			" 我要 (S)卖东西 (W)徵求东西 (N)新增群组 (Q)取消？[Q] ",
			ans, 2, DOECHO, YEA);
#ifdef HAVE_RESTRICTION
	else if (can_post())
		getdata(t_lines - 1, 0,
			" 我要 (S)卖东西 (W)徵求东西 (Q)取消？[Q] ", ans,
			2, DOECHO, YEA);
	else {
		prints(MSG_RES);
		return 0;
	}
#else
	else
		getdata(t_lines - 1, 0,
			" 我要 (S)卖东西 (W)徵求东西 (Q)取消？[Q] ", ans,
			2, DOECHO, YEA);
#endif
	if (ans[0] == 'n' || ans[0] == 'N') {
		// a new group will appear at the next
		// time entering second hand market
		SLOT grp;
		if (!(currentuser.userlevel & PERM_SYSOP))
			return 0;
		clrhalf();
		move(f_size + 3, 4);
		outs("\033[1;33m◆ 新增群组\033[0m");
		do {
			getdata(f_size + 5, 0, "    群组的英文目录名称：",
				grp.fn, 9, DOECHO, YEA);
			sprintf(log, "2nd/%s", grp.fn);
		} while (stat(log, &xx) == 0);	// loop when dir exists
		getdata(f_size + 7, 0, "    群组标题：", grp.title,
			sizeof (grp.title), DOECHO, YEA);
		getdata(f_size + 9, 0, "    请您确定？[N] ", ans, 2,
			DOECHO, YEA);
		if (ans[0] == 'y' || ans[0] == 'Y') {
			char dir[80];
			grp.chrono = time(NULL);
			grp.prop = PROP_G_GROUP;
			grp.reply = 0;	// num of items contain in this grp
			strcpy(grp.userid, currentuser.userid);
			short_stamp(grp.date, &grp.chrono);
			append_record("2nd/" FN_GRP, &grp, sizeof (SLOT));
			sprintf(dir, "2nd/%s", grp.fn);
			mkdir(dir, 0770);
			return 1;
		}
	} else if (ans[0] == 'w' || ans[0] == 'w' || ans[0] == 's'
		   || ans[0] == 'S') {
		// a new item will appear at the
		// next time opening the group
		SLOT item;
		SDL *x;
		char title[30], fpath[80], folder[80];
		time_t now;
		if (!sdl_head) {
			prints(" 尚未建立群组，请先建立分类群组");
			return 0;
		}
		if (curr_grp->slot.prop & PROP_G_CANCEL)
			return 0;
//    prints(" 记得先把光棒移动至你想要的分类群组! ^_^");
//    egetch();
		sprintf(fpath, "在「%s」贴布告，标题：", curr_grp->slot.title);
		sprintf(folder, "2nd/%s/" FN_ITEM, curr_grp->slot.fn);
		if (!getdata
		    (t_lines - 1, 0, fpath, title, sizeof (title), DOECHO, YEA))
			return 0;
		now = time(NULL);
		sprintf(fpath, "2nd/%s/M.%ld.A", curr_grp->slot.fn,
			(long int) now);
		while ((fp = open(fpath, O_CREAT | O_EXCL | O_WRONLY, 0660)) ==
		       -1) {
			now++;
			sprintf(fpath, "2nd/%s/M.%ld.A", curr_grp->slot.fn,
				(long int) now);
			if (count++ > MAX_POSTRETRY) {
				return 0;
			}
		}
		close(fp);
		if (vedit(fpath, NA, NA) < 0) {
			unlink(fpath);
			prints("取消");
			return -1;
		}
		memset(&item, 0, sizeof (item));
		item.chrono = now;
		sprintf(item.fn, "M.%ld.A", (long int) now);
		short_stamp(item.date, &(item.chrono));
		getdata(3, 0, " 您所希望的价格？ ", item.price,
			sizeof (item.price), DOECHO, YEA);
		getdata(5, 0, " 除回信外的其他联络方式？ ", item.contact,
			sizeof (item.contact), DOECHO, YEA);
		if (ans[0] == 'w' || ans[0] == 'W')
			item.prop = PROP_I_WANT;
		else
			item.prop = PROP_I_SELL;
		item.reply = 0;
		strcpy(item.userid, currentuser.userid);
		strcpy(item.title, title);
		insert_record(folder, &item, sizeof (SLOT), 0, 1);	// the newer the upper
		slot_change("2nd/" FN_GRP, &curr_grp->slot, func_inc);
		x = curr_grp;
		close_grp(last_grp);
		open_grp(curr_grp);
		last_grp = curr_grp;
		curr_item = curr_slot = curr_grp->next;
		top = ptr_move(curr_slot, PREV, 3);
		set_frame(top);
		return 2;
	}
	return 0;
}

static void
reply_item()
{
	struct fileheader hdr;
	SLOT *s;
	if (!curr_item || (curr_item->slot.prop & PROP_I_CANCEL))
		return;
#ifdef HAVE_RESTRICTION
	if (!can_post()) {
		prints(MSG_RES);
		return;
	}
#endif
	if (askyn(" 回应给作者？[N] ", NA, YEA) == NA)
		return;
	s = &(curr_item->slot);
	sprintf(quote_file, "2nd/%s/%s", curr_grp->slot.fn, s->fn);
	memset(&hdr, 0, sizeof (struct fileheader));
	strcpy(hdr.owner, s->userid);
	strcpy(hdr.title, s->title);
	mail_reply(-1, &hdr, "");
	if (1) {
		sprintf(genbuf, "2nd/%s/%s", curr_grp->slot.fn, FN_ITEM);
		slot_change(genbuf, s, func_inc);
	}
	*quote_file = '\0';
	draw_frame();
	draw(top, DRAW_ALL);
	more_item();
	return;
}

static void
bar_up()
{
	if (curr_slot->prev) {
		SDL *prev = curr_slot->prev;
		curr_slot = prev;
		if ((curr_slot->next)->slot.prop & PROP_IS_GROUP) {
			if (curr_slot->slot.prop & PROP_IS_GROUP)
				curr_grp = prev;
			else
				curr_grp = seek_curr_grp();
		}
		curr_slot = prev;
		if (curr_slot == top->prev) {
			set_frame(top->prev);
			draw(top, DRAW_ALL);
		} else
			draw(curr_slot, DRAW_BAR_UP);
	}
}
static void
bar_down()
{
	if (curr_slot->next) {
		SDL *next = curr_slot->next;
		curr_slot = next;
		if (next->slot.prop & PROP_IS_GROUP)
			curr_grp = next;
		if (curr_slot == bot->next) {
			set_frame(top->next);
			draw(top, DRAW_ALL);
		} else
			draw(curr_slot, DRAW_BAR_DOWN);
	}
}
static void
bar_right()
{
	if (curr_slot->slot.prop & PROP_IS_CANCEL)
		return;
	if (curr_slot->slot.prop & PROP_IS_GROUP) {
		// here, curr_slot == curr_grp
		if (curr_grp->slot.prop & PROP_G_OPEN) {
			curr_slot = close_grp(curr_grp);
			last_grp = curr_grp;
			set_frame(top);
		} else if (curr_grp->slot.reply) {
			close_grp(last_grp);
			open_grp(curr_grp);
			last_grp = curr_grp;
			set_frame(curr_grp);
		}
		draw(top, DRAW_TO_BOT);
	} else if (curr_slot->slot.prop & PROP_IS_ITEM) {
		curr_item = curr_slot;
		more_item();
	}
}
static int
bar_left()
{
	if (curr_slot->slot.prop & PROP_IS_GROUP) {
		if (curr_grp->slot.prop & PROP_G_OPEN) {
			curr_slot = close_grp(curr_grp);
			last_grp = curr_grp;
			set_frame(top);
		} else
			return -1;
	} else {
		curr_grp = curr_slot = seek_curr_grp();
		set_frame(curr_slot);
	}
	draw(top, DRAW_ALL);
	return 0;
}

static void
bar_pgup()
{
	curr_slot = ptr_move(curr_slot, PREV, f_size - 1);
	if (curr_slot->slot.prop & PROP_IS_GROUP)
		curr_grp = curr_slot;
	curr_grp = seek_curr_grp();
	top = ptr_move(top, PREV, f_size - 1);
	set_frame(top);
	draw(top, DRAW_ALL);
}

static void
bar_pgdn()
{
	curr_slot = ptr_move(curr_slot, NEXT, f_size - 1);
	if (curr_slot->slot.prop & PROP_IS_GROUP)
		curr_grp = curr_slot;
	else
		curr_grp = seek_curr_grp();
	if (bot->next)
		set_frame(bot);
	draw(top, DRAW_ALL);
}

static void
bar_home()
{
	curr_grp = curr_slot = sdl_head;
	set_frame(sdl_head);
	draw(top, DRAW_ALL);
}

static void
bar_end()
{
	while (curr_slot->next)
		curr_slot = curr_slot->next;
	curr_grp = seek_curr_grp();
	top = ptr_move(curr_slot, PREV, 6);
	set_frame(top);
	draw(top, DRAW_ALL);
}

int
sec_hand()
{
	int key;
	modify_user_mode(M_2NDHAND);
	f_size = 7;
	sdl_new();
	draw_frame();
	more_it("公告事项", "etc/" FN_2ND_DECL);
	while (!sdl_head) {
		read_in_rec("2nd/" FN_GRP, sdl_head);
		if (!sdl_head) {
			if (new_item() == 1)
				continue;
			else
				return -1;
		}
	}
	curr_slot = curr_grp = sdl_head;
	last_grp = sdl_head;
	curr_item = NULL;
	set_frame(sdl_head);
	draw(top, DRAW_ALL);
	while (1) {
		key = egetch();
		switch (key) {
		case 'q':
		case 'e':
		case 'Q':
		case 'E':
			goto exit;
		case '+':
			(f_size <= 15) ? ++f_size : f_size;
			goto REFRESH;
		case '-':
			f_size ? --f_size : f_size;
		      REFRESH:
			set_frame(top);
			draw_frame();
			draw(top, DRAW_ALL);
			clrhalf();
			more_item();
			break;
			// keys of moving bar:
			//   1. set curr_slot first
			//   2. then set curr_grp
			//   3. set top and bot
			//   4. redraw screen
		case KEY_UP:
			if (f_size)
				bar_up();
			break;
		case KEY_DOWN:
			if (f_size)
				bar_down();
			break;
		case KEY_PGDN:
		case ' ':
			if (f_size)
				bar_pgdn();
			break;
		case KEY_PGUP:
			if (f_size)
				bar_pgup();
			break;
		case KEY_HOME:
		case '0':
			if (f_size)
				bar_home();
			break;
		case KEY_END:
		case '$':
			if (f_size)
				bar_end();
			break;
		case KEY_RIGHT:
		case 13:
			bar_right();
			break;
		case KEY_LEFT:
			if (bar_left())
				goto exit;
			break;
		case 'y':
		case 'r':
		case 'm':
		case 'Y':
		case 'R':
		case 'M':
			reply_item();
			break;
		case 'd':
		case 'D':
			if (sdl_del())
				draw(top, DRAW_ALL);
			break;
		case Ctrl('P'):
			if (new_item()) {
				draw_frame();
				draw(top, DRAW_ALL);
				more_item();
			}
			break;
		case 'h':
		case 'H':
			more_it("操作说明", "etc/" FN_2ND_HELP);
			break;
		case Ctrl('D'):
			more_it("公告事项", "etc/" FN_2ND_DECL);
			break;
		}
		move(t_lines - 1, 0);
		clrtoeol();
	}
      exit:
	modify_user_mode(MMENU);
	sdl_new();
	return 0;
}
