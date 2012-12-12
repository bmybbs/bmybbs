#include "bbslib.h"
//#include "struct.h"
#define COMMENDFILE     MY_BBS_HOME"/.COMMEND"
#define COMMENDFILE2     MY_BBS_HOME"/.COMMEND2"
#define SHOWBOARDS 5    //add by lsssl@072706 you will see "SHOWBOARDS" boards every section at the firstlook of bmy; 

#define AREA_DIR		"etc/Area_Dir"	// 每个区的热门话题文件的存放目录

void show_area_top(char c);

void show_banner();
int show_commend();
int show_content();
void show_sec(struct sectree *sec);
void show_sec_by_name(char secid);
void show_boards(char* secstr);
void show_sec_boards(struct boardmem *(data[]), int total);
void show_top10();
void show_right_click_header(int i);
// int show_manager_team(); 未使用

int
bbsboa_main()
{
	struct boardmem *(data[MAXBOARD]), *x;
	int i, total = 0;
	char *secstr; //, session_name[STRLEN], pname[STRLEN], *p;
	const struct sectree *sec;
	//int show_Commend();
	int hasintro = 0, len;

	secstr = getparm("secstr");
	sec = getsectree(secstr);
	//if (secstr[0] != '*') {   暂时去掉缓存 by IronBlood@bmy 20120329
	////get_session_string(session_name);
	////if (secstr[0] != '*' && !no_cache_header) {
	//	    if (cache_header
	//	    (max(thisversion, file_time(MY_BBS_HOME "/wwwtmp")), 120))
	//		return 0;
	//}
	/*
	if (no_cache_header) {
		p = strchr(session_name, '.');
		if (NULL != p) {
			*p = '\0';
		}
		
		sprintf(pname, "/%s%s/", SMAGIC, session_name);
		print_session_string(pname);
	}
	*/

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
		printf("<div class=rhead>%s --<span class=h11> 预定讨论区总览</span></div><hr>", BBSNAME);
		if (total)
			showboardlist(data, total, "*", sec);
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
				/* modified by freely@BMY@20060525 */
				showboardlist(data, total, secstr, sec);
				//showboardlist(data, total, secstr);
		} 

		/* modified by freely@BMY@20060529 
		 showsecintro 函数功能改变，注释掉*/
		/*
		else if (!strncmp(buf, "#showsecintro", 13))
			showsecintro(sec);
			*/
	
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
//	showsechead(sec);
	printf("<tr><td height=30 colspan=2></td></tr>\n"
		"<tr><td height=70 colspan=2><table width=\"100%%\" border=0 cellpadding=0 cellspacing=0 bgcolor=#efefef>\n"
		"<tr><td width=40>&nbsp;</td>\n");
	printf("<td width=400 height=70><a href=\"boa?secstr=%s\" class=btnsubmittheme>%s </a> </td>", sec->basestr, nohtml(sec->title));
//	printf("<h2>%s</h2>", nohtml(sec->title));
	printf("%s", "<td width=300><table width=164 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td><input name=textfield type=text class=\"inputsearch\" size=20>\n"
	"</td>\n<td width=102><input name=Submit type=button class=sumbitgrey value=Search></td>\n"
	"<td width=102 height=20>&nbsp;</td></tr></table></td></tr>\n");
