#include "bbslib.h"
#if defined(ENABLE_GHTHASH) && defined(ENABLE_FASTCGI)
#include <ght_hash_table.h>
#endif
#include "sys/shm.h"
#include "stdarg.h"
//#ifdef GPROF
//#include <iconv_glibc.h>
//#else
#include <iconv.h>
//#endif
#include "bmy/cookie.h"
#include "ythtbbs/session.h"

char needcgi[STRLEN];
char rframe[STRLEN];

static void __unhcode(char *s);
static void extraparam_init(const char *extrastr);
static int user_init(struct userec *x, struct user_info **y, const char *userid, const char *sessid);
static int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);
static void sig_append(FILE * fp, char *id, int sig);
static int useridhash(char *id);
static int setbmhat(struct boardmanager *bm, int *online);

struct wwwstyle wwwstyle[NWWWSTYLE] = {
	// 橘红(大字体)
	{"\xE9\xD9\xBA\xEC(\xB4\xF3\xD7\xD6\xCC\xE5)", CSSPATH "orab.css", "#b3b7e6", "yellow"},
	// 橘红(小字体)
	{"\xE9\xD9\xBA\xEC(\xD0\xA1\xD7\xD6\xCC\xE5)", CSSPATH "oras.css", "#b3b7e6", "yellow"},
	// 蓝色(大字体)
	{"\xC0\xB6\xC9\xAB(\xB4\xF3\xD7\xD6\xCC\xE5)", CSSPATH "blub.css", "#ffc8ce", "#ff8000"},
	// 蓝色(小字体)
	{"\xC0\xB6\xC9\xAB(\xD0\xA1\xD7\xD6\xCC\xE5)", CSSPATH "blus.css", "#ffc8ce", "#ff8000"},
	// 绿色(大字体)
	{"\xC2\xCC\xC9\xAB(\xB4\xF3\xD7\xD6\xCC\xE5)", CSSPATH "greb.css", "#c0c0c0", "yellow"},
	// 绿色(小字体)
	{"\xC2\xCC\xC9\xAB(\xD0\xA1\xD7\xD6\xCC\xE5)", CSSPATH "gres.css", "#c0c0c0", "yellow"},
	// 黑色(大字体)
	{"\xBA\xDA\xC9\xAB(\xB4\xF3\xD7\xD6\xCC\xE5)", CSSPATH "blab.css", "#c0c0c0", "yellow"},
	// 黑色(小字体)
	{"\xBA\xDA\xC9\xAB(\xD0\xA1\xD7\xD6\xCC\xE5)", CSSPATH "blas.css", "#c0c0c0", "yellow"},
	// 自定义的界面
	{"\xD7\xD4\xB6\xA8\xD2\xE5\xB5\xC4\xBD\xE7\xC3\xE6", "bbsucss/ubbs.css", "", ""}
};
struct wwwstyle *currstyle = wwwstyle;
int wwwstylenum = 0;
int usedMath = 0; //本页面中曾经使用数学公式
int usingMath = 0; //当前文章（当前hsprintf方式）在使用数学公式
int withinMath = 0; //正在数学公式中

int
junkboard(char *board)
{
	// 请自定义junkboard.
	return ytht_file_has_word("etc/junkboards", board);
}

int loginok = 0;
int isguest = 0;
int tempuser = 0;
int utmpent = 0;
volatile int incgiloop = 0;
int thispid;
time_t now_t;

jmp_buf cgi_start;

struct userec currentuser;
struct user_info *u_info;
struct wwwsession *w_info;
struct UTMPFILE *shm_utmp;
struct BCACHE *shm_bcache;
struct UCACHE *shm_ucache;
struct UCACHEHASH *uidhashshm;
struct mmapfile mf_badwords  = { .ptr = NULL };
struct mmapfile mf_sbadwords = { .ptr = NULL };
struct mmapfile mf_pbadwords = { .ptr = NULL };
char *ummap_ptr = NULL;
int ummap_size = 0;
char fromhost[BMY_IPV6_LEN]; // 从环境变量获取 IP 地址，IPv4/IPv6 已经由 apache 处理过
struct in6_addr from_addr;   //ipv6 by leoncom

struct boardmem *getbcache();
struct userec *getuser();
char *anno_path_of();
static void updatelastboard(void);

int
f_write(char *file, char *buf)
{
	FILE *fp;
	fp = fopen(file, "w");
	if (fp == 0)
		return -1;
	fputs(buf, fp);
	fclose(fp);
	return 0;
}

int
f_append(char *file, char *buf)
{
	FILE *fp;
	fp = fopen(file, "a");
	if (fp == 0)
		return -1;
	fputs(buf, fp);
	fclose(fp);
	return 0;
}

int
put_record(void *buf, int size, int num, char *file)
{
	int fd;
	if (size < 1 || size > 4096)
		return 0;
	if (num < 0 || num > 1000000)
		return 0;
	fd = open(file, O_WRONLY);
	if (fd == -1)
		return 0;
	flock(fd, LOCK_EX);
	lseek(fd, num * size, SEEK_SET);
	write(fd, buf, size);
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

//num=0 refer to the first record
int
del_record(char *file, int size, int num)
{
	return !delete_file(file, size, num + 1, NULL);
}

char *
noansi(char *s)
{
	static char buf[1024];
	int i = 0, mode = 0;
	while (s[0] && i < 1023) {
		if (mode == 0) {
			if (s[0] == 27) {
				mode = 1;
			} else {
				buf[i] = s[0];
				i++;
			}
		} else {
			if (!strchr(";[0123456789", s[0]))
				mode = 0;
		}
		s++;
	}
	buf[i] = 0;
	return buf;
}

char *
nohtml(const char *s)
{
	static char buf[1024];
	int i = 0;

	while (*s && i < 1019) {
		if (*s == '<') {
			strcpy(buf + i, "&lt;");
			i += 4;
		} else if (*s == '>') {
			strcpy(buf + i, "&gt;");
			i += 4;
		} else {
			buf[i] = *s;
			i++;
		}
		s++;
	}
	buf[i] = 0;
	return buf;
}

char *
getsenv(char *s)
{
	char *t = getenv(s);
	if (t)
		return t;
	return "";
}

int
http_quit()
{
	if (incgiloop)
		longjmp(cgi_start, 1);
	else
		exit(3);
}

void
http_fatal(char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023] = 0;
	html_header(1);
	printf("<br>\xB4\xED\xCE\xF3! %s! <br><br>\n", buf); // 错误
	fputs("<a href=javascript:history.go(-1)>\xBF\xEC\xCB\xD9\xB7\xB5\xBB\xD8</a>", stdout); // 快速返回
	http_quit();
}

void
strnncpy(char *s, int *l, char *s2)
{
	int l2 = strlen(s2);
	memcpy(s + (*l), s2, l2);
	(*l) += l2;
}

void
strnncpy2(char *s, int *l, char *s2, int len)
{
	memcpy(s + (*l), s2, len);
	(*l) += len;
}

static int istitle = 0;

/*modify by macintosh 050619 for Tex Math Equ*/
/*modify by macintosh 051215 for highlight & background color*/
static void hsprintf(char *s, char *s0) {
	char ansibuf[80], buf2[80];
	char *tmp;
	int c, bold, m, i, len, lsp = -1;
	len = 0;
	bold = 0;
	for (i = 0; (c = s0[i]); i++) {
		switch (c) {
		case '\\':
			if (quote_quote)
				strnncpy2(s, &len, "\\\\", 2);
			else
				s[len++] = c;
			break;
		case '$':
			/*errlog("usingMath=%d withinMath=%d", usingMath, withinMath);*/
			if (usingMath && !withinMath) {
				strnncpy2(s, &len, "<span class=math>", 17);
				withinMath = 1;
			} else if (usingMath && withinMath == 1) {
				strnncpy2(s, &len, "</span>", 7);
				withinMath = 0;
			} else
				s[len++] = c;
			break;
		case '"':
			if (usingMath && !withinMath && s0[i + 1] == '[') {
				strnncpy2(s, &len, "<div class=math>", 16);
				i++;
				withinMath = 2;
			} else if (usingMath && withinMath == 2 && s0[i + 1] == ']') {
				strnncpy2(s, &len, "</div>", 6);
				i++;
				withinMath = 0;
			} else if (usingMath && s0[i + 1] == '$') {
				s[len++] = '$';
				i++;
			} else if (quote_quote)
				strnncpy2(s, &len, "\\\"", 2);
			else
				s[len++] = c;
			break;
		case '&':
			strnncpy2(s, &len, "&amp;", 5);
			break;
		case '<':
			strnncpy2(s, &len, "&lt;", 4);
			break;
		case '>':
			strnncpy2(s, &len, "&gt;", 4);
			break;
		case ' ':
			if (lsp != i - 1) {
				s[len++] = c;
				lsp = i;
			} else {
				strnncpy2(s, &len, "&nbsp;", 6);
			}
			break;
		case '\r':
			break;
		case '\n':
			if (withinMath) {
				s[len++]=' ';
				break;
			}
			if (quote_quote)
				strnncpy2(s, &len, " \\\n<br>", 7);
			else if (!istitle)
				strnncpy2(s, &len, "\n<br>", 5);
			break;
		case '\033':
			if (s0[i + 1] != '[')
				continue;
			for (m = i + 2; s0[m] && m < i + 24; m++)
				if (strchr("0123456789;", s0[m]) == 0)
					break;
			ytht_strsncpy(ansibuf, &s0[i + 2], m - (i + 2) + 1);
			i = m;
			if (s0[i] != 'm')
				continue;
			if (strlen(ansibuf) == 0) {
				bold = 0;
				strnncpy2(s, &len, "</font><font class=b40><font class=c37>", 39);//23
			}
			tmp = strtok(ansibuf, ";");
			while (tmp) {
				c = atoi(tmp);
				tmp = strtok(0, ";");
				if (c == 1)
					bold = 1;
				if (c == 0) {
					strnncpy2(s, &len, "</font><font class=b40><font class=c37>", 39);//23
					bold = 0;
				}
				if (c >= 30 && c <= 37) {
					if (bold == 1) {
						sprintf(buf2, "<font class=h%d>", c);//</font>
						strnncpy2(s, &len, buf2, 16);//23
					}
					if (bold == 0) {
						sprintf(buf2, "<font class=c%d>", c);//</font>
						strnncpy2(s, &len, buf2, 16);//23
					}
				}
				if (c >= 40 && c <= 47){
					sprintf(buf2, "<font class=b%d>", c);//</font>
					strnncpy2(s, &len, buf2, 16);//23
				}
			}
			break;
		default:
			s[len++] = c;
		}
	}
	s[len] = 0;
}

