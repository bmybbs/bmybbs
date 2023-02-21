#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include "config.h"
#include "apilib.h"
#include "error_code.h"
#include "ytht/strlib.h"
#include "ytht/timeop.h"
#include "ytht/fileop.h"
#include "ytht/numbyte.h"
#include "bmy/convcode.h"
#include "bmy/board.h"
#include "ythtbbs/attach.h"
#include "ythtbbs/binaryattach.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/permissions.h"
#include "ythtbbs/user.h"
#include "ythtbbs/record.h"
#include "ythtbbs/article.h"
#include "ythtbbs/binaryattach.h"
#include "ythtbbs/docutil.h"
#include "ythtbbs/session.h"

typedef struct selem *pelem;
typedef struct selem {
	unsigned char digit[8];
	unsigned char digitcount;
	pelem next;
} telem;

/**
 * @brief 解析 ansi 控制符
 * 该方法来自 theZiz/aha。
 * @param s
 * @return
 */
pelem parseInsert(char* s);

/**
 * @brief 删除 ansi 解析
 * 该方法来自 theZiz/aha。
 * @param elem
 */
void deleteParse(pelem elem);

/**
 * @ 获取下一个字符
 * 该方法来自 theZiz/aha。
 * @param fp
 * @param future
 * @param future_char
 * @return
 */
int getNextChar(register FILE* fp, int *future, int *future_char);

/** 再应用程序启动的时候初始化共享内存
 *
 * @return <ul><li>0:成功</li><li>-1:失败</li></ul>
 */
int shm_init()
{
	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_Board_resolve();
	return 0;
}

int getuser_s(struct userec *user, const char *id) {
	int uid;
	uid = getusernum(id);
	if (uid < 0 || user == NULL)
		return -1;

	if (uid + 1 > ythtbbs_cache_UserTable_get_number())
		ythtbbs_cache_UserTable_resolve();

	if (get_record(PASSFILE, user, sizeof(struct userec), uid + 1) < 0) {
		return -2;
	}

	return 0;
}

int getusernum(const char *id) {
	return ythtbbs_cache_UserIDHashTable_find_idx(id);
}

/** 依据用户权限位获取本站职位名称
 *
 * @param userlevel
 * @return
 */
char * getuserlevelname(unsigned userlevel)
{
	if((userlevel & PERM_SYSOP) && (userlevel & PERM_ARBITRATE))
		return "本站顾问团";
	else if(userlevel & PERM_SYSOP)
		return "现任站长";
	else if(userlevel & PERM_OBOARDS)
		return "实习站长";
	else if(userlevel & PERM_ARBITRATE)
		return "现任纪委";
	else if(userlevel & PERM_SPECIAL4)
		return "区长";
	else if(userlevel & PERM_WELCOME)
		return "系统美工";
	else if(userlevel & PERM_SPECIAL7) {
		if((userlevel & PERM_SPECIAL1) && (userlevel & PERM_CLOAK))
			return "离任程序员";
		else
			return "程序组成员";
	} else if(userlevel & PERM_ACCOUNTS)
		return "帐号管理员";
	else if(userlevel & PERM_BOARDS)
		return "版主";
	else
		return "";
}

const char *calc_exp_str_utf8(int exp) {
	int expbase = 0;

	if (exp == -9999)
		return "没等级";
	if (exp <= 100 + expbase)
		return "新手上路";
	if (exp <= 450 + expbase)
		return "一般站友";
	if (exp <= 850 + expbase)
		return "中级站友";
	if (exp <= 1500 + expbase)
		return "高级站友";
	if (exp <= 2500 + expbase)
		return "老站友";
	if (exp <= 3000 + expbase)
		return "长老级";
	if (exp <= 5000 + expbase)
		return "本站元老";
	return "开国大老";
}

const char *calc_perf_str_utf8(int perf) {
	if (perf == -9999)
		return "没等级";
	if (perf <= 5)
		return "赶快加油";
	if (perf <= 12)
		return "努力中";
	if (perf <= 35)
		return "还不错";
	if (perf <= 50)
		return "很好";
	if (perf <= 90)
		return "优等生";
	if (perf <= 140)
		return "太优秀了";
	if (perf <= 200)
		return "本站支柱";
	if (perf <= 500)
		return "神～～";
	return "机器人！";
}

/** 保存用户数据到 passwd 文件中
 * @warning 线程安全有待检查。
 * @param x
 * @return
 */
int save_user_data(struct userec *x)
{
	int n = getusernum(x->userid);
	if(n < 0 || n > 1000000)
		return 0;
	return substitute_record(PASSFILE, x, sizeof(struct userec), n + 1);
}

