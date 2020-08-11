// BMY RSS Page
// created by interma 
// 2006-3-21 
// refer tju_bbsrss and ytht_bbsrss(thanks their works)

#include <time.h>
#include "bbslib.h"

#define CHARSET "gb2312"
// #define SMAGIC "BMYALLPGGUNZAIJJWPKSMZHTCMPNTOYACOOH_B" 			
#define TOP10MARK "TOP10"

static void prt_summary(int j, int *rssform, struct fileheader *rssdata, char* board);

static void
rss_chartrans(char *s, char *s0)  // transfer characters according to XML standard
{
	char ansibuf[80], buf2[80];
	char *tmp;
	int c, bold = 0, m, i, len;
	len = 0;
	for (i = 0; (c = s0[i]); i++) {
		switch (c) {
		case '&':
			strnncpy2(s, &len, "&amp;", 5);
			break;
		case '<':
			strnncpy2(s, &len, "&lt;", 4);
			break;
		case '>':
			strnncpy2(s, &len, "&gt;", 4);
			break;
		case '\'':  // not strictly needed
			strnncpy2(s, &len, "&apos;", 6);
			break;
		case '"':   // not strictly needed
			strnncpy2(s, &len, "&quot;", 6);
		case '\033':
			if (s0[i + 1] != '[')
				continue;
			for (m = i + 2; s0[m] && m < i + 24; m++)
				if (strchr("0123456789;", s0[m]) == 0)
					break;
			strsncpy(ansibuf, &s0[i + 2], m - (i + 2) + 1);
			i = m;
			if (s0[i] != 'm')
				continue;
			if (strlen(ansibuf) == 0) {
				bold = 0;
				strnncpy2(s, &len, "</font><font class=c37>",
					  23);
			}
			tmp = strtok(ansibuf, ";");
			while (tmp) {
				c = atoi(tmp);
				tmp = strtok(0, ";");
				if (c == 0) {
					strnncpy2(s, &len,
						  "</font><font class=c37>",
						  23);
					bold = 0;
				}
				if (c >= 30 && c <= 37) {
					if (bold == 1) {
						sprintf(buf2,
							"</font><font class=d%d>",
							c);
						strnncpy2(s, &len, buf2, 23);
					}
					if (bold == 0) {
						sprintf(buf2,
							"</font><font class=c%d>",
							c);
						strnncpy2(s, &len, buf2, 23);
					}
				}
			}
			break;
		default:
			s[len++] = c;
		}
	}
	s[len] = 0;
}

static int
rss_fshow_file(FILE* output, char* board, struct fileheader* x, int n)
{
	FILE *fp;
	const int max_output_length = 1024 + 512;  // summary maximum length
	int output_len = 0;
	char path[80], buf[512], *ptr;
	int ano = 0, nquote = 0, lastq = 0;
	sprintf(path, "boards/%s/%s", board, fh2fname(x));
	fp = fopen(path, "r");
	if (fp == 0)
		return -1;
	fdisplay_attach(NULL, NULL, NULL, NULL);
	while (1) {
		if (fgets(buf, 500, fp) == 0)
			break;
		if (!strncmp(buf, "begin 644 ", 10)) {
			errlog("old attach %s", path);
			ano++;
			fdisplay_attach(output, fp, buf, fh2fname(x));
			fprintf(output, "\n<br>");
			continue;
		}
		if (!strncmp(buf, "beginbinaryattach ", 18)) {
			unsigned int len;
			char ch;
			char buf2[256], buf3[256];
			fread(&ch, 1, 1, fp);
			if (ch != 0) {
				ungetc(ch, fp);
				fhhprintf(output, "%s", buf);
				continue;
			}
			ptr = strchr(buf, '\r');
			if (ptr)
				*ptr = 0;
			ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			ano++;
			ptr = buf + 18;
			fread(&len, 4, 1, fp);
			len = ntohl(len);
			sprintf(buf2, "attach/bbscon/%s?B=%s&F=%s", ptr,
				board, fh2fname(x));
			sprintf(buf3, "%s/%s", board, fh2fname(x));
			fprintbinaryattachlink(output, ano, ptr,
					       -4 + (int) ftell(fp), len, buf2,
					       buf3);
			fseek(fp, len, SEEK_CUR);
			continue;
		}
		ptr = buf;
		if (!strncmp(buf, ": : ", 4) || !strncmp(buf, ": 发信站", 8) ||
		    !strncmp(buf, ": 标  题", 8))
			continue;
		if (!strncmp(buf, ": ", 2)) {
			if (nquote > 3 || strlen(buf) < 4)
				continue;
			nquote++;
			if (!lastq)
				fprintf(output, "<div class=quote>");
			lastq = 1;
		} else {
			if (lastq)
				fprintf(output, "</div>");
			lastq = 0;
		}
		if (0 && !strncmp(buf, "【 在 ", 4))
			continue;
		fhhprintf(output, "%s", noansi(buf));  // noansi() remove #27 char which is not alowed to appear in xml
		output_len += strlen(buf);
		if (output_len > max_output_length) break;  // exceed output length limit
	}
	if (lastq)
		fprintf(output, "</font>");
	fclose(fp);
	return 0;
}

