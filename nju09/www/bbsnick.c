#include "bbslib.h"

int
bbsnick_main()
{	//modify by mintbaggio 20040829 for new www
	int i;
	char nick[80];
	unsigned c;
	html_header(1);
	check_msg();
	//printf("<body>");
	printf("<body leftmargin=0 topmargin=0>\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr> <td height=30 colspan=2></td></tr>\n");
	if (!loginok || isguest)
		http_fatal("匆匆过客无法改变昵称");
	changemode(GMENU);
	strsncpy(nick, getparm("nick"), 30);
	if (nick[0] == 0) {
		printf("<tr><td height=70 colspan=2> \n"
			"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n"
			"<tr> <td width=40>&nbsp; </td>\n");
		printf("<td height=35>%s -- <span id=topmenu_b>临时改变昵称(环顾四方有效)</span> [使用者: <span class=themetext>%s</span>]</td>", BBSNAME, currentuser.userid);
		printf("</tr>\n");
		printf("<tr>\n<td width=40 class=\"level1\">&nbsp;</td>\n");
		printf("<td class=\"level1\"><br>\n"
			"<form action=bbsnick>新昵称<input name=nick size=24 maxlength=24 type=text value='%s'> \n",
			     u_info->username);
		printf("<input type=submit class=sumbitshort value=确定>");
		printf("</form></td></tr></table></td></tr></table></body></html>");
		http_quit();
	}
	for (i = 0, c = (unsigned char) nick[i]; nick[i]; i++)
		if (c < 32 || c == 255)
			nick[i] = ' ';
	strsncpy(u_info->username, nick, 32);
	printf("临时变更昵称成功");
	printf("</table></body></html>");
	return 0;
}