char *string_replace(char *ori, const char *old, const char *new)
{
	int tmp_string_length = strlen(ori) + strlen(new) - strlen(old) + 1;

	char *ch;
	ch = strstr(ori, old);

	if(!ch)
		return ori;

	char *tmp_string = (char *)malloc(tmp_string_length);
	if(tmp_string == NULL) return ori;

	memset(tmp_string, 0, tmp_string_length);
	strncpy(tmp_string, ori, ch - ori);
	*(tmp_string + (ch - ori)) = 0;
	sprintf(tmp_string + (ch - ori), "%s%s", new, ch+strlen(old));
	*(tmp_string + tmp_string_length - 1) = 0;

	free(ori);
	ori = tmp_string;

	return ori;
}

void add_attach_link(struct attach_link **attach_link_list, const char *str_link, const unsigned int size)
{
	struct attach_link *end;
	struct attach_link *a = (struct attach_link *)malloc(sizeof(struct attach_link));
	memset(a, 0, sizeof(*a));
	ytht_strsncpy(a->link, str_link, sizeof(a->link));
	a->size = size;

	if(!(*attach_link_list))
		*attach_link_list = a;
	else {
		end = *attach_link_list;
		while (end->next) {
			end = end->next;
		}
		end->next = a;
	}
}

void add_attach_entity(struct attach_link **root, struct attach_link *node) {
	struct attach_link *end;

	if (*root == NULL) {
		*root = node;
	} else {
		end = *root;
		while (end->next) {
			end = end->next;
		}
		end->next = node;
	}
}

void free_attach_link_list(struct attach_link *root)
{
	struct attach_link *curr = NULL, *next = NULL;
	curr = root;
	while(curr) {
		next = curr->next;
		if (curr->name)
			free(curr->name);
		free(curr);
		curr = next;
	}
}

pelem parseInsert(char* s)
{
	pelem firstelem=NULL;
	pelem momelem=NULL;
	unsigned char digit[8];
	unsigned char digitcount=0;
	unsigned char a;
	int pos=0;
	for (pos=0;pos<1024;pos++)
	{
		if (s[pos]=='[')
			continue;
		if (s[pos]==';' || s[pos]==0)
		{
			if (digitcount<=0)
			{
				digit[0]=0;
				digitcount=1;
			}

			pelem newelem=(pelem)malloc(sizeof(telem));
			for (a=0;a<8;a++)
				newelem->digit[a]=digit[a];
			newelem->digitcount=digitcount;
			newelem->next=NULL;
			if (momelem==NULL)
				firstelem=newelem;
			else
				momelem->next=newelem;
			momelem=newelem;
			digitcount=0;
			memset(digit,0,8);
			if (s[pos]==0)
				break;
		}
		else
		if (digitcount<8)
		{
			digit[digitcount]=s[pos]-'0';
			digitcount++;
		}
	}
	return firstelem;
}

void deleteParse(pelem elem)
{
	while (elem!=NULL)
	{
		pelem temp=elem->next;
		free(elem);
		elem=temp;
	}
}

int getNextChar(register FILE* fp, int *future, int *future_char)
{
	int c;
	if (*future)
	{
		*future=0;
		return *future_char;
	}
	if ((c = fgetc(fp)) != EOF)
		return c;
	return -1; // error
}

char *parse_article(const char *bname, const char *fname, int mode, struct attach_link **attach_link_list)
{
	if(!bname || !fname)
		return NULL;

	if(mode!=ARTICLE_PARSE_WITHOUT_ANSICOLOR && mode!=ARTICLE_PARSE_WITH_ANSICOLOR)
		return NULL;

	char article_filename[256];
	sprintf(article_filename, "boards/%s/%s", bname, fname);
	FILE *article_stream = fopen(article_filename, "r");
	if(!article_stream)
		return NULL;

	FILE *mem_stream, *html_stream;
	char buf[512], attach_link[256], *tmp_buf, *mem_buf, *html_buf, *attach_filename;
	size_t mem_buf_len, html_buf_len, attach_file_size;
	int attach_no = 0;

	mem_stream = open_memstream(&mem_buf, &mem_buf_len);
	fseek(article_stream, 0, SEEK_SET);
	keepoldheader(article_stream, SKIPHEADER);

	while(1) {
		if(fgets(buf, 500, article_stream) == 0)
			break;

		// RAW 模式下跳过qmd
		if(mode == ARTICLE_PARSE_WITHOUT_ANSICOLOR
				&& strncmp(buf, "--\n", 3) == 0)
			break;

		// 附件处理
		if(!strncmp(buf, "begin 644", 10)) {
			// TODO: 老方式暂不实现
			fflush(mem_stream);
			fclose(mem_stream);
			fclose(article_stream);
			free(mem_buf);
			return NULL;
		} else if(checkbinaryattach(buf, article_stream, &attach_file_size)) {
			attach_no++;
			attach_filename = buf + 18;
			fprintf(mem_stream, "#attach %s\n", attach_filename);
			memset(attach_link, 0, 256);
			snprintf(attach_link, 256, "http://%s:8080/%s/%s/%d/%s", MY_BBS_DOMAIN,
					bname, fname, -4+(int)ftell(article_stream), attach_filename);
			add_attach_link(attach_link_list, attach_link, attach_file_size);
			fseek(article_stream, attach_file_size, SEEK_CUR);
			continue;
		}

		// 常规字符处理
		if(mode == ARTICLE_PARSE_WITHOUT_ANSICOLOR
				&& strchr(buf, '\033') != NULL) {
			tmp_buf = strdup(buf);

			while(strchr(tmp_buf, '\033') != NULL)
				tmp_buf = string_replace(tmp_buf, "\033", "[ESC]");

			fprintf(mem_stream, "%s", tmp_buf);
			free(tmp_buf);
			tmp_buf = NULL;
		} else{
			fprintf(mem_stream, "%s", buf);
		}
	}
	fflush(mem_stream);
	fclose(article_stream);

	char *utf_content;
	if(mode == ARTICLE_PARSE_WITHOUT_ANSICOLOR) { // 不包含 '\033'，直接转码
		utf_content = (char *)malloc(3*mem_buf_len);
		memset(utf_content, 0, 3*mem_buf_len);
		g2u(mem_buf, mem_buf_len, utf_content, 3*mem_buf_len);
	} else { // 将 ansi 色彩转为 HTML 标记
		html_stream = open_memstream(&html_buf, &html_buf_len);
		fseek(mem_stream, 0, SEEK_SET);
		fprintf(html_stream, "<article>\n");

		aha_convert(mem_stream, html_stream);

		fprintf(html_stream, "</article>");

		fflush(html_stream);
		fclose(html_stream);

		utf_content = (char*)malloc(3*html_buf_len);
		memset(utf_content, 0, 3*html_buf_len);
		g2u(html_buf, html_buf_len, utf_content, 3*html_buf_len);
		free(html_buf);
	}

	// 释放资源
	fclose(mem_stream);
	free(mem_buf);

	return utf_content;
}

