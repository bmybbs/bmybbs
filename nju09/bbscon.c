#include "bbslib.h"
extern char *cginame;

int
showbinaryattach(char *filename)
{
	char *attachname;
	int pos;
	unsigned int size;
	struct mmapfile mf = { ptr:NULL };
//      no_outcache();
	if (cache_header(file_time(filename), 86400))
		return 0;
	attachname = getparm("attachname");
	pos = atoi(getparm("attachpos"));
	MMAP_TRY {
		if (mmapfile(filename, &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("无法打开附件");
			MMAP_RETURN(-1);
		}
		if (pos >= mf.size - 4 || pos < 1) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("无法打开附件");
			MMAP_RETURN(-1);
		}
		if (mf.ptr[pos - 1] != 0) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("无法打开附件");
			MMAP_RETURN(-1);
		}
		size = ntohl(*(unsigned int *) (mf.ptr + pos));
		if (pos + 4 + size >= mf.size) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("无法打开附件");
			MMAP_RETURN(-1);
		}
		printf("Content-type: %s\n\n", get_mime_type(attachname));
//      printf("Content-Length: %d\n\n", size);
		fwrite(mf.ptr + pos + 4, 1, size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

void
fprintbinaryattachlink(FILE * fp, int ano, char *attachname, int pos, int size,
		       char *alt, char *alt1)
{
	char *ext, link[256], *ptr, *board;
	int pic = 0;
	int atthttp = 0;
	struct boardmem *x;

	//check read_perm for guest, refer to has_read_perm()
	board = getparm2("B", "board");
	x = getboard(board);
	if (x && !x->header.clubnum && !x->header.level) {
		ptr = "/" SMAGIC "/";
		if (w_info->att_mode == 0 && !via_proxy)
			atthttp = 1;
	} else {
		ptr = "";
	}

	if (alt) {
		//这种情况目前可以用atthttpd
		if (!atthttp)
			snprintf(link, sizeof (link),
				 "%s%s&amp;attachpos=%d&amp;attachname=/%s",
				 ptr, alt, pos, attachname);
		else if (wwwcache->accel_ip && wwwcache->accel_port)
			snprintf(link, sizeof (link),
				 "http://%s:%d%s%s&amp;attachpos=%d&amp;attachname=/%s",
				 inet_ntoa(wwwcache->accel_addr),
				 wwwcache->accel_port, ptr, alt, pos, attachname);
		else
			snprintf(link, sizeof (link),
				 "http://%s:8080/%s/%d/%s", MY_BBS_IP,
				 alt1, pos, attachname);

	} else if (!strcmp(cginame, "bbscon")) {
		//同上
		if (!atthttp)
			snprintf(link, sizeof (link),
				 "%sattach/bbscon/%s?B=%s&amp;F=%s&amp;attachpos=%d&amp;attachname=/%s",
				 ptr, attachname, board, getparm2("F", "file"),
				 pos, attachname);

		else if (wwwcache->accel_ip && wwwcache->accel_port)
			snprintf(link, sizeof (link),
				 "http://%s:%d%sattach/bbscon/%s?B=%s&amp;F=%s&amp;attachpos=%d&amp;attachname=/%s",
				 inet_ntoa(wwwcache->accel_addr),
				 wwwcache->accel_port,
				 ptr, attachname, board, getparm2("F", "file"),
				 pos, attachname);
		else
			snprintf(link, sizeof (link),
				 "http://%s:8080/%s/%s/%d/%s", MY_BBS_IP,

				 board, getparm2("F", "file"), pos, attachname);
	} else
		snprintf(link, sizeof (link),
			 "attach/%s/%s?%s&amp;attachpos=%d&amp;attachname=/%s",
			 cginame, attachname, getsenv("QUERY_STRING"), pos,
			 attachname);

	if ((ext = strrchr(attachname, '.')) != NULL) {
		if (!strcasecmp(ext, ".bmp") || !strcasecmp(ext, ".jpg")
		    || !strcasecmp(ext, ".gif")
		    || !strcasecmp(ext, ".jpeg")
		    || !strcasecmp(ext, ".png")
		    || !strcasecmp(ext, ".pcx"))
			pic = 1;
		else if (!strcasecmp(ext, ".swf"))
			pic = 2;
		else
			pic = 0;
	}
	switch (pic) {
	case 1:
		fprintf
		    (fp,
		     "%d 附图: %s (%d 字节)<br>"
			"<a href='%s'> "
						"<IMG style=\" max-width:800px; width: expression(this.width > 800 ? 800: true); height:auto\" SRC='%s'  border=0/> </a>",
		//"<img src='%s'></img>",
		     ano, attachname, size, link, link);
		break;
	case 2:
		fprintf(fp,
			"%d Flash动画: "
			"<a href='%s'>%s</a> (%d 字节)<br>"
			"<OBJECT><PARAM NAME='MOVIE' VALUE='%s'>"
			"<EMBED SRC='%s' width=480 height=360></EMBED></OBJECT>",
			ano, link, attachname, size, link, link);
		break;
	default:
		fprintf(fp,
			"%d 附件: <a href='%s'>%s</a> (%d 字节)",
			ano, link, attachname, size);
	}
}

/* show_iframe: 0  just as normail
		1  just show iframe
		2  show the content of iframe
 */
int
fshowcon(FILE * output, char *filename, int show_iframe)
{
	char *ptr, buf[512];
	FILE *fp;
	int lastq = 0, ano = 0, in_sig = 0;
	if (show_iframe != 2) {
//		fprintf(output, "<table width=100%% border=1><tr>");
		fprintf(output, "<tr><td width=40 class=\"level1\">&nbsp;</td>\n<td class=\"level1\"><br><TABLE width=\"95%%\" cellpadding=5 cellspacing=0><TBODY>\n<tr><td class=tdtitletheme>&nbsp;</td></tr><tr>\n");
		if (testmozilla() && wwwstylenum % 2)
			fprintf(output, "<td class=\"bordertheme\">\n");
		else
			fprintf(output, "<td class=\"bordertheme\">\n");
		if (show_iframe == 1) {
			char interurl[256];
			if (via_proxy)
				snprintf(interurl, sizeof (interurl),
					 "/" SMAGIC "/%s+%s+%s", filename,
					 getparm("T"), usingMath?"m":"");  
			else
				snprintf(interurl, sizeof (interurl),
					 "http://%s:%d/" SMAGIC "/%s+%s%s",
					 inet_ntoa(wwwcache->accel_addr),
					 wwwcache->accel_port, filename,
					 getparm("T"), usingMath?"m":"");
			//modify by macintosh 050619 for Tex Math Equ
			fprintf(output, "<script src=\"%s\"></script>", interurl);
			fputs("\n</td></tr></table>\n", output);
			return 0;
		}
	}
	fp = fopen(filename, "r");
	if (fp == 0)
		return -1;
	fdisplay_attach(NULL, NULL, NULL, NULL);
	printf("<div id='filecontent' style='width:800px;'>\n");
	while (1) {
		if (fgets(buf, sizeof (buf), fp) == 0)
			break;
		if (!strncmp(buf, "begin 644 ", 10)) {
			ano++;
			ptr = strrchr(filename, '/');
			errlog("old attach %s", filename);
			fdisplay_attach(output, fp, buf, ptr + 1);
			fprintf(output, "\n<br>");
			continue;
		}
		if (!strncmp(buf, "beginbinaryattach ", 18)) {
			unsigned int len;
			char ch;
			fread(&ch, 1, 1, fp);
			if (ch != 0) {
				ungetc(ch, fp);
				fhhprintf(output, "%s", buf);
				continue;
			}
			ptr = strchr(buf, '\r');
			if (ptr)
				*ptr = 0;
			ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			ano++;
			ptr = buf + 18;
			fread(&len, 4, 1, fp);
			len = ntohl(len);
			fprintbinaryattachlink(output, ano, ptr,
					       -4 + (int) ftell(fp), len, NULL,
					       NULL);
			fseek(fp, len, SEEK_CUR);
			continue;
		}
		/*
		   if (!strncmp(buf, "发信人: ", 8)) {
		   ptr = strdup(buf);
		   id = strtok(ptr + 8, " ");
		   s = strtok(0, "");
		   if (id == 0)
		   id = " ";
		   if (s == 0)
		   s = "\n";
		   if (strlen(id) < 13 && getuser(id)) {
		   fprintf(output, "发信人: %s%s", userid_str(id),
		   s);
		   fprintf(output, "<br>");
		   free(ptr);
		   continue;
		   }
		   free(ptr);
		   } */
		if (buf[0] == ':' && buf[1] == ' ') {
			if (!lastq)
				fprintf(output, "<font color=808080>");
			lastq = 1;
		} else {
			if (lastq)
				fprintf(output, "</font>");
			lastq = 0;
		}

		if (!strncmp(buf, "--\n", 3)) {
			if (!in_sig) {
				fprintf(output, "<div class=\"con_sig\">");
				in_sig = 1;
			}
		}
		fhhprintf(output, "%s", buf);
	}
	printf("</div>\n");
	if (lastq)
		fprintf(output, "</font>");
	if (in_sig)
		fprintf(output, "</div>");
	fclose(fp);
	if (show_iframe != 2)
		fprintf(output, "\n</td></TR></TBODY></TABLE><br></td></tr>\n");
		//	"</table></td></tr></table></td></tr></table>\n");
	return 0;
}

int
showcon(char *filename)
{
	return fshowcon(stdout, filename, 0);
}

int
showconxml(char *filename, int viewertype)
{
	char filetmp[200], cmd[200], *ptr, *pend;
	struct mmapfile mf = { ptr:NULL };
	FILE *fp;
	int retv = -1;
	if (viewertype != 1)
		printf
		    ("<br>本文使用了<a href=home/boards/BBSHelp/html/itex/itexintro.html target=_blank>Tex数学公式</a><br>");
	fp = fopen("bbstmpfs/tmp/testxml.txt", "w");
	fshowcon(fp, filename, 0);
	fclose(fp);
	sprintf(filetmp, "bbstmpfs/tmp/xml.%d.tmp", thispid);
	if (viewertype != 1)
		sprintf(cmd, MY_BBS_HOME "/bin/tidy -iso2022 -f /dev/null | "
			MY_BBS_HOME "/bin/itex2MML |" MY_BBS_HOME
			"/bin/mathml4mathplayer > %s", filetmp);
	else
		sprintf(cmd,
			MY_BBS_HOME "/bin/tidy -iso2022 -f /dev/null | "
			MY_BBS_HOME
			"/bin/itex2MML | sed -es/'<br>'/'<br \\/>'/g > %s",
			filetmp);
	fp = popen(cmd, "w");
	if (!fp)
		return -1;
	fshowcon(fp, filename, 0);
	pclose(fp);
	MMAP_TRY {
		if (mmapfile(filetmp, &mf) < 0) {
			unlink(filetmp);
			MMAP_RETURN(-1);
		}
		ptr = strncasestr(mf.ptr, "<body>", mf.size);
		if (!ptr) {
			mmapfile(NULL, &mf);
			unlink(filetmp);
			MMAP_RETURN(retv);
		}
		ptr += 6;
		pend = strncasestr(ptr, "</body>", mf.size - (ptr - mf.ptr));
		if (!pend)
			pend = mf.ptr + mf.size;
		fwrite(ptr, pend - ptr, 1, stdout);
		if (strncasestr(ptr, "</table>", pend - ptr) == NULL)
			printf("</td></tr></table>");
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	unlink(filetmp);
	return retv;
}

int
showcon_cache(char *filename, int level, int edittime)
{
	char showfile[NAME_MAX + 1];
	static char showpath[PATH_MAX + 1];
	struct stat st;
	char *ptr = NULL;
	FILE *fp;
	int fd;
	if (!level)
		return fshowcon(stdout, filename, 0);
	if (!edittime)
		strncpy(showfile, filename, NAME_MAX);
	else
		snprintf(showfile, NAME_MAX, "%s:%d", filename, edittime);
	normalize(showfile);
	snprintf(showpath, PATH_MAX, "%s/%s", ATTACHCACHE, showfile);
	if (access(showpath, R_OK)) {
		if (level < 2)
			return fshowcon(stdout, filename, 0);
		fp = fopen(showpath, "w");
		if (!fp)
			return fshowcon(stdout, filename, 0);
		fshowcon(fp, filename, 0);
		fclose(fp);
		//printf("<!--make cache-->");
	}
	//printf("<!--show cache-->");

	fd = open(showpath, O_RDONLY);
	if (fd < 0) {
		fshowcon(stdout, filename, 0);
		return -1;
	}
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)
	    || st.st_size <= 0) {
		close(fd);
		fshowcon(stdout, filename, 0);
		return -1;
	}
	MMAP_TRY {
		ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		if (ptr == NULL) {
			fshowcon(stdout, filename, 0);
			MMAP_RETURN(-1);
		}
		fwrite(ptr, st.st_size, 1, stdout);
	}
	MMAP_CATCH {
		close(fd);
	}
	MMAP_END munmap(ptr, st.st_size);
	return 0;
}