static void
prt_header()
{
	printf("Content-type: text/xml; charset=%s\n\n", CHARSET);  
	printf("<?xml version=\"1.0\" encoding=\"%s\"?>\n", CHARSET);
	printf("<?xml-stylesheet href=\"/bbsrss.xsl\" type=\"text/xsl\" media=\"screen\"?>");
}

static void
prt_channel(struct boardmem *x)
{
	char sec[10],   secname[40];  // 区号 区名
	char board[40], brdname[40];  // 英文版名 中文版名  
	char date[50];
	time_t now = time(0);
	sprintf(date,"%s",ctime(&now));

	strcpy(board, x->header.filename); // 英文版名
	strcpy(sec, x->header.sec1);  // 区号
	strcpy(secname, nohtml(getsectree(x->header.sec1)->title)); // 区名
	strcpy(brdname, x->header.title); // 中文版名

	printf("<rss version=\"2.0\" \n\txmlns:content=\"http://purl.org/rss/1.0/modules/content/\"\n\txmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n");
	printf("<channel>");
	printf("<title>%s - %s</title>\n", MY_BBS_NAME, brdname);
	printf("<link>http://%s/" SMAGIC "/tdoc?B=%s</link>\n", MY_BBS_DOMAIN, board);
	printf("<description>%s - %s区 %s %s(%s) 最近主题文章列表</description>\n", MY_BBS_NAME, sec, secname, brdname, board);
	printf("<language>zh-cn</language>\n");
	printf("<generator>http://%s %s</generator>\n", MY_BBS_DOMAIN, MY_BBS_NAME);
	printf("<webMaster>interma@stu.xjtu.edu.cn</webMaster>\n");
	printf("<pubDate>%s</pubDate>\n", date);  
}

static void
prt_top10_channel()
{  
	char date[50];
	time_t now = time(0);
	sprintf(date,"%s",ctime(&now));

	printf("<rss version=\"2.0\" \n\txmlns:content=\"http://purl.org/rss/1.0/modules/content/\"\n\txmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n");
	printf("<channel>");
	printf("<title>%s - %s</title>\n", MY_BBS_NAME,  "今日10大");
	printf("<link>http://%s/" SMAGIC "/bbstop10\n</link>", MY_BBS_DOMAIN);
	printf("<description>" MY_BBS_NAME " －今日10大</description>\n");
	printf("<language>zh-cn</language>\n");
	printf("<generator>http://%s %s</generator>\n", MY_BBS_DOMAIN, MY_BBS_NAME);
	printf("<webMaster>interma@stu.xjtu.edu.cn</webMaster>\n");
	printf("<pubDate>%s</pubDate>\n", date);
}

