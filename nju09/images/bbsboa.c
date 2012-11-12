#include "bbslib.h"
//#include "struct.h"
#define COMMENDFILE     MY_BBS_HOME"/.COMMEND"

void show_banner();
int show_commend();
int show_content();
void show_sec(struct sectree *sec);
void show_boards(char* secstr);
void show_sec_boards(struct boardmem *(data[]), int total);
void show_top10();
int show_manager_team();

int
bbsboa_main()
{
	struct boardmem *(data[MAXBOARD]), *x;
	int i, total = 0;
	char *secstr;
	const struct sectree *sec;
	//int show_Commend();
	int hasintro = 0, len;

	secstr = getparm("secstr");
	sec = getsectree(secstr);
	if (secstr[0] != '*') {
		    if (cache_header
		    (max(thisversion, file_time(MY_BBS_HOME "/wwwtmp")), 120))
			return 0;
	}
	html_header(1);
	check_msg();
	//printf("<style type=text/css>A {color: #0000f0}</style>");
	changemode(SELECT);
	if (secstr[0] == '*') {
		readmybrd(currentuser.userid);
		for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
			x = &(shm_bcache->bcache[i]);
			if (x->header.filename[0] <= 32
			    || x->header.filename[0] > 'z')
				continue;
			if (!has_read_perm_x(&currentuser, x))
				continue;
			if (!ismybrd(x->header.filename))
				continue;
			data[total] = x;
			total++;
		}
		printf("<body><center>\n");
		printf("%s -- 预定讨论区总览<hr>", BBSNAME);
		if (total)
			showboardlist(data, total, "*");
		printf("<hr>");
		return 0;
	}
	if(!strcmp(secstr,"?")){
		printf("<script src=\"/inc/tog.js\"></script></head><body leftmargin=0 topmargin=0>\n");
		show_banner();
		show_content();
		goto out;
	}
	len = strlen(secstr);
	if (sec->introstr[0])
		hasintro = 1; 
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->header.filename[0] <= 32 || x->header.filename[0] > 'z')
			continue;
		if (hasintro) {
			if (strcmp(secstr, x->header.sec1) &&
			    strcmp(secstr, x->header.sec2)) continue;
		} else {
			if (strncmp(secstr, x->header.sec1, len) &&
			    strncmp(secstr, x->header.sec2, len))
				continue;
		}
		if (!has_read_perm_x(&currentuser, x))
			continue;
		data[total] = x;
		total++;
	}
	printf("<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0>"); 
	showsecpage(sec, data, total, secstr); 
out:
	printf("</body></html>");
	return 0;
}

int
showsecpage(const struct sectree *sec, struct boardmem *(data[]), int total,
	    char *secstr)
{
	FILE *fp;
	char buf[1024];
	sprintf(buf, "wwwtmp/secpage.sec%s", sec->basestr);
	fp = fopen(buf, "rt");
	if (!fp)
		return showdefaultsecpage(sec, data, total, secstr);
	while (fgets(buf, sizeof (buf), fp)) {
		if (buf[0] != '#') {
			fputs(buf, stdout);
			continue;
		}
		if (!strncmp(buf, "#showfile ", 10)) {
			char *ptr;
			ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			showfile(buf + 10);
		} else if (!strncmp(buf, "#showblist", 10)) {
			if (total)
				showboardlist(data, total, secstr);
		} else if (!strncmp(buf, "#showsecintro", 13))
			showsecintro(sec);
		else if (!strncmp(buf, "#showsecnav", 11))
			showsecnav(sec);
		else if (!strncmp(buf, "#showstarline ", 14))
			showstarline(buf + 14);
		else if (!strncmp(buf, "#showhotboard", 13))
			showhotboard(sec, buf + 13);
		else if (!strncmp(buf, "#showsechead", 12))
			showsechead(sec);
		else if (!strncmp(buf, "#showsecmanager", 8))
			showsecmanager(sec);
	}
	fclose(fp);
	return 0;
}