int
testmozilla()
{
	char *ptr = getsenv("HTTP_USER_AGENT");
	if (strcasestr(ptr, "Mozilla") && !strcasestr(ptr, "compatible"))
		return 1;
	return 0;
}

int
testxml()
{
	char *ptr = getsenv("HTTP_USER_AGENT");
	if (strcasestr(ptr, "Mozilla/5") && !strcasestr(ptr, "compatible"))
		return 1;
	if (strcasestr(ptr, "MSIE 6.") || strcasestr(ptr, "MSIE 5.5"))
		return 2;
	return 0;
}

void   
processMath()   
{	//add by macintosh 050619 for Tex Math Equ
	if(usedMath) {
		printf("<script src=/jsMath/jsMath.js></script>");
		printf("<script>jsMath.ProcessBeforeShowing();</script>");
	}   
}   


int
bbscon_main()
{	//modify by mintbaggio 050526 for new www
	char board[80], dir[80], file[80], filename[80], fileback[80], *ptr;
	char buf[2048];
	char bmbuf[IDLEN * 4 + 4];
	int thread;
	int nbuf = 0, usexml = 0;
	struct fileheader *x = NULL, *dirinfo = NULL;
	struct boardmem *bx;
	int num, total, sametitle;
	int prenum, nextnum;
	int outgoing;
	int inndboard;
	struct mmapfile mf = { ptr:NULL };
    char title_utf8[480];
    memset(title_utf8, '\0', sizeof(title_utf8));

	changemode(READING);

	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("F"), 32);
	if (!file[0])
		strsncpy(file, getparm("file"), 32);
	num = atoi((ptr = getparm("N"))) - 1;
	if (!ptr[0])
		num = atoi(getparm("num")) - 1;
	sametitle = atoi(getparm("st"));

	if ((bx = getboard(board)) == NULL)
		http_fatal("错误的讨论区");
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2) && strncmp(file, "T.", 2))	//modify by mintbaggio 040517 for new www
		http_fatal("错误的参数1");
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("错误的参数2");
	sprintf(filename, "boards/%s/%s", board, file);
	if (*getparm("attachname") == '/') {
		showbinaryattach(filename);
		return 0;
	}

	// 删除回复提醒开始 by IronBlood
	time_t article_id = fn2timestamp(file);
	if(is_post_in_notification(currentuser.userid, board, article_id)) {
		del_post_notification(currentuser.userid, board, article_id);
	}
	// 删除回复提醒结束

	strcpy(fileback, file);
	sprintf(dir, "boards/%s/.DIR", board);
	total = bx->total;
	inndboard = bx->header.flag & INNBBSD_FLAG;
	if (total <= 0)
		http_fatal("此讨论区不存在或者为空");
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("此讨论区不存在或者为空");
		}
		dirinfo = findbarticle(&mf, file, &num, 1);
		if (dirinfo == NULL) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文不存在或者已被删除");
		}

