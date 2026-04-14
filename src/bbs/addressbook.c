#include "bbs.h"
#include "ythtbbs/override.h"
#include "xyz.h"
#include "io.h"
#include "smth_screen.h"
#include "stuff.h"
#include "namecomplete.h"
#include "bbs_global_vars.h"
#define NADDRESSITEM 6
static const char addressitems[NADDRESSITEM][16] = {
	// oicq号码
	"oicq\xBA\xC5\xC2\xEB",
	// 常用Email
	("\xB3\xA3\xD3\xC3" "Email"),
	// 固定电话
	"\xB9\xCC\xB6\xA8\xB5\xE7\xBB\xB0",
	// 呼机/手机
	"\xBA\xF4\xBB\xFA/\xCA\xD6\xBB\xFA",
	// 通讯地址
	"\xCD\xA8\xD1\xB6\xB5\xD8\xD6\xB7",
	// 单位/学校
	"\xB5\xA5\xCE\xBB/\xD1\xA7\xD0\xA3"
};
static const char openlevel[4][16] = {
	// 所有人
	"\xCB\xF9\xD3\xD0\xC8\xCB",
	// 以我为友者
	"\xD2\xD4\xCE\xD2\xCE\xAA\xD3\xD1\xD5\xDF",
	// 我的好友
	"\xCE\xD2\xB5\xC4\xBA\xC3\xD3\xD1",
	// 不公开
	"\xB2\xBB\xB9\xAB\xBF\xAA"
};
static int addressbookmode(char *me, char *him);
static void showaddressitem(char *item, int i, int mode);
static void showaddressbook(char items[][STRLEN], int mode);
static void readaddressbook(char *filename, char items[][STRLEN]);
static void saveaddressbook(char *filename, char items[][STRLEN]);

int addressbook(const char *s) {
	(void) s;
	char str[STRLEN], buf[STRLEN], filename[STRLEN], items[NADDRESSITEM][STRLEN];
	int i;
	modify_user_mode(ADDRESSBOOK);
	while (1) {
		move(0, 0);
		clrtobot();
		// (A)查询通讯录 (B)设置通讯录 (Q)离开 [Q]:
		getdata(0, 0, "(A)\xB2\xE9\xD1\xAF\xCD\xA8\xD1\xB6\xC2\xBC (B)\xC9\xE8\xD6\xC3\xCD\xA8\xD1\xB6\xC2\xBC (Q)\xC0\xEB\xBF\xAA [Q]: ",
			buf, 3, DOECHO, YEA);
		switch (buf[0]) {
		case 'A':
		case 'a':
			// 请输入使用者代号:
			usercomplete("\xC7\xEB\xCA\xE4\xC8\xEB\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5: ", buf);
			if (!buf[0])
				continue;
			// %s的通讯录内容如下\n
			prints("%s\xB5\xC4\xCD\xA8\xD1\xB6\xC2\xBC\xC4\xDA\xC8\xDD\xC8\xE7\xCF\xC2\n", buf);
			prints("=================================\n");
			sethomefile_s(filename, sizeof(filename), buf, "addressbook");
			if (!dashf(filename)) {
				// 该用户尚未设置通讯录
				prints("\xB8\xC3\xD3\xC3\xBB\xA7\xC9\xD0\xCE\xB4\xC9\xE8\xD6\xC3\xCD\xA8\xD1\xB6\xC2\xBC");
				pressanykey();
				continue;
			}
			// 项目       内容\n
			prints("\xCF\xEE\xC4\xBF       \xC4\xDA\xC8\xDD\n");
			readaddressbook(filename, items);
			move(5, 0);
			clrtobot();
			showaddressbook(items,
					addressbookmode(currentuser.userid,
							buf));
			pressanykey();
			break;
		case 'B':
		case 'b':
			sethomefile_s(filename, sizeof(filename), currentuser.userid, "addressbook");
			readaddressbook(filename, items);
			// 编辑通讯录(\033[1;31m小心被滥用\033[m)\n
			prints("\xB1\xE0\xBC\xAD\xCD\xA8\xD1\xB6\xC2\xBC(\033[1;31m\xD0\xA1\xD0\xC4\xB1\xBB\xC0\xC4\xD3\xC3\033[m)\n");
			prints("=================================\n");
			// 项目       公开级别   内容\n
			prints("\xCF\xEE\xC4\xBF       \xB9\xAB\xBF\xAA\xBC\xB6\xB1\xF0   \xC4\xDA\xC8\xDD\n");
			for (i = 0; i < NADDRESSITEM; i++) {
				move(4, 0);
				clrtobot();
				showaddressbook(items, -1);
				move(NADDRESSITEM + 4, 0);
				prints("=================================\n");
				showaddressitem(items[i], i, -1);
				sprintf(str, "%s: ", addressitems[i]);
				getdata(NADDRESSITEM + 5, 0, str, items[i] + 1,
					60, DOECHO, items[i][0] ? NA : YEA);
				if (items[i][0] < '0' || items[i][0] > '3')
					items[i][0] = '3';
				sprintf(buf,
					// 公开级别 (0)所有人 (1)与我为友者 (2)我的好友 (3)不公开 [%c]
					"\xB9\xAB\xBF\xAA\xBC\xB6\xB1\xF0 (0)\xCB\xF9\xD3\xD0\xC8\xCB (1)\xD3\xEB\xCE\xD2\xCE\xAA\xD3\xD1\xD5\xDF (2)\xCE\xD2\xB5\xC4\xBA\xC3\xD3\xD1 (3)\xB2\xBB\xB9\xAB\xBF\xAA [%c]",
					items[i][0]);
				if (items[i][1])
					getdata(NADDRESSITEM + 6, 0, buf, str,
						3, DOECHO, YEA);
				if (str[0] < '0' || str[0] > '3')
					str[0] = items[i][0];
				items[i][0] = str[0];
			}
			move(3, 0);
			clrtobot();
			showaddressbook(items, 1);
			prints("=================================\n");
			getdata(NADDRESSITEM + 5, 0,
				// 以上资料是否正确, 按 Q 放弃 (Y/Quit)? [Y]:
				"\xD2\xD4\xC9\xCF\xD7\xCA\xC1\xCF\xCA\xC7\xB7\xF1\xD5\xFD\xC8\xB7, \xB0\xB4 Q \xB7\xC5\xC6\xFA (Y/Quit)? [Y]:",
				str, 3, DOECHO, YEA);
			if (str[0] != 'Q' || str[0] == 'q')
				saveaddressbook(filename, items);
			break;
		default:
			return FULLUPDATE;
		}
	}
}

