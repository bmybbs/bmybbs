/***************************************************************************
* add by macintosh 2006.3,  模板发文
***************************************************************************/

#include <sys/mman.h>
#include "bbs.h"
#include "tmpl.h"
#include "smth_screen.h"
#include "io.h"
#include "bbsinc.h"
#include "stuff.h"
#include "announce.h"
#include "more.h"
#include "main.h"
#include "edit.h"
#include "list.h"
#include "xyz.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

#define BBS_PAGESIZE    (t_lines - 4)

extern int page, range;
extern char IScurrBM;

static struct a_template template[MAX_TEMPLATE];
static size_t template_num = 0;
static size_t t_now = 0;

static int orig_tmpl_init(char * nboard, int mode, struct a_template temp_array[], size_t size);
static int orig_tmpl_free(struct a_template temp_array[], size_t temp_num);
static int orig_tmpl_save(struct a_template temp_array[], size_t temp_num, char *board);

static int tmpl_init(int mode);
static void tmpl_free(void);
static int tmpl_save(void);
static int tmpl_add(void);
static int content_add(void);
static int tmpl_show(void);
static int content_show(void);
static void tmpl_refresh(void);
static void content_refresh(void);
static int content_key(int key, size_t allnum, int pagenum);
static int tmpl_key(int key, int allnum, int pagenum);
static int tmpl_select(int star, int curr);
int m_template(void);
static void choose_tmpl_refresh(void);
static int choose_tmpl_key(int key, int allnum, int pagenum);
static int choose_tmpl_post(int star, int curr);
int choose_tmpl(void);
static int tmpl_edit(int allnum);

static int
orig_tmpl_init(char * nboard, int mode, struct a_template template_array[], size_t size){
/***********
* mode 0: 用户看，不显示有斑竹权限的
* mode 1: 斑竹管理
************/

	int fd;
	char tmpldir[STRLEN];
	struct s_template tmpl;
	struct s_content content;
	size_t templ_num;
	ssize_t size_read;

	setbfile(tmpldir, sizeof(tmpldir), nboard, TEMPLATE_DIR);

	memset(template_array, 0, size * sizeof(struct a_template));
	templ_num = 0;

	if( (fd = open( tmpldir, O_RDONLY ) ) == -1 ){
		return 0;
	}
	while( read(fd, &tmpl, sizeof( struct s_template )) == sizeof(struct s_template) ){
		if( tmpl.version > TMPL_NOW_VERSION ){
			close(fd);
			return -1;
		}
		if( mode == 0 && ( tmpl.flag & TMPL_BM_FLAG ) ) {
			lseek( fd, sizeof(struct s_content) * tmpl.content_num , SEEK_CUR );
			continue;
		}
		memset(&content, 0, sizeof(struct s_content));
		if ((size_read = read(fd, &content, sizeof(struct s_content))) < 0)
			break;

		if ((unsigned) size_read != sizeof(struct s_content))
			continue;

		template_array[templ_num].tmpl = malloc(sizeof(struct s_template));
		if(template_array[templ_num].tmpl == NULL ){
			break;
		}

		template_array[templ_num].cont = malloc(sizeof(struct s_content));
		if (template_array[templ_num].cont == NULL) {
			free(template_array[templ_num].tmpl);
			break;
		}

		memcpy(template_array[templ_num].tmpl, &tmpl, sizeof(struct s_template) );
		memcpy(template_array[templ_num].cont, &content, sizeof(struct s_content) );
		templ_num ++;
		if( templ_num >= MAX_TEMPLATE )
			break;
	}
	close(fd);

	return templ_num;
}

static int
orig_tmpl_free(struct a_template temp_array[], size_t temp_num){

	size_t i;
	for(i=0; i<temp_num; i++){
		if (temp_array[i].tmpl)
			free(temp_array[i].tmpl);
		if (temp_array[i].cont)
			free(temp_array[i].cont);
		memset(&temp_array[i], 0, sizeof(struct a_template));
	}
	return 0;
}

