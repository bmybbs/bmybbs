#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "config.h"
#include "ytht/crypt.h"
#include "ytht/timeop.h"
#include "ytht/fileop.h"
#include "ytht/common.h"
#include "ytht/strlib.h"
#include "ytht/msg.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/permissions.h"
#include "ythtbbs/modes.h"
#include "ythtbbs/user.h"
#include "ythtbbs/record.h"
#include "ythtbbs/msg.h"
#include "ythtbbs/override.h"
#include "ythtbbs/identify.h"

/**
 * just use to pass a param to fillmboard()
 */
struct myparam1 {
	struct userec user;
	int fd;
	short bid;
};

static const char *KILLFILE = MY_BBS_HOME "/tmp/killuser";

// 用于加载好友、黑名单的条数以及用户 id 到 user_info 结构体中
static int ythtbbs_user_init_override(struct user_info *u, enum ythtbbs_override_type override_type);

static bool ythtbbs_user_has_perm(struct userec *x, int level);
/**
 * @brief 比较用户 id 先后顺序的函数
 * 用于 qsort，暂时设定为私有函数。
 */
static int ythtbbs_user_cmp_uid(const unsigned *a, const unsigned *b);

/**
 * @brief 更新用户对应的版面信息
 */
static int ythtbbs_user_set_bm_status(const struct userec *user, bool online, bool invisible);
/* mytoupper: 将中文ID映射到A-Z的目录中 */

static int fillmboard(struct boardheader *bh, struct myparam1 *param);


/**
 * @brief 删除文件夹
 * 在 BMY 的文件结构里，用户目录和信件目录都只有一层，因此本函数
 * 对于嵌套文件夹仅作简单的递归调用。
 * 参考: https://stackoverflow.com/a/2256974/803378
 * @warning 多线程不安全
 */
static int remove_directory(const char *path);

char
mytoupper(unsigned char ch)
{
	if (isalpha(ch))
		return toupper(ch);
	else
		return ch % ('Z' - 'A') + 'A';
}

char *
sethomepath(char *buf, const char *userid)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s", mytoupper(userid[0]), userid);
	return buf;
}

char *
sethomepath_s(char *buf, size_t buf_size, const char *userid)
{
	snprintf(buf, buf_size, MY_BBS_HOME "/home/%c/%s", mytoupper(userid[0]), userid);
	return buf;
}

char *setmailpath_s(char *buf, const size_t buf_size, const char *userid) {
	snprintf(buf, buf_size, MY_BBS_HOME "/mail/%c/%s", mytoupper(userid[0]), userid);
	return buf;
}

char *
sethomefile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid, filename);
	return buf;
}

char *
sethomefile_s(char *buf, size_t buf_size, const char *userid, const char *filename)
{
	snprintf(buf, buf_size, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid, filename);
	return buf;
}

char *
setmailfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/mail/%c/%s/%s", mytoupper(userid[0]), userid, filename);
	return buf;
}

char *
setmailfile_s(char *buf, size_t buf_size, const char *userid, const char *filename)
{
	snprintf(buf, buf_size, MY_BBS_HOME "/mail/%c/%s/%s", mytoupper(userid[0]), userid, filename);
	return buf;
}

/**
get the file path of sent mail box
*/
char *
setsentmailfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/sent_mail/%c/%s/%s", mytoupper(userid[0]), userid, filename);
	return buf;
}

int
saveuservalue(char *userid, char *key, char *value)
{
	char path[256];
	sethomefile_s(path, sizeof(path), userid, "values");
	return savestrvalue(path, key, value);
}

int
readuservalue(char *userid, char *key, char *value, int size)
{
	char path[256];
	sethomefile_s(path, sizeof(path), userid, "values");
	return readstrvalue(path, key, value, size);
}

char *
charexp(int exp)
{
	int expbase = 0;

	if (exp == -9999)
		return "\xC3\xBB\xB5\xC8\xBC\xB6"; // 没等级
	if (exp <= 100 + expbase)
		return "\xD0\xC2\xCA\xD6\xC9\xCF\xC2\xB7"; // 新手上路
	if (exp <= 450 + expbase)
		return "\xD2\xBB\xB0\xE3\xD5\xBE\xD3\xD1"; // 一般站友
	if (exp <= 850 + expbase)
		return "\xD6\xD0\xBC\xB6\xD5\xBE\xD3\xD1"; // 中级站友
	if (exp <= 1500 + expbase)
		return "\xB8\xDF\xBC\xB6\xD5\xBE\xD3\xD1"; // 高级站友
	if (exp <= 2500 + expbase)
		return "\xC0\xCF\xD5\xBE\xD3\xD1"; // 老站友
	if (exp <= 3000 + expbase)
		return "\xB3\xA4\xC0\xCF\xBC\xB6"; // 长老级
	if (exp <= 5000 + expbase)
		return "\xB1\xBE\xD5\xBE\xD4\xAA\xC0\xCF"; // 本站元老
	return "\xBF\xAA\xB9\xFA\xB4\xF3\xC0\xCF"; // 开国大老
}