#define YTHTBBS_ATTACH_HEADER "beginbinaryattach "

char *parse_article_js_internal(struct mmapfile *pmf, struct attach_link **attach_link_list, const char *bname, const char *fname) {
	char *content = NULL, *token;
	size_t i, j, m, n, line, pos;
	struct attach_link *attach = NULL;
	bool is_first_line;

	if (pmf->ptr == NULL || pmf->size == 0 || bname == NULL || fname == NULL)
		return NULL;

	content = calloc(pmf->size, 1); // content < 文章长度
	if (content != NULL) {
		// 处理 content 和 attach 链表
		i = 0; // 用于 mf 索引
		j = 0; // 用于 content
		line = 0; // 行号，用于跳过开头
		while (i < pmf->size && line < 4) {
			if (pmf->ptr[i++] == '\n')
				line++;
		}

		is_first_line = true;
		while (i < pmf->size) {
			if (is_first_line || pmf->ptr[i] == '\n') {
				is_first_line = false;
				// 换行符，先拷贝
				if (pmf->ptr[i] == '\n')
					content[j++] = pmf->ptr[i++];

				// 判断新行是否为签名档，目前跳过
				if (pmf->size - i > 3 && strncmp(pmf->ptr + i, "--\n", 3) == 0) {
					break;
				}

				if (pmf->size - i > 18) {
					// 判断新行是否为附件
					if (strncmp(pmf->ptr + i, YTHTBBS_ATTACH_HEADER, strlen(YTHTBBS_ATTACH_HEADER)) == 0) {
						m = i + strlen(YTHTBBS_ATTACH_HEADER);
						if (m < pmf->size) {
							token = pmf->ptr + m;
							n = m;
							while (n < pmf->size - 6 /* 终止符 + 大小 + 换行符 */) {
								if (pmf->ptr[n] == '\n') {
									if (pmf->ptr[n + 1] != 0) {
										// 不是附件模式，跳出
										break;
									}
									// 确实存在附件，输出 content
									j += sprintf(content + j, "#attach %s", token);

									// 追加附件链表
									attach = malloc(sizeof(struct attach_link));
									if (attach == NULL) {
										goto ERROR;
									}
									memset(attach, 0, sizeof(struct attach_link));
									attach->name = malloc(n - m + 1);
									if (attach->name == NULL) {
										goto ERROR;
									}
									strncpy(attach->name, token, n - m);
									attach->name[n - m] = 0;

									pos = n + 2;
									memcpy(&attach->size, pmf->ptr + pos, sizeof(unsigned int));
									attach->size = ntohl(attach->size);

									// link: char[256] 和 nju09 一样，如果溢出，则 nju09 也需要更新
									snprintf(attach->link, sizeof(attach->link), "/%s/bbscon/%s?B=%s&F=%s&attachpos=%zu&attachname=/%s", SMAGIC, attach->name, bname, fname, pos, attach->name);

									// 拷贝文件头部的 file signature
									memcpy(attach->signature, pmf->ptr + pos + 4 /* sizeof int */, (attach->size > BMY_SIGNATURE_LEN) ? BMY_SIGNATURE_LEN : attach->size);

									add_attach_entity(attach_link_list, attach);

									// 偏移 i
									i = pos + 4 /* sizeof int */ + attach->size;

									// 跳出 while
									break;
								} else {
									n++;
								}
							}
						}
					}
				}
			} else {
				// 普通内容，直接拷贝
				content[j++] = pmf->ptr[i++];
			}
		}
	}

	return content;

ERROR:
	if (content)
		free(content);
	if (attach) {
		if (attach->name)
			free(attach->name);
		free(attach);
	}
	return NULL;
}