static int
orig_tmpl_save(struct a_template * ptemp, size_t temp_num, char *board){

	size_t i;
	FILE *fp;
	char tmpldir[STRLEN];

	setbfile(tmpldir, sizeof(tmpldir), board, TEMPLATE_DIR);
	if( (fp = fopen( tmpldir, "w") ) == NULL ){
		return -1;
	}
	for(i=0; i<temp_num; i++){
		if(ptemp[i].tmpl == NULL)
			continue;
		fwrite( ptemp[i].tmpl, sizeof(struct s_template), 1, fp );
		fwrite( ptemp[i].cont, sizeof(struct s_content), ptemp[i].tmpl->content_num, fp);
	}
	fclose(fp);

	return 0;
}

static int
tmpl_init(int mode){
	int newmode=0;
	int ret;

	if(mode==1 || HAS_PERM(PERM_BLEVELS, currentuser) || IScurrBM)
		newmode = 1;
	ret = orig_tmpl_init(currboard, newmode, template, MAX_TEMPLATE);
	if(ret >= 0)
		template_num = ret;
	return ret;
}

static void
tmpl_free(){
	orig_tmpl_free(template, template_num);
	template_num = 0;
}

static int
tmpl_save(){
	return orig_tmpl_save(template, template_num, currboard);
}

static int
tmpl_add(){
	char buf[60];
	struct s_template tmpl;

	if( template_num >= MAX_TEMPLATE )
		return -1;
	bzero(&tmpl, sizeof(struct s_template));

	clear();
	buf[0]='\0';
	getdata(t_lines - 1, 0, "模板标题: ", buf, 50, DOECHO, YEA);
	if( buf[0]=='\0' || buf[0]=='\n' ){
		return -1;
	}
	strncpy(tmpl.title, buf, 50);
	ytht_strsncpy(tmpl.authorid, currentuser.userid, sizeof(tmpl.authorid));
	tmpl.title[49] = '\0';

	template[template_num].tmpl = (struct s_template *) malloc( sizeof(struct s_template) );
	if (template[template_num].tmpl == NULL) {
		return -1;
	}
	memcpy(template[template_num].tmpl, &tmpl, sizeof(struct s_template) );
	template_num ++;

	tmpl_save();
	return 0;
}

static int
content_add(){
	struct s_content ct;
	char buf[60];

	if( template[t_now].tmpl->content_num >= MAX_CONTENT )
		return -1;

	bzero(&ct, sizeof(struct s_content));
	clear();
	buf[0]='\0';
	getdata(t_lines - 1, 0, "选项标题: ", buf, 50, DOECHO, YEA);
	if( buf[0]=='\0' || buf[0]=='\n' ){
		return -1;
	}
	strncpy(ct.text, buf, 50);
	ct.text[49]='\0';

	buf[0]='\0';
	getdata(t_lines - 1, 0, "选项长度: ", buf, 5, DOECHO, YEA);
	ct.length = atoi(buf);
	if(ct.length <= 0 || ct.length > MAX_CONTENT_LENGTH )
		return -1;

	template[t_now].cont = (struct s_content *) realloc( template[t_now].cont, sizeof(struct s_content) * (template[t_now].tmpl->content_num+1));
	memcpy( &( template[t_now].cont[template[t_now].tmpl->content_num] ), &ct, sizeof(struct s_content) );
	template[t_now].tmpl->content_num ++;

	tmpl_save();
	return 0;
}


static int
tmpl_show(){
	int i;
	setlistrange(template_num);
	if (range < 1)
		return -1;
	clrtoeol();
	for (i = 0; i < BBS_PAGESIZE && i + page < range; i++) {
		move(i+3, 0);
		prints(" %2d %s %-13s %-50s %3d", i+1, template[i].tmpl->flag & TMPL_BM_FLAG ? "\033[1;31mB\033[m":" ", template[i].tmpl->authorid, template[i].tmpl->title, template[i].tmpl->content_num);
	}
	clrtobot();
	update_endline();
	return 0;
}