char *
titlestr(char *str)
{
	static char buf[8096];
	istitle = 1;
	hsprintf(buf, str);
	istitle = 0;
	return buf;
}

int
hprintf(char *fmt, ...)
{
	static char buf[32768], buf2[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf2, 1023, fmt, ap);
	va_end(ap);
	buf2[1023] = 0;
	hsprintf(buf, buf2);
	fputs(buf, stdout);
	return 0;
}

void
fqhprintf(FILE * output, char *str)
{
	static char buf[8096];
	hsprintf(buf, str);
	fputs(buf, output);
}

int fhhprintf(FILE * output, char *fmt, ...) {
	char buf0[1024], buf[1024], *s;
	int len = 0;
	char vlink[STRLEN];
	char vfile[STRLEN];
	char cmdline[STRLEN];
	FILE* vfp;

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023] = 0;
	s = buf;
	if (w_info->link_mode) {
		fqhprintf(output, buf);
		return 0;
	}
	if ((s[0] && s[1] && s[2] && !strchr(s + 3, ':')) || memchr(s, 0, 10)) {
		fqhprintf(output, buf);
		return 0;
	}
	if (!strcasestr(s, "http://") && !strcasestr(s, "ftp://")
			&& !strcasestr(s, "mailto:") && !strcasestr(s, "#video")) {
		fqhprintf(output, buf);
		return 0;
	}
	while (s[0]) {
		if (!strncasecmp(s, "http://", 7) || !strncasecmp(s, "mailto:", 7) || !strncasecmp(s, "ftp://", 6)) {
			char *tmp, *noh, tmpchar;
			if (len > 0) {
				buf0[len] = 0;
				fqhprintf(output, buf0);
				len = 0;
			}
			tmp = s;
			while (*s && !strchr("<>\'\" \r\t)(,;\n", *s))
				s++;
			tmpchar = *s;
			*s = 0;
			if (1) {
				if (!strcasecmp(s - 4, ".gif")
						|| !strcasecmp(s - 4, ".jpg")
						|| !strcasecmp(s - 4, ".bmp")
						|| !strcasecmp(s - 4, ".png")
						|| !strcasecmp(s - 5, ".jpeg")) {
					fprintf(output,
						"<a href='%s'> "
						"<IMG style=\" max-width:800px; width: expression(this.width > 800 ? 800: true); height:auto\" SRC='%s' border=0/> </a>",
						nohtml(tmp), nohtml(tmp));
					*s = tmpchar;
					continue;
				}
				if (!strcasecmp(s - 4, ".swf")) {
					fprintf(output,
						"<a href='%s'>%s</a> <br>"
						"<OBJECT><PARAM NAME='MOVIE' VALUE='%s' >"
						"<EMBED SRC='%s' width=480 height=360></EMBED></OBJECT>",
					nohtml(tmp), nohtml(tmp), nohtml(tmp), nohtml(tmp));
					*s = tmpchar;
					continue;
				}
			}
			noh = nohtml(tmp);
			fprintf(output, "<a target=_blank href='%s'>%s</a>", noh, noh);
			*s = tmpchar;
			continue;
		}
		/*
		else if (!strncasecmp(s, "#video", 6)) {
			char *tmp, *noh, tmpchar;
			char fbuf[0x4000];
			size_t flen;
			int i;
			if (len > 0) {
				buf0[len] = 0;
				fqhprintf(output, buf0);
				len = 0;
			}
			if (1) {
				if (!strncasecmp(s+7, "http://v.youku.com"), 18) {
					strcpy(vlink, s+7);
					sethomefile(vfile, currentuser.userid, "vfile");
					sprintf(cmdline, "wget -o %s %s", vfile, vlink);
					system(cmdline);
					fopen(vfile, "r");
					memset(fbuf, 0, 0x4000);
					flen=fread(fbuf, sizeof(char), 0x4000, vfp);
					for (i=0; i<flen; ++i) {
						i
					fprintf(output,
						"<a href='%s'> "
						"<IMG style=\" max-width:800px; height:auto\" SRC='%s' border=0/> </a>",
						nohtml(tmp), nohtml(tmp));
					*s = tmpchar;
					fclose(vfp);
					unlink(vfp);
					continue;
				}
			}
			noh = nohtml(tmp);
			fprintf(output, "<a target=_blank href='%s'>%s</a>",
				noh, noh);
			*s = tmpchar;
			continue;
		}
		*/
		else {
			buf0[len] = s[0];
			if (len < 1000)
				len++;
			s++;
		}
	}
	if (len) {
		buf0[len] = 0;
		fqhprintf(output, buf0);
	}
	return 0;
}

char parm_name[256][80], *parm_val[256];
int parm_num = 0;

void
parm_add(char *name, char *val)
{
	int len = strlen(val);
	if (parm_num >= 255)
		http_fatal("too many parms.");
	parm_val[parm_num] = calloc(len + 1, 1);
	if (parm_val[parm_num] == 0)
		http_fatal("memory overflow2 %d %d", len, parm_num);
	ytht_strsncpy(parm_name[parm_num], name, 78);
	ytht_strsncpy(parm_val[parm_num], val, len + 1);
	parm_num++;
}

static char *domainname[] = {
	MY_BBS_DOMAIN,
	"bbs.xjtu.edu.cn",
	"bbs.xanet.edu.cn",
	NULL
};

static char *specname[] = {
	"1999",
	"2001",
	"www",
	"bbs",
	NULL
};

int
isaword(char *dic[], char *buf)
{
	char **a;
	for (a = dic; *a != NULL; a++)
		if (!strcmp(buf, *a))
			return 1;
	return 0;
}

struct wwwsession guest = {
	.used      = 1,
	.t_lines   = 20,
	.link_mode = 0,
	.def_mode  = 0,
	.att_mode  = 0,
	.doc_mode  = 1,
};

int cookie_parse() {
	const char *cookie_str;
	char cookie_buf[128];
	struct bmy_cookie cookie;

	cookie_str = getenv("HTTP_COOKIE");
	if (cookie_str == NULL) {
		goto GUEST;
	} else {
		strncpy(cookie_buf, cookie_str, sizeof(cookie_buf));
		cookie_buf[sizeof(cookie_buf) - 1] = 0;

		memset(&cookie, 0, sizeof(struct bmy_cookie));
		bmy_cookie_parse(cookie_buf, &cookie);
		if (cookie.sessid == NULL)
			goto GUEST;

		loginok = user_init(&currentuser, &u_info, cookie.userid, cookie.sessid);

		if (cookie.extraparam == NULL)
			cookie.extraparam = "";
		extraparam_init(cookie.extraparam);

		if (!loginok) {
			goto GUEST;
		}
	}

	return 0;
GUEST:
	loginok = false;
	isguest = true;
	w_info  = &guest;
	return -1;
}

int
url_parse()
{
	char *url, *end, name[STRLEN], *p, *extraparam;
	// e.g. url = /BMY/foo
	url = getenv("SCRIPT_URL");
	if (NULL == url)
		return -1;
	strcpy(name, "/");
	if (!strncmp(url, "/" SMAGIC, sizeof (SMAGIC))) {
		snprintf(name, STRLEN, "%s", url + sizeof (SMAGIC));
		p = strchr(name, '/');
		if (NULL != p) {
			*p = 0;
			url = strchr(url + 1, '/');
		} else
			return -1;
	}

	if (!strcmp(url, "/") || nologin) {
		strcpy(needcgi, "bbsindex");
		strcpy(rframe, "");
		ytht_strsncpy(name, getsenv("HTTP_HOST"), 70);
		p = strchr(name, '.');
		if (p != NULL && isaword(domainname, p + 1)) {
			*p = 0;
			ytht_strsncpy(rframe, name, 60);
		}
		if (rframe[0] && isaword(specname, rframe))
			rframe[0] = 0;
		return 0;
	}
	snprintf(needcgi, STRLEN, "%s", url + 1);
	if (NULL != (end = strchr(needcgi, '/')))
		*end = 0;
	return 0;
}

static void http_parm_free(void)
{
	int i;
	for (i = 0; i < parm_num; i++)
		free(parm_val[i]);
	parm_num = 0;
}

void http_parm_init(void) {
	char *buf, buf2[1024], *t2, *t3;
	int n;
	http_parm_free();
	n = atoi(getsenv("CONTENT_LENGTH"));
	if (n > 5000000)
		n = 5000000;
	buf = malloc(n + 1);
	if (NULL == buf)
		http_fatal("Out of memory.");
	n = fread(buf, 1, n, stdin);
	buf[n] = 0;
	t2 = strtok(buf, "&");
	while (t2) {
		t3 = strchr(t2, '=');
		if (t3 != 0) {
			t3[0] = 0;
			t3++;
			__unhcode(t3);
			parm_add(ytht_strtrim(t2), t3);
		}
		t2 = strtok(0, "&");
	}
	free(buf);
	ytht_strsncpy(buf2, getsenv("QUERY_STRING"), 1024);
	t2 = strtok(buf2, "&");
	while (t2) {
		t3 = strchr(t2, '=');
		if (t3 != 0) {
			t3[0] = 0;
			t3++;
			__unhcode(t3);
			parm_add(ytht_strtrim(t2), t3);
		}
		t2 = strtok(0, "&");
	}
}

