/*-------------------------------------------------------*/
/* util/2nd_expire.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : 跳蚤市场布告删除程式			 */
/* create : 00/05/05				 	 */
/* update : 						 */
/*-------------------------------------------------------*/
/* author : Ernie.bbs@bbs.cs.nthu.edu.tw		 */
/*-------------------------------------------------------*/

#include "bbs.h"
#include "sec_hand.h"
#define EXP 90
int
report(err)
char *err;
{
	return printf(err);
}

void
expire_grp()
{
	int pos, fd;
	char fgrp[80], fold[80], fnew[80];
	char buf[200];
	SLOT grp;
	pos = 1;
	sprintf(fgrp, "%s/2nd/%s", MY_BBS_HOME, FN_GRP);
	sprintf(fnew, "%s/2nd/%s.new", MY_BBS_HOME, FN_GRP);
	fd = open(fnew, O_CREAT | O_TRUNC, 0600);	// touch a new file 
	if (fd > 0)
		close(fd);
	while (get_record(fgrp, &grp, sizeof (SLOT), pos) == 0) {
		if (grp.prop & PROP_G_CANCEL) {
			sprintf(fold, "2nd/%s", grp.fn);
			sprintf(buf,
				"/bin/rm -fr %s/%s 1>/dev/null 2>/dev/null",
				MY_BBS_HOME, fold);
			system(buf);
		} else {
			append_record(fnew, &grp, sizeof (SLOT));
		}
		pos++;
	}
	sprintf(fold, "%s/2nd/%s.old", MY_BBS_HOME, FN_GRP);
	sprintf(buf, "cp %s %s 1>/dev/null 2>/dev/null", fgrp, fold);
	system(buf);
	sprintf(buf, "mv %s %s 1>/dev/null 2>/dev/null", fnew, fgrp);
	system(buf);
	return;
}

void
expire_item()
{

	int pos, pos1, fd;
	char fgrp[80], fitem[80], fname[80], fold[80], fnew[80];
	char buf[200];
	time_t expire;
	SLOT grp, item;
	expire = time(0) - EXP * 86400;
	pos = pos1 = 1;
	sprintf(fgrp, "%s/2nd/%s", MY_BBS_HOME, FN_GRP);
	while (get_record(fgrp, &grp, sizeof (SLOT), pos) == 0) {
		sprintf(fitem, "%s/2nd/%s/%s", MY_BBS_HOME, grp.fn, FN_ITEM);
		-sprintf(fnew, "%s/2nd/%s/%s.new", MY_BBS_HOME, grp.fn,
			 FN_ITEM);
		fd = open(fnew, O_CREAT | O_TRUNC, 0600);	// touch a new file 
		if (fd > 0)
			close(fd);
		fd = 0;
		while (get_record(fitem, &item, sizeof (SLOT), pos1) == 0) {
			if ((item.prop & PROP_I_CANCEL)
			    || item.chrono < expire) {
				sprintf(fname, "%s/2nd/%s/%s", MY_BBS_HOME,
					grp.fn, item.fn);
				unlink(fname);
			} else {
				fd++;
				append_record(fnew, &item, sizeof (SLOT));
			}
			pos1++;
		}
		sprintf(fold, "%s/2nd/%s/%s.old", MY_BBS_HOME, grp.fn, FN_ITEM);
		sprintf(buf, "cp %s %s 1>/dev/null 2>/dev/null", fitem, fold);
		system(buf);
		sprintf(buf, "mv %s %s 1>/dev/null 2>/dev/null", fnew, fitem);
		system(buf);
		grp.reply = fd;
		substitute_record(fgrp, &grp, sizeof (SLOT), pos);
		pos++;
		pos1 = 1;
	}
	return;
}

int
main()
{
	expire_grp();
	expire_item();
	return 0;
}
