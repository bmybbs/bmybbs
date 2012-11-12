#include "bbslib.h"
#include "tshirt.h"

char *stylestr[3] = { "飞机图案", "字图案", "糊涂的主" };
char *fmstr[3] = { "男款", "女款一字领", "女款圆领" };
char *sizestr[5] = { "S号", "M号", "L号", "XL号", "XXL号" };
char *pricestr[2] = { "20元档(低档)", "40元档(高档)" };
char *colorstr[7] = { "白", "深蓝", "大红", "灰绿", "水蓝", "桔红", "黑" };

void
print_radio(char *cname, char *name, char *str[], int len, int select)
{
	int i;
	puts("<td valign=top>\n");
	printf("<p><p>%s：<br>", cname);
	for (i = 0; i < len; i++) {
		printf("<input type=radio name=%s value=%d %s>", name, i + 1,
		       (i == select) ? "checked" : "");
		printf("%s<br>\n", str[i]);
	}
	puts("</td>\n");
}

int
bbslt_main()
{
	int index;
	int i;
	struct tshirt t;
	html_header(1);
	if (!loginok || isguest) {
		printf("<script src=/function.js></script>\n");
		printf("匆匆过客不能订购T Shirt,请先登录!<br><br>");
		printf("<script>openlog();</script>");
		http_quit();
	}
	changemode(SELECT);
	if(1 /*localtime(&now_t)->tm_hour>=14*/)
	{
		printf("已经截至了!");
		printf("<br>[<a href='bbst'>查看订购情形</a>]");
		return 0;
	}
	index = atoi(getparm("index")) - 1;
	bzero(&t, sizeof (t));
	if (index >= 0) {
		FILE *fp;
		int ret;
		fp = fopen("tshirt", "r");
		if (fp == 0)
			http_fatal("错误的参数2");
		flock(fileno(fp), LOCK_SH);
		ret = fseek(fp, sizeof (struct tshirt) * index, SEEK_SET);
		if (ret) {
			fclose(fp);
			http_fatal("内部错误1");
		}
		fread(&t, sizeof (struct tshirt), 1, fp);
		fclose(fp);
		if (strcmp(t.id, currentuser.userid))
			bzero(&t, sizeof (t));
	}

	if (currentuser.firstlogin >= 1055509320) {
		printf("您看上去太年轻了点...");
		http_quit();
	}
	printf("<center>%s -- 订购 T Shirt [使用者: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	printf("<table width=70%%><tr>\n");
	printf
	    ("<td colspan=3><form name=form1 method=post action=bbsdt><p>请先选择类型：</p></td><tr><tr><td><table width=75%% border=0 cellspacing=0 cellpadding=0 align=top><tr><td><input type=hidden name=index value=%d></td></tr><tr>\n",
	     index + 1);
	print_radio("图案选择", "style", stylestr, 3, t.style);
	print_radio("底衫档次", "price", pricestr, 2, t.price);
	print_radio("款式选择", "fm", fmstr, 3, t.fm);
	print_radio("大小选择", "size", sizestr, 5, t.size);
	print_radio("颜色选择", "color", colorstr, 7, t.color);
	printf
	    ("</tr></table><p>打算买<input type=text name=num size=10 maxlength=3 value=%d>件这种型号的。<br>您的联系方式（地址、电话）：<input type=text name=address size=40 maxlength=250 value='%s'><br><input type=submit name=Submit value=提交> <input type=reset name=Submit2 value=重新填></p></form>",
	     t.num, t.address);

	printf("</td></tr></table>");
	printf
	    ("<table><tr><td><font class=c32>关于图案的说明"
	    "参见<a href=con?B=Painter&F=M.1055771238.A target=_blank>这篇文章</a><br>\n"
	     "关于颜色款式大小的说明 参见<a href=/contact/index.htm target=_blank>这里<a><br>");
	printf("效果请点击下面链接<br></font>");
	for (i = 0; i < 5; i++) {
		printf("<a href=/tshirt/%d.jpg target=_blank>效果 %d</a><br>\n",
		       i + 1, i + 1);
	}
	printf("需要注意的是 不是所有的组合都可以的,有下列限制<br>\n"
	       "20元档只有白色 没有女款<br>40元档 男款有 白 深蓝 大红 灰绿 黑色,女款一字领有水蓝 大红 黑 女款圆领有 灰绿 桔红 白 黑<br>大小的组合请参考前面的说明<br></td></tr>");
	printf
	    ("<tr><td><font color=red>您所填写的联系方式将不会公开,只用于和您联系T Shirt购买事宜,所以请选择最及时有效的,非BBS的联系方式.</font></td></tr><tr><td><font color=red>我们将以最快速度通知你何时在何地领取Tshirt.自发放之日起7天内未来领取(有特殊情况请通知站务组)的,将视为放弃自己的报名.</font></td></tr></table>");
	printf("<br>[<a href='bbst'>查看订购情形</a>]");
	http_quit();
	return 0;
}
