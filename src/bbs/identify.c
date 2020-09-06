#include "bbs.h"
#include "identify.h"

#ifdef POP_CHECK
/*
int x_active_user(void);
int active_mail(struct userec* cuser, session_t* session);
int active_phone(struct userec* cuser, session_t* session);
int active_manual(struct userec* cuser, session_t* session);

int continue_active(struct userec* cuser);
*/


int x_active_manager();
// int active_manual_confirm();
int query_active(char* userid);
int force_comfirm(char* userid);
int delete_active(char* userid);
int update_active(char* userid);
int query_value(char* value, int style);
//int update_email(char* value);


static const char *active_style_str[] = {"none", "email", "phone", "idnum", "force", NULL};

/*
// ÓÃ»§°ó¶¨²Ù×÷µÄÈë¿Ú
int x_active_user(void)
{
    char style[4];
    char genbuf[STRLEN];
    struct userec* cuser;
    struct session_t* session;
    cuser=getCurrentUser();
    session=getSession();
    if (cuser->flags & ACTIVATED_FLAG) {
        clear();
        move(2, 0);
        prints("ÄúÒÑ¾­³É¹¦Íê³ÉÁËÑéÖ¤ÊÖĞø!\n");
        prints("»¶Ó­Ê¹ÓÃ±¾Õ¾·şÎñ!");
        pressreturn();
        return 0;
    }

    setactivefile(genbuf,cuser->userid,ACTIVE_FILE);
    //Èç¹ûÎÄ¼ş´æÔÚ£¬ÔñÔø¾­»ñÈ¡¹ıÑéÖ¤Âë
    if (access(genbuf, 0)==0) {
        clear();
        move(2, 0);
        prints("ÄúÒÑ¾­½øĞĞ¹ı»ñÈ¡ÑéÖ¤Âë²Ù×÷£¬µ«Î´½øĞĞÑéÖ¤.\n");
        prints("ÏÖÔÚ¼ÌĞøÑéÖ¤²Ù×÷ÇëÊäÈë 'Y' £¬¸ü»»ÑéÖ¤·½Ê½»òÖØĞÂ»ñÈ¡ÑéÖ¤ÂëÇëÊäÈë 'N' \n");
        getdata(4, 0, "Y/N? [Y] >> ", style, 2, DOECHO, NULL, true);
        if (*style!='N' && *style!='n') {
            continue_active(cuser);
            return 1;
        }
    }
    clear();
    move(2, 0);
    prints("Äú»¹Ã»ÓĞ½øĞĞÊµÃûÖÆÈÏÖ¤£¬ÄúÔÚ±¾Õ¾½«Ã»ÓĞ·¢ÎÄÈ¨ÏŞ¡£\n");
    prints("ÄúµÄÆäÓàÈ¨Àû½«²»ÊÜÓ°Ïì¡£ÎªÁËÕı³£Ê¹ÓÃ±¾Õ¾·şÎñ£¬ÎÒÃÇ½¨ÒéÄúÏÖÔÚ½øĞĞÈÏÖ¤\n\n");
    prints("[1] ac.cnĞÅÏäÈÏÖ¤\n");
    prints("[2] ÊÖ»úºÅÂëÈÏÖ¤\n");
    prints("[3] ÉÏ´«Éí·İÖ¤¼şÈÏÖ¤(ÔİÎ´¿ªÍ¨)\n");
    prints("[4] ÎÒ²»ÏëÈÏÖ¤\n");

    getdata(10, 0, "ÇëÑ¡ÔñÈÏÖ¤·½Ê½ >> ",  style, 2, DOECHO, NULL, true);
    if (!strcmp(style, "1")) {
        active_mail(cuser, session);
        return 1;
    } else if (!strcmp(style, "2")) {
        active_phone(cuser, session);
        return 1;
    } else if (!strcmp(style, "3")) {
        active_manual(cuser, session);
        return 1;
    }

    return 0;
}

//Ôø¾­»ñÈ¡¹ıÑéÖ¤ÂëµÄÓÃ»§¼ÌĞøÑéÖ¤½ø³Ì
int continue_active(struct userec* cuser)
{
    char value[VALUELEN];
    char code[CODELEN+1];
    char incode[CODELEN+1];
    char genbuf[STRLEN];
    int style;
    int response;
    struct active_data act_data;

    move(13, 0);
    clrtobot();
    //¶ÁÈ¡Ôø¾­»ñÈ¡µÄÑéÖ¤Âë
    response=get_active_code(cuser->userid, code, value, &style);
    if (response==FILE_NOT_FOUND) {
        clear();
        move(2, 0);
        prints("Î´ÕÒµ½Ôø¾­ÑéÖ¤µÄ¼ÇÂ¼£¬ÇëÖØĞÂ»ñÈ¡ÑéÖ¤Âë!\n");
        pressreturn();
        return 0;
    }
    prints("ÄúµÄÑéÖ¤·½Ê½:     \t%s\n", style_to_str(style));
    prints("ÄúµÄÑéÖ¤Âë·¢ËÍÖÁ:\t%s\n", value);
    prints("ÇëÊäÈëÄúµÄÑéÖ¤Âë£¬·ÅÆúÑéÖ¤ÇëÖ±½Ó»Ø³µ:\n");
    getdata(16 ,0, ">> ", incode,CODELEN+1,DOECHO,NULL ,true);
    while (strcmp(code, incode)!=0) {
        if (!incode[0]) {
            return 0;
        }
        getdata(16 ,0, "ÑéÖ¤Âë´íÎó£¬ÇëÖØĞÂÊäÈë >> ", incode,CODELEN+1,DOECHO,NULL ,true);
    }
   if (query_record_num(value, style)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("ÄúÒÑ¾­ÑéÖ¤¹ıÈı¸öid£¬ÎŞ·¨ÔÙÓÃÓÚÑéÖ¤ÁË!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    read_active(cuser->userid, &act_data);
    act_data.status=style;
    strcpy(act_data.ip, cuser->lasthost);
    response=write_active(&act_data);

    if (response==WRITE_SUCCESS || response==UPDATE_SUCCESS) {
        cuser->flags |= ACTIVATED_FLAG;
        clear();
        move(3, 0);
        prints("  ÑéÖ¤³É¹¦!");
	 //±¾Ğ£Ñ§Éú
	 str_to_lowercase(value);
	 if (style==MAIL_ACTIVE && !strcmp(strstr(value, "@")+1, "mails.gucas.ac.cn")) {
	 	cuser->userlevel |= PERM_DEFAULT;
		move(5, 0);
		prints("ÄúÒÑ¾­Íê³É×¢²á³ÌĞò¡£»¶Ó­Ê¹ÓÃ±¾Õ¾µÄ·şÎñ!");
	 }
	 else if (!HAS_PERM(cuser, PERM_LOGINOK)){
	 	move(5, 0);
		prints("Äú»¹Ã»ÓĞÌîĞ´×¢²áµ¥¡£ÇëÌîĞ´×¢²áµ¥£¬Íê³É×¢²áÊÖĞø");
		x_fillform();
		return 1;
	 }
        pressreturn();
        return 1;
    }
    clear();
    move(3, 0);
    prints("  ÑéÖ¤Ê§°Ü!");
    pressreturn();
    return 0;
}

//ÓÊ¼şÑéÖ¤
int active_mail(struct userec * cuser, session_t* session)
{
    char mbox[VALUELEN];
    char stunum[VALUELEN];
    char an[2];
    char code[CODELEN+1];
    //char incode[CODELEN+1];
    int response;
    char genbuf[STRLEN];
    struct active_data act_data;

    memset(&act_data, 0, sizeof(struct active_data));
    strcpy(act_data.userid, cuser->userid);
    strcpy(act_data.ip, cuser->lasthost);
    strcpy(act_data.operator, cuser->userid);
    act_data.status=0;
	
    clear();
    move(2, 0);
    prints("ÏÖÔÚ½«½øĞĞĞÅÏäÑéÖ¤£¬Çë×¼±¸ºÃac.cnÓòÃûµÄµç×ÓĞÅÏä ...\n");
    prints("ÏµÍ³½«ÏòÄúµÄĞÅÏä·¢ËÍÒ»·âĞÅ¼ş£¬Ã¿¸öĞÅÏä¿ÉÒÔÑéÖ¤3¸öID¡£\n\n");
    prints("ÇëÊäÈëĞÅÏäµØÖ·:");
    getdata(6, 0, ">> ", mbox, VALUELEN, DOECHO, NULL, true);
    //Í³Ò»±ä»»´ó´óĞ´±ÜÃâ´óĞ¡Ğ´µÄÒ»ÖÂĞÔÎÊÌâ
    str_to_lowercase(mbox);
    while (invalid_mail(mbox)) {
        clear();
        move(2, 0);
        prints("ÄúµÄĞÅÏäµØÖ·²»ÊôÓÚac.cnÓò»òÎª·Ç·¨µØÖ· ...\n");
        getdata(4, 0, "ÊÇ·ñÖØĞÂÊäÈë?(Y/N) [Y]  >> ", an, 2, DOECHO, NULL, true);
        if (*an != 'n' && *an != 'N') {
            move(5, 0);
            prints("ÇëÊäÈëĞÅÏäµØÖ·:");
            getdata(6, 0, ">> ", mbox, VALUELEN, DOECHO, NULL, true);
            str_to_lowercase(mbox);
        } else {
            return 0;
        }
    }
    if (!strcmp(strstr(mbox, "@")+1, "mails.gucas.ac.cn")) {
		move(7, 0);
		prints("ÄúµÄÉí·İÊÇ±¾Ğ£Ñ§Éú»òĞ£ÓÑ£¬ÇëÊäÈëÑ§ºÅÑéÖ¤:");
		getdata(8, 0, ">> ", stunum, VALUELEN, DOECHO, NULL, true);
		if (!valid_stunum(mbox, stunum)) {
			clear();
			move(3, 0);
			prints("ÄúÊäÈëµÄÑ§ºÅºÍĞÅÏä²»¶ÔÓ¦! Çë¼ì²éÊäÈë");
			pressreturn();
			return 0;
		}
		strcpy(act_data.email, mbox);
		strcpy(act_data.stdnum,stunum);
		get_official_data(&act_data);
    	}
    //µÚÒ»´ÎÑéÖ¤ĞÅÏäÊıÄ¿Ô¼Êø
    if (query_record_num(mbox, MAIL_ACTIVE)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("ÄúµÄĞÅÏäÒÑ¾­ÑéÖ¤¹ıÈı¸öid£¬ÎŞ·¨ÔÙÓÃÓÚÑéÖ¤ÁË!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    //Éú³ÉËæ»úÂë
    gencode(code);
    //Éú³ÉµÄËæ»úÂëĞ´ÈëÓÃ»§µÄhomeÄ¿Â¼
    set_active_code(cuser->userid, code, mbox, MAIL_ACTIVE);

    //·¢ËÍÑéÖ¤ĞÅº¯
    send_active_mail(mbox, code, cuser->userid, session);
    //¼ÇÂ¼ÖÁÊı¾İ¿â

    strcpy(act_data.email, mbox);
	
    int ret = write_active(&act_data);
    if (ret!=WRITE_SUCCESS && ret!=UPDATE_SUCCESS) {
        clear();
        move(3, 0);
        prints("Êı¾İ¿â±£´æ´íÎó£¬ÇëÁªÏµSYSOP!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        unlink(genbuf);
        pressreturn();
        return 0;
    }
#endif
#if 0
    move(8, 0);
    prints("%s, write ret: %d", code, ret);
#endif
#ifdef KYXK
    move(10, 0);
    prints("ÈÏÖ¤ĞÅº¯ÒÑ¾­¼Ä³ö£¬ÄÚº¬ÑéÖ¤Âë£¬Çë²éÊÕ!\n");
    pressreturn();
    continue_active(cuser);
    return 0;
}

//ÊÖ»úºÅÑéÖ¤
int active_phone(struct userec* cuser, session_t* session)
{
    char phone[VALUELEN];
    char an[2];
    char code[CODELEN+1];
    int response;
    char genbuf[STRLEN];
    struct active_data act_data;

    memset(&act_data, 0, sizeof(struct active_data));
    strcpy(act_data.userid, cuser->userid);
    strcpy(act_data.ip, cuser->lasthost);
    strcpy(act_data.operator, cuser->userid);
    act_data.status=0;
	
    clear();
    move(2, 0);
    prints("ÏÖÔÚ½«½øĞĞÊÖ»úÑéÖ¤ ...\n");
    prints("ÏµÍ³½«ÏòÄúµÄÊÖ»ú·¢ËÍÒ»Ìõ¶ÌĞÅ£¬Ã¿¸öÊÖ»úºÅ¿ÉÒÔÑéÖ¤3¸öID¡£\n\n");
    prints("ÇëÊäÈëÊÖ»úºÅÂë:");
    getdata(6, 0, ">> ", phone, VALUELEN, DOECHO, NULL, true);

    //µÚÒ»´ÎÑéÖ¤ĞÅÏäÊıÄ¿Ô¼Êø
    if (query_record_num(phone, PHONE_ACTIVE)>=MAX_USER_PER_RECORD) {
        clear();
        move(3, 0);
        prints("ÄúµÄÊÖ»úÒÑ¾­ÑéÖ¤¹ıÈı¸öid£¬ÎŞ·¨ÔÙÓÃÓÚÑéÖ¤ÁË!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    //Éú³ÉËæ»úÂë
    gencode(code);
    //Éú³ÉµÄËæ»úÂëĞ´ÈëÓÃ»§µÄhomeÄ¿Â¼
    set_active_code(cuser->userid, code, phone, PHONE_ACTIVE);

    //·¢ËÍÑéÖ¤ĞÅº¯
    send_active_msg(phone, code, cuser->userid);
    //¼ÇÂ¼ÖÁÊı¾İ¿â

    strcpy(act_data.phone, phone);
	
    int ret = write_active(&act_data);
    if (ret!=WRITE_SUCCESS && ret!=UPDATE_SUCCESS) {
        clear();
        move(3, 0);
        prints("Êı¾İ¿â±£´æ´íÎó£¬ÇëÁªÏµSYSOP!\n");
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        unlink(genbuf);
        pressreturn();
        return 0;
    }
#endif
#if 0
    move(8, 0);
    prints("%s, write ret: %d", code, ret);
#endif
#ifdef KYXK
    move(10, 0);
    prints("ÈÏÖ¤¶ÌĞÅÒÑ¾­·¢³ö£¬ÄÚº¬ÑéÖ¤Âë£¬Çë²éÊÕ!\n");
    pressreturn();
    continue_active(cuser);

    return 0;
}

//ÉÏ´«Ö¤¼şÑéÖ¤
int active_manual(struct userec* cuser, session_t* session)
{
    clear();
    move(2, 0);
    prints("´Ë¹¦ÄÜ»¹ÔÚ¿ª·¢ÖĞ!");
    pressreturn();
    return 1;
}

*/



