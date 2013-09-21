#include "bbslib.h"

static int
seek_in_file(filename, seekstr)
char filename[STRLEN], seekstr[STRLEN];
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

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
		http_fatal("���������������");
	strcpy(board, brd->header.filename);
	sprintf(dir, "boards/%s/.DIR", board);
	ref = getparm("ref");
	r = atoi(getparm("rid"));
	thread = atoi(getparm("th"));
	if (ref[0]) {
		MMAP_TRY {
			if (mmapfile(dir, &mf) == -1) {
				MMAP_UNTRY;
				http_fatal("�����������");
			}
			x = findbarticle(&mf, ref, &r, 1);
		}
		MMAP_CATCH {
		}
		MMAP_END mmapfile(NULL, &mf);
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
			http_fatal("�Ҵҹ��Ͳ��ܷ������£����ȵ�¼");
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost) && (!guestre || !loginok) )
		http_fatal("����ip����ֹʹ��guest�ڱ��淢������!��");
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
		http_fatal("���±���Ҫ�б���");
	if(strcmp(board, "welcome") &&
	   strcmp(board, "KaoYan")){   //add by mintbaggio 040614 for post at "welcome" + "KaoYan"(by wsf)
		if (!has_post_perm(&currentuser, brd) && !guestre)
			http_fatal("����������Ψ����, ����������Ȩ���ڴ˷�������.");
	}
	if (noadm4political(board))
		http_fatal("�Բ���,��Ϊû�а��������Ա����,������ʱ���.");

	if ((now_t - w_info->lastposttime) < 6) {
		w_info->lastposttime = now_t;
		http_fatal("���η��ļ������, ����Ϣ���������");
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
			sprintf(mtitle, "[������] %s %.60s", board, title);
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
				 "����������ʹ", "������ʹ�ļ�", 0, mark,
				 outgoing, currentuser.userid, thread);
	else
		r = post_article(board, title, filename, currentuser.userid,
				 currentuser.username, fromhost, sig - 1, mark,
				 outgoing, currentuser.userid, thread);
	if (r <= 0)
		http_fatal("�ڲ������޷�����");
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
		printf("[<a href='javascript:history.go(-2)'>����</a>]");
	} else {
		sprintf(buf, "%s%s", showByDefMode(), board);
		redirect(buf);
	}

	// ���ͻ������ѿ�ʼ by IronBlood
	char noti_userid[14] = { '\0' };
	if(x!=NULL && r>0 && strchr(x->owner, '.') == NULL) { // x ��Ϊ�գ����ظ�ģʽ���������ɹ�������Ϊ��վ�û�
		if(x->owner[0] == '\0') { // �����û�
			memcpy(noti_userid, &x->owner[1], IDLEN);
		} else {
			memcpy(noti_userid, x->owner, IDLEN);
		}
		if(strcmp(currentuser.userid, noti_userid) != 0) { // �����û��͵�ǰ�û�����ͬ��ʱ��
			add_post_notification(noti_userid, (anony) ? "Anonymous" : currentuser.userid, board, r, title);
		}
	} // ���ͻ������ѽ���
	return 0;
}