int
showdefaultsecpage(const struct sectree *sec, struct boardmem *(data[]),
		   int total, char *secstr)
{	//modify by mintbaggio 040522 for new www
//	printf("<center>");
	printf("<body leftmargin=0 topmargin=0>\n");
//	showsechead(sec);
	printf("<tr><td height=30 colspan=2></td></tr>\n"
		"<tr><td height=70 colspan=2><table width=\"100%\" border=0 cellpadding=0 cellspacing=0 bgcolor=#efefef>\n"
		"<tr><td width=40>&nbsp;</td>\n");
	printf("<td width=400 height=70><a href=\"boa?secstr=%s\" class=N0030>%s </a> </td>", sec->basestr, nohtml(sec->title));
//	printf("<h2>%s</h2>", nohtml(sec->title));
	printf("%s", "<td width=300><table width=164 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td><input name=textfield type=text style=\"font-size:11px;font-family:verdana;\" size=20>\n"
	"</td>\n<td width=102><input name=Submit type=button class=2014 value=Search></td>\n"
	"<td width=102 height=20>&nbsp;</td></tr></table></td></tr>\n");
//	printf("<div align=right>");
//	showsecmanager(sec);
//	printf("</div>");
//	printf("<hr>");

	showsecintro(sec);
	if (total) {
		showboardlist(data, total, secstr);
//		printf("<hr>");
	}
//	printf("</center>");
	printf("</body></html>\n");
	return 0;
}

int
showsechead(const struct sectree *sec)
{
	const struct sectree *sec1, *sec2;
	int i;
	sec2 = sec;
	while (sec2->parent && !sec2->introstr[0])
		sec2 = sec2->parent;
	printf("<table border=1 bgcolor=%s><tr>", currstyle->colortb1);
	if (sec == &sectree)
		printf("<td align=center>&nbsp;<b>%s</b>&nbsp;</td>",
		       nohtml(sectree.title));
	else if (sec2 == &sectree)
		printf
		    ("<td align=center>&nbsp;<b><a href=boa?secstr=?>%s</a></b>&nbsp;</td>",
		     nohtml(sectree.title));
	else
		printf
		    ("<td align=center>&nbsp;<a href=boa?secstr=?>%s</a>&nbsp;</td>",
		     nohtml(sectree.title));
	for (i = 0; i < sectree.nsubsec; i++) {
		sec1 = sectree.subsec[i];
		if (!sec1->introstr[0])
			continue;
		if (sec1 == sec)
			printf("<td align=center>&nbsp;<b>%s</b>&nbsp;</td>",
			       nohtml(sec1->title));
		else if (sec1 == sec2)
			printf
			    ("<td align=center>&nbsp;<b><a href=boa?secstr=%s>%s</a></b>&nbsp;</td>",
			     sec1->basestr, nohtml(sec1->title));
		else
			printf
			    ("<td align=center>&nbsp;<a href=boa?secstr=%s>%s</a>&nbsp;</td>",
			     sec1->basestr, nohtml(sec1->title));
	}
	printf("</tr></table>");
	return 0;
}

int
showstarline(char *str)
{
	printf("<tr><td class=tb2_blk><font class=star>★</font>"
	       "&nbsp;%s</td></tr>", str);
	return 0;
}

int
showsecnav(const struct sectree *sec)
{
	char buf[256];
	printf("<table width=100%%>");
	sprintf(buf,
		"近日精彩话题推荐 &nbsp;(<a href=bbsshownav?secstr=%s class=blk>"
		"查看全部</a>)", sec->basestr);
	showstarline(buf);
	printf("<tr><td>");
	shownavpart(0, sec->basestr);
	printf("</td></tr></table>");
	return 0;
}