//ÑéÖ¤ĞÅÏ¢¹ÜÀíµÄÈë¿Ú
int x_active_manager()
{
    char an[2];
//	char style[2];
    char userid[IDLEN+2];
    char value[VALUELEN];
    if (!HAS_PERM(PERM_ACCOUNTS) && !HAS_PERM(PERM_SYSOP)) {
        clear();
        move(2, 0);
        prints("ÄãÃ»ÓĞ¹ÜÀíÈ¨ÏŞ!");
        return 0;
    }
    clear();
 INPUT:   
    stand_title("ÊµÃûÈÏÖ¤¹ÜÀíÑ¡µ¥\n\n");
    clrtobot();
    move(3, 0);
    prints("[1] ²»½øĞĞÈÏÖ¤¶øÇ¿ĞĞ¼¤»îÄ³ÓÃ»§\n");
    prints("[2] ²éÑ¯ÊµÃûÈÏÖ¤¼ÇÂ¼\n");
    prints("[3] ĞŞ¸ÄÊµÃûÈÏÖ¤¼ÇÂ¼(±£³ÖÒÑÈÏÖ¤×´Ì¬)\n");
    prints("[4] É¾³ıÊµÃûÈÏÖ¤¼ÇÂ¼(ÊÍ·ÅĞÅÏä)\n");
    prints("[5] ²éÑ¯Ä³¼ÇÂ¼ÏÂ°ó¶¨µÄid\n");
    prints("[6] Àë¿ª");


    getdata(10 ,0, ">> ", an,2,DOECHO ,YEA);


    if (!strcmp(an, "1")) {
        clear();
        move(2, 0);
        prints("ÊäÈëÒª¼¤»îµÄid: ");
        usercomplete(" ", userid);
	if (*userid)    force_comfirm(userid);
       goto INPUT;
    } else if (!strcmp(an, "2")) {
        clear();
        move(1, 0);
        prints("ÊäÈëÒª²éÑ¯µÄid: ");
        usercomplete(" ", userid);
        if (*userid) query_active(userid);
        goto INPUT;
    } else if (!strcmp(an, "3")) {
        clear();
        move(1, 0);
        prints("ÊäÈëÒªĞŞ¸ÄµÄid: ");
        usercomplete(" ", userid);
        if (*userid) update_active(userid);
        goto INPUT;
    } else if (!strcmp(an, "4")) {
        clear();
        move(1, 0);
        prints("ÊäÈëÒª½â³ıÈÏÖ¤µÄid: ");
	getdata(3, 0, ">> ", userid, VALUELEN, DOECHO, YEA);
        if (*userid) delete_active(userid);
        goto INPUT;
    } else if (!strcmp(an, "5")) {
        clear();
        move(1, 0);
	 prints("ÊäÈëÒª²éÑ¯µÄ%s:\n", style_to_str(MAIL_ACTIVE));
	 getdata(3, 0, ">> ", value, VALUELEN, DOECHO, YEA);
        if (*value) query_value(value, MAIL_ACTIVE);
        goto INPUT;
    } 
    return 0;
}

