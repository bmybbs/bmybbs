#include "bbslib.h"
#include "bmy/convcode.h"

extern char *cginame;
bool g_has_code = false;

int testmozilla(void);
extern char *get_mime_type(const char *name);

int showbinaryattach(char *filename) {
	char *attachname;
	int pos_i;
	size_t pos;
	unsigned int size;
	struct mmapfile mf = { .ptr = NULL };
//      no_outcache();
	if (cache_header(file_time(filename), 86400))
		return 0;
	attachname = getparm("attachname");
	pos_i = atoi(getparm("attachpos"));
	if (pos_i < 0) {
		http_fatal("无法打开附件");
		return -1;
	}

	pos = pos_i;
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

		printf("Content-Length: %d", size);
		printf("Content-type: %s\n\n", get_mime_type(attachname));
		fwrite(mf.ptr + pos + 4, 1, size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

void fprintbinaryattachlink(FILE * fp, int ano, char *attachname, int pos, int size, char *alt, char *alt1) {
	char *ext, link[256], *ptr, *board;
	int pic = 0;
	struct boardmem *x;

	//check read_perm for guest, refer to has_read_perm()
	board = getparm2("B", "board");
	x = getboard(board);
	if (x && !x->header.clubnum && !x->header.level) {
		ptr = "/" SMAGIC "/";
	} else {
		ptr = "";
	}

	if (alt) {
		//这种情况目前可以用atthttpd
		if (w_info->att_mode)
			snprintf(link, sizeof (link),
				"%s%s&attachpos=%d&attachname=/%s",
				ptr, alt, pos, attachname);
		else
			snprintf(link, sizeof (link),"/attach/%s/%d/%s", alt1, pos, attachname);

	} else if (!strcmp(cginame, "bbscon")) {
		//同上
		if (w_info->att_mode)
			snprintf(link, sizeof (link),
				"%sbbscon/%s?B=%s&F=%s&attachpos=%d&attachname=/%s",
				ptr, attachname, board, getparm2("F", "file"),
				pos, attachname);
		else
			snprintf(link, sizeof (link),"/attach/%s/%s/%d/%s", board, getparm2("F", "file"), pos, attachname);
	} else
		snprintf(link, sizeof (link),
			"bbscon/%s?%s&attachpos=%d&attachname=/%s",
			attachname, getsenv("QUERY_STRING"), pos,
			attachname);

	if ((ext = strrchr(attachname, '.')) != NULL) {
		if (!strcasecmp(ext, ".bmp") || !strcasecmp(ext, ".jpg")
				|| !strcasecmp(ext, ".gif")
				|| !strcasecmp(ext, ".jpeg")
				|| !strcasecmp(ext, ".png")
				|| !strcasecmp(ext, ".pcx"))
			pic = 1;
		else if (!strcasecmp(ext, ".mp4"))
			pic = 2;
		else
			pic = 0;
	}
	switch (pic) {
	case 1:
		fprintf(fp, "%d 附图: <a href='%s'>%s</a> (%d 字节)<br>"
			"<IMG style=\" max-width:800px; width: expression(this.width > 800 ? 800: true); height:auto\" SRC='%s'  border=0/></a>",
			ano, link, attachname, size, link);
		break;
	case 2:
		fprintf(fp, "%d 视频: <a href='%s'>%s</a> (%d 字节)<br>"
			"<video controls src='%s'>",
			ano, link, attachname, size, link);
		break;
	default:
		fprintf(fp, "%d 附件: <a href='%s'>%s</a> (%d 字节)",
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
	bool in_code_block = false;
	if (show_iframe != 2) {
//		fprintf(output, "<table width=100%% border=1><tr>");
		fprintf(output, "<tr><td width=40 class=\"level1\">&nbsp;</td>\n<td class=\"level1\"><br><TABLE width=\"95%%\" cellpadding=5 cellspacing=0><TBODY>\n<tr><td class=tdtitletheme>&nbsp;</td></tr><tr>\n");
		fprintf(output, "<td class=\"bordertheme\">\n");
	}
	fp = fopen(filename, "r");
	if (fp == 0)
		return -1;
	printf("<div id='filecontent' style='width:800px;'>\n");
	while (1) {
		if (fgets(buf, sizeof (buf) - 1, fp) == 0)
			break;
		buf[sizeof(buf) - 1] = 0;
		if (!strncmp(buf, "begin 644 ", 10)) {
			ano++;
			ptr = strrchr(filename, '/');
			errlog("old attach %s", filename);
			fdisplay_attach(output, fp, buf, ptr + 1);
			fprintf(output, "\n<br>");
			continue;
		}
		if (!strncmp(buf, "```", 3)) {
			if (in_code_block) {
				in_code_block = false;
				fprintf(output, "</code></pre>");
			} else {
				in_code_block = true;
				g_has_code = true;

				char lang[12 /* 当前最长 unrealscript */ + 1];
				char *p = lang;
				ytht_strsncpy(lang, buf + 3, sizeof(lang));
				while (*p) {
					if (*p >= 'A' && *p <= 'Z') {
						*p += 32;
					}

					if (!(*p >= 'a' && *p <= 'z')) {
						*p = 0;
						break;
					}
					p++;
				}

				fprintf(output, "<pre><code class=\"language-%s\">", lang);
			}
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
			fprintbinaryattachlink(output, ano, ptr, -4 + (int) ftell(fp), len, NULL, NULL);
			fseek(fp, len, SEEK_CUR);
			continue;
		}
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
	return 0;
}

int
showcon(char *filename)
{
	return fshowcon(stdout, filename, 0);
}

int testmozilla() {
	char *ptr = getsenv("HTTP_USER_AGENT");
	if (strcasestr(ptr, "Mozilla") && !strcasestr(ptr, "compatible"))
		return 1;
	return 0;
}

void
processMath()
{
	// add by macintosh 050619 for Tex Math Equ
	// switching from jsMath to mathjax by IronBlood Feb 14th, 2021.
	if(usedMath) {
		printf("<script src=\"/node_modules/mathjax-full/es5/tex-svg.js\"></script>");
		printf("<style>mjx-container { font-size: 120%% }</style>");
	}
}

int
bbscon_main()
{	//modify by mintbaggio 050526 for new www
	char board[32 /* max 24 */], dir[80], file[32], filename[80], fileback[128], *ptr;
	char buf[2048];
	char bmbuf[IDLEN * 4 + 4];
	int thread;
	int nbuf = 0;
	struct fileheader *x = NULL, *dirinfo = NULL;
	struct boardmem *bx;
	int num, total, sametitle;
	int prenum, nextnum;
	int outgoing;
	int inndboard;
	struct mmapfile mf = { .ptr = NULL };
	char title_utf8[480];
	memset(title_utf8, '\0', sizeof(title_utf8));

	changemode(READING);

	ytht_strsncpy(board, getparm("B"), sizeof(board));
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), sizeof(board));
	ytht_strsncpy(file, getparm("F"), sizeof(file));
	if (!file[0])
		ytht_strsncpy(file, getparm("file"), sizeof(file));
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

		html_header(1);
		if (dirinfo->accessed & FH_MATH) {
			usingMath = 1;
			usedMath = 1;
			withinMath = 0;
		} else {
			usingMath = 0;
		}
		check_msg();
		// output post title and link by IronBlood@bmy 2011.12.06
		x = (struct fileheader *)(mf.ptr + num * sizeof (struct fileheader));
		g2u(x->title, strlen(x->title), title_utf8, sizeof(title_utf8));
		printf("<title>%s | 兵马俑BBS</title>", x->title);

		printf("<script src='/function.js'></script></head>\n");

		printf("<body leftmargin=0 topmargin=0>\n<img src=\"/images/bmy.gif\" style=\"position: absolute;top:-160px;\"/>\n");
		printf("%s", "<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
			"<tr><td height=30 colspan=2> \n"
			"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
			"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
			"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n");
		if (loginok && !isguest && (dirinfo->accessed & FH_ATTACHED))
			printf("<a href=bbsmywww><font color=red>看不了图片？</font></a>");

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
		nbuf = sprintf(buf, "<tr><td><div class=\"menu\">\n<DIV class=btncurrent>&lt;%s&gt;</DIV>\n", void1(titlestr(bx->header.title)));
		nbuf += sprintf(buf+nbuf,
			"<A href='fwd?B=%s&F=%s' class=btnfunc>/ 转寄</A>\n",
			board, file);
		nbuf += sprintf(buf + nbuf,
			"<DIV><A href='ccc?B=%s&F=%s' class=btnfunc>/ 转贴</a>\n",
			board, file);

		if (num >= 0 && num < total) {
			brc_initial(currentuser.userid, board);
			brc_add_read(dirinfo);
			brc_update(currentuser.userid);
		}
		if (!strncmp(currentuser.userid, dirinfo->owner, IDLEN + 1)) {
			//|| has_BM_perm(&currentuser, bx)) {
			nbuf += sprintf(buf + nbuf,
				"<A onclick='return confirm(\"你真的要删除本文吗?\")' href='del?B=%s&F=%s' class=btnfunc>/ 删除</a>\n",
				board, file);
			nbuf += sprintf(buf + nbuf,
				"<A href='edit?B=%s&F=%s' class=btnfunc>/ 修改</a>\n",
				board, file);
		}
		dirinfo->title[sizeof(dirinfo->title) - 1] = 0;
		ptr = dirinfo->title;
		if (!strncmp(ptr, "Re: ", 4))
			ptr += 4;
		outgoing = (dirinfo->accessed & FH_INND) || strchr(dirinfo->owner, '.');

		fputs(buf, stdout);
		nbuf = 0;
		nbuf += sprintf(buf + nbuf,
			"<a href='pstmail?B=%s&F=%s&num=%d' class=btnfunc title=\"回信给作者 accesskey: m\" accesskey=\"m\">/ 回信给作者</a>\n", board, file, num);
		nbuf += sprintf(buf + nbuf,
			"<a href='tfind?B=%s&th=%lu&T=%s' class=btnfunc>/ 同主题列表</a>\n", board, (long)dirinfo->thread, encode_url(ptr));
		nbuf += sprintf(buf + nbuf,
			"<a href='bbstcon?board=%s&start=%d&th=%lu' class=btnfunc>/ 同主题由此展开</a>\n", board, num, (long)dirinfo->thread);
		nbuf += sprintf(buf + nbuf,
			"<a href='%s%s&S=%d' class=btnfunc title=\"返回讨论区 accesskey: b\" accesskey=\"b\">/ 返回讨论区</a>\n",
			showByDefMode(), board, (num > 4) ? (num - 4) : 1);
		nbuf += sprintf(buf + nbuf, "</div></td></tr></table></td></tr>\n");
		nbuf += sprintf(buf + nbuf, "<tr><td width=\"60%%\">");

		if (!(dirinfo->accessed & FH_NOREPLY))
			nbuf += sprintf(buf + nbuf,
		"<a href='pst?B=%s&F=%s&num=%d%s' class=btnsubmittheme title=\"回复本文 accesskey: r\" accesskey=\"r\">回复本文</a> </td>\n", board, file, num, outgoing ? "" : (inndboard ? "&la=1" : ""));

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
				x = (struct fileheader *) (mf.ptr + prenum * sizeof (struct fileheader));
				if (x->thread == thread)
					break;
				prenum--;
			}
			if (prenum >= 0 && num - prenum < 100)
				nbuf += sprintf(buf + nbuf,
					"<a href='con?B=%s&F=%s&N=%d&st=1&T=%lu'>同主题上篇 </a>",
					board, fh2fname(x), prenum + 1, feditmark(*x));
			nbuf += sprintf(buf + nbuf,
					"<a href='%s%s&S=%d'>本讨论区 </a>",
					showByDefMode(), board, (num > 4) ? (num - 4) : 1);
			while (nextnum < total && nextnum - num < 100) {
				x = (struct fileheader *) (mf.ptr + nextnum * sizeof (struct fileheader));
				if (x->thread == thread)
					break;
				nextnum++;
			}
			if (nextnum < total && nextnum - num < 100)
				nbuf += sprintf(buf + nbuf,
					"<a href='con?B=%s&F=%s&N=%d&st=1&T=%lu'>同主题下篇</a>",
					board, fh2fname(x), nextnum + 1, feditmark(*x));
		} else {
			if (num > 0) {
				x = (struct fileheader *) (mf.ptr + (num - 1) * sizeof (struct fileheader));
				nbuf += sprintf(buf + nbuf,
					"<a href='con?B=%s&F=%s&N=%d&T=%lu' title=\"上篇 accesskey: f\" accesskey=\"f\">上篇 </a>",
					board, fh2fname(x), num, feditmark(*x));
			}
			nbuf += sprintf(buf + nbuf,
				"<a href='%s%s&S=%d' title=\"本讨论区 accesskey: c\" accesskey=\"c\">本讨论区 </a>",
				showByDefMode(), board, (num > 4) ? (num - 4) : 1);
			if (num < total - 1) {
				x = (struct fileheader *) (mf.ptr + (num + 1) * sizeof (struct fileheader));
				nbuf += sprintf(buf + nbuf,
					"<a href='con?B=%s&F=%s&N=%d&T=%lu' title=\"下篇 accesskey: n\" accesskey=\"n\">下篇</a>",
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
	fshowcon(stdout, filename, 0);

	printf("<tr><td></td><td height=\"20\" valign=\"middle\">");
	snprintf(fileback, sizeof(fileback), "%s/%s/con?B=%s&F=%s", MY_BBS_DOMAIN, SMAGIC, board, file);
	printf("本文链接&nbsp;&nbsp;<a href='//%s'>%s</a>", fileback, fileback);
	printf("</td></tr>");
	printf("</table></td></tr></table></td></tr></table>\n");
#ifdef ENABLE_MYSQL
	if (loginok && now_t - atoi(file + 2) <= 3 * 86400) {
		printf("<br /><script>eva('%s','%s');</script>", board, file);
	}
#endif

	processMath();
	if (g_has_code) {
		printf("<link rel='stylesheet' href='/node_modules/prismjs/themes/prism-tomorrow.css'/>");
		printf("<script src='/node_modules/prismjs/components/prism-core.min.js'></script>");
		printf("<script src='/node_modules/prismjs/plugins/autoloader/prism-autoloader.min.js'></script>");
	}
	printf("</body></html>\n");

	return 0;
}