int
showhotboard(const struct sectree *sec, char *s)
{
	int count = 0, i, j, len, max;
	struct boardmem *bmem[MAXBOARD], *x, *x1;
	max = atoi(s);
	if (max < 3 || max > 30)
		max = 10;
	len = strlen(sec->basestr);
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->header.filename[0] <= 32 || x->header.filename[0] > 'z')
			continue;
		if (hideboard_x(x))
			continue;
		if (strncmp(sec->basestr, x->header.sec1, len) &&
		    strncmp(sec->basestr, x->header.sec2, len))
			continue;
		for (j = 0; j < count; j++) {
			if (x->score > bmem[j]->score)
				break;
			if (x->score == bmem[j]->score
			    && x->inboard > bmem[j]->inboard) break;
		}
		for (; j < count; j++) {
			x1 = bmem[j];
			bmem[j] = x;
			x = x1;
		}
		if (count < max)
			bmem[count++] = x;
	}
	printf
	    ("<table width=588 border=1><tr><td bgcolor=%s width=55 align=center>热门讨论区推荐</td><td>",
	     currstyle->colortb1);
	for (i = 0; i < count; i++) {
		if (i)
			printf("%s", " &nbsp;");
		printf("<a href=home?B=%s class=pur><u>%s</u></a>",
		       bmem[i]->header.filename,
		       void1(nohtml(bmem[i]->header.title)));
	}
	printf("</td></tr></table>");
	return 0;
}