static void 
prt_item(int j, int *rssform, struct fileheader *rssdata, char* board, int nodes)
{
	char t[128];
	memset(t,0,sizeof(t));
	rss_chartrans(t, rssdata[j].title);
	printf("<item>\n");
	printf("<title> %s </title>\n", t);
	printf("<link>http://" MY_BBS_DOMAIN "/" SMAGIC "/con?B=%s&amp;F=M.%ld.A</link>\n", board, rssdata[j].thread);
	//printf("<guid isPermaLink=\"true\">http://%s/" SMAGIC "/bbstcon?board=%s&amp;start=%d&amp;th=%d</guid>\n", 
	//	MY_BBS_DOMAIN, board, rssform[j], rssdata[j].thread);
	printf("<dc:creator>%s</dc:creator>\n", fh2owner(&rssdata[j]));
	printf("<pubDate>%s</pubDate>\n", ctime((time_t*)&rssdata[j].filetime));
	printf("<description>\n");
	printf("<![CDATA[\n");
	if (!nodes)
		prt_summary(j, rssform, rssdata, board);  // print each post as an item description
	printf("...]]>\n");
	printf("</description>\n");
	printf("</item>\n");	
}

static void
prt_summary(int j, int *rssform, struct fileheader *rssdata, char* board)
{
	char title[256], dir[80];
	char ffowner[15]; //楼主的ID
	struct fileheader *x;
	int num = 0, found = 0, total;
	int start, thread, floor;

	struct mmapfile mf = { ptr:NULL };

	start = rssform[j];
	thread = rssdata[j].thread;

	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("目录错误");
		}
		total = mf.size / sizeof (struct fileheader);
		if (start < 0 || start >= total) {
			start = Search_Bin(mf.ptr, thread, 0, total - 1);
			if (start < 0)
				start = -(start + 1);
		}
		floor = 0;
		for (num = 0; num < start; num++)
		{
			x = (struct fileheader *) (mf.ptr +
						   num *
						   sizeof (struct fileheader));
			if (thread != 0) {
				if (x->thread != thread) {
					continue;
				}
			} else {
				if (strncmp(x->title, title, 39)) {
					continue;
				}
			}
			floor++;
			if (floor == 1) {
				strcpy(ffowner,fh2owner(x));
			}
		}

		for (num = start; num < total; num++) {
			x = (struct fileheader *) (mf.ptr +
						   num *
						   sizeof (struct fileheader));
			if (thread != 0) {
				if (x->thread != thread) {
					continue;
				}
			} else {
				if (strncmp(x->title, title, 39)) {
					continue;
				}
			}
			floor++;
			if (floor == 1) {
				strcpy(ffowner,fh2owner(x));  // 记下楼主名字
			}

			rss_fshow_file(stdout, board, x, num);

			if (!found) found = 1;
			if (floor == 1) break;  // only display the first post

		}
	}
	MMAP_CATCH {
		found = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (found == 0)
		http_fatal("错误的文件名");
}

static void 
prt_footer()
{
	printf("</channel>\n</rss>\n");
}

static void  
showtop10(int nodes)
{
	char board[10][80];
	char title[10][80];
						
	FILE *fp2 = fopen(MY_BBS_HOME "/etc/dayf_index", "r");
	int totalnum = 0;
	while (fgets(board[totalnum], 80, fp2) != NULL)
	{
		board[totalnum][strlen(board[totalnum]) - 1] = 0;
		fgets(title[totalnum], 80, fp2);
		title[totalnum][strlen(title[totalnum]) - 1] = 0;
		totalnum ++;
	}
	fclose(fp2);

	struct mmapfile mf = { ptr:NULL };
	struct fileheader *x;
	char dir[256];
	//int total;
	
	//html_header(1);
	prt_header();
	prt_top10_channel();

	int i;
	for (i = 0; i < totalnum; i++)
	{
		sprintf(dir, MY_BBS_HOME "/boards/%s/.DIR", board[i]);
		mmapfile(NULL, &mf);	
		if (mmapfile(dir, &mf) < 0)
		{
			continue;
		}
		x = (struct fileheader *) mf.ptr;
		size_t nr = mf.size / sizeof (struct fileheader);

		if (nr == 0)
			continue;

		time_t starttime;
		time_t now = time(0);

		starttime = now - 5 * 86400; //最多查找5  天之前的
		
		//int start = 0;
		int start = Search_Bin(mf.ptr, starttime, 0, nr - 1);
		if (start < 0)
			start = - (start + 1);			

		int j;
		for (j = start; j < nr; j++) 
		{
			if (!strncmp(x[j].title, title[i], 80))
			{
				int rssform = 0;
				prt_item(0, &rssform, x + j, board[i], nodes);
				break;
			}
		}
	}
	
	prt_footer();
	
}