static int
addressbookmode(char *me, char *him)
{
	if (!strcasecmp(me, him))
		return 3;
	if (ythtbbs_override_included(me, YTHTBBS_OVERRIDE_FRIENDS, him))
		return 2;
	if (ythtbbs_override_included(him, YTHTBBS_OVERRIDE_FRIENDS, me))
		return 1;
	return 0;
}

static void
showaddressitem(char *item, int i, int mode)
{
	if (item[0] == 0) {
		// %-10s 尚未设定\n
		prints("%-10s \xC9\xD0\xCE\xB4\xC9\xE8\xB6\xA8\n", addressitems[i]);
	} else if (mode == -1) {
		prints("%-10s %-10s %s\n", addressitems[i], openlevel[item[0] - '0'], item + 1);
	} else {
		// 保密
		prints("%-10s %s\n", addressitems[i], (mode >= item[0] - '0') ? (item + 1) : "\xB1\xA3\xC3\xDC");
	}
}

static void
showaddressbook(char items[][STRLEN], int mode)
{
	int i;
	for (i = 0; i < NADDRESSITEM; i++)
		showaddressitem(items[i], i, mode);
}

static void
readaddressbook(char *filename, char items[][STRLEN])
{
	int i = 0;
	FILE *fp;
	char *ptr;
	fp = fopen(filename, "r");
	if (fp != NULL)
		for (i = 0; i < NADDRESSITEM; i++) {
			if (fgets(items[i], STRLEN, fp) == NULL)
				break;
			if ((ptr = strrchr(items[i], '\n')) != NULL)
				*ptr = 0;
		}
	for (; i < NADDRESSITEM; i++)
		items[i][0] = 0;
	if (fp != NULL)
		fclose(fp);
}

static void
saveaddressbook(char *filename, char items[][STRLEN])
{
	int i;
	FILE *fp;
	if ((fp = fopen(filename, "w")) != NULL) {
		for (i = 0; i < NADDRESSITEM; i++) {
			fprintf(fp, "%s\n", items[i]);
		}
		fclose(fp);
	}
}