char *
cperf(int perf)
{
	if (perf == -9999)
		return "\xC3\xBB\xB5\xC8\xBC\xB6"; // 没等级
	if (perf <= 5)
		return "\xB8\xCF\xBF\xEC\xBC\xD3\xD3\xCD"; // 赶快加油
	if (perf <= 12)
		return "\xC5\xAC\xC1\xA6\xD6\xD0"; // 努力中
	if (perf <= 35)
		return "\xBB\xB9\xB2\xBB\xB4\xED"; // 还不错
	if (perf <= 50)
		return "\xBA\xDC\xBA\xC3"; // 很好
	if (perf <= 90)
		return "\xD3\xC5\xB5\xC8\xC9\xFA"; // 优等生
	if (perf <= 140)
		return "\xCC\xAB\xD3\xC5\xD0\xE3\xC1\xCB"; // 太优秀了
	if (perf <= 200)
		return "\xB1\xBE\xD5\xBE\xD6\xA7\xD6\xF9"; // 本站支柱
	if (perf <= 500)
		return "\xC9\xF1\xA1\xAB\xA1\xAB"; // 神～～
	return "\xBB\xFA\xC6\xF7\xC8\xCB\xA3\xA1"; // 机器人！
}

int
countexp(struct userec *udata)
{
	int exp;

	if (!strcmp(udata->userid, "guest"))
		return -9999;
	exp = udata->numposts /*+post_in_tin( udata->userid ) */  +
		udata->numlogins / 5 + (time(0) - udata->firstlogin) / 86400 +
		udata->stay / 3600;
	return exp > 0 ? exp : 0;
}

int
countperf(struct userec *udata)
{
	int perf;
	int reg_days;

	if (!strcmp(udata->userid, "guest"))
		return -9999;
	reg_days = (time(0) - udata->firstlogin) / 86400 + 1;
	perf = ((float) (udata->numposts /*+post_in_tin( udata->userid ) */ ) /
		(float) udata->numlogins +
		(float) udata->numlogins / (float) reg_days) * 10;
	return perf > 0 ? perf : 0;
}

// unused function detected, commented by IronBlood 2020.08.10
/*
int life_special(char *id)
{
	FILE *fp;
	char buf[128];
	fp=fopen("etc/life", "r");
	if(fp==0) return 0;
	while(1) {
		if(fgets(buf, 128, fp)==0) break;

		buf[strlen(buf)-1] = 0;
		if(!strcmp(buf, id)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
*/

int
countlife(struct userec *urec)
{
	int value, res;

	/* if (urec) has XEMPT permission, don't kick it */
	if ((urec->userlevel & PERM_XEMPT)
			|| strcmp(urec->userid, "guest") == 0)
		return 999;
//	if (life_special(urec->userid)) return 666;
//	life_special(urec->userid);
	value = (time(0) - urec->lastlogin) / 60;	/* min */
	/* new user should register in 30 mins */
	if (strcmp(urec->userid, "new") == 0) {
		return (30 - value) * 60;
	}
	if (urec->numlogins <= 1)
		return (60 * 1440 - value) / 1440;
	if (!(urec->userlevel & PERM_LOGINOK))
		return (60 * 1440 - value) / 1440;
	if (((time(0)-urec->firstlogin)/86400)>365*8)
		return  888;
	if (((time(0)-urec->firstlogin)/86400)>365*5)
		return  666;
	if (((time(0)-urec->firstlogin)/86400)>365*2)
		return  365;

	// if (urec->stay > 1000000)
		// return (365 * 1440 - value) / 1440;
	res=(180 * 1440 - value) / 1440 + urec->numdays;
	if (res>364) res=364;
	return res;
}

int userlock(const char *userid, int locktype) {
	char path[256];
	int fd;
	sethomefile_s(path, sizeof(path), userid, ".lock");
	fd = open(path, O_RDONLY | O_CREAT, 0660);
	if (fd == -1)
		return -1;
	flock(fd, locktype);
	return fd;
}