int
cache_header(time_t t, int age)
{
	char *old;
	time_t oldt = 0;
	char buf[STRLEN];
	if (!t)
		return 0;
	old = getenv("HTTP_IF_MODIFIED_SINCE");
	if (old) {
		struct tm tm;
		if (strptime(old, "%a, %d %b %Y %H:%M:%S %Z", &tm))
			oldt = mktime(&tm) - timezone;
		else {
			ytht_strsncpy(buf, old, STRLEN - 10);
			strcat(buf, "\n");
			f_append("bbstmpfs/tmp/not-known-time", buf);
		}
	}
	if (oldt >= t) {
		printf("Status: 304\n");
		printf("Cache-Control: max-age=%d\n\n", age);
		return 1;
	}
	if (strftime(buf, STRLEN, "%a, %d %b %Y %H:%M:%S", gmtime(&t)))
		printf("Last-Modified: %s GMT\n", buf);
	t = now_t + age;
	if (strftime(buf, STRLEN, "%a, %d %b %Y %H:%M:%S", gmtime(&t)))
		printf("Expires: %s GMT\n", buf);
	printf("Cache-Control: max-age=%d\n", age);
	return 0;
}

void
html_header(int mode)
{
	static int hasrun;
	if (!mode) {
		hasrun = 0;
		return;
	}
	if (hasrun) {
		return;
	}
	hasrun = 1;
	if (mode == 100) {
		xml_header();
		return;
	}
	printf("Content-type: text/html; charset=%s\n\n", CHARSET);
	if (mode < 100)
		printf("<!DOCTYPE html>\n<HTML>\n");
	else
		printf("<HTML XMLNS:m=\"http://www.w3.org/1998/Math/MathML\">\n");
	switch (mode) {
	case 1:
	case 101:
	case 11:		//bbsgetmsg
		printf("<head><meta http-equiv='Content-Type' content='text/html; charset=%s'>\n", CHARSET);
		printf("<link rel=stylesheet type=text/css href='%s'></head>\n", currstyle->cssfile);
		//printf("<link href=\"/images/oras.css\" rel='stylesheet' type='text/css'>\n");
		break;
	case 2:
		printf("<head><meta http-equiv=Content-Type content=\"text/html; charset=%s\">\n", CHARSET);
		//add by mintbaggio 040411 for new www
		//printf("<link href=\"/images/oras.css\" rel='stylesheet' type='text/css'>\n");
		/*omit by mintbaggio 040411 for new www*/
		printf("<link rel='stylesheet' type='text/css' href=%s></head>", currstyle->cssfile);
		/*macintosh changed leftcssfile to cssfile 20060112*/
		break;
	case 3:
		//printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
		//break;
	case 4:
		printf("<head><meta http-equiv=Content-Type content=\"text/html; charset=%s\"></head>", CHARSET);
		break;
	default:
		break;
	}
	if (mode == 101)
		printf("<OBJECT ID=MathPlayer CLASSID=\"clsid:32F66A20-7614-11D4-BD11-00104BD3F987\">"
				"</OBJECT><?IMPORT NAMESPACE=\"m\" IMPLEMENTATION=\"#MathPlayer\" ?>");
/*
	if (isguest && reg_req())
		printf("<SCRIPT language=\"JavaScript\">P1 = open('regreq', 'WINregreq', 'width=600,height=460');</SCRIPT > ");
*/
}

void
json_header()
{
	printf("Content-type: application/json; charset=utf-8\n\n");
}

void
xml_header()
{
	printf("Content-type: text/xml; charset=%s\n\n", CHARSET);
	printf("<?xml version=\"1.0\" encoding=\"%s\"?>\n", CHARSET);
	printf("<?xml-stylesheet type=\"text/css\" href=\"%s\"?>\n", currstyle->cssfile);
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\"\n"
			"\"http://www.w3.org/TR/MathML2/dtd/xhtml-math11-f.dtd\">\n");
	printf("<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
			"xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n"
			"xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");
	printf("<head>");
}

static int __to16(char c) {
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0;
}

static void __unhcode(char *s) {
	int m, n;
	for (m = 0, n = 0; s[m]; m++, n++) {
		if (s[m] == '+') {
			s[n] = ' ';
			continue;
		}
		if (s[m] == '%') {
			s[n] = __to16(s[m + 1]) * 16 + __to16(s[m + 2]);
			m += 2;
			continue;
		}
		s[n] = s[m];
	}
	s[n] = 0;
}

char *
getparm(char *var)
{
	int n;
	for (n = 0; n < parm_num; n++)
		if (!strcasecmp(parm_name[n], var))
			return parm_val[n];
	return "";
}

char *
getparm2(char *v1, char *v2)
{
	char *ptr;
	ptr = getparm(v1);
	if (!ptr[0])
		ptr = getparm(v2);
	return ptr;
}

int
shm_init()
{
	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_Board_resolve();

	return 0;
}

int
ummap()
{
	int fd;
	struct stat st;
	if (ummap_ptr)
		munmap(ummap_ptr, ummap_size);
	ummap_ptr = NULL;
	ummap_size = 0;
	fd = open(".PASSWDS", O_RDONLY);
	if (fd < 0)
		exit(7);
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode) || st.st_size <= 0) {
		close(fd);
		exit(8);
	}
	ummap_ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	if (ummap_ptr == (void *) -1)
		exit(9);
	ummap_size = st.st_size;
	return 0;
}

const char *getextrparam_str(unsigned int param) {
	static const char *str_arr[] = {
		"A", "B", "C", "D",
		"E", "F", "G", "H",
		"I"
	};
	return str_arr[param % NWWWSTYLE];
}

int
addextraparam(char *ub, int size, int n, int param)
{
	char *base;
	int len = strlen(ub), i;
	base = strchr(ub, '_');
	if (!base) {
		len = strlen(ub);
		if (len > size - 3)
			return -1;
		base = ub + len;
		*base = '_';
	}
	base++;
	if (base + n + 1 >= ub + size)
		return -1;
	for (i = 0; base[i] && i < n; i++) ;
	for (; i < n; i++)
		base[i] = '_';
	base[i++] = 'A' + param % 26;
	base[i] = 0;
	return 0;
}

static void extraparam_init(const char *extrastr) {
	int i;
	if (*extrastr) {
		i = *extrastr - 'A';
		if (i < 0 || i >= NWWWSTYLE)
			i = 0;
		wwwstylenum = i;
		currstyle = &wwwstyle[i];
		extrastr++;
	} else
		currstyle = &wwwstyle[0];
}

//static int user_init(struct userec *x, struct user_info **y, char *ub) {
static int user_init(struct userec *x, struct user_info **y, const char *userid, const char *sessid) {
	struct userec *x2;
	int i;
	utmpent = 0;
	isguest = 0;
	tempuser = 0;

	i = ythtbbs_session_get_utmp_idx(sessid, userid);
	if (i < 0)
		return 0;

	*y = ythtbbs_cache_utmp_get_by_idx(i);
	if (strcmp((*y)->sessionid, sessid) || strcmp((*y)->userid, userid))
		return 0;

	(*y)->lasttime = now_t;
	if (!strcasecmp((*y)->userid, "guest")) {
		isguest = 1;
		return 0;
	}

	x2 = getuser((*y)->userid);
	if (x2 == 0)
		return 0;

	utmpent = i + 1;
	memcpy(x, x2, sizeof (*x));
	return 1;
}

