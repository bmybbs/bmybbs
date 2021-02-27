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
	"oicq号码",
	"常用Email",
	"固定电话",
	"呼机/手机",
	"通讯地址",
	"单位/学校"
};
static const char openlevel[4][16] = {
	"所有人",
	"以我为友者",
	"我的好友",
	"不公开"
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
		getdata(0, 0, "(A)查询通讯录 (B)设置通讯录 (Q)离开 [Q]: ",
			buf, 3, DOECHO, YEA);
		switch (buf[0]) {
		case 'A':
		case 'a':
			usercomplete("请输入使用者代号: ", buf);
			if (!buf[0])
				continue;
			prints("%s的通讯录内容如下\n", buf);
			prints("=================================\n");
			sethomefile(filename, buf, "addressbook");
			if (!dashf(filename)) {
				prints("该用户尚未设置通讯录");
				pressanykey();
				continue;
			}
			prints("项目       内容\n");
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
			prints("编辑通讯录(\033[1;31m小心被滥用\033[m)\n");
			prints("=================================\n");
			prints("项目       公开级别   内容\n");
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
					"公开级别 (0)所有人 (1)与我为友者 (2)我的好友 (3)不公开 [%c]",
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
				"以上资料是否正确, 按 Q 放弃 (Y/Quit)? [Y]:",
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
		prints("%-10s 尚未设定\n", addressitems[i]);
	} else if (mode == -1) {
		prints("%-10s %-10s %s\n", addressitems[i], openlevel[item[0] - '0'], item + 1);
	} else {
		prints("%-10s %s\n", addressitems[i], (mode >= item[0] - '0') ? (item + 1) : "保密");
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
	fp = fopen(filename, "w");
	for (i = 0; i < NADDRESSITEM; i++) {
		fprintf(fp, "%s\n", items[i]);
	}
	fclose(fp);
}