static int
tmpl_edit(int allnum)
{
	char ans[4], filepath[STRLEN];
	int aborted;

	clear();
	move(1, 0);
	prints("编辑/删除模板正文");

	setbfile(filepath, sizeof(filepath), currboard, template[allnum].tmpl->filename);

	getdata(3, 0, "(E)编辑 (D)删除 (Q)取消? [E]: ", ans, 2, DOECHO, YEA);
	if (ans[0] == 'Q' || ans[0] == 'q')
		aborted = -1;
	else if (ans[0] == 'D' || ans[0] == 'd') {
		move(6, 0);
		if (askyn("真的要删除模板正文", NA, NA)) {
			unlink(filepath);
			template[allnum].tmpl->filename[0] = '\0';
			move(7, 0);
			prints("模板正文已经删除...\n");
			pressanykey();
			aborted = 1;
		} else
			aborted = -1;
	} else
		aborted = vedit(filepath, NA, YEA);
	if (aborted == -1)
		pressreturn();
	return 1;
}


static int
content_show()
{
	int i;
	clrtoeol();
	setlistrange(template[t_now].tmpl->content_num);
	if (range < 1)
		return -1;
	for (i = 0; i < BBS_PAGESIZE && i + page < range; i++) {
		move(i+3, 0);
		prints(" %2d     %-50s  %3d", i+1, template[t_now].cont[i].text, template[t_now].cont[i].length);
	}
	clrtobot();
	update_endline();
	return 0;
}

static void
tmpl_refresh(){
	//clear();
	docmdtitle("[版面模板设置]",
			"添加[\x1b[1;32ma\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m 改名[\033[1;32mt\033[0;37m] \033[1;33m查看\033[m 标题[\033[1;32mx\033[m] 正文[\033[1;32ms\033[m] \033[1;33m修改\033[m 标题[\033[1;32mi\033[0;37m] 正文[\033[1;32mf\033[0;37m]");
	move(2, 0);
	prints("\033[0;1;37;44m %4s %-13s %-50s %8s", "序号", "创建人","模板名称","问题个数");
	clrtoeol();
	update_endline();
}

static void
content_refresh(){
	//clear();
	docmdtitle("[版面模板问题设置]",
			"添加[\x1b[1;32ma\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m 修改问题名称[\033[1;32mt\033[0;37m] 修改回答长度[\033[1;32ml\033[0;37m] 改变问题次序[\033[1;32mm\033[0;37m]");
	move(2, 0);
	prints("\033[0;1;37;44m %4s     %-50s  %8s", "序号", "问题名称","回答长度");
	clrtoeol();
	update_endline();
}


