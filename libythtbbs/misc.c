#include <string.h>
#include "ytht/random.h"
#include "ythtbbs/ythtbbs.h"

// add by IronBlood@bmy 20120107
static char *get_login_pic_link (char *picname, char *linkback) {
	FILE *fp;
	char link[256];
	memset(link, '\0',sizeof(link));
	char linkfile[256];
	sprintf(linkfile, MY_BBS_HOME "/loglinks/%s", picname);
	if (!(fp = fopen ( linkfile,"r")))
		return "BMY/home?B=XJTUnews";
	if (!fgets (link,sizeof (link),fp))
		return "BMY/home?B=XJTUnews";
	fclose (fp);
	if (link[strlen(link) - 1] == '\n')
		link[strlen(link) - 1] = '\0';
	return strcpy(linkback, link);
}

// added by IronBlood@11.09.05
void get_no_more_than_four_login_pics(char *buf, size_t len) {
	FILE *fp;
	if(!(fp = fopen(MY_BBS_HOME "/logpics","r")))
		ytht_strsncpy(buf, "cai.jpg", len);

	char pics[256];
	const char *pics_dir ="bmyMainPic/using/";
	char pics_list[4096];
	char file[16][256];
	int file_line=0;
	unsigned int randnum;
	char link[256];
	memset(pics_list, '\0', sizeof(pics_list));

	// 读取文件
	while(fgets(pics,sizeof(pics),fp)!=NULL)
	{
		char *tmp=file[file_line];
		if (pics[strlen(pics) - 1] == '\n')
			pics[strlen(pics) - 1] = 0;
		strcpy(tmp,pics);
		++file_line;
	}
	// 释放句柄
	fclose(fp);

	if (file_line < 2) {
		// logpics 格式：第一行是计数，因此如果存在进站画面，应该至少2行
		ytht_strsncpy(buf, "cai.jpg", len);
	}

	int i=0;

	while( (i != file_line - 1) && i !=4) // 不超过总图片个数、不超过最大上限
	{
		ytht_get_random_int(&randnum);
		randnum = 1 + randnum % file_line;
		char *tmp = file[randnum];

		if( strstr(pics_list,tmp)==NULL ) //不包含图片字符串，才执行下面的操作
		{
			get_login_pic_link(tmp,link);
			if(i>0)
				strcat(pics_list, ";;");
			strcat(pics_list, pics_dir);
			strcat(pics_list, tmp);
			strcat(pics_list, ";");
			strcat(pics_list, link);
			++i;
		}
	}

	ytht_strsncpy(buf, pics_list, len);
}