//²éÑ¯Ä³idµÄÑéÖ¤ĞÅÏ¢
int query_active(char* userid)
{
    struct active_data act_data;
    char value[VALUELEN];
    unsigned int i;
    struct associated_userid *au;

    i=read_active(userid, &act_data);
    getuser(userid);

    if (i>0) {
        str_to_lowercase(act_data.email);
	 //if (act_data.status==MAIL_ACTIVE && !strcmp(strstr(act_data.email, "@")+1, "mails.gucas.ac.cn")) {
	//	char gname[VALUELEN];
	//	char gdept[VALUELEN];
	//	u2g(act_data.name, strlen(act_data.name), gname, VALUELEN);
	//	u2g(act_data.dept, strlen(act_data.dept), gdept, VALUELEN);
	//	strcpy(act_data.name, gname);
	//	strcpy(act_data.dept, gdept);
       // }
        clear();
        move(1, 0);
        prints("ÓÃ»§Ãû   :\t%s\n", act_data.userid);
        prints("ĞÕÃû     :\t%s\n", *act_data.name?act_data.name:lookupuser.realname);
        prints("%sĞÅÏä     :\t%s\n", act_data.status==1?"\033[31m":"\033[37m", act_data.email);
        prints("%sµç»°     :\t%s\n", act_data.status==2?"\033[31m":"\033[37m", act_data.phone);
        prints("%sÉí·İÖ¤ºÅ :\t%s\n", act_data.status==3?"\033[31m":"\033[37m", act_data.idnum);
        prints("Ñ§ºÅ     :\t%s\n", act_data.stdnum);
        prints("ÅàÑøµ¥Î» :\t%s\n", *act_data.dept?act_data.dept:lookupuser.realmail);
	 prints("µØÖ·     :\t%s\n", lookupuser.address);
        prints("ÈÏÖ¤Ê±¼ä :\t%s\n", act_data.status<1?"N/A":act_data.uptime);
        prints("ÈÏÖ¤ÀàĞÍ :\t%s\n", style_to_str(act_data.status));
	prints("²Ù×÷id   :\t%s\n", act_data.operator);
        if (act_data.status==IDCARD_ACTIVE) {
            //ÏÔÊ¾Í¼Æ¬µØÖ·
        }
        if (act_data.status>0 && act_data.status<4) {
            get_active_value(value, &act_data);
            prints("\n----------------------------------------------------------\n\n");
            prints("Í¬ÈÏÖ¤¼ÇÂ¼ÏÂµÄÆäËûID:\n");
            au = get_associated_userid_by_style(act_data.status, value);
            //ÁĞ³öÍ¬¼ÇÂ¼ÏÂµÄÆäËûid
            if (au != NULL) {
				for (i = 0; i < au->count; ++i) {
					prints("%-12s\t%s\n", au->id_array[i], style_to_str(au->status_array[i]));
				}
				free_associated_userid(au);
			}
        }
    } else {
        move(5, 0);
        prints("Î´ÕÒµ½ÓÃ»§ %s µÄÈÏÖ¤Óë°ó¶¨ĞÅÏ¢!", userid);
    }
    pressreturn();
    return 1;		
}

