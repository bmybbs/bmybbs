#include "bbslib.h"

int
bbsbadlogins_main()
{
	char file[STRLEN];
	html_header(1);
	printf("<body>");
	if (!loginok || isguest) {
		printf("Ŷ���㲢û�е�½�����ü�������¼��</body></html>");
		return 0;
	}

	sethomefile_s(file, sizeof(file), currentuser.userid, BADLOGINFILE);
	if (!file_exist(file)) {
		printf("û���κ�������������¼</body></html>");
		return 0;
	}
	if (*getparm("del") == '1') {
		unlink(file);
		printf("������������¼�ѱ�ɾ��<br>");
		printf("<a href='#' onClick='javascript:window.close()'>�رմ���</a>");
	} else {
		printf("��������������������¼<br><pre>");
		showfile(file);
		printf("</pre>");
		printf("<a href=bbsbadlogins?del=1>ɾ��������������¼</a>");
	}
	printf("</body></html>");
	return 0;
}
