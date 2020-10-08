#include "bbs.h"
#include "smth_screen.h"
#include "bbsinc.h"
#include "stuff.h"
#include "maintain.h"
#include "main.h"
#include "bbs_global_vars.h"

static int chk_editboardperm(struct boardheader *bh);
static int isExamBoard(struct boardheader *bh);

static int isExamBoard(struct boardheader *bh)
{
	if (!strcmp(bh->filename, "BM_exam") || !strcmp(bh->filename, "BM_examII") || !strcmp(bh->filename, "BM_examIII"))
		return 1;
	return 0;
}

static int
chk_editboardperm(struct boardheader *bh)
{
	if (HAS_PERM(PERM_SYSOP, currentuser) || HAS_PERM(PERM_OBOARDS, currentuser))
		return YEA;
	if (HAS_PERM(PERM_ARBITRATE, currentuser) && isExamBoard(bh))
		return YEA;
	if (!HAS_PERM(PERM_SPECIAL4, currentuser))
		return NA;
	if (issecm(bh->sec1, currentuser.userid))
		return YEA;
	return NA;
}

int
editboard(char *bname)
{
	struct boardheader bh;
	int ch;
	if (!new_search_record(BOARDS, &bh, sizeof (bh), (void *) cmpbnames, bname)) {
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		return 0;
	}
	if (!chk_editboardperm(&bh))
		return 0;
	move(0, 0);
	clear();
	// 修改 %s 版面设置
	prints("\033[1;5m\xD0\xDE\xB8\xC4 %s \xB0\xE6\xC3\xE6\xC9\xE8\xD6\xC3\033[m\n", bh.filename);
	// (A) 任命版主 (B) 版主离职 请选择:
	prints("(A) \xC8\xCE\xC3\xFC\xB0\xE6\xD6\xF7 (B) \xB0\xE6\xD6\xF7\xC0\xEB\xD6\xB0 \xC7\xEB\xD1\xA1\xD4\xF1: ");
	ch = toupper(egetch());
	if (ch == 'A')
		do_ordainBM(NULL, bh.filename);
	else if (ch == 'B')
		do_retireBM(NULL, bh.filename);
	return 0;
}