#if 1		
		html_header(1);		
		if  (dirinfo->accessed & FH_MATH) {			
			usingMath = 1;			
			usedMath = 1;			
			withinMath = 0;		
			} else {			
			usingMath = 0;		
			}
#else
		if (dirinfo->accessed & FH_MATH && (usexml = testxml())) {
			html_header(100 + usexml - 1);
		} else {
			usexml = 0;
			html_header(1);
		}
#endif
		check_msg();
	// output post title and link by IronBlood@bmy 2011.12.06
	x = (struct fileheader *)(mf.ptr + num * sizeof (struct fileheader));
    g2u(x->title,sizeof(x->title),title_utf8,sizeof(title_utf8));
	printf("<title>%s | 兵马俑BBS</title>", x->title);

		printf("<script src='/function.js'></script></head>\n");

		printf("<body leftmargin=0 topmargin=0>\n<img src=\"/images/bmy.gif\" style=\"position: absolute;top:-160px;\"/>\n");
		printf("%s", "<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
			"<tr><td height=30 colspan=2> \n"
			"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
			"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
			"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n");
		if (loginok && !isguest && (dirinfo->accessed & FH_ATTACHED))
			printf
			    ("<a href=bbsmywww><font color=red>看不了图片？</font></a>");
		if (loginok && !isguest && ((!via_proxy && wwwcache->accel_ip
		    && wwwcache->accel_port)||via_proxy) && ! w_info->doc_mode)
			printf
			    ("<a href=bbsmywww><font color=red>看不了文章？</font></a>");

		printf("<tr><td><a href=\"boa?secstr=%s\">%s</a> / <a href=\"%s%s\">%s</a> / 阅读文章 "
			"</td></tr></table></td>\n", bx->header.sec1, nohtml(getsectree(bx->header.sec1)->title), showByDefMode(), board, board);
		printf("<td><table border=0 align=right cellpadding=0 cellspacing=0>\n"
			"<tr><td> 版主 %s</tr></table></td></tr></table></td></tr>\n",
			userid_str(bm2str(bmbuf, &(bx->header))));
		if (dirinfo->accessed & FH_ALLREPLY) {
			FILE *fp;
			fp = fopen("bbstmpfs/dynamic/Bvisit_log", "a");
			if (NULL != fp) {
				fprintf(fp,
					"www user %s from %s visit %s %s %s",
					currentuser.userid, fromhost,
					bx->header.filename, fh2fname(dirinfo),
					ctime(&now_t));
				fclose(fp);
			}
		}
		if (dirinfo->owner[0] == '-') {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文已被删除");
		}
		thread = dirinfo->thread;
		printf("<tr><td height=70 colspan=2>\n"
			"<table width=\"100%%\" height=\"100%%\" border=0 cellpadding=0 cellspacing=0 bgcolor=#efefef>\n"
			"<tr><td width=40>&nbsp; </td>\n"
			"<td height=70>\n"
			"<table width=\"95%%\" height=\"100%%\"  border=0 cellpadding=0 cellspacing=0>\n"
			"<tr><td colspan=2 valign=bottom>\n"
			"<table width=\"100%%\" border=0 cellpadding=0 cellspacing=0>\n");
		nbuf = sprintf(buf, "<tr><td><div class=\"menu\">\n<DIV class=btncurrent>&lt;%s&gt;</DIV>\n", void1((unsigned char *)titlestr(bx->header.title)));
		nbuf += sprintf(buf+nbuf, 
				"<A href='fwd?B=%s&amp;F=%s' class=btnfunc>/ 转寄</A>\n",
				board, file);
		nbuf +=
		    sprintf(buf + nbuf, 
			"<DIV><A href='ccc?B=%s&amp;F=%s' class=btnfunc>/ 转贴</a>\n",
			board, file);

		if (num >= 0 && num < total) {
		brc_initial(currentuser.userid, board);
		brc_add_read(dirinfo);
		brc_update(currentuser.userid);
	}
	if (!strncmp(currentuser.userid, dirinfo->owner, IDLEN + 1)) {
		//|| has_BM_perm(&currentuser, bx)) {
		nbuf += sprintf
		    (buf + nbuf,
		     "<A onclick='return confirm(\"你真的要删除本文吗?\")' href='del?B=%s&amp;F=%s' class=btnfunc>/ 删除</a>\n",
		     board, file);
		nbuf += sprintf(buf + nbuf,
				"<A href='edit?B=%s&amp;F=%s' class=btnfunc>/ 修改</a>\n",
				board, file);
	}
	ptr = dirinfo->title;
	if (!strncmp(ptr, "Re: ", 4))
		ptr += 4;
	ptr[60] = 0;
	outgoing = (dirinfo->accessed & FH_INND)
	    || strchr(dirinfo->owner, '.');
	
	fputs(buf, stdout);
	nbuf = 0;
	nbuf += sprintf(buf + nbuf,
		"<a href='pstmail?B=%s&amp;F=%s&amp;num=%d' class=btnfunc title=\"回信给作者 accesskey: m\" accesskey=\"m\">/ 回信给作者</a>\n", board, file, num);
	nbuf += sprintf(buf + nbuf,
		"<a href='tfind?B=%s&amp;th=%lu&amp;T=%s' class=btnfunc>/ 同主题列表</a>\n", board, (long)dirinfo->thread, encode_url((unsigned char *)ptr));
	nbuf += sprintf(buf + nbuf,
		"<a href='bbstcon?board=%s&amp;start=%d&amp;th=%lu' class=btnfunc>/ 同主题由此展开</a>\n", board, num, (long)dirinfo->thread);
	nbuf += sprintf(buf + nbuf,
		"<a href='%s%s&amp;S=%d' class=btnfunc title=\"返回讨论区 accesskey: b\" accesskey=\"b\">/ 返回讨论区</a>\n",
		showByDefMode(), board, (num > 4) ? (num - 4) : 1);
	nbuf += sprintf(buf + nbuf,
			"</div></td></tr></table></td></tr>\n");
	nbuf += sprintf(buf + nbuf,
			"<tr><td width=\"60%%\">");

	if (!(dirinfo->accessed & FH_NOREPLY))
		nbuf += sprintf(buf + nbuf,
	"<a href='pst?B=%s&amp;F=%s&amp;num=%d%s' class=btnsubmittheme title=\"回复本文 accesskey: r\" accesskey=\"r\">回复本文</a> </td>\n", board, file, num, outgoing ? "" : (inndboard ? "&amp;la=1" : ""));

	nbuf += sprintf(buf + nbuf, "<td width=\"40%%\" align=right>分享到 ");
    char *encoded_title = url_encode(title_utf8);
    nbuf += sprintf(buf + nbuf, "<a href=\"#\" onclick=\"javascript:share('sina','%s','%s','%s');\"><img src=\"/images/share-sina.png\"/></a> ",encoded_title,board,file);
    nbuf += sprintf(buf + nbuf, "<a href=\"#\" onclick=\"javascript:share('renren','%s','%s','%s');\"><img src=\"/images/share-rr.png\"/></a> ",encoded_title,board,file);
	nbuf += sprintf(buf + nbuf, "<a href=\"#\" onclick=\"javascript:share('tencent','%s','%s','%s');\"><img src=\"/images/share-tencent.png\"/></a> | ",encoded_title,board,file);

	free(encoded_title);
		if (sametitle) {
			prenum = num - 1;
			nextnum = num + 1;
			while (prenum >= 0 && num - prenum < 100) {
				x = (struct fileheader *) (mf.ptr +
							   prenum *
							   sizeof (struct
								   fileheader));
				if (x->thread == thread)
					break;
				prenum--;
			}
			if (prenum >= 0 && num - prenum < 100)
				nbuf += sprintf
				    (buf + nbuf,
				     "<a href='con?B=%s&amp;F=%s&amp;N=%d&amp;st=1&amp;T=%lu'>同主题上篇 </a>",
				     board, fh2fname(x), prenum + 1, feditmark(*x));
			nbuf += sprintf(buf + nbuf,
					"<a href='%s%s&amp;S=%d'>本讨论区 </a>",
					showByDefMode(), board, (num > 4) ? (num - 4) : 1);
			while (nextnum < total && nextnum - num < 100) {
				x = (struct fileheader *) (mf.ptr +
							   nextnum *
							   sizeof (struct
								   fileheader));
				if (x->thread == thread)
					break;
				nextnum++;
			}
			if (nextnum < total && nextnum - num < 100)
				nbuf += sprintf
				    (buf + nbuf,
				     "<a href='con?B=%s&amp;F=%s&amp;N=%d&amp;st=1&amp;T=%lu'>同主题下篇</a>",
				     board, fh2fname(x), nextnum + 1, feditmark(*x));
		} else {
			if (num > 0) {
				x = (struct fileheader *) (mf.ptr +
							   (num -
							    1) *
							   sizeof (struct
								   fileheader));
				nbuf +=
				    sprintf(buf + nbuf,
					    "<a href='con?B=%s&amp;F=%s&amp;N=%d&amp;T=%lu' title=\"上篇 accesskey: f\" accesskey=\"f\">上篇 </a>",
					    board, fh2fname(x), num, feditmark(*x));
			}
			nbuf +=
			    sprintf(buf + nbuf,
				    "<a href='%s%s&amp;S=%d' title=\"本讨论区 accesskey: c\" accesskey=\"c\">本讨论区 </a>",
				    showByDefMode(), board, (num > 4) ? (num - 4) : 1);
			if (num < total - 1) {
				x = (struct fileheader *) (mf.ptr +
							   (num +
							    1) *
							   sizeof (struct
								   fileheader));
				nbuf +=
				    sprintf(buf + nbuf,
					    "<a href='con?B=%s&amp;F=%s&amp;N=%d&amp;T=%lu' title=\"下篇 accesskey: n\" accesskey=\"n\">下篇</a>",
					    board, fh2fname(x), num + 2, feditmark(*x));
			}
		}
		nbuf += sprintf(buf+nbuf, "</td></tr></table></td></tr>\n");
	}
	MMAP_CATCH {
		mmapfile(NULL, &mf);
		MMAP_RETURN(-1);
	}
	MMAP_END mmapfile(NULL, &mf);

	fputs(buf, stdout);
	if (usexml)
		showconxml(filename, usexml);
	else if (hideboard(board)
		 || (!via_proxy
		     && (!wwwcache->accel_ip || !wwwcache->accel_port))
		 || w_info->doc_mode)
		fshowcon(stdout, filename, 0);
	else
		fshowcon(stdout, filename, 1);

	printf("<tr><td></td><td height=\"20\" valign=\"middle\">");
	memset(fileback, 0, 80);
	sprintf(fileback, "http://bbs.xjtu.edu.cn/BMY/con?B=%s&F=%s", board,file);
	printf("本文链接&nbsp;&nbsp;<a href=' %s'>%s</a>", fileback, fileback);
	printf("</td></tr>");
	printf("</table></td></tr></table></td></tr></table>\n");
#ifdef ENABLE_MYSQL
	if (loginok && now_t - atoi(file + 2) <= 3 * 86400) {
		printf("<br /><script>eva('%s','%s');</script>", board, file);
	}
#endif
    
    
	processMath();  
	printf("</body></html>\n");

	return 0;
}
