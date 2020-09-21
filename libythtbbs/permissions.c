#include "ythtbbs/ythtbbs.h"
const char * const permstrings[] = {
	"基本权力",		/* PERM_BASIC */
	"进入聊天室",		/* PERM_CHAT */
	"呼叫他人聊天",		/* PERM_PAGE */
	"发表文章",		/* PERM_POST */
	"使用者资料正确",	/* PERM_LOGINOK */
	"禁止使用签名档",	/* PERM_DENYSIG */
	"隐身术",		/* PERM_CLOAK */
	"看穿隐身术",		/* PERM_SEECLOAK */
	"帐号永久保留",		/* PERM_XEMPT */
	"编辑进站画面",		/* PERM_WELCOME */
	"板主",			/* PERM_BOARDS */
	"帐号管理员",		/* PERM_ACCOUNTS */
	"本站仲裁",		/* PERM_ARBITRATE */
	"投票管理员",		/* PERM_OVOTE */
	"系统维护管理员",	/* PERM_SYSOP */
	"Read/Post 限制",	/* PERM_POSTMASK */
	"精华区总管",		/* PERM_ANNOUNCE */
	"讨论区总管",		/* PERM_OBOARDS */
	"活动看版总管",		/* PERM_ACBOARD */
	"不能 ZAP(讨论区专用)",	/* PERM_NOZAP */
	"强制呼叫",		/* PERM_FORCEPAGE */
	"延长发呆时间",		/* PERM_EXT_IDLE */
	"大信箱",		/* PERM_SPECIAL1 */
	"特殊权限 2",		/* PERM_SPECIAL2 */
	"特殊权限 3",		/* PERM_SPECIAL3 */
	"区长",			/* PERM_SPECIAL4 */
	"本站监察组",		/* PERM_SPECIAL5 */
	"本站立法会",		/* PERM_SPECIAL6 */
	"特殊权限 7",		/* PERM_SPECIAL7 */
	"个人文集",		/* PERM_SPECIAL8 */
	"禁止发信权",		/* PERM_DENYMAIL */
};

const char *const user_definestr[NUMDEFINES] = {
	"呼叫器关闭时可让好友呼叫",	/* DEF_FRIENDCALL */
	"接受所有人的讯息",	/* DEF_ALLMSG */
	"接受好友的讯息",	/* DEF_FRIENDMSG */
	"收到讯息发出声音",	/* DEF_SOUNDMSG */
	"使用彩色",		/* DEF_COLOR */
	"显示活动看版",		/* DEF_ACBOARD */
	"显示选单的讯息栏",	/* DEF_ENDLINE */
	"编辑时显示状态栏",	/* DEF_EDITMSG */
	"讯息栏采用一般/精简模式",	/* DEF_NOTMSGFRIEND */
	"选单采用一般/精简模式",	/* DEF_NORMALSCR */
	"分类讨论区以 New 显示",	/* DEF_NEWPOST */
	"阅读文章是否使用绕卷选择",	/* DEF_CIRCLE */
	"阅读文章游标停于第一篇未读",	/* DEF_FIRSTNEW */
	"进站时显示好友名单",	/* DEF_LOGFRIEND */
	"进站时显示备忘录",	/* DEF_INNOTE */
	"离站时显示备忘录",	/* DEF_OUTNOTE */
	"离站时询问寄回所有讯息",	/* DEF_MAILMSG */
	"使用自己的离站画面",	/* DEF_LOGOUT */
	"我是这个组织的成员",	/* DEF_SEEWELC1 */
	"好友上站通知",		/* DEF_LOGINFROM */
	"观看留言版",		/* DEF_NOTEPAD */
	"不要送出上站通知给好友",	/* DEF_NOLOGINSEND */
	"主题式看版",		/* DEF_THESIS */
	"收到讯息等候回应或清除",	/* DEF_MSGGETKEY */
	"汉字整字处理",		/* DEF_DELDBLCHAR */
	"使用GB码阅读",		/* DEF_USEGB KCN 99.09.03 */
	"使用动态底线",		/* DEF_ANIENDLINE */
	"初次访问版面提示进入精华区",	/* DEF_INTOANN */
	"发表文章时暂时屏蔽MSG",	/* DEF_POSTNOMSG */
	"进站时观看统计信息",	/* DEF_SEESTATINLOG */
	"过滤可能令人反感信息",	/* DEF_FILTERXXX */
//	"收取站外信件",		/* DEF_INTERNETMAIL */
	"进站时观看全国十大排行榜"	/* DEF_NEWSTOP10 */
}; 