//Ç¿ÖÆ¼¤»îÄ³ÓÃ»§
int force_comfirm(char* userid)
{
    struct userec cuser;
    struct active_data act_data;
    char an[2];
    char genbuf[STRLEN];
    int response;

    response=getuser(userid);
    //memset(&act_data, 0, sizeof(struct active_data));
    read_active( userid, &act_data);

    if (lookupuser.userlevel& PERM_LOGINOK) {
        clear();
        move(5, 0);
        prints("´ËÓÃ»§ÒÑ¾­¼¤»î!\n");
        pressreturn();
        return 0;
    }
	clear();
    move(5, 0);
    prints("È·¶¨²Ù×÷?\n");
    getdata(8, 0, "Y/N [N] >> ", an, 2, DOECHO, YEA);
    if (*an == 'Y' || *an == 'y') {
        strcpy(act_data.userid, userid);
	 strcpy(act_data.operator, currentuser.userid);
	 act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser.lasthost);
	 write_active(&act_data);
	
	memcpy(&cuser, &lookupuser, sizeof (lookupuser));
	cuser.userlevel |= PERM_DEFAULT;	// by ylsdd
	substitute_record(PASSFILE, &cuser, sizeof (struct userec), response);

	// lookupuser.userlevel |= PERM_DEFAULT;
	 //°æÃæ¼ÇÂ¼Ã»ÓĞĞ´
	 sprintf(genbuf, "%sÈÃ%sÇ¿ĞĞÍ¨¹ıÈÏÖ¤.", currentuser.userid, lookupuser.userid);
	 securityreport(genbuf, genbuf);
        pressreturn();
        return 1;
    }
    return 0;
}