char *parse_article_js(const char *bname, const char *fname, struct attach_link **attach_link_list) {
	if (!bname || !fname)
		return NULL;

	char article_filename[256], *content = NULL, *content_utf8 = NULL;
	size_t content_utf8_size;
	struct mmapfile mf = { .ptr = NULL };
	snprintf(article_filename, sizeof(article_filename), MY_BBS_HOME "/boards/%s/%s", bname, fname);
	if (mmapfile(article_filename, &mf) < 0)
		return NULL;

	content = parse_article_js_internal(&mf, attach_link_list, bname, fname);
	if (content == NULL) {
		mmapfile(NULL, &mf);
		return NULL;
	}

	// 处理转码
	mmapfile(NULL, &mf);
	content_utf8_size = strlen(content) * 2;
	content_utf8 = calloc(content_utf8_size, 1);
	if (content_utf8 == NULL) {
		free(content);
		return NULL;
	}

	g2u(content, strlen(content), content_utf8, content_utf8_size);
	free(content);
	return content_utf8;
}

void aha_convert(FILE *in_stream, FILE *out_stream)
{
	char line_break=0;
	int c;
	int fc = -1; //Standard Foreground Color //IRC-Color+8
	int bc = -1; //Standard Background Color //IRC-Color+8
	int it = 0; //italic
	int ul = 0; //Not underlined
	int bo = 0; //Not bold
	int bl = 0; //No Blinking
	int ofc,obc,oit,oul,obo,obl; //old values
	int line=0;
	int momline=0;
	int newline=-1;
	int temp;

	int future=0;
	int future_char=0;

	while((c=fgetc(in_stream)) != EOF) {
		if(c=='\033') {
			//Saving old values
			ofc=fc;
			obc=bc;
			oit=it;
			oul=ul;
			obo=bo;
			obl=bl;
			//Searching the end (a letter) and safe the insert:
			c='0';
			char buffer[1024];
			int counter=0;
			while ((c<'A') || ((c>'Z') && (c<'a')) || (c>'z')) {
				c=getNextChar(in_stream, &future, &future_char);
				buffer[counter]=c;
				if (c=='>') //end of htop
					break;
				counter++;
				if (counter>1022)
					break;
			}
			buffer[counter-1]=0;
			pelem elem;
			switch (c) {
			case 'm':
				//printf("\n%s\n",buffer); //DEBUG
				elem=parseInsert(buffer);
				pelem momelem=elem;
				while (momelem!=NULL) {
					//jump over zeros
					int mompos=0;
					while (mompos<momelem->digitcount && momelem->digit[mompos]==0)
						mompos++;
					if (mompos==momelem->digitcount) //only zeros => delete all
					{
						bo=0;it=0;ul=0;bl=0;fc=-1;bc=-1;
					} else {
						switch (momelem->digit[mompos]) {
						case 1: bo=1; break;
						case 2:
							if (mompos+1<momelem->digitcount) {
								switch (momelem->digit[mompos+1]) {
								case 1: //Reset blink and bold
									bo=0;
									bl=0;
									break;
								case 3: //Reset italic
									it=0;
									break;
								case 4: //Reset underline
									ul=0;
									break;
								case 7: //Reset Inverted
									temp = bc;
									if (fc == -1 || fc == 9)
									{
										bc = 0;
									}
									else
										bc = fc;
									if (temp == -1 || temp == 9)
									{
										fc = 7;
									}
									else
										fc = temp;
									break;
								}
							}
							break;
						case 3:
							if (mompos+1==momelem->digitcount)
								it=1;
							if (mompos+1<momelem->digitcount)
								fc=momelem->digit[mompos+1];
							break;
						case 4:
							if (mompos+1==momelem->digitcount)
								ul=1;
							else
								bc=momelem->digit[mompos+1];
							break;
						case 5: bl=1; break;
						case 7: //TODO: Inverse
							temp = bc;
							if (fc == -1 || fc == 9) {
								bc = 0;
							} else
								bc = fc;
							if (temp == -1 || temp == 9) {
								fc = 7;
							} else
								fc = temp;
							break;
						}
					}
					momelem=momelem->next;
				}
				deleteParse(elem);
				break;
			case 'H': break;
			}

			//Checking the differeces
			if ((fc!=ofc) || (bc!=obc) || (it!=oit) || (ul!=oul) || (bo!=obo) || (bl!=obl)) //ANY Change
			{
				if ((ofc!=-1) || (obc!=-1) || (oit!=0) || (oul!=0) || (obo!=0) || (obl!=0))
					fprintf(out_stream, "</span>");
				if ((fc!=-1) || (bc!=-1) || (it!=0) || (ul!=0) || (bo!=0) || (bl!=0))
				{
					fprintf(out_stream, "<span class=\"aha ");
					switch (fc)
					{
						case	0: fprintf(out_stream, "aha-fc-black "); break; //Black
						case	1: fprintf(out_stream, "aha-fc-red "); break; //Red
						case	2: fprintf(out_stream, "aha-fc-green "); break; //Green
						case	3: fprintf(out_stream, "aha-fc-olive "); break; //Yellow
						case	4: fprintf(out_stream, "aha-fc-blue "); break; //Blue
						case	5: fprintf(out_stream, "aha-fc-purple "); break; //Purple
						case	6: fprintf(out_stream, "aha-fc-teal "); break; //Cyan
						case	7: fprintf(out_stream, "aha-fc-gray "); break; //White
						case	9: fprintf(out_stream, "aha-fc-black "); break; //Reset
					}
					switch (bc)
					{
						//case -1: printf("background-color:white; "); break; //StandardColor
						case	0: fprintf(out_stream, "aha-bg-black "); break; //Black
						case	1: fprintf(out_stream, "aha-bg-red "); break; //Red
						case	2: fprintf(out_stream, "aha-bg-green "); break; //Green
						case	3: fprintf(out_stream, "aha-bg-olive ");  break; //Yellow
						case	4: fprintf(out_stream, "aha-bg-blue "); break; //Blue
						case	5: fprintf(out_stream, "aha-bg-purple "); break; //Purple
						case	6: fprintf(out_stream, "aha-bg-teal "); break; //Cyan
						case	7: fprintf(out_stream, "aha-bg-gray "); break; //White
						case	9: fprintf(out_stream, "aha-bg-white "); break; //Reset
					}
					if (it)
						fprintf(out_stream, "aha-text-italic ");
					if (ul)
						fprintf(out_stream, "aha-text-underline ");
					if (bo)
						fprintf(out_stream, "aha-text-bold ");
					if (bl)
						fprintf(out_stream, "aha-text-blink ");

					fprintf(out_stream, "\">");
				}
			}
		} else if(c!='\b'){
			line++;
			if (line_break) {
				fprintf(out_stream, "\n");
				line=0;
				line_break=0;
				momline++;
			}
			if (newline>=0) {
				while (newline>line) {
					fprintf(out_stream, " ");
					line++;
				}
				newline=-1;
			}
			switch (c) {
			case '&':	fprintf(out_stream, "&amp;"); break;
			case '\"': 	fprintf(out_stream, "&quot;"); break;
			case '<':	fprintf(out_stream, "&lt;"); break;
			case '>':	fprintf(out_stream, "&gt;"); break;
			case '\n':
			case 13:
				momline++;line=0;
				fprintf(out_stream, "<br />\n"); break;
			default:	fprintf(out_stream, "%c",c);
			}
		}
	}

	if ((fc!=-1) || (bc!=-1) || (it!=0) || (ul!=0) || (bo!=0) || (bl!=0))
		fprintf(out_stream, "</span>\n");
}