static int content_key(int key, size_t allnum, int pagenum) {
	(void) pagenum;
	switch (key) {
	case 'M':
	case 'm':
		{
		char ans[5];
		int newm;
		getdata(t_lines-1, 0, "移动到新次序:", ans, 4, DOECHO, YEA);
		if( ans[0]=='\0' || ans[0]=='\n' || ans[0]=='\r' )
			return 1;
		newm=atoi(ans);

		if (newm <= 0)
			newm = 1;
		if (newm > range)
			newm = range;
		newm--;

		if( /* safe */ (size_t) newm > allnum){
			int i;
			struct s_content sc;
			memcpy(&sc, &template[t_now].cont[allnum], sizeof(struct s_content));
			for(i=allnum;i<=newm-1;i++)
				memcpy(& template[t_now].cont[i], & template[t_now].cont[i+1], sizeof(struct s_content));
			memcpy(&template[t_now].cont[newm], &sc, sizeof(struct s_content));
			tmpl_save();
			return 1;
		}else if( /* safe */ (size_t) newm < allnum){
			size_t i;
			struct s_content sc;
			memcpy(&sc, &template[t_now].cont[allnum], sizeof(struct s_content));
			for(i=allnum; i>= ((size_t)newm) + 1 /* safe */ ;i--)
				memcpy(& template[t_now].cont[i], & template[t_now].cont[i-1], sizeof(struct s_content));
			memcpy(&template[t_now].cont[newm], &sc, sizeof(struct s_content));
			tmpl_save();
			return 1;
		}else
			return 1;
		}
	case 'A':
	case 'a':
		if( template[t_now].tmpl->content_num >= MAX_CONTENT ){
			char ans[STRLEN];
			move(t_lines - 1, 0);
			clrtoeol();
			a_prompt(-1, "选项已满，按回车继续...", ans, 2);
			move(t_lines - 1, 0);
			clrtoeol();
			return -1;
		}
		content_add();
		return 1;
		break;
	case 'D':
	case 'd':
		if (askyn("确实要删除吗", NA, YEA)) {
			int i=0;
			struct s_content *ct;
			if( template[t_now].tmpl->content_num == 1){
				template[t_now].tmpl->content_num = 0;
				free(template[t_now].cont);
				tmpl_save();
				return 1;
			}
			ct = (struct s_content *) malloc( sizeof(struct s_content) * (template[t_now].tmpl->content_num-1));

			memcpy(ct+i,&(template[t_now].cont[i]),sizeof(struct s_content) * (allnum));
			for(i=allnum; i<template[t_now].tmpl->content_num-1;i++)
				memcpy(ct+i, &(template[t_now].cont[i+1]), sizeof(struct s_content));

			free(template[t_now].cont);
			template[t_now].cont = ct;
			template[t_now].tmpl->content_num --;

			tmpl_save();
			return 1;
		}
		return 0;
	case 'T':
	case 't' :
		{
		char newtitle[60];
		strcpy(newtitle, template[t_now].cont[allnum].text);
		getdata(t_lines - 1, 0, "新标题: ", newtitle, 50, DOECHO, NA);

		if( newtitle[0] == '\0' || newtitle[0]=='\n' || !strcmp(newtitle, template[t_now].cont[allnum].text) )
			return 1;
		strncpy(template[t_now].cont[allnum].text, newtitle, 50);
		template[t_now].cont[allnum].text[49]='\0';

		tmpl_save();
		return 1;
		}
		break;
	case 'L':
	case 'l' :
		{
		char newlen[10];
		int nl;
		snprintf(newlen, sizeof(newlen), "%-3zu", template[t_now].cont[allnum].length);
		getdata(t_lines-1, 0, "新长度: ", newlen, 5, DOECHO, NA);
		nl = atoi(newlen);
		if( nl <= 0 || nl > MAX_CONTENT_LENGTH || /* safe */ (unsigned) nl == template[t_now].cont[allnum].length )
			return 1;
		template[t_now].cont[allnum].length = nl;
		tmpl_save();
		return 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int tmpl_key(int key, int _allnum, int pagenum) {
	(void) pagenum;
	if (_allnum < 0)
		return -1;

	size_t allnum = _allnum;
	switch (key) {
	case 'a' :
		{
		char ans[STRLEN];
		if( template_num >= MAX_TEMPLATE ){
			move(t_lines - 1, 0);
			clrtoeol();
			a_prompt(-1, "模板已满，按回车继续...", ans, 2);
			move(t_lines - 1, 0);
			clrtoeol();
			return -1;
		}
		tmpl_add();
		a_prompt(-1, "模板添加成功，按回车继续...", ans, 2);
		return -1;
		}
		break;

	case 'd' :
		if (askyn("确实要删除吗", NA, YEA)) {
			size_t i;
			char filepath[STRLEN];

			if( template[allnum].tmpl->filename[0] ){
				setbfile(filepath, sizeof(filepath), currboard, template[allnum].tmpl->filename);
				if(dashf(filepath))
					unlink(filepath);
			}

			if( template[allnum].tmpl != NULL)
				free(template[allnum].tmpl);
			if( template[allnum].cont != NULL)
				free(template[allnum].cont);

			template_num--;
			for(i = allnum; i < template_num; i++)
				memcpy(template+i, template+i+1, sizeof(struct a_template));
			template[template_num].tmpl = NULL;
			template[template_num].cont = NULL;
			tmpl_save();
		}
	/*	if(template_num > 0)
		return 1;
		else*/
			return -1;
		break;

	case 't' :
		{
		char newtitle[60];
		strcpy(newtitle, template[allnum].tmpl->title);
		getdata(t_lines - 1, 0, "新名称: ", newtitle, 50, DOECHO, NA);
		if( newtitle[0] == '\0' || newtitle[0]=='\n' || !strcmp(newtitle, template[allnum].tmpl->title) )
			return 0;
		strncpy(template[allnum].tmpl->title, newtitle, 50);
		template[allnum].tmpl->title[49]='\0';
		tmpl_save();
		tmpl_show();
		return 1;
		}
		break;

	case 'f' :
		{
		char filepath[STRLEN];
		int oldmode, fp, fail;
		time_t now;
		int count = 0;

		oldmode = uinfo.mode;
		modify_user_mode(EDITUFILE);

		if( template[allnum].tmpl->filename[0] == '\0' ){
			now = time(NULL);
			sprintf(template[allnum].tmpl->filename, "P.%ld.T", now);
			setbfile(filepath, sizeof(filepath), currboard, template[allnum].tmpl->filename);
			while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1) {
				now++;
				sprintf(template[allnum].tmpl->filename, "P.%ld.T", now);
				setbfile(filepath, sizeof(filepath), currboard, template[allnum].tmpl->filename);
				if (count++ > MAX_POSTRETRY)
					fail = 1;
			}
			fail = 0;
			close(fp);
			if (fail) {
				clear();
				move(3,0);
				prints("创建模板文件失败!");
				pressanykey();
				return -1;
			}
			tmpl_save();
		}
		tmpl_edit(allnum);
		modify_user_mode(oldmode);
		tmpl_show();
		return 1;
		}

	case 's' :
		{
		char filepath[STRLEN];
		setbfile(filepath, sizeof(filepath), currboard, template[allnum].tmpl->filename);
		clear();
		ansimore(filepath,1);
		tmpl_show();
		return 1;
		}

	case 'b' :
		if( template[allnum].tmpl->flag & TMPL_BM_FLAG )
			template[allnum].tmpl->flag &= ~TMPL_BM_FLAG ;
		else
			template[allnum].tmpl->flag |= TMPL_BM_FLAG;
		tmpl_save();
		tmpl_show();
		return 1;

	case 'i' :
		{
		char newtitle[STRLEN];
		strcpy(newtitle, template[allnum].tmpl->title_tmpl);
		getdata(t_lines - 1, 0, "新文章标题: ", newtitle, STRLEN, DOECHO, NA);
		if( newtitle[0] == '\0' || newtitle[0]=='\n' || ! strcmp(newtitle, template[allnum].tmpl->title_tmpl) )
			return 1;
		strncpy(template[allnum].tmpl->title_tmpl, newtitle, STRLEN);
		template[allnum].tmpl->title_tmpl[STRLEN-1]='\0';
		tmpl_save();
		tmpl_show();
		return 1;
		}

	case 'x' :
		clear();
		move(2,0);
		prints("此模版的标题设置为");
		move(4,0);
		prints("%s", template[allnum].tmpl->title_tmpl);
		pressanykey();
		tmpl_show();
		return 1;
/*
	case 'h':
		clear();
		move(1,0);
		prints("  x  :  查看标题格式\n");
		prints("  i  :  修改标题格式");
		pressanykey();
		tmpl_show();
		return 1;
*/
	default :
		break;
	}
	return 0;
}

static int tmpl_select(int star, int curr) {
	(void) star;
	int ch, deal;
	int page = -1;
	int number = 0;
	int num = 0;

	t_now = curr;
	setlistrange(template[t_now].tmpl->content_num);

	if(range == 0 ){
		clear();
		if (!askyn("本模板现在没有内容，需要现在增加吗", NA, NA))
			return DOQUIT;
		if(content_add() < 0 )
			return DOQUIT;
	}

	content_refresh();
	while (1) {
		if (num <= 0)
			num = 0;
		if (num >= range)
			num = range - 1;
		if (page < 0) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			move(3, 0);
			clrtobot();
			if (content_show() == -1)
				return -1;
			update_endline();
		}
		if (num < page || num >= page + BBS_PAGESIZE) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			if (content_show() == -1)
				return -1;
			update_endline();
			continue;
		}

		move(3 + num - page, 0);
		prints(">", number);
		move(3 + num - page, 0);
		ch = egetch();
		prints(" ");
		deal = content_key(ch, num, page);
		if (range == 0)
			break;
		if (deal == 1) {
			content_refresh();
			content_show();
			update_endline();
			continue;
		} else if (deal == -1)
			break;

		switch (ch) {
			case 'P':
			case 'b':
			case Ctrl('B'):
			case KEY_PGUP:
				if (num == 0)
					num = range - 1;
				else
					num -= BBS_PAGESIZE;
				break;
			case 'N':
			case Ctrl('F'):
			case KEY_PGDN:
				if (num == range - 1)
					num = 0;
				else
					num += BBS_PAGESIZE;
				break;
			case 'k':
			case KEY_UP:
				if (num-- <= 0)
					num = range - 1;
				break;
			case ' ':
			case 'j':
			case KEY_DOWN:
				if (++num >= range)
					num = 0;
				break;
			case '$':
			case KEY_END:
				num = range - 1;
				break;
			case KEY_HOME:
				num = 0;
				break;
			case '\n':
			case '\r':
				if (number > 0)
					num = number - 1;
				break;
			case KEY_LEFT:
				return DOQUIT;
			default:
				;
			}
			if (ch >= '0' && ch <= '9') {
				number = number * 10 + (ch - '0');
				ch = '\0';
			} else
				number = 0;
	}
	return DOQUIT;
}