//	printf("<div align=right>");
//	showsecmanager(sec);
//	printf("</div>");
//	printf("<hr>");

	/* modified by freely@BMY@20060525 */
	/*在showboardlist 里面调用，注释掉*/
	//showsecintro(sec);
	showboardlist(data, total, secstr, sec);
	/* modified by freely@BMY@20060525
	if (total) {		
		showboardlist(data, total, secstr);
//		printf("<hr>");
	}
	*/
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
		printf("<a href=%s%s class=pur><u>%s</u></a>",
		       showByDefMode(),
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
	if (showfile(filename) < 0 && sec->nsubsec>0) {
/* modified by clearboy@BMY@20050506*/

/*	modified by freely@BMY@20060525
	printf("%s",
			"	  <table width=\"90%\" border=0 cellPadding=2 cellSpacing=0>"
		  "<tr>"
		   " <td colspan=\"2\" class=\"level2\" ><a href=\"#\" class=\"btnsubmittheme\">二级版面</a></td>"
		    "</tr>"
		  "<tr>\n");
		  

		for (i = 0; i < sec->nsubsec; i++) {
			 printf("<td width=\"10%\">&nbsp;</td><td width=\"90%\">\n");
			printf("&#8225;→<a href=bbsboa?secstr=%s>%s</a>",
			     sec->subsec[i]->basestr, sec->subsec[i]->title);
			 printf("&nbsp;</td></tr>\n");
		}

		printf("%s",
			"<tr>"
		    "<td colspan=\"2\" class=\"level2\"><a href=\"#\" class=\"btnsubmittheme\">普通版面</a></td>"
		    "</tr>"
		"</table>");
		*/

		/* add by freely@BMY@20060525 */
		/* 此函数更改后只在showboardlist 里面被调用*/
		for (i = 0; i < sec->nsubsec; i++) {
			printf("<td class=tdborder>＋</td>\n");
			printf("<td class=tduser><a href=bbsboa?secstr=%s>&nbsp;</a></td>",
			     sec->subsec[i]->basestr);
			printf("<td class=tdborder>&nbsp;</td><td class=tdborder>[二级版面]</td>");
			printf("<td class=tdborder><a href=bbsboa?secstr=%s>%s</a></td>",
			     sec->subsec[i]->basestr, sec->subsec[i]->title);	
			printf("<td class=tdborder></td><td class=tdborder></td><td class=tdborder></td><td class=tdborder></td></tr>\n");
		}

		
	}
	return 0;
}