int f_write(const char *filename, const char *buf)
{
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == 0)
		return -1;
	fputs(buf, fp);
	fclose(fp);
	return 0;
}

int f_append(char *filename, char *buf)
{
	FILE *fp;
	fp = fopen(filename, "a");
	if (fp == 0)
		return -1;
	fputs(buf, fp);
	fclose(fp);
	return 0;
}

int mail_count(char *id, int *unread)
{
	struct fileheader *x;
	char path[80];
	int total=0, i=0;
	struct mmapfile mf = { .ptr = NULL };
	*unread = 0;

	setmailfile_s(path, sizeof(path), id, ".DIR");

	if(mmapfile(path, &mf)<0)
		return 0;

	total = mf.size / sizeof(struct fileheader);
	x = (struct fileheader*)mf.ptr;
	for(i=0; i<total; i++) {
		if(!(x->accessed & FH_READ))
			(*unread)++;
		x++;
	}

	mmapfile(NULL, &mf);
	return total;
}

time_t do_article_post(const char *board, const char *title_gbk, const char *content_gbk, const char *id,
		const char *nickname_gbk, const char *ip, int sig, int mark, int outgoing, const char *realauthor, time_t thread)
{
	FILE *fp_final;
	char buf3[1024], *content_gbk_buf;
	size_t content_gbk_buf_len;
	struct fileheader header;
	memset(&header, 0, sizeof(header));
	time_t t;

	if (strcasecmp(id, "Anonymous") != 0)
		fh_setowner(&header, id, 0);
	else
		fh_setowner(&header, realauthor, 1);

	snprintf(buf3, sizeof(buf3), "boards/%s/", board);

	time_t now_t = time(NULL);
	t = trycreatefile(buf3, "M.%ld.A", now_t, 100);
	if (t < 0)
		return -1;

	header.filetime = t;
	header.accessed = mark;

	ytht_strsncpy(header.title, title_gbk, sizeof(header.title));

	char timestr_buf[30];
	char QMD_gbk[300];
	ytht_ctime_r(now_t, timestr_buf);

	content_gbk_buf_len = strlen(content_gbk) + 512;
	content_gbk_buf = malloc(content_gbk_buf_len);
	if (content_gbk_buf == NULL) {
		return -1;
	}

	// TODO: QMD
	// 发信人 信区 标题 发信站
	// 转信 本站
	snprintf(content_gbk_buf, content_gbk_buf_len,
			"\xB7\xA2\xD0\xC5\xC8\xCB: %s (%s), \xD0\xC5\xC7\xF8: %s\n\xB1\xEA  \xCC\xE2: %s\n\xB7\xA2\xD0\xC5\xD5\xBE: %s (%24.24s), %s)\n\n%s",
			id, nickname_gbk, board, title_gbk, MY_BBS_NAME, timestr_buf,
			outgoing ? "\xD7\xAA\xD0\xC5(" MY_BBS_DOMAIN : "\xB1\xBE\xD5\xBE(" MY_BBS_DOMAIN,
			content_gbk);
	snprintf(QMD_gbk, sizeof(QMD_gbk), "\n--\n\033[1;%dm\xA1\xF9 \xC0\xB4\xD4\xB4:\xA3\xAE" MY_BBS_NAME " " MY_BBS_DOMAIN " API [FROM: %.40s]\033[0m\n", 31 + rand() % 7, ip); // 来源

	if (hasbinaryattach(realauthor)) {
		if (insertattachments(buf3, content_gbk_buf, realauthor))
			header.accessed |= FH_ATTACHED;
		if ((fp_final = fopen(buf3, "a")) != NULL) {
			fprintf(fp_final, "%s", QMD_gbk);
			fclose(fp_final);
		}
	} else {
		if (NULL == (fp_final = fopen(buf3, "w"))) {
			free(content_gbk_buf);
			return -1;
		}
		fprintf(fp_final, "%s%s", content_gbk_buf, QMD_gbk);
		fclose(fp_final);
	}

	free(content_gbk_buf);

	sprintf(buf3, "boards/%s/M.%ld.A", board, t);
	header.sizebyte = ytht_num2byte(eff_size(buf3));

	if (thread == -1)
		header.thread = header.filetime;
	else
		header.thread = thread;

	sprintf(buf3, "boards/%s/.DIR", board);
	append_record(buf3, &header, sizeof(header));

	if (!bmy_board_is_system_board(board)) {
		if (thread == -1) {
			bmy_article_add_thread(ythtbbs_cache_Board_get_idx_by_name(board) + 1, header.thread, header.title, header.owner, header.accessed);
		} else {
			bmy_article_add_comment(ythtbbs_cache_Board_get_idx_by_name(board) + 1, thread);
		}
	}

	return t;
}

