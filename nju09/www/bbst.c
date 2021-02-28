#include "bbslib.h"
#include "tshirt.h"

int
bbst_main()
{
	struct tshirt t;
	FILE *fp;
	int num = 0, i, j, k, l, m;
	short tstat[3][3][5][2][7];
	html_header(1);
	changemode(SELECT);
	printf("<center>%s --您的 T Shirt 订购清单 <hr>\n", BBSNAME);
	printf("<table border=1><tr><td> 用户ID <td> 样式 <td> 件数 <td> 联系方式 <td>&nbsp;</tr>");
	fp = fopen("tshirt", "r");
	if (fp == 0) {
		http_fatal("错误的参数");
	}
	flock(fileno(fp), LOCK_SH);
	bzero(tstat, sizeof (tstat));
	while (fread(&t, 1, sizeof (struct tshirt), fp) == sizeof (struct tshirt)) {
		t.id[sizeof(t.id) - 1] = 0;
		if ((!strcmp(t.id, currentuser.userid))
				|| (currentuser.userlevel & PERM_SYSOP)
				|| (!strcmp(currentuser.userid, "duckling"))
				|| (!strcmp(currentuser.userid, "donger")))
			printf("<tr><td>%s<td>%s %s %s %s %s<td> %d <td> %s <td> <a href=bbslt?index=%d>修改(退订把件数设置为0即可)</a></tr>",
					t.id, stylestr[t.style], fmstr[t.fm],
					sizestr[t.size], pricestr[t.price],
					colorstr[t.color], t.num, nohtml(t.address),
					num + 1);
		num++;
		tstat[t.style][t.fm][t.size][t.price][t.color] += t.num;
	}
	fclose(fp);
	printf("</td></table>");
	printf("以下为全站总计：<br><hr>");
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 5; k++) {
				for (l = 0; l < 2; l++) {
					for (m = 0; m < 7; m++) {
						if (tstat[i][j][k][l][m])
							printf("%s %s %s %s %s %d <br>",
									stylestr[i],
									fmstr[j],
									sizestr[k],
									pricestr[l],
									colorstr[m],
									tstat[i][j][k][l]
									[m]);
					}
				}
			}
		}
	}
	printf("[<a href=bbslt>订购</a>]");
	printf("[<a href=bbst>刷新</a>]");
	printf("[<a href='javascript:window.close()'>关闭</a>]");
	http_quit();
	return 0;
}