//É¾³ı¼¤»î¼ÇÂ¼
int delete_active(char* userid)
{
//	struct userec* cuser;
    char an[2];
    char genbuf[STRLEN];
    struct active_data act_data;

    getuser(userid);
    read_active(userid, &act_data);
   // s = mysql_init(s);
   /*
    if (!(cuser->flags & ACTIVATED_FLAG)) {
        clrtobot();
        move(5, 0);
        prints("´ËÓÃ»§²¢Î´¼¤»î!\n");
        //anyway£¬ÈÔÈ»È¥µô¼¤»î¼ÇÂ¼
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, getCurrentUser()->userid);
        write_active(&act_data);
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    */
    clear();
    move(5, 0);
    prints("È·¶¨È¡ÏûÈÏÖ¤¼ÇÂ¼?\n");
    getdata(6, 0, "Y/N [N] >> ", an, 2, DOECHO, YEA);
    if (*an == 'Y' || *an == 'y') {
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, currentuser.userid);
        write_active(&act_data);
        //cuser->flags &= ~ACTIVATED_FLAG;
        //setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //É¾³ıÓÃ»§Ä¿Â¼ÏÂµÄÑéÖ¤ÂëÎÄ¼ş
        //unlink(genbuf);
	 //°æÃæ¼ÇÂ¼Ã»ÓĞĞ´
	 sprintf(genbuf, "%sÉ¾³ı%sµÄĞÅÏä°ó¶¨.", currentuser.userid, lookupuser.userid);
	 securityreport(genbuf, genbuf);
        pressreturn();
        return 1;
    }
    return 0;
}

