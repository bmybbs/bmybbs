#include "bbslib.h"

int
bbscccmail_main()
{
	struct fileheader *x = NULL;
	struct boardmem *brd;
	char file[80], target[80];
	char dir[80];
	struct mmapfile mf = { ptr:NULL };
	int num;
	html_header(1);
	check_msg();
	strsncpy(file, getparm("F"), 30);
	if (!file[0])
		strsncpy(file, getparm("file"), 30);
	strsncpy(target, getparm("target"), 30);
	if (!loginok || isguest)
		http_fatal("´Ò´Ò¹ı¿Í²»ÄÜ½øĞĞ±¾Ïî²Ù×÷");
	changemode(POSTING);
	sprintf(dir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]), currentuser.userid);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("´íÎóµÄÌÖÂÛÇø»òÌÖÂÛÇøÎª¿Õ");
		}
		num = -1;
		x = findbarticle(&mf, file, &num, 1);
	}
	MMAP_CATCH {
		x = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (x == 0)
		http_fatal("´íÎóµÄÎÄ¼şÃû");
	printf("<center>%s -- ×ªÔØÎÄÕÂ [Ê¹ÓÃÕß: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	if (target[0]) {
		brd = getboard(target);
		if (brd == 0)
			http_fatal("´íÎóµÄÌÖÂÛÇøÃû³Æ»òÄãÃ»ÓĞÔÚ¸Ã°æ·¢ÎÄµÄÈ¨ÏŞ");
		if (!has_post_perm(&currentuser, brd))
			http_fatal("´íÎóµÄÌÖÂÛÇøÃû³Æ»òÄãÃ»ÓĞÔÚ¸Ã°æ·¢ÎÄµÄÈ¨ÏŞ");
		if (noadm4political(target))
			http_fatal
			    ("¶Ô²»Æğ,ÒòÎªÃ»ÓĞ°æÃæ¹ÜÀíÈËÔ±ÔÚÏß,±¾°æÔİÊ±·â±Õ.");
		return do_cccmail(x, brd);
	}
	printf("<table><tr><td>\n");
	printf("<font color=red>×ªÌù·¢ÎÄ×¢ÒâÊÂÏî:<br>\n");
	printf
	    ("±¾Õ¾¹æ¶¨Í¬ÑùÄÚÈİµÄÎÄÕÂÑÏ½ûÔÚ3(²»º¬)¸öÒÔÉÏÌÖÂÛÇøÖØ¸´ÕÅÌù¡£");
	printf("Î¥Õß½«°ş¶áÈ«Õ¾·¢±íÎÄÕÂµÄÈ¨Àû¡£<br><br></font>\n");
	printf("ÎÄÕÂ±êÌâ: %s<br>\n", nohtml(x->title));
	printf("ÎÄÕÂ×÷Õß: %s<br>\n", fh2owner(x));
	printf("ÎÄÕÂ³ö´¦: %s µÄĞÅÏä<br>\n", currentuser.userid);
	printf("<form action=bbscccmail method=post>\n");
	printf("<input type=hidden name=file value=%s>", file);
	printf("×ªÔØµ½ <input name=target size=30 maxlength=30> ÌÖÂÛÇø. ");
	printf("<input type=submit value=È·¶¨></form>");
	return 0;
}

int
do_cccmail(struct fileheader *x, struct boardmem *brd)
{
	FILE *fp, *fp2;
	char board[80], title[512], buf[512], path[200], path2[200],
	    i;
	char tmpfn[80];
	int retv;
	int mark = 0;
	strcpy(board, brd->header.filename);
	sprintf(path, "mail/%c/%s/%s", mytoupper(currentuser.userid[0]), currentuser.userid, fh2fname(x));
	if (brd->header.flag & IS1984_FLAG)
		http_fatal("¸Ã°æÃæ½ûÖ¹×ªÔØ");
	fp = fopen(path, "r");
	if (fp == 0)
		http_fatal("ĞÅ¼şÄÚÈİÒÑ¶ªÊ§, ÎŞ·¨×ªÔØ");
	sprintf(path2, "bbstmpfs/tmp/%d.tmp", thispid);
	fp2 = fopen(path2, "w");
	if (fgets(buf, 256, fp) != 0) {
		if (!strncmp(buf, "·¢ĞÅÈË: ", 8) || !strncmp(buf, "¼ÄĞÅÈË: ", 8)) {
			for (i = 0; i < 6; i++) {
				if (fgets(buf, 256, fp) == 0)
					break;
				if (buf[0] == '\n')
					break;
			}
		}
		else
			rewind(fp);
	}
	fprintf(fp2,
		"\033[m\033[1m¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô \033[32m%s \033[m\033[1mµÄĞÅÏä ¡¿\n",
		currentuser.userid);
	fprintf(fp2, "[37;1m¡¾ Ô­ÎÄÓÉ [32m%s [37mÓÚ [32m%s [37m·¢±í ¡¿[m\n\n",
		fh2owner(x) ,Ctime(x->filetime));
	while (1) {
		retv = fread(buf, 1, sizeof (buf), fp);
		if (retv <= 0)
			break;
		fwrite(buf, 1, retv, fp2);
	}
	fclose(fp);
	fclose(fp2);
	if (!strncmp(x->title, "[×ªÔØ]", 6)) {
		strsncpy(title, x->title, sizeof (title));
	} else {
		sprintf(title, "[×ªÔØ] %.55s", x->title);
	}
	sprintf(tmpfn, "bbstmpfs/tmp/filter.%s.%d", currentuser.userid, thispid);
	copyfile(path2, tmpfn);
	filter_attach(tmpfn);
	if (dofilter(title, tmpfn, 2)) {
	#ifdef POST_WARNING
		char mtitle[256];
		sprintf(mtitle, "[×ªÔØ±¨¾¯] %s %.60s", board, title);
		mail_file(path2, "delete", mtitle, currentuser->userid);
		updatelastpost("deleterequest");
	#endif
		mark |= FH_DANGEROUS;
	}
	unlink(tmpfn);
	post_article(board, title, path2, currentuser.userid,
		     currentuser.username, fromhost, -1, mark, 0,
		     currentuser.userid, -1);
	unlink(path2);
	printf("'%s' ÒÑ×ªÌùµ½ %s °æ.<br>\n", nohtml(title), board);
	printf("[<a href='javascript:history.go(-2)'>·µ»Ø</a>]");
	return 0;
}

