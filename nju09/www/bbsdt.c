#include "bbslib.h"
#include "tshirt.h"
static int do_ts(struct tshirt *t, int index);

int
bbsdt_main()
{
	struct tshirt t;
	int index;
	html_header(1);
	changemode(SELECT);
	/*if(localtime(&now_t)->tm_hour>=14)*/http_fatal("已经截至了!");
	index = atoi(getparm("index")) - 1;
	t.style = atoi(getparm("style")) - 1;
	t.fm = atoi(getparm("fm")) - 1;
	t.size = atoi(getparm("size")) - 1;
	t.price = atoi(getparm("price")) - 1;
	t.color = atoi(getparm("color")) - 1;
	t.num = atoi(getparm("num"));
	t.dtime = now_t;
	ytht_strsncpy(t.address, getparm("address"), 200);
	if (!loginok || isguest)
		http_fatal("匆匆过客不能进行本项操作");
	if (0 == t.num)
		bzero(&t, sizeof (struct tshirt));
	if (t.style < 0 || t.fm < 0 || t.size < 0 || t.price < 0
			|| t.color < 0 || t.num < 0 || t.style > 2 || t.fm > 2
			|| t.size > 4 || t.price > 1 || t.color > 6 || (0 == t.num && index < 0))
		http_fatal("错误的参数1");

	if (0 == t.price && 0 != t.num && (0 != t.color || t.fm != 0 || t.size == 0 || t.size == 4))
		http_fatal("低档衫只有白色的,男款的,而且没有S XXL号");
	else if (1 == t.price && 0 == t.fm && ((t.color<6 &&t.color > 3) || t.size == 0|| t.size== 4))
		http_fatal("高档男款没有水蓝和桔红,而且没有S XXL号");
	else if (1 == t.price && 1 == t.fm && ((t.color != 2 && t.color != 4 && t.color != 6 ) || t.size != 1))
		http_fatal("女一字领只有水蓝和大红色和黑色,只有M一个号");
	else if (1 == t.price && 2 == t.fm
			&& ((t.color != 3 && t.color != 5 && t.color != 0 && t.color !=6 )
				|| (t.size != 1 && t.size != 2)))
		http_fatal("女圆领就M和L两个号,而且只有白 灰绿 桔红 黑 四种颜色");
	if (t.num >= 100)
		http_fatal("要做服装生意?那个,另外找站务单独商量好吗?");
	printf("<center>%s -- 订购 T Shirt [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	printf("<table><td>");
	strncpy(t.id, currentuser.userid, IDLEN + 1);
	do_ts(&t, index);
	printf("您要了 %s %s %s %s %s 的站衫 %d 件，您留的联系方式是:%s",
			stylestr[t.style], fmstr[t.fm], colorstr[t.color],
			sizestr[t.size], pricestr[t.price], t.num, t.address);
	printf("</td></table>");
	printf("<br>[<a href='bbst'>查看订购情形</a>]");
	http_quit();
	return 0;
}

static int
do_ts(struct tshirt *t, int index)
{
	FILE *fp;
	struct tshirt tmp;
	int ret;
	fp = fopen("tshirt", "r+");
	if (fp == 0)
		http_fatal("错误的参数2");
	flock(fileno(fp), LOCK_EX);
	if (index < 0)
		ret = fseek(fp, 0, SEEK_END);
	else {
		ret = fseek(fp, sizeof (struct tshirt) * index, SEEK_SET);
		if (ret) {
			fclose(fp);
			http_fatal("内部错误1");
		}
		fread(&tmp, sizeof (struct tshirt), 1, fp);
		if (strcmp(tmp.id, currentuser.userid)) {
			fclose(fp);
			http_fatal("内部错误2");
		}
		ret = fseek(fp, sizeof (struct tshirt) * index, SEEK_SET);
	}
	if (ret) {
		fclose(fp);
		http_fatal("内部错误!");
	}
	fwrite(t, sizeof (struct tshirt), 1, fp);
	fclose(fp);
	return 0;
}
