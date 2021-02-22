#include "bbs.h"
#include "ythtbbs/identify.h"
#include "smth_screen.h"
#include "io.h"
#include "maintain.h"
#include "namecomplete.h"
#include "stuff.h"
#include "bcache.h"
#include "bbs_global_vars.h"

#ifdef POP_CHECK

int x_active_manager();
static int query_active(char* userid);
static int force_comfirm(char* userid);
static int delete_active(char* userid);
static int update_active(char* userid);
static int query_value(char* value, int style);

//验证信息管理的入口
int x_active_manager(const char *s) {
	(void) s;
    char an[2];
//	char style[2];
    char userid[IDLEN+2];
    char value[VALUELEN];
    if (!HAS_PERM(PERM_ACCOUNTS, currentuser) && !HAS_PERM(PERM_SYSOP, currentuser)) {
        clear();
        move(2, 0);
        prints("你没有管理权限!");
        return 0;
    }
    clear();
 INPUT:
    stand_title("实名认证管理选单\n\n");
    clrtobot();
    move(3, 0);
    prints("[1] 不进行认证而强行激活某用户\n");
    prints("[2] 查询实名认证记录\n");
    prints("[3] 修改实名认证记录(保持已认证状态)\n");
    prints("[4] 删除实名认证记录(释放信箱)\n");
    prints("[5] 查询某记录下绑定的id\n");
    prints("[6] 离开");


    getdata(10 ,0, ">> ", an,2,DOECHO ,YEA);


    if (!strcmp(an, "1")) {
        clear();
        move(2, 0);
        prints("输入要激活的id: ");
        usercomplete(" ", userid);
	if (*userid)    force_comfirm(userid);
       goto INPUT;
    } else if (!strcmp(an, "2")) {
        clear();
        move(1, 0);
        prints("输入要查询的id: ");
        usercomplete(" ", userid);
        if (*userid) query_active(userid);
        goto INPUT;
    } else if (!strcmp(an, "3")) {
        clear();
        move(1, 0);
        prints("输入要修改的id: ");
        usercomplete(" ", userid);
        if (*userid) update_active(userid);
        goto INPUT;
    } else if (!strcmp(an, "4")) {
        clear();
        move(1, 0);
        prints("输入要解除认证的id: ");
	getdata(3, 0, ">> ", userid, VALUELEN, DOECHO, YEA);
        if (*userid) delete_active(userid);
        goto INPUT;
    } else if (!strcmp(an, "5")) {
        clear();
        move(1, 0);
	 prints("输入要查询的%s:\n", style_to_str(MAIL_ACTIVE));
	 getdata(3, 0, ">> ", value, VALUELEN, DOECHO, YEA);
        if (*value) query_value(value, MAIL_ACTIVE);
        goto INPUT;
    }
    return 0;
}

//查询某id的验证信息
static int query_active(char* userid)
{
    struct active_data act_data;
    char value[VALUELEN];
    unsigned int i;
    struct associated_userid *au;

    i=read_active(userid, &act_data);
    getuser(userid);

    if (i>0) {
		ytht_str_to_lowercase(act_data.email);
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
        prints("用户名   :\t%s\n", act_data.userid);
        prints("姓名     :\t%s\n", *act_data.name?act_data.name:lookupuser.realname);
        prints("%s信箱     :\t%s\n", act_data.status==1?"\033[31m":"\033[37m", act_data.email);
        prints("%s电话     :\t%s\n", act_data.status==2?"\033[31m":"\033[37m", act_data.phone);
        prints("%s身份证号 :\t%s\n", act_data.status==3?"\033[31m":"\033[37m", act_data.idnum);
        prints("学号     :\t%s\n", act_data.stdnum);
        prints("培养单位 :\t%s\n", *act_data.dept?act_data.dept:lookupuser.realmail);
	 prints("地址     :\t%s\n", lookupuser.address);
        prints("认证时间 :\t%s\n", act_data.status<1?"N/A":act_data.uptime);
        prints("认证类型 :\t%s\n", style_to_str(act_data.status));
	prints("操作id   :\t%s\n", act_data.operator);
        if (act_data.status==IDCARD_ACTIVE) {
            //显示图片地址
        }
        if (act_data.status>0 && act_data.status<4) {
            get_active_value(value, &act_data);
            prints("\n----------------------------------------------------------\n\n");
            prints("同认证记录下的其他ID:\n");
            au = get_associated_userid_by_style(act_data.status, value);
            //列出同记录下的其他id
            if (au != NULL) {
				for (i = 0; i < au->count; ++i) {
					prints("%-12s\t%s\n", au->id_array[i], style_to_str(au->status_array[i]));
				}
				free_associated_userid(au);
			}
        }
    } else {
        move(5, 0);
        prints("未找到用户 %s 的认证与绑定信息!", userid);
    }
    pressreturn();
    return 1;
}

