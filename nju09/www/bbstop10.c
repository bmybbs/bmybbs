#include "ythtbbs/commend.h"
#include "bbslib.h"

/*20121014 New top10 page  Edited by liuche*/

void showTop10Table();

/*
|20121014 以表格形式显示美文和通知公告
|kind==1 显示美文，对应原版的commend；
|kind==2 显示通知，对应原版的commend2；
|其实用兴趣可以把所有的commend兄弟给改写成一个函数。。。
*/
void showCommend(int kind);

int
bbstop10_main()
{
	html_header(1);
	check_msg();
//main frame
	printf("<div style=\"width=100%%; min-height:350px;\">");
	//<!--No.1 Top 10 -->
	showTop10Table();
	printf("</div>");


	printf("<div style=\"width:98%%;margin-top:50px\">");
	printf("<center>");
	printf("<div style=\"width:48%%;min-height:100px;float:left;overflow:hidden;\">");
	//<!--No.2  COMMEND -->
	showCommend(1);
	printf("</div>");

	//<!--No.3 COMMEND2-->
	printf("<div style=\"width:48%%; min-height:100px;float:left; overflow:auto;\">");
	showCommend(2);
	printf("</div>");
	printf("</center>");
	printf("</div>");

	http_quit();
	return 0;
}

void showTop10Table(){
	struct mmapfile mf = {.ptr = NULL};
	MMAP_TRY {
		if (mmapfile("wwwtmp/ctopten", &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("文件错误");
		}
		fwrite(mf.ptr, mf.size, 1, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
}

void showCommend(int kind){
	FILE *fp = NULL;
	struct commend x;
	char allcanre[256];
	char *head[3];
	int i;//, total;
	head[1]="今日美文推荐";
	head[2]="今日通知公告";
	printf("<body><center><div class=rhead>兵马俑 BBS --<span class=h11> %s</span></div>",head[kind]);
	printf("<hr>");
	if(1==kind)
		fp=fopen(COMMENDFILE,"r");
	else if(2==kind)
		fp=fopen(COMMENDFILE2,"r");
	if (!fp)
		http_fatal("目前没有任何推荐文章");
	printf("<table border=1>");
	printf("<tr><td>No.</td><td>讨论区</td><td>标题</td><td>作者</td></tr>");
	fseek(fp, -20*sizeof(struct commend), SEEK_END);
	for(i=20; i>0; i--) {
		strcpy(allcanre, "");
		if(fread(&x, sizeof(struct commend), 1, fp)<=0)
			break;
		if(x.accessed & FH_ALLREPLY)
			strcpy(allcanre," style='color:red;' ");
		x.board[sizeof(x.board) - 1] = 0;
		x.filename[sizeof(x.filename) - 1] = 0;
		x.title[sizeof(x.title) - 1] = 0;
		x.userid[sizeof(x.userid) - 1] = 0;
		printf("<tr><td>  %d  </td> <td><a href=\"%s%s\" >%-13s</a></td> <td><a href=con?B=%s&F=%s%s>%-30s</a></td> <td><a href=qry?U=%s >%-12s</a></td> </tr>",
			21-i, showByDefMode(), x.board, x.board, x.board, x.filename, allcanre, x.title, x.userid, x.userid);

	}
	fclose(fp);
	printf("</table></center></body>");
}