int
showfile(char *fn)
{
	struct mmapfile mf = { ptr:NULL };
	MMAP_TRY {
		if (mmapfile(fn, &mf) < 0) {
			MMAP_RETURN(-1);
		}
		fwrite(mf.ptr, 1, mf.size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

int
showsecintro(const struct sectree *sec)
{
	char filename[80];
	int i;
	if (!sec->introstr[0])
		return -1;
	sprintf(filename, "wwwtmp/lastmark.sec%s", sec->basestr);
	if (showfile(filename) < 0) {
		for (i = 0; i < sec->nsubsec; i++) {
			printf
			    ("<li><a href=bbsboa?secstr=%s>%s</a>",
			     sec->subsec[i]->basestr, sec->subsec[i]->title);
		}
	}
	return 0;
}

int
showboardlist(struct boardmem *(data[]), int total, char *secstr)
{
	char *cgi = "home", bmbuf[IDLEN * 4 + 4], *ptr;
	int sortmode, i;
	sortmode = atoi(getparm("sortmode"));
	if (sortmode <= 0 || sortmode > 3)
		sortmode = 2;
	if (w_info->def_mode)
		cgi = "tdoc";
	switch (sortmode) {
	case 1:
		qsort(data, total, sizeof (struct boardmem *),
		      (void *) cmpboard);
		break;
	case 2:
		qsort(data, total,
		      sizeof (struct boardmem *), (void *) cmpboardscore);
		break;
	case 3:
		qsort(data, total,
		      sizeof (struct boardmem *), (void *) cmpboardinboard);
		break;
	}
	/*printf("<table cellspacing=0 cellpadding=2 border=0>\n");
	printf
	    ("<tr><td><a href=boa?secstr=%s&sortmode=1>讨论区名称</a></td>"
	     "<td>V</d><td>类别</td><td>中文描述　　　&nbsp;</td><td>版主</td><td>文章数</td>"
	     "<td><a href=boa?secstr=%s&sortmode=2>人气</a></td>"
	     "<td><a href=boa?secstr=%s&sortmode=3>在线</td></tr>\n",
	     secstr, secstr, secstr);*/
	printf("%s", "<tr><td width=40 bgcolor=#FFFFFF>&nbsp;</td>\n"
		"<td colspan=2 bgcolor=#FFFFFF><TABLE width=\"90%\" border=0 cellPadding=2 cellSpacing=0>\n"
		"<TBODY>\n");

	printf("<TR>\n<TD class=N0004>未</TD>\n"
		"<TD class=N0003><a href=\"boa?secstr=%s\" class=N0010>讨论区名称</a></TD>\n"
		"<TD class=N0004>V\n"
		"<TD class=N0004>类别</TD>\n"
		"<TD class=N0004>中文描述</TD>\n"
		"<TD class=N0004>版主</TD>\n"
		"<TD class=N0004>文章数</TD>\n"
		"<TD class=N0004><a href=\"boa?secstr=%s&sortmode=2\" class=N0001>人气</a></TD>\n"
		"<TD class=N0004><a href=\"boa?secstr=%s&sortmode=3\" class=N0001>在线</a></TD>\n"
		"</TR>\n", secstr, secstr, secstr);
	brc_initial(currentuser.userid, NULL);
	printf("<tr>\n");
	for (i = 0; i < total; i++) {
/*		printf("<tr bgcolor=%s><td>%s</td>",
		       currstyle->colortb2,
		       board_read(data[i]->header.filename,
				  data[i]->lastpost) ? "◇" : "◆");
*/
		printf("<td class=B0400>%s</td>\n", board_read(data[i]->header.filename, data[i]->lastpost) ? "◇" : "◆");
		printf
		    ("<td class=N0003><a href=%s?B=%s >%s</a></td>",
		     cgi, data[i]->header.filename, data[i]->header.filename);
		printf("<td class=B0400>");
		if (data[i]->header.flag & VOTE_FLAG)
			printf
			    ("<a href=vote?B=%s>V</a>",
			     data[i]->header.filename);
		else
			printf("&nbsp;");
		printf("</td>");
		printf("<td class=B0400>[%4.4s]</td>", data[i]->header.type);
		printf
		    ("<td class=B0400><a href=%s?B=%s>%s</a></td>",
		     cgi, data[i]->header.filename, data[i]->header.title);
		ptr = userid_str(bm2str(bmbuf, &(data[i]->header)));
		if (strlen(ptr) == 0)
			printf("<td class=B0400>诚征版主中</td>");
		else
			printf("<td class=B0400>%s</td>", ptr);
		printf
		    ("<td class=B0400>%d</td><td class=B0400>%d</td><td class=B0400>%d</td></tr>\n",
		     data[i]->total, data[i]->score, data[i]->inboard);
		printlastmark(data[i]->header.filename);
	}
	printf("</TR></TBODY></TABLE></td></tr></table></td></tr></table>\n");
	return 0;
}

int
board_read(char *board, int lastpost)
{
	brc_initial(NULL, board);
	return !brc_un_read_time(lastpost);
}

void
printlastmark(char *board)
{
	char buf[200], *title, *thread;
	FILE *fp;
	int n = 0;
	int th;
	sprintf(buf, "wwwtmp/lastmark/%s", board);
	if (!file_exist(buf))
		goto END;
	if ((fp = fopen(buf, "r")) == NULL)
		goto END;
	while (fgets(buf, 200, fp)) {
		thread = strchr(buf, '\t');
		if (thread == NULL)
			break;
		*thread = 0;
		thread++;
		th = atoi(thread);
		title = strchr(thread, '\t');
		if (title == NULL)
			break;
		*title = 0;
		title++;
		printf("<tr><td></td><td colspan=8>");
		printf
		    ("·<a href='tfind?B=%s&th=%d&T=%s' class=mar>%s</a> 作者[%s]\n",
		     board, th, encode_url(title),
		     void1(titlestr(title)), userid_str(buf));
		printf("</td></tr>");
		n = 1;
	}
	fclose(fp);
      END:
	if (!n)
		printf
		    ("<tr><td colspan=9 height=1></td></tr>");
}

int
showsecmanager(const struct sectree *sec)
{
	struct secmanager *secm;
	int i;
	if (!sec->basestr[0] || !(secm = getsecm(sec->basestr)) || !secm->n)
		return -1;
	printf("区长:");
	for (i = 0; i < secm->n; i++) {
		printf(" <a href=qry?U=%s>%s</a>", secm->secm[i],
		       secm->secm[i]);
	}
	return 0;
}
int 
show_commend()
{
	FILE *fp;
	struct commend x;
//	int no=0, end = 0;
	int i;//, total;
	fp=fopen(COMMENDFILE,"r");

//tj change here 20040421
//modify by mintbaggio 040517 for new www
	if (!fp)
		 http_fatal("目前没有任何推荐文章");


	/*printf("<center>\n");
	printf("<hr color=green>\n");
        printf("<table width=640 border=0>\n");
        printf("<tr><td><nobr>序号test</nobr><td><nobr>推荐时间</nobr><td><nobr>标题</nobr><td><nobr>信区</nobr><td><nobr>作者</nobr>\n");
*/
/*        total=file_size(".COMMEND")/sizeof(struct commend);
        if (total>=20) 
		end = total -20;
*/
	fseek(fp, -20*sizeof(struct commend), SEEK_END);
	for(i=20; i>0; i--) {
//		fseek(fp, sizeof(struct commend)*i, SEEK_SET);
		if(fread(&x, sizeof(struct commend), 1, fp)<=0) break;
		printf("<tr><td class=F0001>&nbsp;</td>\n");
		printf("<td class=F0001><a href=con?B=%s&F=%s class=1102>%-30s</a> / <a href=qry?U=%s class=1014>%-12s</a>" 
			"/<a href=\"home?B=%s\" class=1014>%-13s</a></td></tr>\n",x.board, x.filename, x.title,x.userid,  x.userid, x.board, x.board);
/*			printf("<td><a href=con?B=%s&F=%s N=%dT=0>%s</a> ",x.board, x.filename,no,x.title);
			printf("<td>[<a href=home?B=%s>%s</a>] ", x.board, x.board);
			printf("<td><a href=qry?U=%s>%s</a>", x.userid,x.userid);*/
	}
	fclose(fp);
	return 0; 

/*
	printf("<table width=100% border=0 cellpadding=0 cellspacing=0>");
  printf("<tr> <td height=30></td>");
  printf("</tr><tr><td height=70> ");
      printf("<table width=100% height=100% border=0 cellpadding=0 cellspacing=0 bgcolor=#efefef>");
      printf("<tr><td><img src=/images/bmy.gif width=160 height=60> </td>");
      printf("<td width=100%>");
      printf("<table width=800 border=0 cellpadding=0 cellspacing=0>");
      printf("<tr>");
      printf("<td> <input name=textfield type=text style=font-size:11px;font-family:verdana; size=20 ></td>");
      printf("<td width=102 align=right> <input name=Submit type=button class=2014 value=Search> ");
      printf("</td><td width=802 height=20>&nbsp;</td>");
      printf("</tr></table></td><td align=right>");
      printf("&nbsp;</td></tr></table></td></tr></table>");
*/
}	

void show_banner()
{	//add by mintbaggio 040517 for new www, modify tj's code
/*	printf("<table width=100% border=0 cellpadding=0 cellspacing=0>\n");
	printf("<tr> <td height=30></td>\n");
	printf("</tr><tr><td height=70>\n");
	printf("<table width=100% height=100% border=0 cellpadding=0 cellspacing=0 bgcolor=#efefef>\n");
	printf("<tr><td><img src=/images/bmy.gif width=160 height=60> </td>\n");
	printf("<td width=100%>\n");
	printf("<table width=800 border=0 cellpadding=0 cellspacing=0>\n");
	printf("<tr>\n");
	printf("<td> <input name=textfield type=text style=font-size:11px;font-family:verdana; size=20 ></td>\n");
	printf("<td width=102 align=right> <input name=Submit type=button class=2014 value=Search>\n");
	printf("</td><td width=802 height=20>&nbsp;</td>\n");	
	printf("</tr></table></td><td align=right>\n");
	printf("&nbsp;</td></tr></table></td></tr></table>\n");*/
	printf("%s", "<body leftmargin=0 topmargin=0>\n"
	"<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td height=30></td></tr>\n"
	"<tr><td height=70>\n"
	"<table id=main width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 bgcolor=#efefef>\n"
	"<tr><td><img src=\"/images/bmy.gif\" width=160 height=60></td>\n"
	"<td width=290><table width=164 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td><input name=textfield type=text style=\"font-size:11px;font-family:verdana;\" size=20></td>\n"
	"<td width=102><input name=Submit type=button class=2014 value=Search>\n"
	"</td>\n<td width=102 height=20>&nbsp;</td></tr></table></td>\n"
	"<td align=right>&nbsp;</td></tr></table></td></tr></table>\n");
	return;
}

int show_content()
{	//add by mintbaggio 040517 for new www
	FILE* fp;
	//int fd;
	char buf[512], str[1];	
	int flag = 0;

	fp = fopen("etc/manager_team", "r");
	//fd = open("etc/manager_team", O_RDONLY);
	if(!fp)
		flag = 1;
	//show commend
	printf("%s", "<table width=100% border=0 cellpadding=0 cellspacing=0>\n"
  		"<tr> \n<td width=500 valign=top> \n"
      		"<table width=98% border=0 align=center cellpadding=0 cellspacing=0>\n
       		 <tr> \n"
	          "<td width=7 rowspan=2 align=right><img src=\"/images/bmy_arrowdown_black.gif\" width=6 height=5></td>\n
        	  <td height=5>&nbsp;</td>\n
       		 </tr>\n"
        	"<tr> \n
         	 <td class=F0000>近日精彩话题推荐 <a href=\"#\" class=1001>察看全部</a></td>\n
       		 </tr>     \n ");
	show_commend();
	printf("</table>\n");

	//show boards
	printf("%s", "<table width=98% border=0 align=center cellpadding=0 cellspacing=0>\n"
		"<tr><td width=10 rowspan=2 align=right><img src=\"/images/bmy_arrowdown_black.gif\" width=6 height=5></td>\n"
		"<td width=456 height=5 colspan=2>&nbsp;</td></tr><tr> <td colspan=2 class=F0000>推荐讨论区</td></tr>\n");
	show_sec(&sectree);
	printf("</table><br></td>\n");
	
	//show top10
	printf("%s", "<td width=15><img src=\"/images/spacer.gif\" width=1 height=1></td>\n"
		"<td width=280 valign=top>\n");
	printf("%s", "<table width=98% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td width=10 rowspan=2 align=right><img src=\"/images/bmy_arrowdown_black.gif\" width=6 height=5></td>\n"
		"<td width=456 height=5>&nbsp;</td></tr>\n"
		"<tr><td class=F0000>今日十大</td></tr><table>\n");
	printf("%s", "<div id=topic style=\"display:\">\n"
		"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td width=6 class=F0002>&nbsp;</td>\n<td><div>"
		"<DIV class=N0001>话题 / topic </DIV><DIV class=N1000> </DIV>\n"
		"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog2();\">链接 / link</A> </DIV>\n"
		"<DIV class=N1000> </DIV><DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog3();\">其他 / other</A> </DIV>\n"
		"<DIV class=N1000> </DIV></div></td> </tr></table>\n");
	printf("<table width=275 border=0 cellpadding=0 cellspacing=0>\n");
	show_top10();
	printf("</table></div>\n");

	//show link
	printf("%s","<div id=board style=\"display:none\"> \n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=6 class=F0002>&nbsp;</td>\n<td><div>" 
	"<DIV><a class=N0000 href=\"javascript:;\" onClick=\"Tog1();\">话题 / topic </a></DIV>\n"
	"<DIV class=N1000> </DIV><DIV class=N0001>链接 / link </DIV>\n"
	"<DIV class=N1000> </DIV>"
	"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog3();\">其他 / other</A> </DIV>\n"
	"<DIV class=N1000 </DIV></div></td></tr></table>\n"
        "<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=10 class=F0002>&nbsp;</td>\n"
	"<td width=265 class=F0002><a href=\"http://ftp.xjtu.edu.cn\">思源FTP </a></td></tr>\n"
        "<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"http://webmail.xjtu.edu.cn\">思源WEBMAIL </a></td></tr>\n"
        "<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"http://music.xjtu.edu.cn/\">思源音乐台</a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n");
//	show_link();
	printf("%s", "<td class=F0002><a href=\"http://vod.xjtu.edu.cn/\">思源VOD </a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"http://e.xjtu.edu.cn/\">思源搜索</a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"http://home.xjtu.edu.cn/\">思源空间 </a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002> <a href=\"http://202.117.24.24/\">钱学森图书馆</a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002> <a href=\"http://202.117.21.253/\">Windows Update </a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002>&nbsp;</td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002>&nbsp;</td>\n"
	"</tr></table></div>\n"
	"<div id=wishes style=\"display:none\">\n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=6 class=F0002>&nbsp;</td>\n"
	"<td><div>\n"
	"<DIV><a class=N0000 href=\"javascript:;\" onClick=\"Tog1();\">话题 / topic</a></DIV>\n"
	"<DIV class=N1000> </DIV>\n"
	"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog2();\">链接 / link</a></DIV>\n"
	"<DIV class=N1000> </DIV>\n"
	"<DIV class=N0001>其他 / other </DIV><DIV class=N1000> </DIV>\n"
	"</div></td></tr></table>\n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=10 class=F0002>&nbsp;</td>\n"
	"<td width=265 class=F0002><a href=\"telnet://bbs.xjtu.edu.cn\">Telnet登录BMY</a></td></tr>\n"
        "<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"javascript:window.external.AddFavorite('http://bbs.xjtu.edu.cn/','西安交通大学兵马俑BBS')\">将本站加入收藏夹</a></td>\n"
	"</tr>\n<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"mailto:wwwadmin@mail.xjtu.edu.cn\">联系站务组 </a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n"
	"<td class=F0002><a href=\"javascript: openreg()\">新用户注册 </a></td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n<td class=F0002>&nbsp;</td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n<td class=F0002>&nbsp;</td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n<td class=F0002>&nbsp;</td></tr>\n"
	"<tr><td class=F0002>&nbsp;</td>\n<td class=F0002>&nbsp;</td></tr>\n"
        "<tr><td class=F0002>&nbsp;</td>\n<td class=F0002>&nbsp;</td></tr>\n"
        "<tr><td class=F0002>&nbsp;</td>\n<td class=F0002>&nbsp;</td></tr>\n"
	"</table></div>\n");

	//show info
	printf("%s", "<br><div id=x1 style=\"display:\">\n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td valign=top class=B1111>\n"
	"<table width=143 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=10 rowspan=2 align=right>\n"
	"<img src=\"/images/bmy_arrowdown_orange.gif\" width=6 height=5></td>\n"
	"<td width=32 height=5></td></tr>\n"
	"<tr><td class=0003>常见问题</td></tr></table></td></tr></table>\n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0 class=B0111>\n"
	"<tr><td width=6 rowspan=2 class=B0010></td>\n"
	"<td colspan=2 height=6></td></tr>\n"
	"<tr><td colspan=2>ctrl+g可以根据文章内容、作 者、标题等分类搜索。Tab 查看备忘录，z 查看秘密备忘录，x 进入精华区，h 查看 一般性帮助菜单，编辑时ctrl+Q查看帮助菜单 </td>\n"
        "</tr></table><br></div>\n"
	"<div id=x1 style=\"display:\">\n"
        "<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td valign=top class=B1111>\n"
	"<table width=143 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=10 rowspan=2 align=right>\n"
	"<img src=\"/images/bmy_arrowdown_orange.gif\" width=6 height=5></td>\n"
	"<td width=32 height=5></td></tr>\n"
	"<tr><td class=0003>关于BMY</td></tr></table></td></tr></table>\n"
        "<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=6 rowspan=2 class=B0010>&nbsp;</td>\n"
	"<td colspan=2 height=6></td></tr>\n"
	"<tr><td colspan=2><p>CPU: Intel Xeon MP 1.6GHz × 4<br>RAM: 4GB ECC<br>HD: 120G</p>\n"
	"<p>数据未知，请完善</p></td></tr></table>\n");
	
	printf("%s", "<br></div>\n"
	"<div id=x1 style=\"display:\">\n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td valign=top class=B1111>\n"
	"<table width=143 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=10 rowspan=2 align=right>\n"
	"<img src=\"/images/bmy_arrowdown_orange.gif\" width=6 height=5></td>\n"
	"<td width=32 height=5></td></tr>\n"
	"<tr><td class=0003>管理团队</td></tr></table></td></tr></table>\n"
        "<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=6 rowspan=2 class=B0010>&nbsp;</td>\n"
	"<td colspan=2 height=6></td></tr>\n"
	"<tr><td colspan=2>\n");
	/*printf(" 系统管理员(SysOp): lanboy <br>\n"
	"顾问团:<br>\n"
	"majie,owl,enochtian,byebye,alarding <br>\n"
	"站务总站长:hkmj <br>\n"
	"站务副站长:liangwolf <br>\n"
	"讨论区主管站长:gyt, Haydn, bbsxjtu、shangail、deboran <br>\n"
	"实习主管站长:lo,afraid wjbta <br>\n"
	"系统维护主管:gyt,wjbta(兼)<br>\n"
	"版面管理分工: <br>\n"
	"本站系统 hkmj <br>\n"
	"文学艺术 shangail <br>\n"
	"交通大学 shangail <br>\n"
	"知性感性 deboran <br>\n"
	"开发技术 gyt <br>\n"
	"体育运动 afraid <br>\n"
	"电脑应用 wjbta <br>\n"
	"休闲音乐 Haydn <br>\n"
	"学术科学 bbsxjtu <br>\n"
	"游戏天地 lo <br>\n"
	"社会科学 bbsxjtu <br>\n"
	"兄弟院校 afraid <br>\n"
	"新闻信息 liangwolf <br>\n"
	"乡音乡情 Haydn &amp; deboran");*/
//	fflush(NULL);				//must fflush here
	//fp = fopen("etc/manager_team", "r");

	if(flag){
		printf("fail to open team\n");
		goto fail_out;
	}
	bzero(buf, 512);
	while(fgets(buf, 512, fp)){
		buf[strlen(buf)-1] = 0;
		printf("%s<br>\n", buf);
	}
	fclose(fp);
fail_out:

	/*if(show_manager_team())
		printf("fail to open team\n");

	*/	
	printf("</td>\n"
	"</tr></table><br></div><br><br>"
	"</td></tr></table>\n");
	//fflush(NULL);
	return 1;
}

/*int show_manager_team()
{
	FILE* fp;
	char buf[512];

	fp = fopen("etc/manager_team", "r");
	if(!fp){
		printf("fail to open team\n");
		return 1;
	}
	memcpy(buf, 0, 512);
	while(fgets(buf, 512, fp)){
		buf[strlen(buf)-1] = 0;
		printf("%s<br>\n", buf);
	}
	fclose(fp);

	return 0;
}*/


void show_sec(struct sectree *sec)
{	//add by mintbaggio 040517 for new www
	int i;
	for (i = 0; i < sec->nsubsec; i++) {

		if (sec->subsec[i]->nsubsec)
			continue;
		printf("<tr>");
		if(i == 0)
			printf("<td rowspan=16 class=F0001>&nbsp;</td>\n");
		printf("<td><a href=boa?secstr=%s class=N0002>"
		       "%s</a></td>\n", sec->subsec[i]->basestr,
		       nohtml(sec->subsec[i]->title));
		printf("<td rowspan=2 align=right valign=bottom class=B0300><a href=boa?secstr=%s class=1020>%s</a></td></tr>\n",
			sec->subsec[i]->basestr, sec->subsec[i]->basestr);
		show_boards(sec->subsec[i]->basestr);
	}
	return;
}

/*void show_boards()
{
	
}
*/
void show_boards(char *secstr)
{	//add by mintbaggio 040518 for new www
	struct boardmem *(data[MAXBOARD]), *x;
	int len, hasintro = 0;
	int i, total = 0;
	const struct sectree *sec;
	
	sec = getsectree(secstr);
	len = strlen(secstr);
	if (sec->introstr[0])
		hasintro = 1;
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->header.filename[0] <= 32 || x->header.filename[0] > 'z')
			continue;
		if (hasintro) {
			if (strcmp(secstr, x->header.sec1) &&
			    strcmp(secstr, x->header.sec2)) continue;
		} else {
			if (strncmp(secstr, x->header.sec1, len) &&
			    strncmp(secstr, x->header.sec2, len))
				continue;
		}
		if (!has_read_perm_x(&currentuser, x))
			continue;
		data[total] = x;
		total++;
	}
	show_sec_boards(data, total);
}

void show_sec_boards(struct boardmem *(data[]), int total)
{	//add by mintbaggio 040518 for new www
	int i;

	printf("<tr><td class=B0100><div>\n");
	for(i=0; i<total; i++){
		printf("<a href=home?B=%s class=1100>%s </a>", data[i]->header.filename, data[i]->header.title);
	}
	printf("</div></td></tr>\n");
}

void show_top10()
{
}

