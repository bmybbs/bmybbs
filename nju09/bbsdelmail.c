#include "bbslib.h"

int
bbsdelmail_main()
{
	FILE *fp, *fpw;
	struct fileheader f;
	char path[80], tmppath[80], file[80], *ptr, list[40][20];
	int num, ndelfile = 0;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录");
	changemode(RMAIL);

	strsncpy(list[ndelfile], getparm("file"), 20);
	if (list[ndelfile][0] == 'M')
		ndelfile++;
	for (num = 0; num < 40 && ndelfile < 40; num++) {
		sprintf(file, "F%d", num);
		strsncpy(list[ndelfile], getparm(file), 20);
		if (list[ndelfile][0] == 'M' || list[ndelfile][0] == 'G')
			ndelfile++;
	}

	setmailfile(path, currentuser.userid, ".DIR");
	setmailfile(tmppath, currentuser.userid, ".DIR.tmp");
	fp = fopen(path, "r");
	if (fp == 0)
		http_fatal("错误的参数2");
	fpw = fopen(tmppath, "w");
	if (fpw == 0) {
		fclose(fp);
		http_fatal("无法删除邮件");
	}
	while (1) {
		if (fread(&f, sizeof (f), 1, fp) <= 0)
			break;
		ptr = fh2fname(&f);
		for (num = 0; num < ndelfile; num++) {
			if (!strcmp(ptr, list[num]))
				break;
		}
		if (num == ndelfile) {
			fwrite(&f, sizeof (f), 1, fpw);
			continue;
		}
		setmailfile(file, currentuser.userid, ptr);
		unlink(file);
	}
	fclose(fp);
	fclose(fpw);
	rename(tmppath, path);
	printf("信件已删除.<br><a href=bbsmail>返回所有信件列表</a>\n");
	http_quit();
	return 0;
}
