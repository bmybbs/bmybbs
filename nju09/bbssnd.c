#include "bbslib.h"

int
testmath(char *ptr)
{
	int i = 0;
	if (!*ptr)
		return 0;
	if (*ptr == '$')
		i = 1;
	ptr++;
	while ((ptr = strchr(ptr, '$')) != NULL) {
		if (*(ptr - 1) != '\\')
			i++;
		ptr++;
	}
	return (i + 1) % 2;
}

int
bbssnd_main()
{
	char filename[80], dir[80], board[80], title[80], buf[256], userid[20] ,*content,
	    *ref;
	int r, i, sig, mark = 0, outgoing, anony, guestre = 0, usemath, nore, mailback;
	int is1984, to1984 = 0;
	struct boardmem *brd;
	struct fileheader *x = NULL;
	int thread = -1;
	struct mmapfile mf = { ptr:NULL };
	html_header(1);

	strsncpy(board, getparm("board"), 18);
	strsncpy(title, getparm("title"), 60);
	strsncpy(userid, getparm("replyto"), 14);
	outgoing = strlen(getparm("outgoing"));
	anony = strlen(getparm("anony"));
	usemath = strlen(getparm("usemath"));
	nore = strlen(getparm("nore"));
	mailback = strlen(getparm("mailback"));
	brd = getboard(board);
	if (brd == 0)
		http_fatal("错误的讨论区名称");
	strcpy(board, brd->header.filename);
	sprintf(dir, "boards/%s/.DIR", board);
	ref = getparm("ref");
	r = atoi(getparm("rid"));
	thread = atoi(getparm("th"));
	if (ref[0]) {
		MMAP_TRY {
			if (mmapfile(dir, &mf) == -1) {
				MMAP_UNTRY;
				http_fatal("错误的讨论区");
			}
			x = findbarticle(&mf, ref, &r, 1);
		}
		MMAP_CATCH {
		}
		MMAP_END mmapfile(NULL, &mf);

		if (x->accessed & FH_NOREPLY)
			http_fatal("本文被设为不可Re模式");

		if (x && (x->accessed & FH_ALLREPLY)) {
			if (strncmp(x->title, "Re: ", 4))
				snprintf(title, 60, "Re: %s", x->title);
			else
				snprintf(title, 60, "%s", x->title);
			guestre = 1;
			mark = mark | FH_ALLREPLY;
		}
		if (x)
			thread = x->thread;
	} else {
		thread = -1;
	}
	if(strcmp(board, "welcome") &&
	   strcmp(board, "KaoYan")){	//add by mintbaggio 040614 for post at "welcome" + "KaoYan"(by wsf)
		if (!loginok || (isguest && !guestre))
			http_fatal("匆匆过客不能发表文章，请先登录");
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost) && (!guestre || !loginok) )
		http_fatal("您的ip被禁止使用guest在本版发表文章!″");
	changemode(POSTING);

	if (!(brd->header.flag & ANONY_FLAG))
		anony = 0;
	if (brd->header.flag & IS1984_FLAG)
		is1984 = 1;
	else
		is1984 = 0;
	for (i = 0; i < strlen(title); i++)
		if (title[i] <= 27 && title[i] >= -1)
			title[i] = ' ';
	i = strlen(title) - 1;
	while (i > 0 && isspace(title[i]))
		title[i--] = 0;
	sig = atoi(getparm("signature"));
	currentuser.signature = sig;
	content = getparm("text");
	printf("text=%s\n", content);
	//http_fatal("just for test");
	if (usemath && testmath(content))
		mark |= FH_MATH;
	if (nore)
		mark |= FH_NOREPLY;
	if (mailback)
		mark |= FH_MAILREPLY;
	if (title[0] == 0)
		http_fatal("文章必须要有标题");
	if(strcmp(board, "welcome") &&
	   strcmp(board, "KaoYan")){   //add by mintbaggio 040614 for post at "welcome" + "KaoYan"(by wsf)
		if (!has_post_perm(&currentuser, brd) && !guestre)
			http_fatal("此讨论区是唯读的, 或是您尚无权限在此发表文章.");
	}
	if (noadm4political(board))
		http_fatal("对不起,因为没有版面管理人员在线,本版暂时封闭.");

	if ((now_t - w_info->lastposttime) < 6) {
		w_info->lastposttime = now_t;
		http_fatal("两次发文间隔过密, 请休息几秒后再试");
	}
	w_info->lastposttime = now_t;
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	f_write(filename, content);
	if (!hideboard_x(brd)) {
		int dangerous =
		    dofilter(title, filename, political_board(board));
		if (dangerous == 1){
			to1984 = 1;
			mail_file(filename, currentuser.userid, title, currentuser.userid);
		}else if (dangerous == 2) {
			char mtitle[256];
			sprintf(mtitle, "[发表报警] %s %.60s", board, title);
			post_mail("delete", mtitle, filename,
				  currentuser.userid, currentuser.username,
				  fromhost, -1, 0);
			updatelastpost("deleterequest");
			mark |= FH_DANGEROUS;
		}
	}
	
	if (userid[0])
		post_mail(userid, title, filename, currentuser.userid, currentuser.username, currentuser.lasthost, sig-1, 0);

	if (insertattachments(filename, content, currentuser.userid))
		mark = mark | FH_ATTACHED;

	if (is1984 || to1984) {
		r = post_article_1984(board, title, filename,
				      currentuser.userid, currentuser.username,
				      fromhost, sig - 1, mark, outgoing,
				      currentuser.userid, thread);
	} else if (anony)
		r = post_article(board, title, filename, "Anonymous",
				 "我是匿名天使", "匿名天使的家", 0, mark,
				 outgoing, currentuser.userid, thread);
	else
		r = post_article(board, title, filename, currentuser.userid,
				 currentuser.username, fromhost, sig - 1, mark,
				 outgoing, currentuser.userid, thread);
	if (r <= 0)
		http_fatal("内部错误，无法发文");
	if (!is1984 && !to1984) {
		brc_initial(currentuser.userid, board);
		brc_add_readt(r);
		brc_update(currentuser.userid);
	}
	unlink(filename);
	sprintf(buf, "%s post %s %s", currentuser.userid, board, title);
	newtrace(buf);
	if (brd->header.clubnum ==0 && !junkboard(board)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}
	if (to1984) {
		printf("%s<br>\n", BAD_WORD_NOTICE);
		printf("[<a href='javascript:history.go(-2)'>返回</a>]");
	} else {
		sprintf(buf, "%s%s", showByDefMode(), board);
		redirect(buf);
	}

	// 发送回帖提醒开始 by IronBlood
	char noti_userid[14] = { '\0' };
	if(x!=NULL && r>0 && strchr(x->owner, '.') == NULL) { // x 不为空（即回复模式）、发帖成功、并且为本站用户
		if(x->owner[0] == '\0') { // 匿名用户
			memcpy(noti_userid, &x->owner[1], IDLEN);
		} else {
			memcpy(noti_userid, x->owner, IDLEN);
		}
		if(strcmp(currentuser.userid, noti_userid) != 0) { // 提醒用户和当前用户不相同的时候
			add_post_notification(noti_userid, (anony) ? "Anonymous" : currentuser.userid, board, r, title);
		}
	} // 发送回帖提醒结束
	return 0;
}