int userunlock(const char *userid, int fd) {
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

static int
checkbansitefile(const char *addr, const char *filename)
{
	FILE *fp;
	char temp[STRLEN];
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(temp, STRLEN, fp) != NULL) {
		strtok(temp, " \n");
		if ((!strncmp(addr, temp, 16)) || (!strncmp(temp, addr, strlen(temp))
			&& temp[strlen(temp) - 1] == '.')
			|| (temp[0] == '.' && strstr(addr, temp) != NULL)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int
checkbansite(const char *addr)
{
	return checkbansitefile(addr, MY_BBS_HOME "/.bansite") || checkbansitefile(addr, MY_BBS_HOME "/bbstmpfs/dynamic/bansite");
}

int
userbansite(const char *userid, const char *fromhost)
{
	char path[STRLEN];
	FILE *fp;
	char buf[STRLEN];
	int i, deny;
	char addr[STRLEN], mask[STRLEN], allow[STRLEN];
	char *tmp[3] = { addr, mask, allow };
	unsigned int banaddr, banmask;
	unsigned int from;
	from = inet_addr(fromhost);
	sethomefile_s(path, sizeof(path), userid, "bansite");
	if ((fp = fopen(path, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		i = ytht_strtok(buf, ' ', tmp, 3);
		if (i == 1) {	//单独 ip
			banaddr = inet_addr(addr);
			banmask = inet_addr("255.255.255.255");
			deny = 1;
		} else if (i == 2) {
			banaddr = inet_addr(addr);
			banmask = inet_addr(mask);
			deny = 1;
		} else if (i == 3) {	//带 allow 项
			banaddr = inet_addr(addr);
			banmask = inet_addr(mask);
			deny = !strcmp(allow, "allow");
		} else		//空行？
			continue;
		if ((from & banmask) == (banaddr & banmask)) {
			fclose(fp);
			return deny;
		}
	}
	fclose(fp);
	return 0;
}

void
logattempt(const char *user, const char *from, const char *zone, time_t time)
{
	char buf[256], filename[80];
	char time_buf[30];
	int fd, len;

	sprintf(buf, "system passerr %s", from);
	newtrace(buf);
	ytht_ctime_r(time, time_buf);
	snprintf(buf, 256, "%-12.12s  %-30s %-16s %-6s\n", user, time_buf, from, zone);
	len = strlen(buf);
	if ((fd = open(MY_BBS_HOME "/" BADLOGINFILE, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0) {
		write(fd, buf, len);
		close(fd);
	}
	sethomefile_s(filename, sizeof(filename), user, BADLOGINFILE);
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0) {
		write(fd, buf, len);
		close(fd);
	}
}

int check_user_perm(struct userec *x, int level) {
	return (x->userlevel & level);
}

int check_user_read_perm(const struct user_info *user, const char *board)
{
	return check_user_read_perm_x(user, ythtbbs_cache_Board_get_board_by_name(board));
}

bool check_guest_read_perm_x(const struct boardmem *board) {
	if (!board)
		return false;

	if (board->header.clubnum != 0) {
		return (board->header.flag & CLUBTYPE_FLAG);
	}

	// guest.userlevel = 0
	if (board->header.level == 0 || (board->header.level & (PERM_POSTMASK | PERM_NOZAP)))
		return true;

	return false;
}

int check_user_read_perm_x(const struct user_info *user, const struct boardmem *board)
{
	if(!board || !user)
		return 0;

	if(board->header.clubnum != 0) {
		if(board->header.flag & CLUBTYPE_FLAG)
			return 1;
		if(user->active == 0 || strcasecmp(user->userid, "guest")==0)
			return 0;
		return user->clubrights[board->header.clubnum / 32] & (1<<((board->header.clubnum) % 32));
	}

	if(board->header.level == 0)
		return 1;

	if(board->header.level & (PERM_POSTMASK | PERM_NOZAP))
		return 1;

	if((user->userlevel & PERM_BASIC) == 0)
		return 0;

	if((user->userlevel & board->header.level))
		return 1;

	return 0;
}

int check_user_post_perm_x(const struct user_info *user, const struct boardmem *board)
{
	char buf[256];

	if(!board || !check_user_read_perm_x(user, board))
		return 0;

	sprintf(buf, "boards/%s/deny_users", board->header.filename);
	if(seek_in_file(buf, user->userid))
		return 0;

	sprintf(buf, "boards/%s/deny_anony", board->header.filename);
	if(seek_in_file(buf, user->userid))
		return 0;

	if(!strcasecmp(board->header.filename, "sysop"))
		return 1;

	if(!strcasecmp(board->header.filename, "Freshman"))
		return 1;

	if(!strcasecmp(board->header.filename, "welcome"))
		return 1;

	if(!strcasecmp(board->header.filename, "KaoYan"))
		return 1;

	if(user->userlevel & PERM_SYSOP)
		return 1;

	if(!(user->userlevel & PERM_BASIC))
		return 0;

	if(!(user->userlevel & PERM_POST))
		return 0;

	if(!strcasecmp(board->header.filename, "Appeal"))
		return 1;

	if(!strcasecmp(board->header.filename, "committee"))
		return 1;

	if(seek_in_file("deny_user", user->userid))
		return 0;

	if(board->header.clubnum != 0) {
		if(!(board->header.level & PERM_NOZAP) && board->header.level && !(user->userlevel & board->header.level))
			return 0;
		return user->clubrights[board->header.clubnum / 32] & (1 << (board->header.clubnum % 32));
	}

	if(!(board->header.level & PERM_NOZAP) && board->header.level && !(user->userlevel & board->header.level))
		return 0;

	return 1;
}

int id_with_num(const char *userid)
{
	const char *s;
	for (s = userid; *s != '\0'; s++)
		if (*s < 1 || !isalpha(*s)) return 1;
	return 0;
}

int
chk_BM(struct userec *user, struct boardheader *bh, int isbig)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (bh->bm[i][0] == 0)
			break;
		if (!strcmp(bh->bm[i], user->userid)
			&& bh->hiretime[i] >= user->firstlogin)
			return i + 1;
	}
	if (isbig)
		return 0;
	for (i = 4; i < BMNUM; i++) {
		if (bh->bm[i][0] == 0)
			break;
		if (!strcmp(bh->bm[i], user->userid)
			&& bh->hiretime[i] >= user->firstlogin)
			return i + 1;
	}
	return 0;
}

int
chk_BM_id(char *user, struct boardheader *bh)
{
	int i;
	for (i = 0; i < BMNUM; i++) {
		if (bh->bm[i][0] == 0) {
			if (i < 4) {
				i = 3;
				continue;
			}
			break;
		}
		if (!strcmp(bh->bm[i], user))
			return i + 1;
	}
	return 0;
}

int
bmfilesync(struct userec *user)
{
	char path[256];
	struct myparam1 mp;
	struct stat st_path, st_boards;

	sethomefile_s(path, sizeof(path), user->userid, "mboard");
	f_stat_s(&st_path, path);
	f_stat_s(&st_boards, BOARDS);
	if (st_path.st_mtime > st_boards.st_mtime)
		return 0;
	memcpy(&(mp.user), user, sizeof (struct userec));
	mp.fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);	//touch a new file
	if (mp.fd == -1) {
		errlog("touch new mboard error");
		return -1;
	}
	mp.bid = 0;
	new_apply_record(BOARDS, sizeof (struct boardheader), (void *)fillmboard, &mp);
	close(mp.fd);
	return 0;
}

int ythtbbs_user_bmfile_sync(const struct userec *user) {
	char path[256];
	struct myparam1 mp;
	struct stat st1, st2;

	sethomefile_s(path, sizeof(path), user->userid, "mboard");
	f_stat_s(&st1, path);
	f_stat_s(&st2, BOARDS);
	if (st1.st_mtime > st2.st_mtime)
		return 0;

	memcpy(&(mp.user), user, sizeof (struct userec));
	mp.fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);	//touch a new file
	if (mp.fd == -1) {
		errlog("touch new mboard error");
		return -1;
	}

	mp.bid = 0;
	new_apply_record(BOARDS, sizeof (struct boardheader), (void *)fillmboard, &mp);
	close(mp.fd);
	return 0;
}

int
fillmboard(struct boardheader *bh, struct myparam1 *mp)
{
	struct boardmanager bm;
	int i;
	if ((i = chk_BM(&(mp->user), bh, 0))) {
		bzero(&bm, sizeof (bm));
		strncpy(bm.board, bh->filename, sizeof(bm.board));
		bm.board[sizeof(bm.board) - 1] = '\0';
		bm.bmpos = i - 1;
		bm.bid = mp->bid;
		write(mp->fd, &bm, sizeof (bm));
	}
	(mp->bid)++;
	return 0;
}

const char *ythtbbs_user_get_login_type_str(enum ythtbbs_user_login_type type) {
	switch(type) {
	case YTHTBBS_LOGIN_TELNET: return "TELNET";
	case YTHTBBS_LOGIN_SSH   : return "SSH";
	case YTHTBBS_LOGIN_NJU09 : return "NJU09";
	case YTHTBBS_LOGIN_API   : return "API";
	case YTHTBBS_LOGIN_OAUTH : return "OAUTH";
	default                  : return "unknown";
	}
}

int ythtbbs_user_login(const char *userid, const char *passwd, const char *fromhost, const enum ythtbbs_user_login_type login_type, struct user_info *out_info, struct userec *out_userec, int *out_utmp_idx) {
	int              user_idx;
	long             login_interval;
	int              local_utmp_idx;
	char             local_buf[512];
	time_t           local_now;
	struct userec    local_lookup_user;
	struct user_info local_uinfo;
	FILE             *fp1;
	int              clubnum;
	time_t           dtime;
	int              day;
	struct tm        tm;

	if (checkbansite(fromhost))
		return YTHTBBS_USER_SITE_BAN;

	time(&local_now);
	ythtbbs_cache_UserTable_resolve();
	user_idx = ythtbbs_cache_UserIDHashTable_find_idx(userid);
	if (user_idx < 0)
		return YTHTBBS_USER_NOT_EXIST;

	get_record(PASSFILE, &local_lookup_user, sizeof(struct userec), user_idx + 1);
	local_lookup_user.userid[IDLEN + 1] = '\0'; // 显式设置终止符
	local_lookup_user.passwd[PASSLEN - 1] = 0;
	local_lookup_user.username[NAMELEN - 1] = 0;
	local_lookup_user.realname[NAMELEN - 1] = 0;
	if (out_userec) {
		memcpy(out_userec, &local_lookup_user, sizeof(struct userec));
	}

	if (strcasecmp(userid, "guest") == 0) {
		if(out_userec)
			out_userec->userlevel = 0;
		return YTHTBBS_USER_LOGIN_OK; // TODO
	}

	if (userbansite(local_lookup_user.userid, fromhost)) {
		return YTHTBBS_USER_USER_BAN;
	}

	if (ytht_file_has_word(MY_BBS_HOME "/etc/prisonor", local_lookup_user.userid)) {
		return YTHTBBS_USER_IN_PRISON;
	}

	if (login_type != YTHTBBS_LOGIN_OAUTH) {
		if (!ytht_crypt_checkpasswd(local_lookup_user.passwd, passwd)) {
			logattempt(local_lookup_user.userid, fromhost, ythtbbs_user_get_login_type_str(login_type), local_now);
			return YTHTBBS_USER_WRONG_PASSWORD;
		}
	}

	if (!ythtbbs_user_has_perm(&local_lookup_user, PERM_BASIC))
		return YTHTBBS_USER_SUSPENDED;

	// TODO 大富翁相关校验

	// 检查间隔从 NJU09 20s 减少为 5s
	login_interval = local_now - local_lookup_user.lastlogin;
	if (login_interval < 0)
		login_interval = -login_interval;
	if (login_interval < 5)
		return YTHTBBS_USER_TOO_FREQUENT;

	// 其他对于 struct userec 数据更新
	strncpy(local_lookup_user.lasthost, fromhost, BMY_IPV6_LEN);
	local_lookup_user.lasthost[BMY_IPV6_LEN - 1] = '\0';
	local_lookup_user.lastlogin = time(NULL);
	local_lookup_user.numlogins++;

	dtime = time(NULL) - 4 * 3600;
	localtime_r(&dtime, &tm);
	day = tm.tm_mday;
	dtime = local_lookup_user.lastlogin - 4 * 3600;
	localtime_r(&dtime, &tm);
	if (day > tm.tm_mday && local_lookup_user.numdays < 800) {
		local_lookup_user.numdays++;
	}

	sethomepath_s(local_buf, sizeof(local_buf), local_lookup_user.userid);
	mkdir(local_buf, 0755);

	// update struct user_info
	memset(&local_uinfo, 0, sizeof(struct user_info));

	local_uinfo.active    = true;
	local_uinfo.pid       = (login_type == YTHTBBS_LOGIN_TELNET || login_type == YTHTBBS_LOGIN_SSH) ? getpid() : 1 /* magic number for www/api/oauth */;
	local_uinfo.mode      = LOGIN;
	local_uinfo.uid       = user_idx + 1;
	local_uinfo.userlevel = local_lookup_user.userlevel;
	local_uinfo.lasttime  = local_now;
	local_uinfo.curboard  = 0;
	local_uinfo.unreadmsg = strcasecmp(local_lookup_user.userid, "guest") ? get_unreadmsg(local_lookup_user.userid) : 0;
	local_uinfo.invisible = (ythtbbs_user_has_perm(&local_lookup_user, PERM_LOGINCLOAK) && (local_lookup_user.flags[0] & CLOAK_FLAG)) ? true : false; // 移除 term 模式中对 dietime > 0 的处理 by IronBlood 2020.10.09
	local_uinfo.isssh     = (login_type == YTHTBBS_LOGIN_SSH) ? true : false;
	local_uinfo.login_type = login_type;

	// pager start
	local_uinfo.pager = 0;

	if (local_lookup_user.userdefine & DEF_FRIENDCALL)
		local_uinfo.pager |= FRIEND_PAGER;
	if (local_lookup_user.flags[0] & PAGER_FLAG) {
		local_uinfo.pager |= ALL_PAGER;
		local_uinfo.pager |= FRIEND_PAGER;
	}

	if (local_lookup_user.userdefine & DEF_FRIENDMSG)
		local_uinfo.pager |= FRIENDMSG_PAGER;
	if (local_lookup_user.userdefine & DEF_ALLMSG) {
		local_uinfo.pager |= ALLMSG_PAGER;
		local_uinfo.pager |= FRIENDMSG_PAGER;
	}
	// pager end

	ytht_strsncpy(local_uinfo.from, fromhost, BMY_IPV6_LEN);
	ytht_strsncpy(local_uinfo.username, local_lookup_user.username, NAMELEN);
	ytht_strsncpy(local_uinfo.realname, local_lookup_user.realname, NAMELEN);
	ytht_strsncpy(local_uinfo.userid, local_lookup_user.userid, IDLEN + 1);

	// friends
	ythtbbs_user_init_override(&local_uinfo, YTHTBBS_OVERRIDE_FRIENDS);
	ythtbbs_user_init_override(&local_uinfo, YTHTBBS_OVERRIDE_REJECTS);

	// 处理俱乐部权限
	if (strcasecmp(local_lookup_user.userid, "guest")) {
		sethomefile_s(local_buf, sizeof(local_buf), local_lookup_user.userid, "clubrights");
		if ((fp1 = fopen(local_buf, "r")) == NULL) {
			memset(local_uinfo.clubrights, 0, 4 * sizeof(int));
		} else {
			memset(local_buf, 0, sizeof(local_buf));
			while (fgets(local_buf, STRLEN, fp1) != NULL) {
				clubnum = atoi(local_buf);
				local_uinfo.clubrights[clubnum / 32] |= (1 << clubnum % 32);
				memset(local_buf, 0, sizeof(local_buf));
			}
			fclose(fp1);
		}
	} else {
		memset(local_uinfo.clubrights, 0, 4 * sizeof(int));
	}

	// wwwinfo
	if (login_type == YTHTBBS_LOGIN_NJU09 || login_type == YTHTBBS_LOGIN_API || login_type == YTHTBBS_LOGIN_OAUTH) {
		local_uinfo.wwwinfo.login_start_time = local_now;

		if (strcasecmp(local_lookup_user.userid, "guest")) {
			// 非 guest
			local_uinfo.wwwinfo.t_lines = 20;
			if (readuservalue(local_lookup_user.userid, "t_lines", local_buf, sizeof(local_buf)) > 0)
				local_uinfo.wwwinfo.t_lines = atoi(local_buf);
			if (readuservalue(local_lookup_user.userid, "link_mode", local_buf, sizeof(local_buf)) >= 0)
				local_uinfo.wwwinfo.link_mode = atoi(local_buf);
			if (readuservalue(local_lookup_user.userid, "def_mode", local_buf, sizeof(local_buf)) >= 0)
				local_uinfo.wwwinfo.def_mode = atoi(local_buf);

			local_uinfo.wwwinfo.att_mode = 0;
			local_uinfo.wwwinfo.doc_mode = 1;

			if (local_uinfo.wwwinfo.t_lines < 10 || local_uinfo.wwwinfo.t_lines > 40)
				local_uinfo.wwwinfo.t_lines = 20;
		} else {
			// guest 用户
			local_uinfo.wwwinfo.t_lines  = 20;
			local_uinfo.wwwinfo.att_mode = 0;
			local_uinfo.wwwinfo.doc_mode = 1;
		}
	}

	if (local_lookup_user.userlevel & PERM_BOARDS) {
		ythtbbs_user_set_bm_status(&local_lookup_user, true /* online */, local_uinfo.invisible);
	}

	ythtbbs_cache_utmp_resolve();
	local_utmp_idx = ythtbbs_cache_utmp_insert(&local_uinfo);

	if (local_uinfo.invisible) {
		srand((unsigned)time(NULL));
		local_lookup_user.lastlogout = local_lookup_user.lastlogin + 1 + (int) (10000.0 * rand() / (RAND_MAX + 1.0)); //add by bjgyt
	} else {
		local_lookup_user.lastlogout = 0;
	}

	if (strcmp(local_lookup_user.userid, "SYSOP") == 0) {
		local_lookup_user.userlevel = ~0; /* SYSOP gets all permission bits */
		local_lookup_user.userlevel &= ~PERM_DENYMAIL; //add by wjbta
	}

	if (local_lookup_user.firstlogin == 0) {
		local_lookup_user.firstlogin = local_lookup_user.lastlogin - 7 * 86400;
	}

	substitute_record(PASSFILE, &local_lookup_user, sizeof(struct userec), user_idx + 1);

	sprintf(local_buf, "%s enter %s using %s", local_lookup_user.userid, fromhost, ythtbbs_user_get_login_type_str(login_type));
	newtrace(local_buf);

	if (out_info)
		memcpy(out_info, &local_uinfo, sizeof(struct user_info));
	if (out_userec)
		memcpy(out_userec, &local_lookup_user, sizeof(struct userec));
	if (out_utmp_idx)
		*out_utmp_idx = local_utmp_idx;

	return YTHTBBS_USER_LOGIN_OK;
}

int ythtbbs_user_logout(const char *userid, const int utmp_idx) {
	int              user_idx;
	struct userec    local_lookup_user;
	struct user_info *ptr_info;
	time_t           local_now;
	char             local_buf[128];

	ythtbbs_cache_UserTable_resolve();
	user_idx = ythtbbs_cache_UserIDHashTable_find_idx(userid);
	if (user_idx < 0)
		return YTHTBBS_USER_NOT_EXIST;

	ptr_info = ythtbbs_cache_utmp_get_by_idx(utmp_idx);
	if (strcmp(userid, ptr_info->userid) != 0)
		return YTHTBBS_USER_SESSION_ERROR;

	// ptr_info belongs to this user
	get_record(PASSFILE, &local_lookup_user, sizeof(struct userec), user_idx + 1);
	local_lookup_user.stay += time(NULL) - ptr_info->lasttime; // TODO
	local_lookup_user.userid[IDLEN + 1] = '\0';
	snprintf(local_buf, sizeof(local_buf), "%s exitbbs %ld", local_lookup_user.userid, local_lookup_user.stay);
	newtrace(local_buf);
	ythtbbs_cache_utmp_remove(utmp_idx);

	// 更新 PASSFILE 中的在线时间
	substitute_record(PASSFILE, &local_lookup_user, sizeof(struct userec), user_idx + 1);

	// TODO update bmstatus
	return 0;
}

/**
 * 参考 nju09 user_perm 实现，可以替代 HAS_PERM 宏
 */
static bool ythtbbs_user_has_perm(struct userec *x, int level) {
	return x && (x->userlevel & level);
}

static int ythtbbs_user_init_override(struct user_info *u, enum ythtbbs_override_type override_type) {
	int i;
	long total;
	long count = 0;
	int uid;
	//char buf[128];
	//FILE *fp;
	memset(u->friend, 0, sizeof(u->friend));
	total = ythtbbs_override_count(u->userid, override_type);
	if(total <= 0)
		return 0;

	if (override_type == YTHTBBS_OVERRIDE_FRIENDS) {
		if (total > MAXFRIENDS)
			total = MAXFRIENDS;
		u->fnum = total;
	} else {
		if (total > MAXREJECTS)
			total = MAXREJECTS;
		u->rnum = total;
	}

	struct ythtbbs_override *array = (struct ythtbbs_override *) calloc(total, sizeof(struct ythtbbs_override));
	// TODO: 判断 calloc 调用失败
	ythtbbs_override_get_records(u->userid, array, total, override_type);

	ythtbbs_cache_UserTable_resolve();
	for(i = 0; i < total; ++i) {
		array[i].id[IDLEN] = '\0';
		uid = ythtbbs_cache_UserIDHashTable_find_idx(array[i].id) + 1;
		if(uid) {
			count++;
			if (override_type == YTHTBBS_OVERRIDE_FRIENDS) {
				u->friend[i] = uid;
			} else {
				u->reject[i] = uid;
			}
		} else {
			array[i].id[0]=0;
		}
	}

	if (override_type == YTHTBBS_OVERRIDE_FRIENDS) {
		qsort(u->friend, total, sizeof(unsigned), (void *)ythtbbs_user_cmp_uid);
	} else {
		qsort(u->reject, total, sizeof(unsigned), (void *)ythtbbs_user_cmp_uid);
	}

	if(count != total) {
		ythtbbs_override_set_records(u->userid, array, count, override_type);
	}

	if (override_type == YTHTBBS_OVERRIDE_FRIENDS)
		u->fnum = count;
	else
		u->rnum = count;
	free(array);
	return count;
}

static int ythtbbs_user_cmp_uid(const unsigned *a, const unsigned *b) {
	return *a - *b;
}

static int ythtbbs_user_set_bm_status(const struct userec *user, bool online, bool invisible) {
	char path[256];
	sethomefile_s(path, sizeof(path), user->userid, "mboard");
	ythtbbs_user_bmfile_sync(user);
	ythtbbs_record_apply_v(path, ythtbbs_cache_Board_set_bm_hat_v, sizeof(struct boardmanager), &online, &invisible);
	return 0;
}

void ythtbbs_user_clean(void) {
	time_t local_now;
	int fd;
	int i;
	int size;
	int val;
	struct userec utmp, zerorec;
	struct stat st;
	char local_buf[128];

	local_now = time(NULL);

	// 这个值为0的结构体用于覆盖原有的记录
	memset(&zerorec, 0, sizeof(struct userec));

	// 只有当前系统启动后尚未清理过过期用户，或者距离上一次清理超过 1h，才会执行清理
	if (stat(KILLFILE, &st) == -1 || st.st_mtime < local_now - 3600) {
		if ((fd = open(KILLFILE, O_RDWR | O_CREAT, 0600)) == -1) {
			return; // 无法记录，中断操作
		}

		ctime_r(&local_now, local_buf);
		write(fd, local_buf, 25);
		close(fd);

		if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0600)) == -1) {
			return; // 无法创建 TODO 返回值
		}

		flock(fd, LOCK_EX); // 给 PASSFILE 加记录锁

		size = sizeof(struct userec);
		for (i = 0; i < MAXUSERS; i++) {
			if (read(fd, &utmp, size) != size)
				break;

			val = countlife(&utmp);
			if (utmp.userid[0] != '\0' && val < 0) {
				// userid 是合法字符，且生命力已小于 0
				utmp.userid[IDLEN + 1] = '\0';
				snprintf(local_buf, sizeof(local_buf), "system kill %s %d", utmp.userid, val);
				newtrace(local_buf);

				if (utmp.userlevel & PERM_OBOARDS) {
					// TODO retire_allBM
				}

				sethomepath_s(local_buf, sizeof(local_buf), utmp.userid);
				remove_directory(local_buf);

				setmailpath_s(local_buf, sizeof(local_buf), utmp.userid);
				remove_directory(local_buf);

				release_email(utmp.userid, utmp.email); // TODO 通过身份验证的用户顺带发送提醒邮件

				// 最后一步，清零原来的 utmp 数据
				lseek(fd, -size, SEEK_CUR);
				write(fd, &zerorec, sizeof (utmp));
			}
		}
		close(fd);
	}
}