int m_template() {
	//int tmplist;
	if (!IScurrBM)
		return DONOTHING;
	if( tmpl_init(1) < 0 )
		return FULLUPDATE;

	setlistrange(template_num);
	if(range == 0 ){
		clear();
		if (!askyn("本版现在没有模板，需要现在增加吗", NA, YEA)){
			tmpl_free();
			return FULLUPDATE;
		}
		if( tmpl_add() < 0 ){
			tmpl_free();
			return FULLUPDATE;
		}
	}
	clear();
	//tmplist = choose(NA, 0, tmpl_refresh, tmpl_key, tmpl_show, tmpl_select);
	choose(NA, 0, tmpl_refresh, tmpl_key, tmpl_show, tmpl_select);
	tmpl_free();
	return FULLUPDATE;
}


/***********普通用户模板选择**************/
static void
choose_tmpl_refresh(){
	//clear();
	docmdtitle("[版面模板选择]",
			"退出[\x1b[1;32m←\x1b[0;37m] 选择[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 使用[\x1b[1;32m→\x1b[0;37m] 查看正文[\033[1;32ms\033[0;37m] 查看问题[\033[1;32mw\033[0;37m] 查看标题[\033[1;32mx\033[m]");
	move(2, 0);
	prints("\033[0;1;37;44m %4s %-13s %-50s %8s", "序号", "创建人", "名称","问题个数");
	clrtoeol();
	update_endline();
}

