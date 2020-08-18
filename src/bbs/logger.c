// logger.c
// 作者: interma@BMY
// 创建时间：2006-04-06
// 功能：程序中的log记录函数

//修改: clearboy@BMY
//因为可能存在一些内存的问题，做了部分修改

#include "bbs.h"
#define MAIL_ID "program" //这个是邮件的收信人id，将这个id的信箱映射到版面

int alter = 0;
extern int mail_buf(char *buf, char *userid, char *title); //这个函数在mail.c中，给userid发信

static const char *header[] = {
	"其他",
	"邮箱帮定",
	"随机掉线"
};

// 将buf中的内容进行log
void logbuf(char *buf, char *title, int headerindex)
{
	// headerindex注意不要越界
	// buf不要太大，因为innerbuf只有2048
	// title不要太大，因为innertitle只有64
	char innerbuf[2048];
	char innertitle[64];
	
	if (headerindex < 0 || headerindex > sizeof(header)/sizeof(char *) - 1 )
		return;

	if (alter)
	{
		sprintf(innertitle, "{%s} %s", header[headerindex], title);
	}
	else
	{
		sprintf(innertitle, "<%s> %s", header[headerindex], title);
	}
	
	alter = abs(alter -1); // 为了间隔阅读方便

	strncpy(innerbuf, "<此封信件是由logger程序产生>\n\n", 34);
	strncat(innerbuf, buf, 990);
	mail_buf(innerbuf, MAIL_ID, innertitle);
}