static int remove_directory(const char *path) {
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;

	if (d) {
		struct dirent *p;

		r = 0;
		while (!r && (p = readdir(d))) {
			int r2 = -1;
			char *buf;
			size_t len;

			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;

			len = path_len + strlen(p->d_name) + 2;
			buf = malloc(len);

			if (buf) {
				struct stat statbuf;
				snprintf(buf, len, "%s/%s", path, p->d_name);
				if (!stat(buf, &statbuf)) {
					if (S_ISDIR(statbuf.st_mode))
						r2 = remove_directory(buf); // 递归调用
					else
						r2 = unlink(buf);
				}
				free(buf);
			}

			r = r2;
		}
		closedir(d);
	}

	if (!r)
		r = rmdir(path);

	return r;
}

void ythtbbs_user_touchnew(const char *userid) {
	int fd;
	char local_buf[128];

	snprintf(local_buf, sizeof(local_buf), "touch by: %s\n", userid);
	if ((fd = open(FLUSH, O_WRONLY | O_CREAT, 0644)) == -1)
		return;
	write(fd, local_buf, strlen(local_buf));
	close(fd);
}

enum ythtbbs_register_status ythtbbs_user_create(const struct userec *user, int *usernum, int *time_interval) {
	int fd, local_usernum, local_time_interval;
	struct stat st;

	if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0600)) == -1)
		return YTHTBBS_REGISTER_FILE_ERROR;
	flock(fd, LOCK_EX);

	local_usernum = ythtbbs_cache_UserTable_searchnewuser();
	if (local_usernum <= 0 || local_usernum > MAXUSERS) {
		if (stat(KILLFILE, &st) == -1)
			local_time_interval = 60;
		else
			local_time_interval = (st.st_mtime - time(NULL) + 3660) / 60 + 1;
		if (time_interval != NULL)
			*time_interval = local_time_interval;

		close(fd);
		return YTHTBBS_REGISTER_FULL;
	}

	if (lseek(fd, sizeof(struct userec) * (local_usernum - 1), SEEK_SET) == -1) {
		close(fd);
		return YTHTBBS_REGISTER_CANNOT_SEEK;
	}

	write(fd, user, sizeof(struct userec));
	close(fd);

	// 原实现中包含了 uidshm uidhashshm 的操作，这里暂不处理

	if (usernum != NULL)
		*usernum = local_usernum;
	return YTHTBBS_REGISTER_OK;
}