int
mail_file(char *filename, char *userid, char *title, char *sender)
{
	FILE *fp, *fp2;
	char buf[256], dir[256];
	struct fileheader header;
	int t;
	if (getuser(userid) == NULL)
		return -1;
	bzero(&header, sizeof (header));
	fh_setowner(&header, sender, 0);
	setmailfile_s(buf, sizeof(buf), userid, "");
	mkdir(buf, 0770);
	t = trycreatefile(buf, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	header.thread = t;
	ytht_strsncpy(header.title, title, sizeof(header.title));
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fp2 = fopen(filename, "r");
	if (fp2) {
		while (1) {
			int retv;
			retv = fread(buf, 1, sizeof (buf), fp2);
			if (retv <= 0)
				break;
			fwrite(buf, 1, retv, fp);
		}
		fclose(fp2);
	}
	fclose(fp);
	setmailfile_s(dir, sizeof(dir), userid, ".DIR");
	append_record(dir, &header, sizeof (header));
	return 0;
}

/** 保存邮件到发件箱
 *
 * @param userid 发信人的 ID，用于生成存放信件的路径
 * @param title 标题
 * @param file 文件
 * @param id 收信人，用于 .DIR 文件中标明收信人的 ID，以及 struct fileheader 中
 * @param nickname
 * @param ip
 * @param sig
 * @param mark
 * @return
 */
int
post_mail_to_sent_box(char *userid, char *title, char *file,
		char *id, char *nickname, char *ip, int sig, int mark)
{
	FILE *fp, *fp2;
	char buf[256], dir[256];
	struct fileheader header;
	int t;
	snprintf(buf, sizeof (buf), ".bbs@%s", MY_BBS_DOMAIN);
	if (strstr(userid, buf) || strstr(userid, ".bbs@localhost")) {
		char *pos;
		pos = strchr(userid, '.');
		*pos = '\0';
	}
	bzero(&header, sizeof (header));
	fh_setowner(&header, id, 0);
	setsentmailfile(buf, userid, "");
	t = trycreatefile(buf, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	header.thread = t;
	ytht_strsncpy(header.title, title, sizeof(header.title));
	header.accessed |= mark;
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fp2 = fopen(file, "r");
	fprintf(fp, "\xCA\xD5\xD0\xC5\xC8\xCB: %s (%s)\n", id, nickname); // 收信人
	fprintf(fp, "\xB1\xEA  \xCC\xE2: %s\n", title); // 标题
	fprintf(fp, "\xB7\xA2\xD0\xC5\xD5\xBE: %s (%s)\n", BBSNAME, ytht_ctime(now_t)); // 发信站
	fprintf(fp, "\xC0\xB4  \xD4\xB4: %s\n\n", ip); // 来源
	if (fp2) {
		while (1) {
			int retv;
			retv = fread(buf, 1, sizeof (buf), fp2);
			if (retv <= 0)
				break;
			fwrite(buf, 1, retv, fp);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "\n\n\033[1;%dm\xA1\xF9 \xC0\xB4\xD4\xB4:\xA3\xAE%s %s [FROM: %.40s]\033[m\n",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	setsentmailfile(dir, userid, ".DIR");
	append_record(dir, &header, sizeof (header));
	return 0;
}

int
post_mail(char *userid, char *title, char *file, char *id,
		char *nickname, char *ip, int sig, int mark)
{
	FILE *fp, *fp2;
	char buf[256], dir[256];
	struct fileheader header;
	int t;
	snprintf(buf, sizeof (buf), ".bbs@%s", MY_BBS_DOMAIN);
	if (strstr(userid, buf) || strstr(userid, ".bbs@localhost")) {
		char *pos;
		pos = strchr(userid, '.');
		*pos = '\0';
	}
	if (strstr(userid, "@"))
		return post_imail(userid, title, file, id, nickname, ip, sig);
	if (getuser(userid) == NULL)
		http_fatal("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xD5\xD0\xC5\xC8\xCB\xB5\xD8\xD6\xB7"); // 错误的收信人地址
	bzero(&header, sizeof (header));
	fh_setowner(&header, id, 0);
	setmailfile_s(buf, sizeof(buf), userid, "");
	t = trycreatefile(buf, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	header.thread = t;
	ytht_strsncpy(header.title, title, sizeof(header.title));
	header.accessed |= mark;
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fp2 = fopen(file, "r");
	fprintf(fp, "\xBC\xC4\xD0\xC5\xC8\xCB: %s (%s)\n", id, nickname); // 寄信人
	fprintf(fp, "\xB1\xEA  \xCC\xE2: %s\n", title); // 标题
	fprintf(fp, "\xB7\xA2\xD0\xC5\xD5\xBE: %s (%s)\n", BBSNAME, ytht_ctime(now_t)); // 发信站
	fprintf(fp, "\xC0\xB4  \xD4\xB4: %s\n\n", ip);  // 来源
	if (fp2) {
		while (1) {
			int retv;
			retv = fread(buf, 1, sizeof (buf), fp2);
			if (retv <= 0)
				break;
			fwrite(buf, 1, retv, fp);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "\n\n\033[1;%dm\xA1\xF9 \xC0\xB4\xD4\xB4:\xA3\xAE%s %s [FROM: %.40s]\033[m\n",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	setmailfile_s(dir, sizeof(dir), userid, ".DIR");
	append_record(dir, &header, sizeof (header));
	return 0;
}

int
post_mail_buf(char *userid, char *title, char *buf, char *id, char *nickname,
		char *ip, int sig, int mark)
{
	char path[80];
	FILE *fp;
	sprintf(path, "bbstmpfs/tmp/edit.%d.tmp", thispid);
	fp = fopen(path, "w");
	fprintf(fp, "%s", buf);
	fclose(fp);
	post_mail(userid, title, path, id, nickname, ip, sig, mark);
	unlink(path);
	return 0;
}

static int post_imail(char *userid, char *title, char *file, char *id,
		char *nickname, char *ip, int sig)
{
	FILE *fp1, *fp2;
	char buf[256], *ptr;
	size_t len;

	if (strlen(userid) > 100)
		http_fatal("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xD5\xD0\xC5\xC8\xCB\xB5\xD8\xD6\xB7"); // 错误的收信人地址
	for (ptr = userid; *ptr; ptr++) {
		if (strchr(";~-|`!$#%%&^*()'\"<>?/ ", *ptr) || !isprint(*ptr))
			http_fatal("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xD5\xD0\xC5\xC8\xCB\xB5\xD8\xD6\xB7"); // 错误的收信人地址
	}
	sprintf(buf, "/usr/lib/sendmail -f %s.bbs@%s '%s'", id, BBSHOST,
		userid);
	fp2 = popen(buf, "w");
	if (NULL == fp2)
		return -1;
	fp1 = fopen(file, "r");
	if (NULL == fp1) {
		pclose(fp2);
		return -1;
	}
	fprintf(fp2, "From: %s.bbs@%s\n", id, BBSHOST);
	fprintf(fp2, "To: %s\n", userid);
	fprintf(fp2, "Subject: %s\n\n", title);

	while (fgets(buf, sizeof (buf), fp1) != NULL) {
		if (NULL != (ptr = checkbinaryattach(buf, fp1, &len))) {
			uuencode(fp1, fp2, len, ptr);
			continue;
		}
		if (buf[0] == '.' && buf[1] == '\n')
			fputs(". \n", fp2);
		else
			fputs(buf, fp2);
	}

	fputs("\n--\n", fp2);
	sig_append(fp2, id, sig);
	fprintf(fp2, "\n\n\033[1;%dm\xA1\xF9 \xC0\xB4\xD4\xB4:\xA3\xAE%s %s [FROM: %.40s]\033[m\n",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fprintf(fp2, ".\n");
	fclose(fp1);
	pclose(fp2);
	snprintf(buf, sizeof (buf), "%s mail %s", currentuser.userid, userid);
	newtrace(buf);
	return 0;
}

int
post_article_1984(char *board, char *title, char *file, char *id,
		char *nickname, char *ip, int sig, int mark,
		int outgoing, char *realauthor, int thread)
{
	FILE *fp, *fp2;
	char buf3[1024], buf[80];
	struct fileheader header;
	int t;
	struct tm *n;
	n = localtime(&now_t);
	sprintf(buf, "boards/.1984/%04d%02d%02d", n->tm_year + 1900,
		n->tm_mon + 1, n->tm_mday);
	if (!file_isdir(buf))
		if (mkdir(buf, 0770) < 0)
			return -1;

	bzero(&header, sizeof (header));
	fh_setowner(&header, id, 0);
	strcpy(buf3, buf);
	t = trycreatefile(buf3, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	if (thread != -1)
		header.thread = thread;
	ytht_strsncpy(header.title, title, sizeof(header.title));
	fp = fopen(buf3, "w");
	if (NULL == fp)
		return -1;
	fp2 = fopen(file, "r");
	// 发信人 信区 标题 发信站
	// 转信 本站
	fprintf(fp,
			"\xB7\xA2\xD0\xC5\xC8\xCB: %s (%s), \xD0\xC5\xC7\xF8: %s\n\xB1\xEA  \xCC\xE2: %s\n\xB7\xA2\xD0\xC5\xD5\xBE: %s (%24.24s), %s)\n\n",
			id, nickname, board, title, BBSNAME, ytht_ctime(now_t),
		outgoing ? "\xD7\xAA\xD0\xC5(" MY_BBS_DOMAIN : "\xB1\xBE\xD5\xBE(" MY_BBS_DOMAIN);
	if (fp2 != 0) {
		while (1) {
			int retv;
			retv = fread(buf3, 1, sizeof (buf3), fp2);
			if (retv <= 0)
				break;
			fwrite(buf3, 1, retv, fp);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "\n\033[1;%dm\xA1\xF9 \xC0\xB4\xD4\xB4:\xA3\xAE%s %s [FROM: %.40s]\033[m\n",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	sprintf(buf3, "%s/M.%d.A", buf, t);
	header.sizebyte = ytht_num2byte(eff_size(buf3));
	sprintf(buf3, "%s/.DIR", buf);
	append_record(buf3, &header, sizeof (header));
	return t;
}

int
post_article(char *board, char *title, char *file, char *id,
		char *nickname, char *ip, int sig, int mark,
		int outgoing, char *realauthor, int thread)
{
	FILE *fp, *fp2;
	char buf3[1024];
	struct fileheader header;
	int t;
	bzero(&header, sizeof (header));
	if (strcasecmp(id, "Anonymous"))
		fh_setowner(&header, id, 0);
	else
		fh_setowner(&header, realauthor, 1);
	setbfile(buf3, board, "");
	t = trycreatefile(buf3, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	ytht_strsncpy(header.title, title, sizeof(header.title));
	header.accessed |= mark;
	if (outgoing)
		header.accessed |= FH_INND;
	fp = fopen(buf3, "w");
	if (NULL == fp)
		return -1;
	fp2 = fopen(file, "r");
	// 发信人 信区 标题 发信站
	// 转信 本站
	fprintf(fp,
			"\xB7\xA2\xD0\xC5\xC8\xCB: %s (%s), \xD0\xC5\xC7\xF8: %s\n\xB1\xEA  \xCC\xE2: %s\n\xB7\xA2\xD0\xC5\xD5\xBE: %s (%24.24s), %s)\n\n",
			id, nickname, board, title, BBSNAME, ytht_ctime(now_t),
		outgoing ? "\xD7\xAA\xD0\xC5(" MY_BBS_DOMAIN : "\xB1\xBE\xD5\xBE(" MY_BBS_DOMAIN);
	if (fp2 != 0) {
		while (1) {
			int retv;
			retv = fread(buf3, 1, sizeof (buf3), fp2);
			if (retv <= 0)
				break;
			fwrite(buf3, 1, retv, fp);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "\033[1;%dm\xA1\xF9 \xC0\xB4\xD4\xB4:\xA3\xAE%s %s [FROM: %.40s]\033[m",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	sprintf(buf3, "boards/%s/M.%d.A", board, t);
	header.sizebyte = ytht_num2byte(eff_size(buf3));
	if (thread == -1)
		header.thread = header.filetime;
	else
		header.thread = thread;
	setbfile(buf3, board, ".DIR");
	append_record(buf3, &header, sizeof (header));
	if (outgoing)
		outgo_post(&header, board, id, nickname);
	updatelastpost(board);
	return t;
}

int
securityreport(char *title, char *content)
{
	char fname[STRLEN];
	FILE *se;
	sprintf(fname, "bbstmpfs/tmp/security.%s.%05d", currentuser.userid, getpid());
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "\xCF\xB5\xCD\xB3\xB0\xB2\xC8\xAB\xBC\xC7\xC2\xBC\xCF\xB5\xCD\xB3\n\xD4\xAD\xD2\xF2: \n%s\n", content); // 系统安全记录系统 原因
		fprintf(se, "\xD2\xD4\xCF\xC2\xCA\xC7\xB2\xBF\xB7\xD6\xB8\xF6\xC8\xCB\xD7\xCA\xC1\xCF\n"); // 以下是部分个人资料
		fprintf(se, "\xD7\xEE\xBD\xFC\xB9\xE2\xC1\xD9\xBB\xFA\xC6\xF7: %s", fromhost); // 最近光临机器
		fclose(se);
		post_article("syssecurity", title, fname, currentuser.userid,
				currentuser.username, fromhost, -1, 0, 0,
				currentuser.userid, -1);
		unlink(fname);
	}
	return 0;
}

static void sig_append(FILE * fp, char *id, int sig) {
	FILE *fp2;
	char path[256];
	char buf[256];
	int total, hasnl = 1, i, emptyline = 0, sigln, numofsig;
	if (HAS_PERM(PERM_DENYSIG, currentuser))
		return;
	if (sig < -2 || sig > 10)
		return;
	sethomefile_s(path, sizeof(path), id, "signatures");
	sigln = countln(path);
	numofsig = (sigln + MAXSIGLINES - 1) / MAXSIGLINES;
	if (sig==-2) {
		sig=rand()%numofsig;
	}
	if (sig==-1) {
		return;
	}
	fp2 = fopen(path, "r");
	if (fp2 == 0)
		return;
	for (total = 0; total < 100; total++) {
		if (fgets(buf, 255, fp2) == 0)
			break;
		if (total < sig * 6)
			continue;
		if (total >= (sig + 1) * 6)	// || buf[0] == '\r' || buf[0] == '\n')
			break;
		if (buf[0] == '\r' || buf[0] == '\n') {
			emptyline++;
			continue;
		}
		for (i = 0; i < emptyline; i++)
			fputs("\n", fp);
		emptyline = 0;
		fputs(buf, fp);
		hasnl = (strchr(buf, '\n') == NULL) ? 0 : 1;
	}
	fclose(fp2);
	if (!hasnl)
		fputs("\n", fp);
}

#if defined(ENABLE_GHTHASH) && defined(ENABLE_FASTCGI)
char *
anno_path_of(char *board)
{
	static char annpath[MAXBOARD][80];
	int num;
	static time_t uptime = 0;
	static ght_hash_table_t *p_table = NULL;
	int j;
	char buf1[80], *ptr;
	if (p_table && shm_bcache->uptime > uptime) {
		ght_finalize(p_table);
		p_table = NULL;
	}
	if (p_table == NULL) {
		FILE *fp;
		char buf2[80];
		uptime = now_t;
		p_table = ght_create(MAXBOARD, NULL, 0);
		fp = fopen("0Announce/.Search", "r");
		if (fp == 0)
			return "";
		num = 0;
		while (num < MAXBOARD && fscanf(fp, "%s %s", buf1, buf2) > 0) {
			buf1[79] = 0;
			buf1[strlen(buf1) - 1] = 0;
			for (j = 0; buf1[j]; j++)
				buf1[j] = toupper(buf1[j]);
			sprintf(annpath[num], "/%s", buf2);
			ght_insert(p_table, annpath[num], j, buf1);
			num++;
		}
		fclose(fp);
	}
	strsncpy(buf1, board, sizeof (buf1));
	for (j = 0; buf1[j]; j++)
		buf1[j] = toupper(buf1[j]);
	ptr = ght_get(p_table, j, buf1);
	if (ptr)
		return ptr;
	return "";
}
#else
char *
anno_path_of(char *board)
{
	FILE *fp;
	static char buf[256], buf1[80], buf2[80];
	fp = fopen("0Announce/.Search", "r");
	if (fp == 0)
		return "";
	while (1) {
		if (fscanf(fp, "%s %s", buf1, buf2) <= 0)
			break;
		buf1[79] = 0;
		buf1[strlen(buf1) - 1] = 0;
		if (!strcasecmp(buf1, board)) {
			sprintf(buf, "/%s", buf2);
			fclose(fp);
			return buf;
		}
	}
	fclose(fp);
	return "";
}
#endif
int
has_BM_perm(struct userec *user, struct boardmem *x)
{
	if (user_perm(user, PERM_BLEVELS))
		return 1;
	if (!user_perm(user, PERM_BOARDS))
		return 0;
	if (x == 0)
		return 0;
	return chk_BM(user, &(x->header), 0);
}

int
has_read_perm(struct userec *user, char *board)
{
	return has_read_perm_x(user, getbcache(board));
}

int
has_read_perm_x(struct userec *user, struct boardmem *x)
{
	/* 版面不存在返回0, p和z版面返回1, 有权限版面返回1. */
//	char fn[256];

	if (x == 0)
		return 0;
	if (x->header.clubnum != 0) {
		if ((x->header.flag & CLUBTYPE_FLAG))
			return 1;
		if (!loginok && !tempuser)
			return 0;
		//sprintf(fn, "boards/%s/club_users", x->header.filename);
		//return file_has_word(fn, user->userid);			new from ytht cvs
		return u_info->clubrights[(x->header.clubnum) / 32] & (1 << ((x->header.clubnum) % 32));
	}
	if (x->header.level == 0)
		return 1;
	if (x->header.level & (PERM_POSTMASK | PERM_NOZAP))
		return 1;
	if (!user_perm(user, PERM_BASIC))
		return 0;
	if (user_perm(user, x->header.level))
		return 1;
	return 0;
}

int
hideboard(char *bname)
{
	struct boardmem *x;
	if (bname[0] <= 32)
		return 1;
	x = getbcache(bname);
	if (x == 0)
		return 0;
	return hideboard_x(x);
}

int
hideboard_x(struct boardmem *x)
{
	if (x->header.clubnum != 0)
		return (!(x->header.flag & CLUBTYPE_FLAG));
	if (x->header.level & PERM_NOZAP)
		return 0;
	return (x->header.level & PERM_POSTMASK) ? 0 : x->header.level;
}

int
innd_board(char *bname)
{
	struct boardmem *x;
	x = getbcache(bname);
	if (x == 0)
		return 0;
	return (x->header.flag & INNBBSD_FLAG);
}

int
political_board(char *bname)
{
	struct boardmem *x;
	x = getbcache(bname);
	if (x == 0)
		return 0;
	if (x->header.flag & POLITICAL_FLAG)
		return 1;
	else
		return 0;
}

int
anony_board(char *bname)
{
	struct boardmem *x;
	x = getbcache(bname);
	if (x == 0)
		return 0;
	return (x->header.flag & ANONY_FLAG);
}

int
noadm4political(bname)
char *bname;
{
	time_t t = ythtbbs_cache_utmp_get_watchman();
	return (!t || now_t < t) ? 0 : political_board(bname);
}

int
has_post_perm(struct userec *user, struct boardmem *x)
{
	char buf3[256];
	if (!loginok || isguest || !x || !has_read_perm_x(user, x))
		return 0;

	sprintf(buf3, "boards/%s/deny_users", x->header.filename);
	if (ytht_file_has_word(buf3, user->userid))
		return 0;
	sprintf(buf3, "boards/%s/deny_anony", x->header.filename);
	if (ytht_file_has_word(buf3, user->userid))
		return 0;
	if (!strcasecmp(x->header.filename, "sysop"))
		return 1;
	if (!strcasecmp(x->header.filename, "Freshman"))
		return 1;
	if (user_perm(user, PERM_SYSOP))
		return 1;
	if (!user_perm(user, PERM_BASIC))
		return 0;
	if (!user_perm(user, PERM_POST))
		return 0;
	if (!strcasecmp(x->header.filename, "Appeal"))
		return 1;
	if (!strcasecmp(x->header.filename, "committee"))
		return 1;
	if (ytht_file_has_word("deny_users", user->userid))
		return 0;
	if (x->header.clubnum != 0) {
		if (!(x->header.level & PERM_NOZAP) && x->header.level && !user_perm(user, x->header.level))
			return 0;
		return u_info->clubrights[(x->header.clubnum) / 32] & (1 << ((x->header.clubnum) % 32));
	}
	if (!(x->header.level & PERM_NOZAP) && x->header.level && !user_perm(user, x->header.level))
		return 0;
	return 1;
}

int
has_vote_perm(struct userec *user, struct boardmem *x)
{
	char buf3[256];
	if (!loginok || isguest || !x || !has_read_perm_x(user, x))
		return 0;
	if (!strcasecmp(x->header.filename, "sysop"))
		return 1;
	if (user_perm(user, PERM_SYSOP))
		return 1;
	if (!user_perm(user, PERM_BASIC))
		return 0;
	if (!user_perm(user, PERM_POST))
		return 0;
	if (x->header.clubnum != 0) {
		sprintf(buf3, "boards/%s/club_users", x->header.filename);
		if (ytht_file_has_word(buf3, user->userid))
			return 1;
		else
			return 0;
	}
	if (!(x->header.level & PERM_NOZAP) && x->header.level && !user_perm(user, x->header.level))
		return 0;
	return 1;
}

#if defined(ENABLE_GHTHASH) && defined(ENABLE_FASTCGI)
struct boardmem *
getbcache(char *board)
{
	int i, j;
	static int num = 0;
	char upperstr[STRLEN];
	static ght_hash_table_t *p_table = NULL;
	static time_t uptime = 0;
	if (board[0] == 0)
		return 0;
	if (p_table && (num != shm_bcache->number || shm_bcache->uptime > uptime)) {
		//errlog("getbcache: num %d, bcache->number %d, would reload",
		//       num, shm_bcache->number);
		ght_finalize(p_table);
		p_table = NULL;
	}
	if (p_table == NULL) {
		num = 0;
		p_table = ght_create(MAXBOARD, NULL, 0);
		for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
			num++;
			if (!shm_bcache->bcache[i].header.filename[0])
				continue;
			strsncpy(upperstr, shm_bcache->bcache[i].header.filename, sizeof (upperstr));
			for (j = 0; upperstr[j]; j++)
				upperstr[j] = toupper(upperstr[j]);
			ght_insert(p_table, &shm_bcache->bcache[i], j, upperstr);
		}
		uptime = now_t;
	}
	strsncpy(upperstr, board, sizeof (upperstr));
	for (j = 0; upperstr[j]; j++)
		upperstr[j] = toupper(upperstr[j]);
	return ght_get(p_table, j, upperstr);
}
#else
struct boardmem *
getbcache(char *board)
{
	int i;
	if (board[0] == 0)
		return 0;
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++)
	{
		//printf("board:%s, header:%s\n", board, shm_bcache->bcache[i].header.filename);   add by mint
		if (!strcasecmp(board, shm_bcache->bcache[i].header.filename))
			return &shm_bcache->bcache[i];
	}
	//modified by safari 20100102
	//printf("end");
	return 0;
}
#endif

/**
 * 依据版面名称获取 boardmem 对象，应逐渐采用 libythtbbs 库函数。
 * @see struct boardmem *getboardbyname(char *board_name)
 */
struct boardmem *
getboard(char board[80])
{
	struct boardmem *x1;
	x1 = getbcache(board);
	if (x1 == 0)
		return NULL;
	if (!has_read_perm_x(&currentuser, x1))
		return NULL;
	strcpy(board, x1->header.filename);
	return x1;
}

int
send_msg(char *myuserid, int i, char *touserid, int topid, char *msg, int offline)
{
	struct msghead head, head2;
	head.time = now_t;
	head.sent = 0;
	head.mode = 5;		//mode 5 for www msg
	strncpy(head.id, currentuser.userid, IDLEN + 2);
	head.frompid = 1;
	head.topid = topid;
	memcpy(&head2, &head, sizeof (struct msghead));
	head2.sent = 1;
	strncpy(head2.id, touserid, IDLEN + 2);

	if (save_msgtext(touserid, &head, msg) < 0)
		return -2;
	if (save_msgtext(currentuser.userid, &head2, msg) < 0)
		return -2;
	if (offline)
		return 1;
	if (topid != 1)
		kill(topid, SIGUSR2);
	else
		shm_utmp->uinfo[i].unreadmsg++;
	return 1;
}

int count_life_value(struct userec *urec)
{
	int i, res;
//	i = (now_t - urec->lastlogin) / 60;
	if ((urec->userlevel & PERM_XEMPT) || !strcasecmp(urec->userid, "guest"))
		return 999;
	i = (now_t - urec->lastlogin) / 60;

	/* new user should register in 30 mins */
	if (strcmp(urec->userid, "new") == 0) {
		return (30 - i) * 60;
	}
	if (urec->numlogins <= 1)
		return (60 * 1440 - i) / 1440;
	if (!(urec->userlevel & PERM_LOGINOK))
		return (60 * 1440 - i) / 1440;
	if (((time(0)-urec->firstlogin)/86400)>365*8)
		return  888;
	if (((time(0)-urec->firstlogin)/86400)>365*5)
		return  666;
	if (((time(0)-urec->firstlogin)/86400)>365*2)
		return  365;

	res=(120 * 1440 - i) / 1440 + urec->numdays;
	if (res>364) res=364;
	return res;
}

int
save_user_data(struct userec *x)
{
	int n, fd;
	n = getusernum(x->userid);
	if (n < 0 || n > 1000000)
		return 0;
	fd = open(".PASSWDS", O_WRONLY);
	if (fd < 0)
		return 0;
	if (lseek(fd, n * sizeof (struct userec), SEEK_SET) < 0) {
		close(fd);
		return 0;
	}
	write(fd, x, sizeof (struct userec));
	close(fd);
	return 1;
}

int
user_perm(struct userec *x, int level)
{
	return (x->userlevel & level);
}

static int useridhash(char *id) {
	int n1 = 0;
	int n2 = 0;
	while (*id) {
		n1 += ((unsigned char) toupper(*id)) % 26;
		id++;
		if (!*id)
			break;
		n2 += ((unsigned char) toupper(*id)) % 26;
		id++;
	}
	n1 %= 26;
	n2 %= 26;
	return n1 * 26 + n2;
}

int
insertuseridhash(struct useridhashitem *ptr, int size, char *userid, int num)
{
	int h, s, i, j = 0;
	if (!*userid)
		return -1;
	h = useridhash(userid);
	s = size / 26 / 26;
	i = h * s;
	while (j < s * 5 && ptr[i].num > 0 && ptr[i].num != num) {
		i++;
		if (i >= size)
			i %= size;
	}
	if (j == s * 5)
		return -1;
	ptr[i].num = num;
	strcpy(ptr[i].userid, userid);
	return 0;
}

int
finduseridhash(struct useridhashitem *ptr, int size, char *userid)
{
	int h, s, i, j;
	h = useridhash(userid);
	s = size / 26 / 26;
	i = h * s;
	for (j = 0; j < s * 5; j++) {
		if (!strcasecmp(ptr[i].userid, userid))
			break;
		i++;
		if (i >= size)
			i %= size;
	}
	if (j == s * 5)
		return -1;
	return ptr[i].num;
}

// 返回索引值，原本实现中会判断 shm_ucache->userid[i]
int getusernum(char *id) {
	int i;
	if (id[0] == 0 || strchr(id, '.'))
		return -1;
	i = finduseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE, id) - 1;
	if (i >= 0 && !strcasecmp(shm_ucache->userid[i], id))
		return i;
	for (i = 0; i < MAXUSERS; i++) {
		if (!strcasecmp(shm_ucache->userid[i], id))
			return i;
	}
	return -1;
}

struct userec *
getuser(char *id)
{
	static struct userec userec1;
	int uid;
	uid = getusernum(id);
	if (uid < 0)
		return NULL;
	if ((uid + 1) * sizeof (struct userec) > ummap_size)
		ummap();
	if (!ummap_ptr)
		return NULL;
	memcpy(&userec1, ummap_ptr + sizeof (struct userec) * uid, sizeof (userec1));
	return &userec1;
}

// 对 ythtbbs_cache_utmp 的更新，使用接口取代原实现
int count_online() {
	return ythtbbs_cache_utmp_count_active();
}

struct ythtbbs_override fff[MAXFRIENDS];
struct ythtbbs_override bbb[MAXREJECTS];
size_t friendnum = 0;
int badnum = 0;

int
changemode(int mode)
{
	if (!loginok)
		return 1;
	u_info->mode = mode;
	if (mode != BACKNUMBER && mode != READING && mode != POSTING)
		updatelastboard();
	return 0;
}

char *
encode_url(char *s)
{
	int i, j, half = 0;
	static char buf[512];
	char a[3];
	j = 0;
	for (i = 0; s[i]; i++) {
		if ((!half && strchr("~`!@#$%%^&*()-_=+[{]}\\|;:'\",<.>/? ", s[i])) || (s[i + 1] == 0 && !half && s[i] < 0)) {
			buf[j++] = '%';
			sprintf(a, "%02X", s[i]);
			buf[j++] = a[0];
			buf[j++] = a[1];
		} else
			buf[j++] = s[i];
		if (half)
			half = 0;
		else if (s[i] < 0)
			half = 1;
	}
	buf[j] = 0;
	return buf;
}

char *
noquote_html(char *s)
{
	int i, j;
	static char buf[512];
	j = 0;
	for (i = 0; s[i]; i++) {
		if (s[i] != '\'')
			buf[j++] = s[i];
		else {
			buf[j++] = '&';
			buf[j++] = '#';
			buf[j++] = '3';
			buf[j++] = '9';
			buf[j++] = ';';
		}
	}
	buf[j] = 0;
	return buf;
}

char *
void1(char *s)
{
	int i;
	int flag = 0;
	for (i = 0; s[i]; i++) {
		if (flag == 0) {
			if (s[i] < 0)
				flag = 1;
			continue;
		}
		flag = 0;
		if (s[i] < 32 && s[i] > 0)
			s[i - 1] = 32;
	}
	if (flag)
		s[strlen(s) - 1] = 0;
	return s;
}

char *
flag_str_bm(int access)
{
	static char buf[80];
	strcpy(buf, "  ");
	if (access & FH_DIGEST)
		buf[0] = 'G';
	if (access & FH_MARKED)
		buf[0] = 'M';
	if ((access & FH_MARKED) && (access & FH_DIGEST))
		buf[0] = 'B';
	if (access & FH_ATTACHED)
		buf[1] = '@';
	if (access & FH_DEL)
		buf[0] = 'T';
	//add by bjgyt for top
	if (access & FILE_TOP1)
		buf[0] = '#';
	return buf;
}

char *
flag_str(int access)
{
	static char buf[80];
	strcpy(buf, "  ");
	if (access & FH_DIGEST)
		buf[0] = 'G';
	if (access & FH_MARKED)
		buf[0] = 'M';
	if ((access & FH_MARKED) && (access & FH_DIGEST))
		buf[0] = 'B';
	if (access & FH_ATTACHED)
		buf[1] = '@';
	return buf;
}

char *
flag_str2(int access, int has_read)
{
	static char buf[3];
	buf[0] = 'N';
	buf[1] = 0;
	buf[2] = 0;
	if (access & FH_DIGEST)
		buf[0] = 'G';
	if (access & FH_MARKED)
		buf[0] = 'M';
	if ((access & FH_MARKED) && (access & FH_DIGEST))
		buf[0] = 'B';
	if (access & FH_ATTACHED)
		buf[1] = '@';
	//add by bjgyt for top
	if (access & FILE_TOP1)
		buf[0] = '#';
	//end add
	if (has_read)
		buf[0] = tolower(buf[0]);
	if (buf[0] == 'n') {
		buf[0] = buf[1];	//modify by mintbaggio 040522 for new www
		buf[1] = 0;
	}
	return buf;
}

char *
userid_str(char *s)
{
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	ytht_strsncpy(tmp, s, 255);
	buf[0] = 0;
	ptr = strtok(tmp, " ,();\r\n\t");
	while (ptr && strlen(buf) < 400) {
		if ((ptr2 = strchr(ptr, '.'))) {
			ptr2[1] = 0;
			strcat(buf, ptr);
		} else {
			ptr = nohtml(ptr);
			sprintf(buf2, "<a href=qry?U=%s>%s</a>", ptr, ptr);
			strcat(buf, buf2);
		}
		ptr = strtok(0, " ,();\r\n\t");
		if (ptr)
			strcat(buf, " ");
	}
	return buf;
}

char *
getbfroma(char *path)
{
	static char board[30];
	char *ptr;

	if (*path == '/')
		path++;
	if (strncmp(path, "groups/GROUP_", 13))
		return "";
	ptr = strchr(path + 13, '/');
	if (!ptr)
		return "";
	ytht_strsncpy(board, ptr + 1, sizeof(board));
	ptr = strchr(board, '/');
	if (ptr)
		*ptr = 0;
	return board;
}

int
set_my_cookie()
{
	char buf[256];
	w_info->t_lines = 20;
	if (readuservalue(currentuser.userid, "t_lines", buf, sizeof (buf)) > 0)
		w_info->t_lines = atoi(buf);
	if (readuservalue(currentuser.userid, "link_mode", buf, sizeof (buf)) >= 0)
		w_info->link_mode = atoi(buf);
	if (readuservalue(currentuser.userid, "def_mode", buf, sizeof (buf)) >= 0)
		w_info->def_mode = atoi(buf);
/*
	if (readuservalue(currentuser.userid, "att_mode", buf, sizeof (buf)) >= 0)
		w_info->att_mode = atoi(buf);
*/
	w_info->att_mode = 0;
	w_info->doc_mode = 1;
	if (w_info->t_lines < 10 || w_info->t_lines > 40)
		w_info->t_lines = 20;
	return 0;
}

int
has_fill_form()
{
	FILE *fp;
	int r;
	char userid[256], tmp[256], buf[256];
	fp = fopen("new_register", "r");
	if (fp == 0)
		return 0;
	while (1) {
		if (fgets(buf, 100, fp) == 0)
			break;
		r = sscanf(buf, "%s %s", tmp, userid);
		if (r == 2) {
			if (!strcasecmp(tmp, "userid:") && !strcasecmp(userid, currentuser.userid)) {
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

int
cmpboard(b1, b2)
struct boardmem **b1, **b2;
{
	return strcasecmp((*b1)->header.filename, (*b2)->header.filename);
}

int
cmpboardscore(b1, b2)
struct boardmem **b1, **b2;
{
	return ((*b2)->score - (*b1)->score);
}

int
cmpboardinboard(b1, b2)
struct boardmem **b1, **b2;
{
	return ((*b2)->inboard - (*b1)->inboard);
}

int
cmpuser(a, b)
struct user_info *a, *b;
{
	return strcasecmp(a->userid, b->userid);
}

struct fileheader *
findbarticle(struct mmapfile *mf, char *file, int *num, int mode)
// mode = 1 表示 .DIR 按时间排序  0 表示 不排序
{
	static struct fileheader x;
	struct fileheader *ptr;
	int total, i, filetime;
	filetime = atoi(file + 2);
	total = mf->size / sizeof (struct fileheader);
	if (total == 0)
		return NULL;
	if (*num >= total)
		*num = total - 1;
	if (*num < 0) {
		*num = Search_Bin(mf->ptr, filetime, 0, total - 1);
		if (*num >= 0) {
			ptr = (struct fileheader *) (mf->ptr + *num * sizeof (struct fileheader));
			memcpy(&x, ptr, sizeof (struct fileheader));
			return &x;
		}
		return NULL;
	}
	ptr = (struct fileheader *) (mf->ptr + *num * sizeof (struct fileheader));
	for (i = (*num); i >= 0; i--) {
		if (mode && ptr->filetime < filetime)
			return NULL;
		if (ptr->filetime == filetime) {
			memcpy(&x, ptr, sizeof (struct fileheader));
			*num = i;
			return &x;
		}
		ptr--;
	}
	return NULL;
}

char *
utf8_decode(char *src)
{
	static iconv_t cd = (iconv_t) - 1;
	static char out[2048];
	char *outbuf;
	char *inbuf;
	size_t flen, tlen, n;
	if (cd == (iconv_t) - 1) {
		cd = iconv_open("GB2312", "UTF-8");
		if (cd == (iconv_t) - 1)
			return src;
	}
	flen = strlen(src);
	if (flen > 2000)
		return src;
	tlen = flen;
	outbuf = &(out[0]);
	inbuf = src;
	n = iconv(cd, &inbuf, &flen, &outbuf, &tlen);
	if (n != (size_t) - 1) {
		*outbuf = 0;
		return out;
	} else
		return src;
}

char mybrd[GOOD_BRC_NUM][80];
int mybrdnum = 0;

void
fdisplay_attach(FILE * output, FILE * fp, char *currline, char *nowfile)
{
	char buf[1024], *attachfile, *download, *ext, *ptr;
	int pic;
	static int ano = 0;
	off_t size;
	if (NULL == fp) {
		ano = 0;
		return;
	}
	strncpy(buf, currline, sizeof (buf));
	buf[1023] = 0;
	attachfile = buf + 10;
	ptr = attachfile;
	while (*ptr) {
		if (*ptr == '\n' || *ptr == '\r') {
			*ptr = 0;
			break;
		}
		if ((*ptr > 0 && *ptr < ' ') || isspace(*ptr) || strchr("\\/~`!@#$%^&*()|{}[];:\"'<>,?", *ptr)) {
			*ptr = '_';
		}
		ptr++;
	}
	if (strlen(attachfile) < 2)
		return;

	download = attachdecode(fp, nowfile, attachfile);
	if (download == NULL) {
		fprintf(output, "\xB2\xBB\xC4\xDC\xD5\xFD\xC8\xB7\xBD\xE2\xC2\xEB\xB5\xC4\xB8\xBD\xBC\xFE\xC4\xDA\xC8\xDD..."); // 不能正确解码的附件内容
		return;
	}
	if ((ext = strrchr(attachfile, '.')) != NULL) {
		if (!strcasecmp(ext, ".bmp") || !strcasecmp(ext, ".jpg")
				|| !strcasecmp(ext, ".gif")
				|| !strcasecmp(ext, ".jpeg")
				|| !strcasecmp(ext, ".png")
				|| !strcasecmp(ext, ".pcx"))
			pic = 1;
		else if (!strcasecmp(ext, ".swf"))
			pic = 2;
		else
			pic = 0;
	} else
		pic = 0;
	size = file_size(download);
	if ((ext = strrchr(download, ':')) != NULL)
		*ext = '/';
	download += sizeof (ATTACHCACHE);
	switch (pic) {
	case 1:
		fprintf(output,
			"%d \xB8\xBD\xCD\xBC: %s (%ld \xD7\xD6\xBD\xDA)<br>"
			"<a href='/attach/%s'> "
			"<IMG style=\" max-width:800px; width: expression(this.width > 800 ? 800: true); height:auto\" SRC='/attach/%s' border=0/> </a>",
	//	"<img src='/attach/%s'></img>",
			++ano, attachfile, size, download, download);
		break;
	case 2:
		fprintf(output,
			"%d Flash\xB6\xAF\xBB\xAD: <a href='/attach/%s'>%s</a> (%ld \xD7\xD6\xBD\xDA)<br>"
			"<OBJECT><PARAM NAME='MOVIE' VALUE='/attach/%s'>"
			"<EMBED SRC='/attach/%s' width=480 height=360></EMBED></OBJECT>",
			++ano, download, attachfile, size, download, download);
		break;
	default:
		fprintf(output,
			"%d \xB8\xBD\xBC\xFE: <a href='/attach/%s'>%s</a> (%ld \xD7\xD6\xBD\xDA)",
			++ano, download, attachfile, size);
		break;
	}
}

void
printhr()
{
	printf("<div class=\"linehr\"></div>");
}

static void updatelastboard(void) {
	struct boardmem *last;
	char buf[80];
	if (u_info->curboard) {
		last = &(shm_bcache->bcache[u_info->curboard - 1]);
		if (last->inboard > 0)
			last->inboard--;
		if (now_t > w_info->lastinboardtime && w_info->lastinboardtime != 0)
			snprintf(buf, 80, "%s use %s %ld", currentuser.userid, last->header.filename, (long int) (now_t - w_info->lastinboardtime));
		else
			snprintf(buf, 80, "%s use %s 1", currentuser.userid, last->header.filename);
		newtrace(buf);
	}
	u_info->curboard = 0;
}

void updateinboard(struct boardmem *x) {
	int bnum;
	if (!loginok)
		return;
	bnum = x - (struct boardmem *) (shm_bcache);
	if (bnum + 1 == u_info->curboard)
		return;
	updatelastboard();
	u_info->curboard = bnum + 1;
	w_info->lastinboardtime = now_t;
	x->inboard++;
	return;
}

#include "bbsupdatelastpost.c"
#include "boardrc.c"
#include "deny_users.c"
#include "bbsred.c"
int
max_mail_size()
{
	int maxsize;
	/*maxsize = (HAS_PERM(PERM_SYSOP)
		|| HAS_PERM(PERM_SPECIAL1)) ?
	MAX_SYSOPMAIL_HOLD : (HAS_PERM(PERM_ARBITRATE)
				|| HAS_PERM(PERM_BOARDS)) ?
	MAX_MAIL_HOLD * 2 : MAX_MAIL_HOLD;
	maxsize = maxsize * 10;
	return maxsize;*/
	maxsize= (HAS_PERM(PERM_SYSOP, currentuser)) ? MAX_SYSOPMAIL_HOLD : HAS_PERM(PERM_SPECIAL1, currentuser) ? MAX_MAIL_HOLD * 20 : (HAS_PERM(PERM_BOARDS, currentuser)) ? MAX_MAIL_HOLD * 8 : MAX_MAIL_HOLD * 3;
	maxsize = maxsize * 10;
	//modified by wjbta@bmy 修改信箱容量控制
	return maxsize;
}

int
get_mail_size()
{
	int currsize = 0;
	char currmaildir[STRLEN], tmpmail[STRLEN];
	struct fileheader tmpfh;
	FILE *fp;
	time_t t;
	sethomefile_s(tmpmail, sizeof(tmpmail), currentuser.userid, "msgindex");
	if (file_time(tmpmail))
		currsize += file_size(tmpmail);
	sethomefile_s(tmpmail, sizeof(tmpmail), currentuser.userid, "msgindex2");
	if (file_time(tmpmail))
		currsize += file_size(tmpmail);
	sethomefile_s(tmpmail, sizeof(tmpmail), currentuser.userid, "msgcontent");
	if (file_time(tmpmail))
		currsize += file_size(tmpmail);
	sprintf(currmaildir, "mail/%c/%s/%s", mytoupper(currentuser.userid[0]),
		currentuser.userid, DOT_DIR);
	t = file_time(currmaildir);
	if (!t) {
		currsize /= 1024;
		return currsize;
	}
	currsize += file_size(currmaildir);
	fp = fopen(currmaildir, "r");
	if (!fp) {
		currsize /= 1024;
		return currsize;
	}
	while (fread(&tmpfh, 1, sizeof (tmpfh), fp) == sizeof (tmpfh)) {
		setmailfile_s(tmpmail, sizeof(tmpmail), currentuser.userid, fh2fname(&tmpfh));
		currsize += file_size(tmpmail);
	}
	fclose(fp);
	currsize /= 1024;
	return currsize;

}

int
check_maxmail(char *currmaildir)
{
	int currsize, maxsize;
	if(HAS_PERM(PERM_SYSOP|PERM_OBOARDS, currentuser))	//add by mintbaggio 040323 for unlimitted mail volum of SYSOPs
		return 0;
	currsize = 0;
	maxsize = max_mail_size();
	currsize = get_mail_size();
	if (currsize > maxsize + 20)
		return (1);
	else
		return (0);
}

int
countln(fname)
char *fname;
{
	FILE *fp;
	char tmp[256];
	int count = 0;
	if ((fp = fopen(fname, "r")) == NULL)
		return 0;
	while (fgets(tmp, sizeof (tmp), fp) != NULL)
		count++;
	fclose(fp);
	return count;
}

double *
system_load()
{
	static double load[3] = {
		0, 0, 0
	};
#if defined(LINUX)
	FILE *fp;
	fp = fopen("/proc/loadavg", "r");
	if (!fp)
		load[0] = load[1] = load[2] = 0;
	else {
		float av[3];
		fscanf(fp, "%g %g %g", av, av + 1, av + 2);
		fclose(fp);
		load[0] = av[0];
		load[1] = av[1];
		load[2] = av[2];
	}
#elif defined(BSD44)
	getloadavg(load, 3);
#else
	struct statstime rs;
	rstat("localhost", &rs);
	load[0] = rs.avenrun[0] / (double) (1 << 8);
	load[1] = rs.avenrun[1] / (double) (1 << 8);
	load[2] = rs.avenrun[2] / (double) (1 << 8);
#endif
	return load;
}

int
setbmstatus(struct userec *u, int online)
{
	char path[256];
	sethomefile_s(path, sizeof(path), u->userid, "mboard");
	bmfilesync(u);
	new_apply_record(path, sizeof (struct boardmanager), (void *) setbmhat, &online);
	return 0;
}

static int setbmhat(struct boardmanager *bm, int *online) {
	if (strcmp(shm_bcache->bcache[bm->bid].header.filename, bm->board)) {
		errlog("error board name %s, %s. user %s",
				shm_bcache->bcache[bm->bid].header.filename, bm->board,
				currentuser.userid);
		return -1;
	}
	if (*online) {
		shm_bcache->bcache[bm->bid].bmonline |= (1 << bm->bmpos);
		if (u_info->invisible)
			shm_bcache->bcache[bm->bid].bmcloak |= (1 << bm->bmpos);
		else
			shm_bcache->bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	} else {
		shm_bcache->bcache[bm->bid].bmonline &= ~(1 << bm->bmpos);
		shm_bcache->bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	}
	return 0;
}

int
dofilter(char *title, char *fn, int level)
{
	struct mmapfile *mb;
	char *bf;
	switch (level) {
	case 1:
		mb = &mf_badwords;
		bf = BADWORDS;
		break;
	case 0:
		mb = &mf_sbadwords;
		bf = SBADWORDS;
		break;
	case 2:
		mb = &mf_pbadwords;
		bf = PBADWORDS;
		break;
	default:
		return 1;
	}
	if (mmapfile(bf, mb) < 0)
		goto CHECK2;

	if (ytht_smth_filter_article(title, fn, mb)) {
		if (level != 2)
			return 1;
		return 2;
	}
CHECK2:
	if (level != 1)
		return 0;
	mb = &mf_pbadwords;
	bf = PBADWORDS;
	if (mmapfile(bf, mb) < 0)
		return 0;
	if (ytht_smth_filter_article(title, fn, mb))
		return 2;
	else
		return 0;
}

int
dofilter_edit(char *title, char *buf, int level)
{
	struct mmapfile *mb;
	char *bf;
	switch (level) {
	case 1:
		mb = &mf_badwords;
		bf = BADWORDS;
		break;
	case 0:
		mb = &mf_sbadwords;
		bf = SBADWORDS;
		break;
	case 2:
		mb = &mf_pbadwords;
		bf = PBADWORDS;
		break;
	default:
		return 1;
	}
	if (mmapfile(bf, mb) < 0)
		goto CHECK2;

	if (ytht_smth_filter_string(buf, mb) || ytht_smth_filter_string(title, mb)) {
		if (level != 2)
			return 1;
		return 2;
	}
CHECK2:
	if (level != 1)
		return 0;
	mb = &mf_pbadwords;
	bf = PBADWORDS;
	if (mmapfile(bf, mb) < 0)
		return 0;
	if (ytht_smth_filter_string(buf, mb) || ytht_smth_filter_string(title, mb))
		return 2;
	else
		return 0;
}

int
search_filter(char *pat1, char *pat2, char *pat3)
{
	if (mmapfile(BADWORDS, &mf_badwords) < 0)
		return 0;
	if (ytht_smth_filter_string(pat1, &mf_badwords)
		|| ytht_smth_filter_string(pat2, &mf_badwords)
		|| ytht_smth_filter_string(pat3, &mf_badwords)) {
		return -1;
	}
	return 0;
}

char *
setbfile(buf, boardname, filename)
char *buf, *boardname, *filename;
{
	sprintf(buf, "boards/%s/%s", boardname, filename);
	return buf;
}

void sstrcat(char *s, const char *format, ...){
	va_list ap;
	char temp[4096];
	va_start(ap, format);
	vsprintf(temp,format,ap);
	strcat(s,temp);
	va_end(ap);
}


//add by liuche@BMY 20120312 for show boards by w_info->def_mode
char* showByDefMode(){
	if(0 == w_info->def_mode)
		return "home?B=";
	else
		return "tdoc?B=";

}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
	char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
	memset(buf, 0, strlen(str) * 3 + 1);
	while (*pstr) {
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
			*pbuf++ = *pstr;
		else if (*pstr == ' ')
			*pbuf++ = '%', *pbuf++ = '2', *pbuf++ = '0';
		else
			*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}

char to_hex(char code) {
	static char hex[] = "0123456789ABCDEF";
	return hex[code & 15];
}

//not high light Add by liuche 20121112
void
NHsprintf(char *s, char *s0)
{
	char ansibuf[80], buf2[80];
	char *tmp;
	int bold, m, i, len, lsp = -1;
	char c;
	len = 0;
	bold = 0;
	for (i = 0; (c = s0[i]); i++) {
		switch (c) {
		case '\\':
			if (quote_quote)
				strnncpy2(s, &len, "\\\\", 2);
			else
				s[len++] = c;
			break;
		case '$':
			/*errlog("usingMath=%d withinMath=%d", usingMath, withinMath);*/
			if (usingMath && !withinMath) {
				strnncpy2(s, &len, "<span class=math>", 17);
				withinMath = 1;
			} else if (usingMath && withinMath == 1) {
				strnncpy2(s, &len, "</span>", 7);
				withinMath = 0;
			} else
				s[len++] = c;
			break;
		case '"':
			if (usingMath && !withinMath && s0[i + 1] == '[') {
				strnncpy2(s, &len, "<div class=math>", 16);
				i++;
				withinMath = 2;
			} else if (usingMath && withinMath == 2 && s0[i + 1] == ']') {
				strnncpy2(s, &len, "</div>", 6);
				i++;
				withinMath = 0;
			} else if (usingMath && s0[i + 1] == '$') {
				s[len++] = '$';
				i++;
			} else if (quote_quote)
				strnncpy2(s, &len, "\\\"", 2);
			else
				s[len++] = c;
			break;
		case '&':
			strnncpy2(s, &len, "&amp;", 5);
			break;
		case '<':
			strnncpy2(s, &len, "&lt;", 4);
			break;
		case '>':
			strnncpy2(s, &len, "&gt;", 4);
			break;
		case ' ':
			if (lsp != i - 1) {
				s[len++] = c;
				lsp = i;
			} else {
				strnncpy2(s, &len, "&nbsp;", 6);
			}
			break;
		case '\r':
			break;
		case '\n':
			if (withinMath) {
				s[len++]=' ';
				break;
			}
			if (quote_quote)
				strnncpy2(s, &len, " \\\n<br>", 7);
			else if (!istitle)
				strnncpy2(s, &len, "\n<br>", 5);
			break;
		case '\033':
			if (s0[i + 1] != '[')
				continue;
			for (m = i + 2; s0[m] && m < i + 24; m++)
				if (strchr("0123456789;", s0[m]) == 0)
					break;
			ytht_strsncpy(ansibuf, &s0[i + 2], m - (i + 2) + 1);
			i = m;
			if (s0[i] != 'm')
				continue;
			if (strlen(ansibuf) == 0) {
				bold = 0;
				//strnncpy2(s, &len, "</font><font class=b40><font class=c37>", 39);//23
			}
			tmp = strtok(ansibuf, ";");
			while (tmp) {
				c = atoi(tmp);
				tmp = strtok(0, ";");
				if (c == 1)
					bold = 1;
				if (c == 0) {
					// strnncpy2(s, &len, "</font><font class=b40><font class=c37>", 39);//23
					bold = 0;
				}
				if (c >= 30 && c <= 37) {
					if (bold == 1) {
						//sprintf(buf2,
							//"<font class=h%d>",
							//c);//</font>
						//strnncpy2(s, &len, buf2, 16);//23
					}
					if (bold == 0) {
						//sprintf(buf2, "<font class=c%d>", c);//</font>
						//strnncpy2(s, &len, buf2, 16);//23
					}
				}
				if (c >= 40 && c <= 47){
					//sprintf(buf2, "<font class=b%d>", c);//</font>
					//strnncpy2(s, &len, buf2, 16);//23
				}
			}
			break;
		default:
			s[len++] = c;
		}
	}
	s[len] = 0;
}

