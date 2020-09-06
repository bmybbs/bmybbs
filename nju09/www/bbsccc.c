#include "bbslib.h"

int
bbsccc_main()
{
	struct fileheader *x = NULL;
	struct boardmem *brd, *brd1;
	char board[80], file[80], target[80];
	char dir[80];
	struct mmapfile mf = { ptr:NULL };
	int num;
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 30);
	strsncpy(file, getparm("F"), 30);
	if (!file[0])
		strsncpy(file, getparm("file"), 30);
	strsncpy(target, getparm("target"), 30);
	if (!loginok || isguest)
		http_fatal("´Ò´Ò¹ı¿Í²»ÄÜ½øĞĞ±¾Ïî²Ù×÷");
	if (!(brd1=getboard(board)))
		http_fatal("´íÎóµÄÌÖÂÛÇø");
	changemode(POSTING);
	sprintf(dir, "boards/%s/.DIR", board);
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
		return do_ccc(x, brd1, brd);
	}
	printf("<table><tr><td>\n");
	printf("<font color=red>×ªÌù·¢ÎÄ×¢ÒâÊÂÏî:<br>\n");
	printf
	    ("±¾Õ¾¹æ¶¨Í¬ÑùÄÚÈİµÄÎÄÕÂÑÏ½ûÔÚ 4 ¸ö»ò 4 ¸öÒÔÉÏÌÖÂÛÇøÄÚÖØ¸´·¢±í¡£");
	printf("Î¥Õß½«±»·â½ûÔÚ±¾Õ¾·¢ÎÄµÄÈ¨Àû<br><br></font>\n");
	printf("ÎÄÕÂ±êÌâ: %s<br>\n", nohtml(x->title));
	printf("ÎÄÕÂ×÷Õß: %s<br>\n", fh2owner(x));
	printf("Ô­ÌÖÂÛÇø: %s<br>\n", board);
	printf("<form action=bbsccc method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf("×ªÔØµ½ <input name=target size=30 maxlength=30> ÌÖÂÛÇø. ");
	printf("<input type=submit value=È·¶¨></form>");
	return 0;
}

int
do_ccc(struct fileheader *x, struct boardmem *brd1, struct boardmem *brd)
{
	FILE *fp, *fp2;
	char board2[80], board[80], title[512], buf[512], path[200], path2[200], i;
	int hide1, hide2, retv;
	int mark = 0;
	strcpy(board2, brd->header.filename);
	strcpy(board, brd1->header.filename);
	sprintf(path, "boards/%s/%s", board, fh2fname(x));
	if (brd->header.flag & IS1984_FLAG)
		http_fatal("¸Ã°æÃæ½ûÖ¹×ªÔØ");
	hide1 = hideboard_x(brd1);
	hide2 = hideboard_x(brd);
	if (hide1 && !hide2)
		http_fatal("·Ç·¨×ªÔØ");
	fp = fopen(path, "r");
	if (fp == 0)
		http_fatal("ÎÄ¼şÄÚÈİÒÑ¶ªÊ§, ÎŞ·¨×ªÔØ");
	sprintf(path2, "bbstmpfs/tmp/%d.tmp", thispid);
	fp2 = fopen(path2, "w");
	for (i = 0; i < 3; i++)
		if (fgets(buf, 256, fp) == 0)
			break;
	fprintf(fp2, "[37;1m¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô [32m%s [37mÌÖÂÛÇø ¡¿\n",
		board);
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
	if (dofilter(title, path2, 2)) {
		char mtitle[256];
		sprintf(mtitle, "[×ªÔØ±¨¾¯] %s %.60s", board, title);
		post_mail("delete", mtitle, path2,
			  currentuser.userid, currentuser.username,
			  fromhost, -1, 0);
		updatelastpost("deleterequest");
		mark |= FH_DANGEROUS;
	}
	post_article(board2, title, path2, currentuser.userid,
		     currentuser.username, fromhost, -1, mark, 0,
		     currentuser.userid, -1);
	unlink(path2);
	printf("'%s' ÒÑ×ªÌùµ½ %s °æ.<br>\n", nohtml(title), board2);
	printf("[<a href='javascript:history.go(-2)'>·µ»Ø</a>]");
	return 0;
}