int do_mail_post(const char *to_userid, const char *title, const char *filename, const char *id,
				const char *nickname, const char *ip, int sig, int mark)
{
	FILE *fp, *fp2;
	char buf[256], dir[256], tmp_utf_buf[1024], tmp_gbk_buf[1024];
	struct fileheader header;
	int t;

	memset(&header, 0, sizeof(header));
	fh_setowner(&header, id, 0);
	setmailfile_s(buf, sizeof(buf), to_userid, "");

	time_t now_t = time(NULL);
	t = trycreatefile(buf, "M.%d.A", now_t, 100);

	if(t<0)
		return -1;

	header.filetime = t;
	header.thread = t;
	u2g(title, strlen(title), header.title, sizeof(header.title));
	header.accessed |= mark;

	fp = fopen(buf, "w");
	if(fp == 0)
		return -2;

	fp2 = fopen(filename, "r");

	char timestr_buf[30];
	ytht_ctime_r(now_t, timestr_buf);
	sprintf(tmp_utf_buf, "寄信人: %s (%s)\n标  题: %s\n发信站: 兵马俑BBS (%s)\n来  源: %s\n\n",
			id, nickname, title, timestr_buf, ip);
	u2g(tmp_utf_buf, 1024, tmp_gbk_buf, 1024);
	fwrite(tmp_gbk_buf, 1, strlen(tmp_gbk_buf), fp);

	if(fp2) {
		int retv;
		while(1) {
			retv = fread(buf, 1, sizeof(buf), fp2);
			if(retv<=0)
				break;
			fwrite(buf, 1, retv, fp);
		}
		fclose(fp2);
	}

	fprintf(fp, "\n--\n");
	// TODO QMD
	// sig_append();

	sprintf(tmp_utf_buf, "\033[1;%dm※ 来源:．兵马俑BBS %s [FROM: %.20s]\033[0m\n",
			31+rand()%7, MY_BBS_DOMAIN " API", ip);
	u2g(tmp_utf_buf, 1024, tmp_gbk_buf, 1024);
	fwrite(tmp_gbk_buf, 1, strlen(tmp_gbk_buf), fp);
	fclose(fp);	// 输出完成

	setmailfile_s(buf, sizeof(buf), to_userid, ".DIR");
	append_record(buf, &header, sizeof(header));
	return 0;
}