static void
getfield(line, info, desc, buf, len)
int line, len;
char *info, *desc, *buf;
{
	char prompt[STRLEN];
	char genbuf[STRLEN];

	sprintf(genbuf, "  Ô­ÏÈÉè¶¨: %-46.46s [1;32m(%s)[m",
		(buf[0] == '\0') ? "(Î´Éè¶¨)" : buf, info);
	move(line, 0);
	prints("%s", genbuf);
	sprintf(prompt, "  %s: ", desc);
	getdata(line + 1, 0, prompt, genbuf, len, DOECHO, YEA);
	if (genbuf[0] != '\0') {
		strncpy(buf, genbuf, len);
	}
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

//¸üĞÂ¼¤»îĞÅÏ¢¼ÇÂ¼
//Ò»°ãÇé¿öÏÂÓ¦¸ÃÉ¾³ı¼¤»î¼ÇÂ¼ÔÙÒªÇóÓÃ»§ÖØĞÂÊÖ¹¤¼¤»î
//¶ø²»ÊÇÊ¹ÓÃ±¾¹¦ÄÜ
int update_active(char* userid)
{
//    struct userec* cuser;
    struct active_data act_data;
	char genbuf[STRLEN];
    char an[2];
	int response;

    getuser(userid);
    response=read_active(userid, &act_data);

    move(4,0);
    clear();
    getfield(5, "", "ÕæÊµĞÕÃû", act_data.name, STRLEN);
    getfield(6, "", "¹¤×÷µ¥Î»", act_data.dept, STRLEN);
    getfield(7, "", "Email", act_data.email, STRLEN);
    getfield(8, "", "ÊÖ»úºÅ", act_data.phone, STRLEN);
    getfield(9, "", "Éí·İÖ¤¼şºÅÂë", act_data.idnum, STRLEN);
    getfield(10, "", "Ñ§ºÅ", act_data.stdnum, STRLEN);
    str_to_lowercase(act_data.email);
    prints("È·¶¨²Ù×÷?\n");
    getdata(12, 0, "Y/N [N] >> ", an, STRLEN, DOECHO, YEA);
    if (*an == 'Y' || *an == 'y') {
        //¼ÇÂ¼²Ù×÷Õ¾ÎñµÄid
        act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser.lasthost);
	 strcpy(act_data.operator, currentuser.userid);
	 //°æÃæ¼ÇÂ¼Ã»ÓĞĞ´
	 sprintf(genbuf, "%sĞŞ¸Ä%sµÄÊµÃûÈÏÖ¤¼ÇÂ¼.", currentuser.userid, lookupuser.userid);
	 securityreport(genbuf, genbuf);
	 write_active(&act_data);
        pressreturn();
        return 1;
    }
    return 0;
}


//²éÑ¯Ä³¼ÇÂ¼ÏÂ°ó¶¨µÄid
int query_value(char* value, int style)
{
    size_t i;
    struct associated_userid *au;

    clear();
    move(5, 0);
    str_to_lowercase(value);
    prints("Í¬ÈÏÖ¤¼ÇÂ¼ÏÂµÄID:\n");
    au = get_associated_userid_by_style(style, value);
    //ÁĞ³öÍ¬¼ÇÂ¼ÏÂµÄÆäËûid
    if (au != NULL) {
		for (i = 0; i < au->count; ++i) {
			prints("%-12s\t%s\n", au->id_array[i], style_to_str(au->status_array[i]));
		}
		free_associated_userid(au);
	}
    pressreturn();
    return 1;
}
    

#endif