static int choose_tmpl_key(int key, int _allnum, int pagenum){
	(void) pagenum;
	if (_allnum < 0)
		return -1;

	size_t allnum = _allnum;
	switch (key) {
	case 's' :
		{
		char filepath[STRLEN];
		if( allnum > template_num )
			return -1;
		if( template[allnum].tmpl->filename[0] ){
			clear();
			setbfile(filepath, sizeof(filepath), currboard, template[allnum].tmpl->filename);
			ansimore(filepath, 1);
			tmpl_show();
			return 1;
		}
		return 0;
		}
		break;

	case 'w':
		clear();
		if( template[allnum].tmpl->content_num <= 0 ){
			move(5,0);
			prints("版主还没有设置问题，本模板暂不可用\n");
		}else{
			int i;
			for(i=0;i<template[allnum].tmpl->content_num;i++){
				move(i+2,0);
				prints("\033[1;32m问题 %d\033[m:%s  \033[1;32m最长回答\033[m%d\033[1;32m字节\033[m", i+1, template[allnum].cont[i].text, template[allnum].cont[i].length);
			}
		}
		pressanykey();
		tmpl_show();
		return 1;

	case 'x' :
		clear();
		move(2,0);
		prints("此模版的标题设置为");
		move(4,0);
		prints("%s",template[allnum].tmpl->title_tmpl);
		pressanykey();
		tmpl_show();
		return 1;

	default:
		break;
	}
	return 0;
}