int do_mail_post_to_sent_box(const char *userid, const char *title, const char *filename, const char *id,
				const char *nickname, const char *ip, int sig, int mark)
{
	FILE *fp, *fp2;
	char buf[256], dir[256], tmp_utf_buf[1024], tmp_gbk_buf[1024];
	struct fileheader header;
	int t;

	memset(&header, 0, sizeof(header));
	fh_setowner(&header, id, 0);
	setsentmailfile(buf, userid, "");

	time_t now_t = time(NULL);
	t = trycreatefile(buf, "M.%d.A", now_t, 100);

	if(t<0)
		return -1;

	header.filetime = t;
	header.thread = t;
	u2g(title, strlen(title), header.title, sizeof(header.title));
	header.accessed |= mark;

	fp = fopen(buf, "w");
	if(fp == 0)
		return -2;

	fp2 = fopen(filename, "r");

	char timestr_buf[30];
	ytht_ctime_r(now_t, timestr_buf);
	sprintf(tmp_utf_buf, "收信人: %s (%s)\n标  题: %s\n发信站: 兵马俑BBS (%s)\n来  源: %s\n\n",
			id, nickname, title, timestr_buf, ip);
	u2g(tmp_utf_buf, 1024, tmp_gbk_buf, 1024);
	fwrite(tmp_gbk_buf, 1, strlen(tmp_gbk_buf), fp);

	if(fp2) {
		int retv;
		while(1) {
			retv = fread(buf, 1, sizeof(buf), fp2);
			if(retv<=0)
				break;
			fwrite(buf, 1, retv, fp);
		}
		fclose(fp2);
	}

	fprintf(fp, "\n--\n");
	// TODO QMD
	// sig_append();

	sprintf(tmp_utf_buf, "\033[1;%dm※ 来源:．兵马俑BBS %s [FROM: %.20s]\033[0m\n",
			31+rand()%7, MY_BBS_DOMAIN " API", ip);
	u2g(tmp_utf_buf, 1024, tmp_gbk_buf, 1024);
	fwrite(tmp_gbk_buf, 1, strlen(tmp_gbk_buf), fp);
	fclose(fp);	// 输出完成

	setsentmailfile(buf, userid, ".DIR");
	append_record(buf, &header, sizeof(header));
	return 0;
}

static int search_user_article_with_title_keywords_callback(struct boardmem *board, int curr_idx, va_list ap) {
	(void) curr_idx;
	struct user_info *ui = va_arg(ap, struct user_info *);
	struct api_article *articles_array = va_arg(ap, struct api_article *);
	int *article_sum = va_arg(ap, int *);
	int max_searchnum = va_arg(ap, int);
	const char *query_userid = va_arg(ap, const char *);
	const char *title_keyword1 = va_arg(ap, const char *);
	const char *title_keyword2 = va_arg(ap, const char *);
	const char *title_keyword3 = va_arg(ap, const char *);
	time_t starttime = va_arg(ap, time_t);
	time_t now_t = va_arg(ap, time_t);
	int searchtime = va_arg(ap, int);

	char dir[256];
	int nr = 0, start = 0, i;
	struct api_article *tmp_ptr = NULL;
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *x = NULL;

	if (*article_sum >= max_searchnum)
		return QUIT;

	if (!check_user_read_perm_x(ui, board))
		return 0;

	sprintf(dir, "boards/%s/.DIR", board->header.filename);
	mmapfile(NULL, &mf);
	if (mmapfile(dir, &mf) < 0)
		return 0;

	x = (struct fileheader *) mf.ptr;
	nr = mf.size / sizeof(struct fileheader);

	if (nr == 0) {
		mmapfile(NULL, &mf);
		return 0;
	}

	start = Search_Bin(mf.ptr, starttime, 0, nr - 1);
	if (start < 0)
		start = - (start + 1);

	for (i = start; i < nr; i++) {
		if (*article_sum >= max_searchnum)
			break;

		if (labs(now_t - x[i].filetime) > searchtime)
			continue;

		if (query_userid[0] && strcasecmp(x[i].owner, query_userid))
			continue;

		if (title_keyword1 && title_keyword1[0] && !strcasestr(x[i].title, title_keyword1))
			continue;
		if (title_keyword2 && title_keyword2[0] && !strcasestr(x[i].title, title_keyword2))
			continue;
		if (title_keyword3 && title_keyword3[0] && !strcasestr(x[i].title, title_keyword3))
			continue;

		// TODO
		tmp_ptr = &articles_array[*article_sum];
		strcpy(tmp_ptr->board, board->header.filename);
		strcpy(tmp_ptr->title, x[i].title);
		tmp_ptr->filetime = x[i].filetime;
		tmp_ptr->mark = x[i].accessed;
		tmp_ptr->thread = x[i].thread;
		tmp_ptr->sequence_num = i;

		*article_sum = *article_sum + 1;
	}

	mmapfile(NULL, &mf);
	return 0;
}