/*	modified by freely@BMY@20060525 */
int
showboardlist(struct boardmem *(data[]), int total, char *secstr, const struct sectree *sec)
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
	printf("%s", "<tr><td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td colspan=2 class=\"level1\"><TABLE width=\"90%\" border=0 cellPadding=2 cellSpacing=0>\n"
		"<TBODY>\n");

	printf("<TR>\n<TD class=tdtitle>未</TD>\n"
		"<TD class=tduser><a href=\"boa?secstr=%s\" class=linktheme>讨论区名称</a></TD>\n"
		"<TD class=tdtitle>V\n"
		"<TD class=tdtitle>类别</TD>\n"
		"<TD class=tdtitle>中文描述</TD>\n"
		"<TD class=tdtitle>版主</TD>\n"
		"<TD class=tdtitle>文章数</TD>\n"
		"<TD class=tdtitle><a href=\"boa?secstr=%s&sortmode=2\" class=linktheme>人气</a></TD>\n"
		"<TD class=tdtitle><a href=\"boa?secstr=%s&sortmode=3\" class=linktheme>在线</a></TD>\n"
		"</TR>\n", secstr, secstr, secstr);
	brc_initial(currentuser.userid, NULL);
	printf("<tr>\n");

	/* add by freely@BMY@20060525 */
	/*把showsecintro放到showboardlist 里面调用*/
	/*不显示二级版面*/
	if(secstr[0]!='*')
		showsecintro(sec);

	for (i = 0; i < total; i++) {
/*		printf("<tr bgcolor=%s><td>%s</td>",
		       currstyle->colortb2,
		       board_read(data[i]->header.filename,
				  data[i]->lastpost) ? "◇" : "◆");
*/
		printf("<td class=tdborder>%s</td>\n", board_read(data[i]->header.filename, data[i]->lastpost) ? "◇" : "◆");
		printf
		    ("<td class=tduser><a href=%s?B=%s >%s</a></td>",
		     cgi, data[i]->header.filename, data[i]->header.filename);
		printf("<td class=tdborder>");
		if (data[i]->header.flag & VOTE_FLAG)
			printf
			    ("<a href=vote?B=%s>V</a>",
			     data[i]->header.filename);
		else
			printf("&nbsp;");
		printf("</td>");
		printf("<td class=tdborder>[%4.4s]</td>", data[i]->header.type);
		printf
		    ("<td class=tdborder><a href=%s?B=%s>%s</a></td>",
		     cgi, data[i]->header.filename, data[i]->header.title);
		ptr = userid_str(bm2str(bmbuf, &(data[i]->header)));
		if (strlen(ptr) == 0)
			printf("<td class=tdborder>诚征版主中</td>");
		else
			printf("<td class=tdborder>%s</td>", ptr);
		printf
		    ("<td class=tdborder>%d</td><td class=tdborder>%d</td><td class=tdborder>%d</td></tr>\n",
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
	char allcanre[256];
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
	
	for(i=20; i>15; i--) {
//		fseek(fp, sizeof(struct commend)*i, SEEK_SET);
		strcpy(allcanre, "");
		if(fread(&x, sizeof(struct commend), 1, fp)<=0) break;
		if(x.accessed & FH_ALLREPLY)
 			strcpy(allcanre," style='color:red;' ");
		printf("<tr><td></td>\n");
		printf("<td><a href=con?B=%s&F=%s%s>%-30s</a> / <a href=qry?U=%s class=linkdatetheme>%-12s</a>" 
			"/<a href=\"%s%s\" class=linkdatetheme>%-13s</a></td></tr>\n",
			x.board, x.filename, allcanre, x.title,x.userid,  x.userid, showByDefMode(), x.board, x.board);
/*			printf("<td><a href=con?B=%s&F=%s N=%dT=0>%s</a> ",x.board, x.filename,no,x.title);
			printf("<td>[<a href=%s%s>%s</a>] ", showByDefMode(), x.board, x.board);
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

show_commend2()
{
	FILE *fp;
	struct commend x;
	char allcanre[256];
//	int no=0, end = 0;
	int i;//, total;
	fp=fopen(COMMENDFILE2,"r");

//tj change here 20040421
//modify by mintbaggio 040517 for new www
	if (!fp)
		 http_fatal("目前没有任何通知公告");





	fseek(fp, -20*sizeof(struct commend), SEEK_END);
	for(i=20; i>10; i--) {
		strcpy(allcanre, "");
		if(fread(&x, sizeof(struct commend), 1, fp)<=0) break;
		if(x.accessed & FH_ALLREPLY)
 			strcpy(allcanre," style='color:red;' ");
		printf("<tr><td></td>\n");
		printf("<td><a href=con?B=%s&F=%s%s>%-30s</a> / <a href=qry?U=%s class=linkdatetheme>%-12s</a>" 
			"/<a href=\"%s%s\" class=linkdatetheme>%-13s</a></td></tr>\n",
			x.board, x.filename, allcanre, x.title,x.userid,  x.userid, showByDefMode(), x.board, x.board);

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
	FILE* fp;
	char buf[512];
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
	printf("%s", "<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td height=30></td></tr>\n"
	"<tr><td height=70>\n"
	"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n"
	"<tr><td><div id=\"bmy\"><span class=\"hidden\">兵马俑</span></div></td>\n");

	//add 广告 clearboy@20060919	

/*
	"<td align=right width=\"468\" height=\"60\">\n"
	"<a href=\"http://tuanwei.tiaozhan.com/show.php?contentid=939\" target=\"_blank\">"
	"<img src=\"/images/zph.jpg\" width=\"468\" height=\"60\" border=0>"
	"</img></a></td>\n"
	"</td>\n"

*/

/*
      "<td align=right width=\"468\" height=\"60\">\n"
        "<a href=\"http://www.zhongyis.com/\" target=\"_blank\">"
        "<img src=\"/images/zhongyisi.gif\" width=\"468\" height=\"60\" border=0>"
        "</img></a></td>\n"
	
	 "<td align=right width=\"468\" height=\"60\">\n"
        "<a href=\"http://uc.xjtu.edu.cn\" target=\"_blank\">"
        "<img src=\"/images/zgyd.gif\" width=\"468\" height=\"60\" border=0>"
        "</img></a></td>\n");

*/

/*	"<td align=right width=\"468\" height=\"60\">\n"
	"<a href=\"http://www.zhongyis.com/\" target=\"_blank\">"
	"<img src=\"/images/zhongyisi.gif\" width=\"468\" height=\"60\" border=0>"
	"</img></a></td>\n"
*/
/*
        "<td align=right width=\"468\" height=\"60\">\n"
        "<a  target=\"_blank\">"
	"<OBJECT><PARAM NAME='MOVIE' >"
	"<EMBED SRC='/images/liantong.swf' width=468 height=60></EMBED></OBJECT>"
        "</a></td>\n"
*/
	fp = fopen("etc/ad_banner", "r");
	if(!fp){
		//printf("fail to open\n");
		goto endbanner;
	}
	bzero(buf, 512);
	while(fgets(buf, 512, fp)){
		strltrim(strrtrim(buf));
		if (strlen(buf) <= 1)
			continue;
		char *p = strchr(buf, ' ');
		if (p == NULL)
			continue;
		*p = '\0';
		
		printf("<td align=right width=\"468\" height=\"60\">\n"
        		"<a href=\"%s\" target=\"_blank\">"
        		"<img src=\"%s\" width=\"468\" height=\"60\" border=0>"
        		"</img></a></td>\n", p+1, buf);
	}
	fclose(fp);

endbanner:
	printf("</td>\n"

	
	//end 广告 
	//"<td width=290><table border=0 cellpadding=0 cellspacing=0>\n"
	//"<tr><form action=home target=f3><td colspan=2><input name=board type=text size=20>\n"
	//"<input name=Submit type=submit class=sumbitgrey value=Search>\n"
	//"</td>\n</form></tr></table></td>\n"
	"<td align=right width=25>&nbsp;</td></tr></table>\n"
	"</td></tr></table>\n");
	return;
}

void title_begin(char *title)
{
	printf("<br>\n"
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td valign=top class=bordertheme>\n"
	"<table width=143 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=10 rowspan=2 align=right>\n"
	"<img src=\"/images/bmy_arrowdown_orange.gif\" width=6 height=5></td>\n"
	"<td width=32 height=5></td></tr>\n"
	"<tr><td class=themetext>%s</td></tr></table></td></tr></table>\n"
        "<table width=275 border=0 cellpadding=0 cellspacing=0 class=bordergrey2>\n"
	"<tr><td width=6 rowspan=2 class=B0010>&nbsp;</td>\n"
	"<td colspan=2 height=6></td></tr>\n"
	"<tr><td colspan=2>", title);

}

void title_end()
{
	printf("</td></tr></table>\n");
}

/* 兵马俑导读 */
int show_content()
{	//add by mintbaggio 040517 for new www
	FILE* fp, *secorderfile;
	char buf[512], str[1], buf1[512], buf2[512], secorder[16];
	int sec_index;
	struct sectree * psec;
	const char * secorderfilepath = BBSHOME "/etc/secorder";

	//show commend
	printf("%s", "<table width=100% border=0 cellpadding=0 cellspacing=0>\n"
  		"<tr> \n<td valign=top> \n"
      		"<table width=98%% border=0 align=center cellpadding=0 cellspacing=0>\n"
       		 "<tr> \n"
	          "<td width=7 rowspan=2 align=right><img src=\"/images/bmy_arrowdown_black.gif\" width=6 height=5></td>\n"
        	  "<td height=5>&nbsp;</td>\n"
       		 "</tr>\n"
        	"<tr> \n"
         	 "<td class=F0000>美文推荐 /  <a href=\"bbstop10\" target=f3 class=linkdatetheme>查看更多</a></td>\n"
       		 "</tr>     \n ");
	show_commend();
	printf("<tr><td></td><td><div class=\"linediv\"></div></td></tr>");
	printf("</table>\n");

	printf("%s", 
      		"<table width=98%% border=0 align=center cellpadding=0 cellspacing=0>\n"
       		 "<tr> \n"
	          "<td width=7 rowspan=2 align=right><img src=\"/images/bmy_arrowdown_black.gif\" width=6 height=5></td>\n"
        	  "<td height=5>&nbsp;</td>\n"
       		 "</tr>\n"
        	"<tr> \n"
         	 "<td class=F0000>通知公告  /  <a href=\"bbstop10\" target=f3 class=linkdatetheme>查看更多</a></td>\n"
       		 "</tr>     \n ");
	show_commend2();
	printf("<tr><td></td><td><div class=\"linediv\"></div></td></tr>");
	printf("</table>\n");

	//show boards
	printf("<table width=98% border=0 align=center cellpadding=0 cellspacing=0>\n"
		"<tr><td width=456><img src=\"/images/bmy_arrowdown_black.gif\">\n"
		"<span=2 class=F0000>推荐讨论区</span></td>\n"
		"<td>&nbsp;</td></tr>\n"
              "<tr><td><a href='%sXJTUKXFZ' style=\"color: red\">深入学习科学发展观</a>&nbsp;<a href='%sXJTUdevelop' style=\"color: red\">交大发展</a>&nbsp;<a href='%swelcome' style=\"color: red\">欢迎报考西安交大</a>&nbsp;<a href='%skaoyan' style=\"color: red\">考研与保研</a></td></tr>\n",showByDefMode(),showByDefMode(),showByDefMode(),showByDefMode());

	//show_sec(&sectree); 老版本的显示分区的方式，注释掉 by IronBlood@bmy 20120329

	memset(secorder,0,sizeof(secorder));

	if( access(secorderfilepath,F_OK) != -1 ){
		secorderfile = fopen(secorderfilepath, "r");
		while(fgets(secorder,sizeof(secorder),secorderfile)!=NULL){
			if (secorder[strlen(secorder) - 1] == '\n')
				secorder[strlen(secorder) - 1] = 0;
		}
		fclose(secorderfile);
	}
	else{
		strcpy(secorder,"0123456789GNHAC\0"); // 如果站长没有配置，那就按照老版本的来
	}

	for(sec_index=0;sec_index!=strlen(secorder);++sec_index){
		show_sec_by_name(secorder[sec_index]);
	}

	
	//show right top header
	printf("</table>\n<br></td>\n");
	printf("<td width=15></td>\n"
		"<td width=280 valign=top>\n");

	//add 广告 clearboy@20060626
//	printf(" <table width=\"98%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
//		"<tr> <td align=\"right\"><object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=7,0,19,0\" width=\"234\" height=\"30\" align=\"left\">"
//		"  <param name=\"movie\" value=\"/ad/ad1.swf\" />"
//		"  <param name=\"quality\" value=\"high\" />"
//		"  <embed src=\"/ad/ad1.swf\" width=\"234\" height=\"30\" align=\"left\" quality=\"high\" pluginspage=\"http://www.macromedia.com/go/getflashplayer\" type=\"application/x-shockwave-flash\"></embed>"
//		"</object></td></tr></table>");
	//modified search bar

	//add by macintosh 070529 for board searching by keywords
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=290><table border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><form action=bbssbs target=f3><td>\n"
	"<div title=\"支持中英文版名/版面关键字定位至版面。\n例如，输入“铁路”可定位至traffic版。\">"
	"<input type=text name=keyword maxlength=25 size=25 onclick=\"this.select()\" value=\"请输入关键字\">\n"
	"<input type=submit class=sumbitgrey value=\"搜索版面\"></div>\n"
	"</td></form></tr></tabel></td></tr>\n"
	"</table><br>");

	//modified by pzhg 20070526 for google search
	/*
	printf("<table width=98%% border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=290><table border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td>\n"
	"<form id=\"google_search\" action=\"http://www.google.com/cse\">\n"
    	"<input type=\"hidden\" name=\"cx\" value=\"002081393031460154583:jaqgmrcgfmm\" />\n"
	"<input type=\"hidden\" name=\"cof\" value=\"FORID:0\" />\n"
   	"<input name=\"q\" type=\"text\" size=\"25\" />\n"
    	"<input type=\"submit\" class=sumbitgrey  name=\"Submit\" value=\"全文搜索\" />\n"
	"</form>\n"
	"<script type=\"text/javascript\"\n"
	"src=\"/inc/google.js\"></script>\n"
	"</td></tr></table></td></tr>\n"
	"</table>");
	*/
	
	//今日十大
	printf("<table width=98%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n<td width=10 rowspan=2 align=right><img src=\"/images/bmy_arrowdown_black.gif\" width=6 height=5></td>\n"
		"<td width=456 height=5>&nbsp;</td>\n"
		"</tr><tr>\n<td>今日十大&nbsp;<a href=\"http://" MY_BBS_DOMAIN "/" SMAGIC "/rss?board=TOP10\" target=\"blank\"><img  src=\"/images/rss.gif\" border=\"0\" /></a></td></tr></table>\n");
	
	//show top10
#if 0
	printf("%s","<div id=\"topic\" style=\"display:\">\n"
	//header
		"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td width=6 class=F0002>&nbsp;</td>\n<td><div>"
		"<DIV class=N0001>话题 / topic </DIV>\n<DIV class=N1000> </DIV>\n"
		"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog2();\">链接 / link</A> </DIV>\n"
		"<DIV class=N1000> </DIV>\n"
		"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog3();\">其他 / other</A> </DIV>\n"
		"<DIV class=N1000> </DIV></div></td> </tr></table>\n");
#endif
	show_right_click_header(1);
	//content
	printf("<table width=275 border=0 cellpadding=0 cellspacing=0>\n");
	show_top10();
	printf("</table></div>\n");

	//show link
#if 0
	printf("%s","<div id=\"layer2\" style=\"display:none\"> \n"
	//header
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=6 class=F0002>&nbsp;</td>\n"
	"<td><div>\n"
	"<DIV><a class=N0000 href=\"javascript:;\" onClick=\"Tog1();\">话题 / topic </a></DIV>\n"
	"<DIV class=N1000> </DIV>"
	"<DIV class=N0001>链接 / link </DIV>\n"
	"<DIV class=N1000> </DIV>"
	"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog3();\">其他 / other</A> </DIV>\n"
	"<DIV class=N1000> </DIV></div></td></tr></table>\n"
#endif
	show_right_click_header(2);
	//content
	printf("<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td><a href=\"http://ftp.xjtu.edu.cn\">思源FTP </a></td></tr>\n"
        "<tr><td><a href=\"http://webmail.xjtu.edu.cn\">思源WEBMAIL </a></td></tr>\n"
	"<tr><td><a href=\"http://stu.xjtu.edu.cn\">思源学生MAIL </a></td></tr>\n"
        "<tr><td><a href=\"http://music.xjtu.edu.cn/\">思源音乐台</a></td></tr>\n"
	"<tr><td><a href=\"http://vod.xjtu.edu.cn/\">思源VOD </a></td></tr>\n"
	"<tr><td><a href=\"http://e.xjtu.edu.cn/\">思源搜索</a></td></tr>\n"
	"<tr><td><a href=\"http://home.xjtu.edu.cn/\">思源空间 </a></td></tr>\n"
	"<tr><td><a href=\"http://nic.xjtu.edu.cn/\">网络中心</a></td></tr>\n"
	"<tr><td><a href=\"http://www.xjtu.edu.cn/\">交大主页</a></td></tr>\n"
	"<tr><td><a href=\"http://202.117.21.253/\">Windows Update </a></td></tr>\n"
	"<tr><td>&nbsp;</td></tr>\n"
	"</table></div>\n");
	
	//show other
#if 0
	printf("%s","<div id=\"wishes\" style=\"display:none\">\n"
	//header
	"<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td width=6 class=F0002>&nbsp;</td>\n"
	"<td><div>\n"
	"<DIV><a class=N0000 href=\"javascript:;\" onClick=\"Tog1();\">话题 / topic</a></DIV>\n"
	"<DIV class=N1000> </DIV>\n"
	"<DIV><A class=N0000 href=\"javascript:;\" onClick=\"Tog2();\">链接 / link</a></DIV>\n"
	"<DIV class=N1000> </DIV>\n"
	"<DIV class=N0001>其他 / other </DIV>"
	"<DIV class=N1000> </DIV>\n"
	"</div></td></tr></table>\n"
#endif
	show_right_click_header(3);
	//content
	printf("<table width=275 border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td><a href=\"telnet://bbs.xjtu.edu.cn\">Telnet登录BMY</a></td></tr>\n"
	"<tr><td><a href=\"javascript:window.external.AddFavorite('http://bbs.xjtu.edu.cn/','西安交通大学兵马俑BBS')\">将本站加入收藏夹</a></td>\n"
	"</tr>\n"
	"<tr><td><a href=\"mailto:wwwadmin@mail.xjtu.edu.cn\">联系站务组 </a></td></tr>\n"
	"<tr><td><a href=\"javascript: openreg()\">新用户注册 </a></td></tr>\n"
	"</table></div>\n");

	title_begin("滚动广告信息");
	fp = fopen("etc/adpost", "r");
	if(!fp){
		//printf("fail to open\n");
		goto newboard;
	}
	bzero(buf1, 512);
	bzero(buf2, 512);
	printf("<marquee scrollamount=1 scrolldelay=20 direction= UP width=200 height=80  onmouseover=\"this.stop();\" onmouseout=\"this.start();\">\n");
	while(fgets(buf1, 512, fp)){
		strltrim(strrtrim(buf1));
		if (strlen(buf1) <= 1)
			continue;
		
		char *p = strchr(buf1, ' ');
		if (p == NULL)
			continue;
		*p = '\0';
		
		strcpy(buf2, p+1);
		strltrim(strrtrim(buf2));
		if (strlen(buf2) <= 1)
			continue;
		p = strchr(buf2, ' ');
		if (p == NULL)
			continue;
		*p = '\0';

		printf("<a href=\"con?B=%s&F=%s\">%s</a><br>\n", buf1,buf2, p+1);
	}
	fclose(fp);
	printf("</marquee>\n");;


newboard:
	// 20121016 move this line from 2lines up; so that it can't be skipped BY liuche
	// and i do the same thing to every "title_end();"
	title_end(); 
	title_begin("新开版面");
	fp = fopen("etc/newboard", "r");
	if(!fp){
		//printf("fail to open newboard\n");
		goto recommboard;
	}
	bzero(buf, 512);
	while(fgets(buf, 512, fp)){
		strltrim(strrtrim(buf));
		if (strlen(buf) <= 1)
			continue;
		char *p = strchr(buf, ' ');
		if (p == NULL)
			continue;
		*p = '\0';
		printf("<a href=\"%s%s\">%s</a><br>\n", showByDefMode(), buf, p+1);
	}
	fclose(fp);


recommboard:
	title_end();
	title_begin("推荐版面");
	fp = fopen("etc/recommboard", "r");
	if(!fp){
		//printf("fail to open commboard\n");
		goto aboutbmy;
	}
	bzero(buf, 512);
	while(fgets(buf, 512, fp)){
		strltrim(strrtrim(buf));
		if (strlen(buf) <= 1)
			continue;
		char *p = strchr(buf, ' ');
		if (p == NULL)
			continue;
		*p = '\0';
		printf("<a href=\"%s%s\">%s</a><br>\n", showByDefMode(), buf, p+1);
	}
	fclose(fp);

/*
	title_begin("telnet常见问题");
	printf("ctrl+g可以根据文章内容、作 者、标题等分类搜索。Tab 查看备忘录，z 查看秘密备忘录，x 进入精华区，h 查看 一般性帮助菜单，编辑时ctrl+Q查看帮助菜单");
	title_end();
*/
aboutbmy:
	title_end();
	title_begin("关于BMY");
	printf("CPU: Intel Xeon 3GHz × 4<br>RAM: 16GB ECC<br>HD: SAN 1000G<br>\n"
	"网卡: 双1000Mbps NIC<br>\n");
	title_end();

	// management team
	title_begin("管理团队");

	fp = fopen("etc/manager_team", "r");

	if(!fp){
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
	title_end();
	printf("<br></div><br><br>"
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

void show_sec_by_name(char secid){
	struct sectree *sec;
	sec = getsectree(&secid);
	printf("<tr>");
	printf("<td><div class=\"linediv\"><a href=boa?secstr=%s class=linkboardtheme>"
		       "%s</a></div></td>\n", sec->basestr, nohtml(sec->title));
	printf("<td rowspan=2 align=right valign=bottom width=45><a href=boa?secstr=%s class=linkbigtheme>%s</a></td></tr>\n",
				sec->basestr, sec->basestr);
	show_boards(sec->basestr);
	printf("</td></tr>\n");
}

void show_sec(struct sectree *sec)
{	//add by mintbaggio 040517 for new www
	int i;
	for (i = 0; i < sec->nsubsec; i++) {
//modified by clearboy@BMY@20050506
/*		if (sec->subsec[i]->nsubsec)
			continue;
*/
		printf("<tr>");
/*//modified by clearboy @BMY@20050506
		if(i == 0)
			printf("<td rowspan=80>&nbsp;</td>\n");
*/
		printf("<td><div class=\"linediv\"><a href=boa?secstr=%s class=linkboardtheme>"
		       "%s</a></div></td>\n", sec->subsec[i]->basestr,
		       nohtml(sec->subsec[i]->title));
		//modified by safari 20091231
		printf("<td rowspan=2 align=right valign=bottom width=45><a href=boa?secstr=%s class=linkbigtheme>%s</a></td></tr>\n",
			sec->subsec[i]->basestr, sec->subsec[i]->basestr);
		show_boards(sec->subsec[i]->basestr);
		
		
		printf("</td></tr>\n");
		
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
{       //add by mintbaggio 040518 for new www
        int i;
/*
        printf("<tr><td>\n");

        for(i=0; i<total; i++){
                printf("<a href=%s%s>%s</a> ", showByDefMode(), data[i]->header.filename, data[i]->header.title);
*/

       //add by lsssl@072706; sort by boardmem.score
       int scores[total];
           scores[0] = 0;
       for (i = 1; i < total; i++)
       {
                int j, k;
                for (j = 0; j < i; j++)
                        if (data[i]->score > data[scores[j]]->score)
                                break;
                for (k = i - 1; k >= j; k--)
                        scores[k+1] = scores[k];
                scores[j] = i;
        }
        printf("<tr><td>\n");
        if (total < SHOWBOARDS)
        {
                for (i = 0; i < total; i++)
                        printf("<a href=%s%s>%s</a>(%d) ", showByDefMode(), data[scores[i]]->header.filename, data[scores[i]]->header.title, data[scores[i]]->score);
        }
        else
        {
                for(i = 0; i < SHOWBOARDS; i++)
                        printf("<a href=%s%s>%s</a>(%d) ", showByDefMode(), data[scores[i]]->header.filename, data[scores[i]]->header.title, data[scores[i]]->score);
		printf("<a href=boa?secstr=%c> [更多版面...]</a>", data[0]->header.secnumber1);
        }
		 
		if (total == 0)
			return;
		show_area_top(data[0]->header.secnumber1);
} 

void show_top10()
{
	struct mmapfile mf = {ptr:NULL};
    check_msg();
    MMAP_TRY {
            if (mmapfile("wwwtmp/cindextopten", &mf) < 0) {
                    MMAP_UNTRY;
                    http_fatal("文件错误");
            }
            fwrite(mf.ptr, mf.size, 1, stdout);
    }
    MMAP_CATCH {
    }
    MMAP_END mmapfile(NULL, &mf);
}

void show_area_top(char c)
{
	struct mmapfile mf = {ptr:NULL};
	char path[256];

	sprintf(path, AREA_DIR "/%c", c);
    check_msg();
    MMAP_TRY {
            if (mmapfile(path, &mf) < 0) {
                    MMAP_UNTRY;
                    http_fatal("文件错误");
            }
            fwrite(mf.ptr, mf.size, 1, stdout);
    }
    MMAP_CATCH {
    }
    MMAP_END mmapfile(NULL, &mf);	
}

void show_right_click_header(int i)
{	//add by mintbaggio 041225 for new www, need modify: i==1, i==2 else下面的printf，十分不简洁
	//modified by safari 091228
	if(i == 1)
		printf("<!-- begin:十大话题 -->\n<div id=layer%d style=\"display:\">\n", i);
	else
		printf("<!-- begin:十大话题 -->\n<div id=layer%d style=\"display:none\">\n", i);
        printf("<!-- begin:导航栏 -->\n<table><tr>\n");
	if(i == 1)
		printf("<td><DIV class=\"btncurrent\" title=\"话题 accesskey: t\" accesskey=\"t\">话题 / topic</DIV></td>\n"
                "<td><DIV><A class=\"btnlinktheme\" href=\"javascript:;\" onClick=\"Tog('2')\">链接 / link</A></DIV></td>\n"
                "<td><DIV><A class=\"btnlinktheme\" href=\"javascript:;\" onClick=\"Tog('3')\">其他 / other</A></DIV></td>\n");
	else if(i == 2)
		printf("<td><DIV><A class=\"btnlinktheme\" href=\"javascript:;\" onClick=\"Tog('1')\">话题 / topic</A></DIV></td>\n"
			"<td><DIV class=\"btncurrent\">链接 / link</DIV></td>\n"
			"<td><DIV><A class=\"btnlinktheme\" href=\"javascript:;\" onClick=\"Tog('3')\">其他 / other</A></DIV></td>\n");
	else
		printf("<td><DIV><A class=\"btnlinktheme\" href=\"javascript:;\" onClick=\"Tog('1')\">话题 / topic</A></DIV></td>\n"
			"<td><DIV><A class=\"btnlinktheme\" href=\"javascript:;\" onClick=\"Tog('2')\">链接 / link</A></DIV></td>\n"
			"<td><DIV class=\"btncurrent\">其他 / other</DIV></td>\n");
	printf("</tr></table>\n<!-- end:导航栏 -->\n");
}