char fname[STRLEN];

static int choose_tmpl_post(int star, int curr) {
	(void) star;
	FILE *fp;
	FILE *fpsrc;
	char filepath[STRLEN];
	int i, ret=1;
	int write_ok = 0;
	char * tmp[ MAX_CONTENT ];
	char newtitle[STRLEN];
	int oldmode = uinfo.mode;
	char title[STRLEN];
	title[0]='\0';
	unsigned char modifying=0, loop=1;
	char temp[STRLEN];

	if(curr < 0 || /* safe */ (size_t) curr >= MAX_TEMPLATE )
		return DOQUIT;
	t_now = curr;
	if( template[t_now].tmpl->content_num <= 0 ){
		move(5,0);
		prints("版主还没有设置问题，本模板暂不可用\n");
		pressanykey();
		return DOQUIT;
	}

	/*add a loop by macintosh 06.12.10*/
	while(loop) {

	if((fp = fopen(fname, "w"))==NULL){
		move(5,0);
		prints("文件错误!请联系系统维护!\n");
		pressanykey();
		return -1;
	}
	modify_user_mode(POSTING);
	for(i=0; i< template[t_now].tmpl->content_num; i++){
		char *ans;
		if (modifying)
			ans = tmp[i];
		else{
			ans = (char *)malloc(template[t_now].cont[i].length + 2);
			ans[0] = '\0';
		}
		if( ans == NULL ){
			modify_user_mode(oldmode);
			fclose(fp);
			return -1;
		}
		clear();
		move(1,0);
		prints("Ctrl+Q 换行, ENTER 发送");
		move(3,0);
		prints("模板问题:%s", template[t_now].cont[i].text);
		move(4,0);
		prints("模板回答(最长%d字符):",template[t_now].cont[i].length);
		multi_getdata(6, 0, 79, NULL, ans, template[t_now].cont[i].length+1, 11, NA);
		tmp[i] = ans;
	}
	modify_user_mode(oldmode);

	if( template[t_now].tmpl->filename[0] ){
		struct stat st;
		setbfile(filepath, sizeof(filepath), currboard, template[t_now].tmpl->filename);
		if( stat(filepath, &st) == 0 && S_ISREG(st.st_mode) && st.st_size>2){
			if((fpsrc = fopen(filepath,"r"))!=NULL){
				char buf[256];

				while(fgets(buf,255,fpsrc)){
					int l;
					int linex = 0;
					char *pn,*pe;

					for(pn = buf; *pn!='\0'; pn++){
						if( *pn != '[' || *(pn+1)!='$' ){
							fputc(*pn, fp);
							linex++;
						}else{
							pe = strchr(pn,']');
							if(pe == NULL){
								fputc(*pn, fp);
								continue;
							}
							l = atoi(pn+2);
							if (l<=0){
								if (strncmp(pn, "[$userid]", 9) == 0){
									fprintf(fp,"%s", currentuser.userid);
									pn = pe;
									continue;
								} else {
									fputc('[', fp);
									continue;
								}
							} else if( l > template[t_now].tmpl->content_num ){
								fputc('[', fp);
								continue;
							}
							fprintf(fp,"%s",tmp[l-1]);
							pn = pe;
							continue;
						}
					}
				}
				fclose(fpsrc);
				write_ok = 1;
			}
		}
	}
	if(write_ok == 0){
		for(i=0; i< template[t_now].tmpl->content_num; i++)
			fprintf(fp,"\033[1;32m%s:\033[m\n%s\n\n",template[t_now].cont[i].text, tmp[i]);
	}
	fclose(fp);

	if( template[t_now].tmpl->title_tmpl[0] ){
		char *pn,*pe;
		char *buf;
		int l;
		int newl = 0;

		newtitle[0]='\0';
		buf = template[t_now].tmpl->title_tmpl;

		for(pn = buf; *pn!='\0' && newl < STRLEN-1; pn++){
			if( *pn != '[' || *(pn+1)!='$' ){
				if( newl < STRLEN - 1 ){
					newtitle[newl] = *pn ;
					newtitle[newl+1]='\0';
					newl ++;
				}
			}else{
				pe = strchr(pn,']');
				if(pe == NULL){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				l = atoi(pn+2);
				if( l<0 || l > template[t_now].tmpl->content_num ){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}

				if ((l == 0 ) && (strncmp(pn, "[$userid]", 9) == 0)){
					int ti;
					for( ti=0; currentuser.userid[ti]!='\0' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = currentuser.userid[ti] ;
						newtitle[newl+1]='\0';
					}
				} else if( l == 0 ){
					int ti;
					for( ti=0; title[ti]!='\0' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = title[ti] ;
						newtitle[newl+1]='\0';
					}
				}else{
					int ti;
					for( ti=0; tmp[l-1][ti]!='\0' && tmp[l-1][ti]!='\n' && tmp[l-1][ti]!='\r' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = tmp[l-1][ti] ;
						newtitle[newl+1]='\0';
					}
				}
				pn = pe;
				continue;
			}
		}
		strncpy(title, newtitle, STRLEN);
		title[STRLEN-1]='\0';
	}

	/*pzhg's modification*/
	/*if title is not defined, require a new one, or use the default one*/
	if (!title[0])
	{
		clear();
		strcpy(title, "默认模版标题");
		move(t_lines-2, 0);
		prints("\033[1m模版标题为空!  \033[m\n");
		getdata(t_lines-1, 0, " 请输入模版标题[默认模版标题]: ", temp, 50, DOECHO, YEA);
		if (temp[0])
			strcpy(title,temp);
	}
	/*end*/

	/*modify by macintosh 06.12.10*/
	/*can modify after filling in tmpl*/
	clear();
	ansimore(fname, NA);
	move(t_lines-2, 0);
	clrtobot();
	prints("标题: \033[1m%s\033[m", title);
	getdata(t_lines-1, 0, "确认发表 (Y)发表 (N)退出 (E)修改 [Y]: ",temp,2,DOECHO,YEA);
	switch(toupper(temp[0])){
	case 'N':
		loop=0;
		ret=-1;
		break;
	case 'E':
		modifying=1;
		break;
	default:
		loop=0;
		ret=1;
		break;
	}


	}//end while loop

	for(i=0; i< template[t_now].tmpl->content_num; i++)
		free( tmp[i] );

	if (ret == 1) {
		if((fp = fopen(fname, "a"))==NULL)
			return -1;
		if (!(currentuser.signature == 0))
			addsignature(fp, 1);
		fclose(fp);
		add_loginfo(fname);
		if (postfile(fname, currboard, title, 2) == -1)
			return -1;
		else
			unlink(fname);
	}
	return -1;
}

int
choose_tmpl()
{
	//int tmpllist;

	sprintf(fname, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
		currentuser.userid, getpid());

	if( tmpl_init(0) < 0 )
		return  -1;

	setlistrange(template_num);
	if (range == 0 ){
		clear();
		move(3,0);
		prints("本版没有模板可供使用");
		pressanykey();
		tmpl_free();
		return  -1;
	}
	clear();
	t_now = 0;
	//tmpllist = choose(NA, 0, choose_tmpl_refresh, choose_tmpl_key, tmpl_show, choose_tmpl_post);
	choose(NA, 0, choose_tmpl_refresh, choose_tmpl_key, tmpl_show, choose_tmpl_post);
	tmpl_free();
	return 1;
}