int search_user_article_with_title_keywords(struct api_article *articles_array,
		int max_searchnum, struct user_info *ui_currentuser, char *query_userid,
		char *title_keyword1, char *title_keyword2, char *title_keyword3,
		int searchtime)
{
	time_t starttime, now_t;
	now_t = time(NULL);
	starttime = now_t - searchtime;
	if(starttime < 0)
		starttime = 0;

	int article_sum = 0;

	ythtbbs_cache_Board_foreach_v(search_user_article_with_title_keywords_callback, ui_currentuser, articles_array, &article_sum, max_searchnum, query_userid, title_keyword1, title_keyword2, title_keyword3, starttime, now_t, searchtime);

	return 0;
}

bool api_check_method(onion_request *req, onion_request_flags flags) {
	return flags == (onion_request_get_flags(req) & OR_METHODS);
}

int api_check_session(onion_request *req, char *cookie_buf, size_t buf_len, struct bmy_cookie *cookie, int *utmp_idx, struct user_info **pptr_info) {
	const char *cookie_str = onion_request_get_cookie(req, SMAGIC);
	*pptr_info = NULL;
	*utmp_idx = -1;
	if(cookie_str == NULL || cookie_str[0] == '\0') {
		return API_RT_NOTLOGGEDIN;
	}

	ytht_strsncpy(cookie_buf, cookie_str, buf_len);
	bmy_cookie_parse(cookie_buf, cookie);
	if (cookie->userid == NULL || cookie->sessid == NULL || strcasecmp(cookie->userid, "guest") == 0) {
		return API_RT_NOTLOGGEDIN;
	}

	*utmp_idx = ythtbbs_session_get_utmp_idx(cookie->sessid, cookie->userid);
	if (*utmp_idx < 0) {
		return API_RT_NOTLOGGEDIN;
	}

	*pptr_info = ythtbbs_cache_utmp_get_by_idx(*utmp_idx);
	return API_RT_SUCCESSFUL;
}

struct json_object *apilib_convert_fileheader_utf_to_jsonobj(struct fileheader_utf *ptr_header) {
	struct json_object *article_obj;

	if (ptr_header == NULL)
		return NULL;

	article_obj = json_object_new_object();
	if (article_obj == NULL)
		return NULL;

	json_object_object_add(article_obj, "boardname_en", json_object_new_string(ptr_header->boardname_en));
	json_object_object_add(article_obj, "boardname_zh", json_object_new_string(ptr_header->boardname_zh));
	json_object_object_add(article_obj, "author", json_object_new_string(ptr_header->owner));
	json_object_object_add(article_obj, "title", json_object_new_string(ptr_header->title));
	json_object_object_add(article_obj, "tid", json_object_new_int64(ptr_header->thread));
	json_object_object_add(article_obj, "count", json_object_new_int(ptr_header->count));
	json_object_object_add(article_obj, "accessed", json_object_new_int(ptr_header->accessed));
	return article_obj;
}

enum ytht_smth_filter_result api_stringfilter(const char *buf_gbk, enum ytht_smth_filter_option mode) {
	struct mmapfile mf = { .ptr = NULL };
	const char *BF;
	enum ytht_smth_filter_result result;

	switch (mode) {
	case YTHT_SMTH_FILTER_OPTION_NORMAL:
		BF = BADWORDS;
		break;
	case YTHT_SMTH_FILTER_OPTION_SIMPLE:
		BF = SBADWORDS;
		break;
	case YTHT_SMTH_FILTER_OPTION_PLTCAL:
		BF = PBADWORDS;
		break;
	default:
		result = YTHT_SMTH_FILTER_RESULT_1984;
	}

	if (mmapfile(BF, &mf) < 0) {
		if (mode != YTHT_SMTH_FILTER_OPTION_NORMAL) {
			result = YTHT_SMTH_FILTER_RESULT_SAFE;
		} else {
			BF = PBADWORDS;

			if (mmapfile(BF, &mf) < 0) {
				result = YTHT_SMTH_FILTER_RESULT_SAFE;
			} else {
				result = ytht_smth_filter_string(buf_gbk, &mf) ? YTHT_SMTH_FILTER_RESULT_WARN : YTHT_SMTH_FILTER_RESULT_SAFE;
			}
		}
	} else {
		result = ytht_smth_filter_string(buf_gbk, &mf) ? YTHT_SMTH_FILTER_RESULT_1984 : YTHT_SMTH_FILTER_RESULT_SAFE;
	}

	if (mf.ptr) {
		mmapfile(NULL, &mf);
	}

	return result;
}