//强制激活某用户
static int force_comfirm(char* userid)
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
        prints("此用户已经激活!\n");
        pressreturn();
        return 0;
    }
	clear();
    move(5, 0);
    prints("确定操作?\n");
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
	 //版面记录没有写
	 sprintf(genbuf, "%s让%s强行通过认证.", currentuser.userid, lookupuser.userid);
	 securityreport(genbuf, genbuf);
        pressreturn();
        return 1;
    }
    return 0;
}

//删除激活记录
static int delete_active(char* userid)
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
        prints("此用户并未激活!\n");
        //anyway，仍然去掉激活记录
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, getCurrentUser()->userid);
        write_active(&act_data);
        setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
        unlink(genbuf);
        pressreturn();
        return 0;
    }
    */
    clear();
    move(5, 0);
    prints("确定取消认证记录?\n");
    getdata(6, 0, "Y/N [N] >> ", an, 2, DOECHO, YEA);
    if (*an == 'Y' || *an == 'y') {
        act_data.status=NO_ACTIVE;
        strcpy(act_data.operator, currentuser.userid);
        write_active(&act_data);
        //cuser->flags &= ~ACTIVATED_FLAG;
        //setactivefile(genbuf, cuser->userid, ACTIVE_FILE);
        //删除用户目录下的验证码文件
        //unlink(genbuf);
	 //版面记录没有写
	 sprintf(genbuf, "%s删除%s的信箱绑定.", currentuser.userid, lookupuser.userid);
	 securityreport(genbuf, genbuf);
        pressreturn();
        return 1;
    }
    return 0;
}

//更新激活信息记录
//一般情况下应该删除激活记录再要求用户重新手工激活
//而不是使用本功能
static int update_active(char* userid)
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
    getfield(5, "", "真实姓名", act_data.name, STRLEN);
    getfield(6, "", "工作单位", act_data.dept, STRLEN);
    getfield(7, "", "Email", act_data.email, STRLEN);
    getfield(8, "", "手机号", act_data.phone, STRLEN);
    getfield(9, "", "身份证件号码", act_data.idnum, STRLEN);
    getfield(10, "", "学号", act_data.stdnum, STRLEN);
	ytht_str_to_lowercase(act_data.email);
    prints("确定操作?\n");
    getdata(12, 0, "Y/N [N] >> ", an, STRLEN, DOECHO, YEA);
    if (*an == 'Y' || *an == 'y') {
        //记录操作站务的id
        act_data.status=FORCE_ACTIVE;
	 strcpy(act_data.ip, currentuser.lasthost);
	 strcpy(act_data.operator, currentuser.userid);
	 //版面记录没有写
	 sprintf(genbuf, "%s修改%s的实名认证记录.", currentuser.userid, lookupuser.userid);
	 securityreport(genbuf, genbuf);
	 write_active(&act_data);
        pressreturn();
        return 1;
    }
    return 0;
}


//查询某记录下绑定的id
static int query_value(char* value, int style)
{
    size_t i;
    struct associated_userid *au;

    clear();
    move(5, 0);
	ytht_str_to_lowercase(value);
    prints("同认证记录下的ID:\n");
    au = get_associated_userid_by_style(style, value);
    //列出同记录下的其他id
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