static void 
showboard(char *B, int nodes)
{
	char board[80], buf[128];
	struct boardmem *x1;
	struct fileheader *data = NULL;
	int i, j, total = 0, sum = 0;
	int start, direction, num = 0;
	int first = 0, last = 0;
	int nothingmore = 0;
	struct mmapfile mf = { ptr:NULL };
	static struct fileheader *rssdata = NULL;
	static int *rssform = NULL;
	if (NULL == rssdata) {
		rssdata = malloc(50 * sizeof (struct fileheader));
		if (NULL == rssdata)
			http_fatal("faint, memory leak? --from bbsrss");
	}
	if (NULL == rssform) {
		rssform = malloc(50 * sizeof (int));
		if (NULL == rssform)
			http_fatal("faint, memory leak? --from bbsrss");
	}
	changemode(READING);
	memset(board,0,sizeof(board));
	strsncpy(board, B, 32);
	x1 = getboard(board);
	if (x1 == 0) {
		html_header(1);
		nosuchboard(board, "rss");
	}
	if (!has_read_perm_x(&currentuser, x1)) {
		html_header(1);
		http_fatal("Sorry，您没有阅读此版的权利！\n");
	}
	//start = atoi(getparm("S"));
	//direction = atoi(getparm("D"));
	start = direction = 0;
	if (direction == 0)
		direction = -1;
	updateinboard(x1);  // 计算在线记数等..
	strcpy(board, x1->header.filename);
	sprintf(buf, "boards/%s/.DIR", board);

	if (strcmp(board, "hell") == 0)
	{
		html_header(1);
		http_fatal("bt 不是鬼，还捣乱，小心阎王找你。\n");
	}
	if (strcmp(board, "prison") == 0)
	{
		html_header(1);
		http_fatal("bt 屁股发痒，想去坐牢？ \n");
	}

	if (cache_header(file_time(buf), 10))
		return;

	mmapfile(NULL, &mf);
	MMAP_TRY {
		if (mmapfile(buf, &mf) < 0) {
			MMAP_UNTRY;
			html_header(1);
			http_fatal("无法读取文章列表"); 
		}
		data = (void *) mf.ptr;
		total = mf.size / sizeof (struct fileheader);
		if (total <= 0) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			html_header(1);
			http_fatal("本讨论区目前没有文章");
		}
		if (start == 0) {
			num = total - 1;
			direction = -1;
		} else {
			num = Search_Bin(mf.ptr, start, 0, total - 1);
			if (1 == direction) {
				if (num < 0)
					num = -(num + 1);
				else
					num++;
			} else {
				if (num < 0)
					num = -(num + 1);
				num--;
			}
		}
		for (i = num; i >= 0 && i < total; i += direction) {
			if (data[i].thread != data[i].filetime)
				continue;
			memcpy(&(rssdata[sum]), &(data[i]),
			       sizeof (struct fileheader));
			rssform[sum] = i;
			last = data[i].filetime;
			sum++;
			if (sum == 1)
				first = data[i].filetime;
			if (sum > w_info->t_lines - 2)
				break;
		}
		if (i < 0 || i >= total)
			nothingmore = 1;

		prt_header();   
		prt_channel(x1);  // print copyright of TJUBBS and board informations.

		for (i = sum-1; i >= 0; i--) {
			if (direction == -1)
				j = sum - i - 1;
			else
				j = i;
			prt_item(j, rssform, rssdata, board, nodes);  // print each post as an item.
		}
		prt_footer();  // xml footer msg
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	http_quit();
	return;
}

// need parameter: mode, B 
int
bbsrss_main()
{
	char board[32];
	int nodes = 0;

	strsncpy(board, getparm("board"), sizeof(board));
	
	if (strcmp("1", getparm("nodes")) == 0) //if nodes==1, dont show description.
		nodes = 1;

	if (strncmp(board, TOP10MARK, sizeof(board)) == 0) // top10 rss output
	{
		showtop10(nodes);
	}
	else // board rss output
	{
		showboard(board, nodes);
	}

	return 0;
}
