#include "bbslib.h"

struct mystat {
	int key;
	int num;
};

int
search_stat(void *ptr, int size, int key)
{
	int total, low, high, mid;
	struct mystat *totest;
	total = size / sizeof (struct mystat);
	low = 0;
	high = total;
	while (low <= high) {
		mid = (low + high) / 2;
		totest =
		    (struct mystat *) (ptr + mid * sizeof (struct mystat));
		if (key == totest->key)
			return totest->num;
		else if (key > totest->key)
			high = mid - 1;
		else
			low = mid + 1;
	}
	totest = (struct mystat *) (ptr + low * sizeof(struct mystat));
	return totest->num;

}

int
bbsstat_main()
{	//modify by mintbaggio 20040829 for new www
	int logins = 0, posts = 0, stays = 0, lifes = 0, total = 0;
	struct mmapfile mf = { ptr:NULL };
	struct mystat *ms;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("匆匆过客不加入排名");
	changemode(GMENU);

	printf("<body><center><div class=rhead>%s -- 个人排名统计 [使用者: <span class=h11>%s</span>]</div><hr>\n",
	       BBSNAME, currentuser.userid);
	printf
	    ("<table width=320><tr><td>项目<td>数值<td>全站排名<td>相对比例\n");
	mmapfile("wwwtmp/stat_age", &mf);
	lifes =
	    search_stat(mf.ptr, mf.size,
			(now_t - currentuser.firstlogin) / 86400);
	ms = (struct mystat *) (mf.ptr + (mf.size - sizeof (struct mystat)));
	total = ms->num;
	mmapfile(NULL, &mf);
	mf.ptr = NULL;
	printf("<tr><td>本站网龄<td>%ld天<td>%d<td>TOP %5.2f%%",
	       (now_t - currentuser.firstlogin) / 86400, lifes,
	       (lifes * 100.) / total);
	mmapfile("wwwtmp/stat_login", &mf);
	logins = search_stat(mf.ptr, mf.size, (currentuser.numlogins));
	mmapfile(NULL, &mf);
	mf.ptr = NULL;
	printf("<tr><td>上站次数<td>%d次<td>%d<td>TOP %5.2f%%",
	       currentuser.numlogins, logins, logins * 100. / total);
	mmapfile("wwwtmp/stat_post", &mf);
	posts = search_stat(mf.ptr, mf.size, (currentuser.numposts));
	mmapfile(NULL, &mf);
	mf.ptr = NULL;
	printf("<tr><td>发表文章<td>%d次<td>%d<td>TOP %5.2f%%",
	       currentuser.numposts, posts, posts * 100. / total);
	mmapfile("wwwtmp/stat_stay", &mf);
	stays = search_stat(mf.ptr, mf.size, (currentuser.stay/60/60));
	mmapfile(NULL, &mf);
	mf.ptr = NULL;
	printf("<tr><td>在线时间<td>%ld分<td>%d<td>TOP %5.2f%%",
	       currentuser.stay / 60, stays, stays * 100. / total);
	printf("</table><br>总用户数: %d</center></body>", total);
	return 0;
}
