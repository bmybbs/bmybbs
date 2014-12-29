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

char needcgi[STRLEN];
char rframe[STRLEN];

char to_hex(char code);

struct wwwstyle wwwstyle[NWWWSTYLE] = {
	{"éÙºì(´ó×ÖÌå)", CSSPATH "orab.css",
	 "#b3b7e6", "#abc8f2", "yellow"},
	{"éÙºì(Ð¡×ÖÌå)", CSSPATH "oras.css",
	 "#b3b7e6", "#abc8f2", "yellow"},
	{"À¼É«(´ó×ÖÌå)", CSSPATH "blub.css",
	 "#ffc8ce", "#ffe3e7", "#ff8000"},
	{"À¼É«(Ð¡×ÖÌå)", CSSPATH "blus.css",
	 "#ffc8ce", "#ffe3e7", "#ff8000"},
	{"ÂÌÉ«(´ó×ÖÌå)", CSSPATH "greb.css",
	 "#c0c0c0", "#d0d0d0", "yellow"},
	{"ÂÌÉ«(Ð¡×ÖÌå)", CSSPATH "gres.css",
        "#c0c0c0", "#d0d0d0", "yellow"},
	{"ºÚÉ«(´ó×ÖÌå)", CSSPATH "blab.css",
	 "#c0c0c0", "#d0d0d0", "yellow"},
	{"ºÚÉ«(Ð¡×ÖÌå)", CSSPATH "blas.css",
	 "#c0c0c0", "#d0d0d0", "yellow"},
	{"×Ô¶¨ÒåµÄ½çÃæ", "bbsucss/ubbs.css",
	 "", "", ""}
};
struct wwwstyle *currstyle = wwwstyle;
int wwwstylenum = 0;
int usedMath = 0; //±¾Ò³ÃæÖÐÔø¾­Ê¹ÓÃÊýÑ§¹«Ê½
int usingMath = 0; //µ±Ç°ÎÄÕÂ£¨µ±Ç°hsprintf·½Ê½£©ÔÚÊ¹ÓÃÊýÑ§¹«Ê½
int withinMath = 0; //ÕýÔÚÊýÑ§¹«Ê½ÖÐ
int no_cache_header = 0;
int has_smagic = 0;
int go_to_first_page = 0;

void
getsalt(char salt[3])
{
	int s, i, c;

#ifdef LINUX
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &s, 4);
	close(fd);
#else
	s = random();
#endif
	salt[0] = s & 077;
	salt[1] = (s >> 6) & 077;
	salt[2] = 0;
	for (i = 0; i < 2; i++) {
		c = salt[i] + '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		salt[i] = c;
	}
}

void
filter(char *line)
{
	char temp[256];
	int i, stat, j;
	stat = 0;
	j = 0;
	for (i = 0; line[i] && i < 255; i++) {
		if (line[i] == '\033')
			stat = 1;
		if (!stat)
			temp[j++] = line[i];
		if (stat && ((line[i] > 'a' && line[i] < 'z')
			     || (line[i] > 'A' && line[i] < 'Z')
			     || line[i] == '@'))
			stat = 0;
	}
	temp[j] = 0;
	strcpy(line, temp);
}

int
junkboard(char *board)
{
	// Çë×Ô¶¨Òåjunkboard.
	return file_has_word("etc/junkboards", board);
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
struct UINDEX *uindexshm;
struct WWWCACHE *wwwcache;
struct mmapfile mf_badwords = { ptr:NULL };
struct mmapfile mf_sbadwords = { ptr:NULL };
struct mmapfile mf_pbadwords = { ptr:NULL };
char *ummap_ptr = NULL;
int ummap_size = 0;
char fromhost[256];
struct in6_addr from_addr;   //ipv6 by leoncom
int via_proxy = 0;

struct boardmem *getbcache();
struct userec *getuser();
char *anno_path_of();
void updatelastboard(void);

int
file_has_word(char *file, char *word)
{
	FILE *fp;
	char buf[256], buf2[256];
	fp = fopen(file, "r");
	if (fp == 0)
		return 0;
	while (1) {
		bzero(buf, 256);
		if (fgets(buf, 255, fp) == 0)
			break;
		sscanf(buf, "%s", buf2);
		if (!strcasecmp(buf2, word)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

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
get_record(void *buf, int size, int num, char *file)
{
	int fd;
	if (size < 1 || size > 4096)
		return 0;
	if (num < 0 || num > 1000000)
		return 0;
	bzero(buf, size);
	fd = open(file, O_RDONLY);
	if (fd < 0)
		return 0;
	lseek(fd, num * size, SEEK_SET);
	if (read(fd, buf, size) != size) {
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
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

long
get_num_records(filename, size)
char *filename;
int size;
{
	struct stat st;

	if (stat(filename, &st) == -1)
		return 0;
	return (st.st_size / size);
}

int
insert_record(fpath, data, size, pos, num)
char *fpath;
void *data;
int size;
int pos;
int num;
{
	int fd;
	off_t off, len;
	struct stat st;
	char *tmp;

	if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
		return -1;

	flock(fd, LOCK_EX);

	fstat(fd, &st);
	len = st.st_size;

	/* lkchu.990428: ernie patch Èç¹û len=0 & pos>0
	   (ÔÚ¸Õ¿ª¾«»ªÇøÄ¿Â¼½øÈ¥ÌùÉÏ£¬Ñ¡ÏÂÒ»¸ö) Ê±»áÐ´ÈëÀ¬»ø */
	off = len ? size * pos : 0;
	lseek(fd, off, SEEK_SET);

	size *= num;
	len -= off;
	if (len > 0) {
		tmp = (char *) malloc(pos = len + size);
		memcpy(tmp, data, size);
		read(fd, tmp + size, len);
		lseek(fd, off, SEEK_SET);
		data = tmp;
		size = pos;
	}

	write(fd, data, size);

	flock(fd, LOCK_UN);

	close(fd);

	if (len > 0)
		free(data);

	return 0;
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
strright(char *s, int len)
{
	int l = strlen(s);
	if (len <= 0)
		return "";
	if (len >= l)
		return s;
	return s + (l - len);
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
	printf("<br>´íÎó! %s! <br><br>\n", buf);
	fputs("<a href=javascript:history.go(-1)>¿ìËÙ·µ»Ø</a>", stdout);
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
void
hsprintf(char *s, char *s0)
{
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
			strsncpy(ansibuf, &s0[i + 2], m - (i + 2) + 1);
			i = m;
			if (s0[i] != 'm')
				continue;
			if (strlen(ansibuf) == 0) {
				bold = 0;
				strnncpy2(s, &len, "</font><font class=b40><font class=c37>",
					  39);//23
			}
			tmp = strtok(ansibuf, ";");
			while (tmp) {
				c = atoi(tmp);
				tmp = strtok(0, ";");
				if (c == 1)
					bold = 1;
				if (c == 0) {
					strnncpy2(s, &len,
						  "</font><font class=b40><font class=c37>",
						  39);//23
					bold = 0;
				}
				if (c >= 30 && c <= 37) {
					if (bold == 1) {
						sprintf(buf2,
							"<font class=h%d>",
							c);//</font>
						strnncpy2(s, &len, buf2, 16);//23
					}
					if (bold == 0) {
						sprintf(buf2,
							"<font class=c%d>",
							c);//</font>
						strnncpy2(s, &len, buf2, 16);//23
					}
				}
				if (c >= 40 && c <= 47){
					sprintf(buf2,
						"<font class=b%d>",
						c);//</font>
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

int
fhhprintf(FILE * output, char *fmt, ...)
{
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
		if (!strncasecmp(s, "http://", 7)
		    || !strncasecmp(s, "mailto:", 7)
		    || !strncasecmp(s, "ftp://", 6)) {
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
			fprintf(output, "<a target=_blank href='%s'>%s</a>",
				noh, noh);
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
	strsncpy(parm_name[parm_num], name, 78);
	strsncpy(parm_val[parm_num], val, len + 1);
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
	used:1,
	t_lines:20,
	link_mode:0,
	def_mode:0,
	att_mode:0,
	doc_mode:1,
};


void 
get_session_string(char *name) {
	char *cookies_string, *session_string, *p;
	cookies_string = getenv("HTTP_COOKIE");

	if (NULL != cookies_string) {
		session_string = strchr(cookies_string, '/');
		
		snprintf(name, STRLEN, "%s", session_string + sizeof(SMAGIC));
		
	} else {
		strcpy(name, "/");
	}
	p = strchr(name, '.');
	if (NULL != p) {
		no_cache_header = 1;
	} else {
		no_cache_header = 0;
	}

}

void
print_session_string(char *value) {
	printf("Set-Cookie:sessionString=%s;path=/\n", value);
}

int
contains_invliad_char(char *s) {
	char *tmp;
	int ret = 0; 
	tmp = s;
	while (*s != '\0') {
		if (!(*s == '/' ||
			*s == '?' ||
			*s == '=' ||
			*s == '.' ||
			*s == '&' ||
			*s == '~' ||
			*s == '_' ||
			*s == ',' ||
			*s == ';' ||
			*s == ':' ||
			*s == '-' ||
			(*s >= 'a' && *s <= 'z') ||
			(*s >= 'A' && *s <= 'Z') ||
			(*s >= '0' && *s <= '9')) 
			) {
			ret = 1;
			break;
		}
		s++;
	}
	s = tmp;
	return ret;
}

int
url_parse()
{
	char *url, *end, name[STRLEN], *p, *extraparam;
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

	extraparam = strchr(name, '_');
	if (extraparam) {
		*extraparam = 0;
		extraparam++;
	} else
		extraparam = "";
	loginok = user_init(&currentuser, &u_info, name);
	w_info = &guest;
	if (loginok)
		w_info = &(u_info->wwwinfo);
	extraparam_init(extraparam);
	if (!strcmp(url, "/") || nologin) {
		strcpy(needcgi, "bbsindex");
		strcpy(rframe, "");
		strsncpy(name, getsenv("HTTP_HOST"), 70);
		p = strchr(name, '.');
		if (p != NULL && isaword(domainname, p + 1)) {
			*p = 0;
			strsncpy(rframe, name, 60);
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

void
http_parm_free(void)
{
	int i;
	for (i = 0; i < parm_num; i++)
		free(parm_val[i]);
	parm_num = 0;
}

void
http_parm_init(void)
{
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
			parm_add(strtrim(t2), t3);
		}
		t2 = strtok(0, "&");
	}
	free(buf);
	strsncpy(buf2, getsenv("QUERY_STRING"), 1024);
	t2 = strtok(buf2, "&");
	while (t2) {
		t3 = strchr(t2, '=');
		if (t3 != 0) {
			t3[0] = 0;
			t3++;
			__unhcode(t3);
			parm_add(strtrim(t2), t3);
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
			strsncpy(buf, old, STRLEN - 10);
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
		printf
		    ("<HTML XMLNS:m=\"http://www.w3.org/1998/Math/MathML\">\n");
	//printf("<!--%d;%d;%d;%d-->", thispid, sizeof (struct wwwsession),
	//       wwwcache->www_visit, wwwcache->home_visit);
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
	/*if (isguest && reg_req())
	   printf
	   ("<SCRIPT language=\"JavaScript\">P1 = open('regreq', 'WINregreq', 'width=600,height=460');</SCRIPT > ");
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
	printf("<?xml-stylesheet type=\"text/css\" href=\"%s\"?>\n",
	       currstyle->cssfile);
	printf
	    ("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\"\n"
	     "\"http://www.w3.org/TR/MathML2/dtd/xhtml-math11-f.dtd\">\n");
	printf("<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
	       "xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n"
	       "xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");
	printf("<head>");
}

int
__to16(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0;
}

void
__unhcode(char *s)
{
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
	shm_utmp =
	    (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY,
					    sizeof (struct UTMPFILE));
	shm_bcache =
	    (struct BCACHE *) get_old_shm(BCACHE_SHMKEY,
					  sizeof (struct BCACHE));
	shm_ucache =
	    (struct UCACHE *) get_old_shm(UCACHE_SHMKEY,
					  sizeof (struct UCACHE));
	uidhashshm =
	    (struct UCACHEHASH *) get_old_shm(UCACHE_HASH_SHMKEY,
					      sizeof (struct UCACHEHASH));
	uindexshm =
	    (struct UINDEX *) get_old_shm(UINDEX_SHMKEY,
					  sizeof (struct UINDEX));
	if (shm_utmp == 0)
		http_fatal("shm_utmp error");
	if (shm_bcache == 0)
		http_fatal("shm_bcache error");
	if (shm_ucache == 0)
		http_fatal("shm_ucache error");
	if (uidhashshm == NULL)
		http_fatal("uidhashshm error");
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
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)
	    || st.st_size <= 0) {
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

void
extraparam_init(unsigned char *extrastr)
{
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

int
user_init(struct userec *x, struct user_info **y, unsigned char *ub)
{
	struct userec *x2;
	char sessionid[30];
	int i;
	utmpent = 0;
	isguest = 0;
	tempuser = 0;
	//bzero(x, sizeof (*x));
	x2 = getuser("guest");
	if (x2)
		memcpy(x, getuser("guest"), sizeof (*x));
	else
		bzero(x, sizeof (*x));
	*y = NULL;
	if (strlen(ub) < 33) {
		if (strlen(ub) >= 4) {	//´Ó telnet À´µÄÁÙÊ±ÓÃ»§
			i = (ub[0] - 'A') * 26 * 26 + (ub[1] - 'A') * 26 +
			    ub[2] - 'A';
			strcpy(sessionid, ub + 3);
			if (i < 0 || i >= MAXACTIVE)
				return 0;
			(*y) = &(shm_utmp->uinfo[i]);
			if (strncmp((*y)->from, fromhost, 24))
				return 0;
			if (strncmp((*y)->sessionid, sessionid, 6)) {
				return 0;
			}
			if ((*y)->active == 0)
				return 0;
			if (!strcmp((*y)->userid, "guest"))
				isguest = 1;
			if (!strcasecmp((*y)->userid, "new"))
				return 0;
			x2 = getuser((*y)->userid);
			if (x2 == 0)
				return 0;
			utmpent = i + 1;
			memcpy(x, x2, sizeof (*x));
			tempuser = 1;
		}
		return 0;
	}
	ub[33] = 0;
	i = (ub[0] - 'A') * 26 * 26 + (ub[1] - 'A') * 26 + ub[2] - 'A';
	strncpy(sessionid, ub + 3, 30);
	if (i < 0 || i >= MAXACTIVE)
		return 0;
	(*y) = &(shm_utmp->uinfo[i]);
	/*	ÎÞÊÓipmask ipv6 by leoncom
	if ((*y)->wwwinfo.ipmask) {
		struct in_addr ofip;
		if (!inet_aton((*y)->from, &ofip))
			return 0;
		if ((ofip.s_addr >> (*y)->wwwinfo.ipmask) !=
		    (from_addr.s_addr >> (*y)->wwwinfo.ipmask))
			return 0;
	} else 
	*/
	if (strncmp((*y)->from, fromhost, 20))  //ipv6 by leoncom 24->20
		return 0;
	if (strcmp((*y)->sessionid, sessionid))
		return 0;
	if ((*y)->active == 0)
		return 0;
	if ((*y)->userid[0] == 0)
		return 0;
	if (!strcasecmp((*y)->userid, "new"))
		return 0;
	if (now_t - u_info->lasttime > 18 * 60 || u_info->wwwinfo.iskicked)	//18·ÖÖÓÊÇÁôÏÂÒ»¸ö´°¿Ú,·ÀÖ¹Ò»¸öu_info±»Ë¢ÐÂÎª0Á½´Î
		return 0;
	else
		u_info->lasttime = now_t;
	if (!strcasecmp((*y)->userid, "guest"))
		isguest = 1;
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
	if (getuser(userid) <= 0)
		return -1;
	bzero(&header, sizeof (header));
	fh_setowner(&header, sender, 0);
	setmailfile(buf, userid, "");
	mkdir(buf, 0770);
	t = trycreatefile(buf, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	header.thread = t;
	strsncpy(header.title, title, sizeof (header.title));
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
	setmailfile(dir, userid, ".DIR");
	append_record(dir, &header, sizeof (header));
	return 0;
}

/** ±£´æÓÊ¼þµ½·¢¼þÏä
 *
 * @param userid ·¢ÐÅÈËµÄ ID£¬ÓÃÓÚÉú³É´æ·ÅÐÅ¼þµÄÂ·¾¶
 * @param title ±êÌâ
 * @param file ÎÄ¼þ
 * @param id ÊÕÐÅÈË£¬ÓÃÓÚ .DIR ÎÄ¼þÖÐ±êÃ÷ÊÕÐÅÈËµÄ ID£¬ÒÔ¼° struct fileheader ÖÐ
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
	if (strstr(userid, buf)
	    || strstr(userid, ".bbs@localhost")) {
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
	strsncpy(header.title, title, sizeof (header.title));
	header.accessed |= mark;
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fp2 = fopen(file, "r");
	fprintf(fp, "ÊÕÐÅÈË: %s (%s)\n", id, nickname);
	fprintf(fp, "±ê  Ìâ: %s\n", title);
	fprintf(fp, "·¢ÐÅÕ¾: %s (%s)\n", BBSNAME, Ctime(now_t));
	fprintf(fp, "À´  Ô´: %s\n\n", ip);
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
	fprintf(fp, "\n\n[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s][m\n",
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
	if (strstr(userid, buf)
	    || strstr(userid, ".bbs@localhost")) {
		char *pos;
		pos = strchr(userid, '.');
		*pos = '\0';
	}
	if (strstr(userid, "@"))
		return post_imail(userid, title, file, id, nickname, ip, sig);
	if (getuser(userid) <= 0)
		http_fatal("´íÎóµÄÊÕÐÅÈËµØÖ·");
	bzero(&header, sizeof (header));
	fh_setowner(&header, id, 0);
	setmailfile(buf, userid, "");
	t = trycreatefile(buf, "M.%d.A", now_t, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	header.thread = t;
	strsncpy(header.title, title, sizeof (header.title));
	header.accessed |= mark;
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fp2 = fopen(file, "r");
	fprintf(fp, "¼ÄÐÅÈË: %s (%s)\n", id, nickname);
	fprintf(fp, "±ê  Ìâ: %s\n", title);
	fprintf(fp, "·¢ÐÅÕ¾: %s (%s)\n", BBSNAME, Ctime(now_t));
	fprintf(fp, "À´  Ô´: %s\n\n", ip);
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
	fprintf(fp, "\n\n[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s][m\n",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	setmailfile(dir, userid, ".DIR");
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

int
post_imail(char *userid, char *title, char *file, char *id,
	   char *nickname, char *ip, int sig)
{
	FILE *fp1, *fp2;
	char buf[256], *ptr;
	unsigned int len;

	if (strlen(userid) > 100)
		http_fatal("´íÎóµÄÊÕÐÅÈËµØÖ·");
	for (ptr = userid; *ptr; ptr++) {
		if (strchr(";~-|`!$#%%&^*()'\"<>?/ ", *ptr) || !isprint(*ptr))
			http_fatal("´íÎóµÄÊÕÐÅÈËµØÖ·");
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
		if (NULL !=
		    (ptr = checkbinaryattach(buf, FCGI_ToFILE(fp1), &len))) {
			uuencode(FCGI_ToFILE(fp1), FCGI_ToFILE(fp2), len, ptr);
			continue;
		}
		if (buf[0] == '.' && buf[1] == '\n')
			fputs(". \n", fp2);
		else
			fputs(buf, fp2);
	}

	fputs("\n--\n", fp2);
	sig_append(fp2, id, sig);
	fprintf(fp2, "\n\n[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s][m\n",
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
	strsncpy(header.title, title, sizeof (header.title));
	fp = fopen(buf3, "w");
	if (NULL == fp)
		return -1;
	fp2 = fopen(file, "r");
	fprintf(fp,
		"·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n±ê  Ìâ: %s\n·¢ÐÅÕ¾: %s (%24.24s), %s)\n\n",
		id, nickname, board, title, BBSNAME, Ctime(now_t),
		outgoing ? "×ªÐÅ(" MY_BBS_DOMAIN : "±¾Õ¾(" MY_BBS_DOMAIN);
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
	fprintf(fp, "\n\033[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s]\033[m\n",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	sprintf(buf3, "%s/M.%d.A", buf, t);
	header.sizebyte = numbyte(eff_size(buf3));
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
	strsncpy(header.title, title, sizeof (header.title));
	header.accessed |= mark;
	if (outgoing)
		header.accessed |= FH_INND;
	fp = fopen(buf3, "w");
	if (NULL == fp)
		return -1;
	fp2 = fopen(file, "r");
	fprintf(fp,
		"·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n±ê  Ìâ: %s\n·¢ÐÅÕ¾: %s (%24.24s), %s)\n\n",
		id, nickname, board, title, BBSNAME, Ctime(now_t),
		outgoing ? "×ªÐÅ(" MY_BBS_DOMAIN : "±¾Õ¾(" MY_BBS_DOMAIN);
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
	fprintf(fp, "[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s][m",
		31 + rand() % 7, BBSNAME, "http://" MY_BBS_DOMAIN, ip);
	fclose(fp);
	sprintf(buf3, "boards/%s/M.%d.A", board, t);
	header.sizebyte = numbyte(eff_size(buf3));
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
	sprintf(fname, "bbstmpfs/tmp/security.%s.%05d", currentuser.userid,
		getpid());
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\nÔ­Òò£º\n%s\n", content);
		fprintf(se, "ÒÔÏÂÊÇ²¿·Ö¸öÈË×ÊÁÏ\n");
		fprintf(se, "×î½ü¹âÁÙ»úÆ÷: %s", fromhost);
		fclose(se);
		post_article("syssecurity", title, fname, currentuser.userid,
			     currentuser.username, fromhost, -1, 0, 0,
			     currentuser.userid, -1);
		unlink(fname);
	}
	return 0;
}

void
sig_append(FILE * fp, char *id, int sig)
{
	FILE *fp2;
	char path[256];
	char buf[256];
	int total, hasnl = 1, i, emptyline = 0, sigln, numofsig;
	if (HAS_PERM(PERM_DENYSIG))
		return;
	if (sig < -2 || sig > 10)
		return;
	sethomefile(path, id, "signatures");
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
		while (num < MAXBOARD
		       && fscanf(FCGI_ToFILE(fp), "%s %s", buf1, buf2) > 0) {
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
		if (fscanf(FCGI_ToFILE(fp), "%s %s", buf1, buf2) <= 0)
			break;
		buf1[79] = 0;
		buf1[strlen(buf1) - 1] = 0;
		if (!strcasecmp(buf1, board)) {
			sprintf(buf, "/%s", buf2);
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
	/* °æÃæ²»´æÔÚ·µ»Ø0, pºÍz°æÃæ·µ»Ø1, ÓÐÈ¨ÏÞ°æÃæ·µ»Ø1. */
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
		return u_info->clubrights[(x->header.clubnum) /
					  32] & (1 << ((x->header.clubnum) %
						       32));
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
	if (!shm_utmp->watchman || now_t < shm_utmp->watchman)
		return 0;
	return political_board(bname);

}

int
has_post_perm(struct userec *user, struct boardmem *x)
{
	char buf3[256];
	if (!loginok || isguest || !x || !has_read_perm_x(user, x))
		return 0;

	sprintf(buf3, "boards/%s/deny_users", x->header.filename);
	if (file_has_word(buf3, user->userid))
		return 0;
	sprintf(buf3, "boards/%s/deny_anony", x->header.filename);
	if (file_has_word(buf3, user->userid))
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
	if (file_has_word("deny_users", user->userid))
		return 0;
	if (x->header.clubnum != 0) {
		if (!(x->header.level & PERM_NOZAP) && x->header.level
		    && !user_perm(user, x->header.level))
			return 0;
		return u_info->clubrights[(x->header.clubnum) /
					  32] & (1 << ((x->header.clubnum) %
						       32));
	}
	if (!(x->header.level & PERM_NOZAP) && x->header.level
	    && !user_perm(user, x->header.level))
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
		if (file_has_word(buf3, user->userid))
			return 1;
		else
			return 0;
	}
	if (!(x->header.level & PERM_NOZAP) && x->header.level
	    && !user_perm(user, x->header.level))
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
	if (p_table
	    && (num != shm_bcache->number || shm_bcache->uptime > uptime)) {
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
			strsncpy(upperstr,
				 shm_bcache->bcache[i].header.filename,
				 sizeof (upperstr));
			for (j = 0; upperstr[j]; j++)
				upperstr[j] = toupper(upperstr[j]);
			ght_insert(p_table, &shm_bcache->bcache[i], j,
				   upperstr);
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
 * ÒÀ¾Ý°æÃæÃû³Æ»ñÈ¡ boardmem ¶ÔÏó£¬Ó¦Öð½¥²ÉÓÃ libythtbbs ¿âº¯Êý¡£
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
findnextutmp(char *id, int from)
{
	int i;
	if (from < 0)
		from = 0;
	for (i = from; i < MAXACTIVE; i++)
		if (shm_utmp->uinfo[i].active)
			if (!strcasecmp(shm_utmp->uinfo[i].userid, id))
				return i;
	return -1;
}

int
send_msg(char *myuserid, int i, char *touserid, int topid, char *msg,
	 int offline)
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

char *
horoscope(int month, int day)
{
	int date = month * 100 + day;
	if (month < 1 || month > 12 || day < 1 || day > 31)
		return "²»Ïê";
	if (date < 121 || date >= 1222)
		return "Ä¦ôÉ×ù";
	if (date < 219)
		return "Ë®Æ¿×ù";
	if (date < 321)
		return "Ë«Óã×ù";
	if (date < 421)
		return "ÄµÑò×ù";
	if (date < 521)
		return "½ðÅ£×ù";
	if (date < 622)
		return "Ë«×Ó×ù";
	if (date < 723)
		return "¾ÞÐ·×ù";
	if (date < 823)
		return "Ê¨×Ó×ù";
	if (date < 923)
		return "´¦Å®×ù";
	if (date < 1024)
		return "Ìì³Ó×ù";
	if (date < 1123)
		return "ÌìÐ«×ù";
	/*if(date<1222) */
	return "ÉäÊÖ×ù";
}

//add by wjbta@bmy for 666ÉúÃüÁ¦
int life_special_web(char *id)
{
        FILE *fp;
        char id1[80],buf[80];
        fp=fopen("etc/life", "r");
        if(fp==0) return 0;
        while(1) {
                if(fgets(buf, 80, fp)==0) break;
//		printf("%s",buf);
                if(sscanf(buf, "%s", id1)<1) continue;
//		printf("%s",id1);
                if(!strcasecmp(id1,id)) return 1;
                }
        fclose(fp);
	return 0;
} //add by wjbta@bmy 666ÉúÃüÁ¦

int count_life_value(struct userec *urec)
{
	int i, res;
//	i = (now_t - urec->lastlogin) / 60;
	if ((urec->userlevel & PERM_XEMPT)
	    || !strcasecmp(urec->userid, "guest"))
		return 999;
	//if (life_special_web(urec->userid)) return 666;
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
modify_mode(struct user_info *x, int newmode)
{
	if (x == 0)
		return 0;
	x->mode = newmode;
	return 0;
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
is_bansite(char *ip)
{
	FILE *fp;
	char buf3[256];
	fp = fopen(".bansite", "r");
	if (fp == 0)
		return 0;
	while (fscanf(FCGI_ToFILE(fp), "%s", buf3) > 0)
		if (!strcasecmp(buf3, ip))
			return 1;
	fclose(fp);
	return 0;
}

int
user_perm(struct userec *x, int level)
{
	return (x->userlevel & level);
}

int
useridhash(char *id)
{
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

int
getusernum(char *id)
{
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
		return 0;
	if ((uid + 1) * sizeof (struct userec) > ummap_size)
		ummap();
	if (!ummap_ptr)
		return 0;
	memcpy(&userec1, ummap_ptr + sizeof (struct userec) * uid,
	       sizeof (userec1));
	return &userec1;
}

int
checkuser(char *id, char *pw)
{
	struct userec *x;
	x = getuser(id);
	if (x == 0)
		return 0;
	return checkpasswd(x->passwd, pw);
}

int
count_id_num(char *id)
{
	int i, total = 0;
	for (i = 0; i < MAXACTIVE; i++)
		if (shm_utmp->uinfo[i].active
		    && !strcasecmp(shm_utmp->uinfo[i].userid, id))
			total++;
	return total;
}

int
count_online()
{
	int i, total = 0;
	if (now_t <= shm_utmp->activetime + 60)
		return shm_utmp->activeuser;
	for (i = 0; i < MAXACTIVE; i++)
		if (shm_utmp->uinfo[i].active && shm_utmp->uinfo[i].pid)
			total++;
	shm_utmp->activetime = now_t;
	shm_utmp->activeuser = total;
	return total;
}

int
count_online2()
{
	int i, total = 0;
	for (i = 0; i < MAXACTIVE; i++)
		if (shm_utmp->uinfo[i].active
		    && shm_utmp->uinfo[i].invisible == 0)
			total++;
	return total;
}

struct override fff[200];
int friendnum = 0;
int
loadfriend(char *id)
{
	FILE *fp;
	char file[256];
	sethomefile(file, id, "friends");
	fp = fopen(file, "r");
	friendnum = 0;
	if (fp) {
		friendnum = fread(fff, sizeof (fff[0]), 200, fp);
		fclose(fp);
	}
	return 0;
}

int
cmpfuid(a, b)
unsigned *a, *b;
{
	return *a - *b;
}

int
initfriends(struct user_info *u)
{
	int i, fnum = 0;
	char buf[128];
	FILE *fp;
	memset(u->friend, 0, sizeof (u->friend));
	sethomefile(buf, u->userid, "friends");
	u->fnum = file_size(buf) / sizeof (struct override);
	if (u->fnum <= 0)
		return 0;
	u->fnum = (u->fnum >= MAXFRIENDS) ? MAXFRIENDS : u->fnum;
	loadfriend(u->userid);
	for (i = 0; i < u->fnum; i++) {
		u->friend[i] = getusernum(fff[i].id) + 1;
		if (u->friend[i])
			fnum++;
		else
			fff[i].id[0] = 0;
	}
	qsort(u->friend, u->fnum, sizeof (u->friend[0]), (void *) cmpfuid);
	if (fnum == u->fnum)
		return fnum;
	fp = fopen(buf, "w");
	for (i = 0; i < u->fnum; i++) {
		if (fff[i].id[0])
			fwrite(&(fff[i]), sizeof (struct override), 1, fp);
	}
	fclose(fp);
	u->fnum = fnum;
	return fnum;
}

int
isfriend(char *id)
{
	int n, num;
	if (!loginok || isguest)
		return 0;
	if (u_info->fnum < 40) {
		for (n = 0; n < u_info->fnum; n++)
			if (!strcasecmp
			    (id, shm_ucache->userid[u_info->friend[n] - 1]))
				return 1;
		return 0;
	}
	if ((num = getusernum(id)) < 0)
		return 0;
	num++;
	for (n = 0; n < u_info->fnum; n++)
		if (num == u_info->friend[n])
			return 1;
	return 0;
}

struct override bbb[MAXREJECTS];
int badnum = 0;
int
loadbad(char *id)
{
	FILE *fp;
	char file[256];
	sethomefile(file, id, "rejects");
	fp = fopen(file, "r");
	if (fp) {
		badnum = fread(bbb, sizeof (bbb[0]), MAXREJECTS, fp);
		fclose(fp);
	}
	return 0;
}

int
isbad(char *id)
{
	int n;
	if (!loginok || isguest)
		return 0;
	loadbad(currentuser.userid);
	for (n = 0; n < badnum; n++)
		if (!strcasecmp(id, bbb[n].id))
			return 1;
	return 0;
}

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
encode_url(unsigned char *s)
{
	int i, j, half = 0;
	static char buf[512];
	char a[3];
	j = 0;
	for (i = 0; s[i]; i++) {
		if ((!half
		     && strchr("~`!@#$%%^&*()-_=+[{]}\\|;:'\",<.>/? ", s[i]))
		    || (s[i + 1] == 0 && !half && (unsigned char) s[i] >= 128)) {
			buf[j++] = '%';
			sprintf(a, "%02X", s[i]);
			buf[j++] = a[0];
			buf[j++] = a[1];
		} else
			buf[j++] = s[i];
		if (half)
			half = 0;
		else if ((unsigned char) s[i] >= 128)
			half = 1;
	}
	buf[j] = 0;
	return buf;
}

char *
noquote_html(unsigned char *s)
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
void1(unsigned char *s)
{
	int i;
	int flag = 0;
	for (i = 0; s[i]; i++) {
		if (flag == 0) {
			if (s[i] >= 128)
				flag = 1;
			continue;
		}
		flag = 0;
		if (s[i] < 32)
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
	strsncpy(tmp, s, 255);
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

int
fprintf2(FILE * fp, char *s)
{
	int i, tail = 0, sum = 0;
	if (s[0] == ':' && s[1] == ' ' && strlen(s) > 79) {
		sprintf(s + 76, "..\n");
		fprintf(fp, "%s", s);
		return 0;
	}
	for (i = 0; s[i]; i++) {
		fprintf(fp, "%c", s[i]);
		sum++;
		if (tail) {
			tail = 0;
		} else if (s[i] < 0) {
			tail = s[i];
		}
		if (sum >= 78 && tail == 0) {
			fprintf(fp, "\n");
			sum = 0;
		}
	}
	return 0;
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
	strsncpy(board, ptr + 1, sizeof (board));
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
	if (readuservalue(currentuser.userid, "link_mode", buf, sizeof (buf)) >=
	    0)
		w_info->link_mode = atoi(buf);
	if (readuservalue(currentuser.userid, "def_mode", buf, sizeof (buf)) >=
	    0)
		w_info->def_mode = atoi(buf);
/*	if (readuservalue(currentuser.userid, "att_mode", buf, sizeof (buf)) >=
	    0) w_info->att_mode = atoi(buf);
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
			if (!strcasecmp(tmp, "userid:")
			    && !strcasecmp(userid, currentuser.userid)) {
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
// mode = 1 ±íÊ¾ .DIR °´Ê±¼äÅÅÐò  0 ±íÊ¾ ²»ÅÅÐò
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
			ptr =
			    (struct fileheader *) (mf->ptr +
						   *num *
						   sizeof (struct fileheader));
			memcpy(&x, ptr, sizeof (struct fileheader));
			return &x;
		}
		return NULL;
	}
	ptr =
	    (struct fileheader *) (mf->ptr + *num * sizeof (struct fileheader));
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
		if ((*ptr > 0 && *ptr < ' ') || isspace(*ptr)
		    || strchr("\\/~`!@#$%^&*()|{}[];:\"'<>,?", *ptr)) {
			*ptr = '_';
		}
		ptr++;
	}
	if (strlen(attachfile) < 2)
		return;

	download = attachdecode(FCGI_ToFILE(fp), nowfile, attachfile);
	if (download == NULL) {
		fprintf(output, "²»ÄÜÕýÈ·½âÂëµÄ¸½¼þÄÚÈÝ...");
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
		fprintf
		    (output,
		     "%d ¸½Í¼: %s (%ld ×Ö½Ú)<br>"
			"<a href='/attach/%s'> "
			"<IMG style=\" max-width:800px; width: expression(this.width > 800 ? 800: true); height:auto\" SRC='/attach/%s' border=0/> </a>",
	//	"<img src='/attach/%s'></img>",
		     ++ano, attachfile, size, download, download);
		break;
	case 2:
		fprintf(output,
			"%d Flash¶¯»­: <a href='/attach/%s'>%s</a> (%ld ×Ö½Ú)<br>"
			"<OBJECT><PARAM NAME='MOVIE' VALUE='/attach/%s'>"
			"<EMBED SRC='/attach/%s' width=480 height=360></EMBED></OBJECT>",
			++ano, download, attachfile, size, download, download);
		break;
	default:
		fprintf
		    (output,
		     "%d ¸½¼þ: <a href='/attach/%s'>%s</a> (%ld ×Ö½Ú)",
		     ++ano, download, attachfile, size);
		break;
	}
}

void
printhr()
{
	printf("<div class=\"linehr\"></div>");
}

void
updatelastboard(void)
{
	struct boardmem *last;
	char buf[80];
	if (u_info->curboard) {
		last = &(shm_bcache->bcache[u_info->curboard - 1]);
		if (last->inboard > 0)
			last->inboard--;
		if (now_t > w_info->lastinboardtime
		    && w_info->lastinboardtime != 0)
			snprintf(buf, 80, "%s use %s %ld",
				 currentuser.userid,
				 last->header.filename,
				 (long int) (now_t - w_info->lastinboardtime));
		else
			snprintf(buf, 80, "%s use %s 1", currentuser.userid,
				 last->header.filename);
		newtrace(buf);
	}
	u_info->curboard = 0;
}

void
updateinboard(struct boardmem *x)
{
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
	maxsize= (HAS_PERM(PERM_SYSOP))?MAX_SYSOPMAIL_HOLD:HAS_PERM(PERM_SPECIAL1)?MAX_MAIL_HOLD*20:
                (HAS_PERM(PERM_BOARDS))?MAX_MAIL_HOLD*8:MAX_MAIL_HOLD*3;
        maxsize=maxsize*10;
	//modified by wjbta@bmy ÐÞ¸ÄÐÅÏäÈÝÁ¿¿ØÖÆ
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
	sethomefile(tmpmail, currentuser.userid, "msgindex");
	if (file_time(tmpmail))
		currsize += file_size(tmpmail);
	sethomefile(tmpmail, currentuser.userid, "msgindex2");
	if (file_time(tmpmail))
		currsize += file_size(tmpmail);
	sethomefile(tmpmail, currentuser.userid, "msgcontent");
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
		setmailfile(tmpmail, currentuser.userid, fh2fname(&tmpfh));
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
	if(HAS_PERM(PERM_SYSOP|PERM_OBOARDS))	//add by mintbaggio 040323 for unlimitted mail volum of SYSOPs
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
		fscanf(FCGI_ToFILE(fp), "%g %g %g", av, av + 1, av + 2);
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
	sethomefile(path, u->userid, "mboard");
	bmfilesync(u);
	new_apply_record(path, sizeof (struct boardmanager), (void *) setbmhat,
			 &online);
	return 0;
}

int
setbmhat(struct boardmanager *bm, int *online)
{
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
			shm_bcache->bcache[bm->bid].bmcloak &=
			    ~(1 << bm->bmpos);
	} else {
		shm_bcache->bcache[bm->bid].bmonline &= ~(1 << bm->bmpos);
		shm_bcache->bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	}
	return 0;
}

void
add_uindex(int uid, int utmpent)
{
	int i, uent;
	if (uid <= 0 || uid > MAXUSERS)
		return;
	for (i = 0; i < 6; i++)
		if (uindexshm->user[uid - 1][i] == utmpent)
			return;
	//Ö»·ÅÔÚºóÈý¸öÎ»ÖÃ, ÕâÑùÀ´±£Ö¤telnetµÄÎ»ÖÃ×Ü¿ÉÒÔ±£Áô
	for (i = 3; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0 || !shm_utmp->uinfo[uent - 1].active ||
		    shm_utmp->uinfo[uent - 1].uid != uid) {
			uindexshm->user[uid - 1][i] = utmpent;
			return;
		}
	}
}

void
remove_uindex(int uid, int utmpent)
{
	int i;
	if (uid <= 0 || uid > MAXUSERS)
		return;
	for (i = 0; i < 6; i++) {
		if (uindexshm->user[uid - 1][i] == utmpent) {
			uindexshm->user[uid - 1][i] = 0;
			return;
		}
	}
}

int
count_uindex(int uid)
{
	int i, uent, count = 0;
	struct user_info *uentp;
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &(shm_utmp->uinfo[uent - 1]);
		if (!uentp->active || !uentp->pid || uentp->uid != uid)
			continue;
		if (uentp->pid > 1 && kill(uentp->pid, 0) < 0) {
			uindexshm->user[uid - 1][i] = 0;
			continue;
		}
		count++;
	}
	return count;
}

int
cachelevel(int filetime, int attached)
{
	return 0;
	if (attached)
		return 2;
	/*
	   if (now_t - filetime < 2 * 24 * 60 * 60)
	   return 2;
	   if (now_t - filetime < 3 * 26 * 60 * 60)
	   return 1;
	 */
	return 0;
}

#if 0
int
reg_req()
{
	time_t stay;
	stay = now_t - (w_info->login_start_time);
	if ((stay / 1800) % 2 != w_info->show_reg) {
		w_info->show_reg = (stay / 1800) % 2;
		return 1;
	}
	return 0;
}
#endif

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

	if (filter_article(title, fn, mb)) {
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
	if (filter_article(title, fn, mb))
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

	if (filter_string(buf, mb)
	    || filter_string(title, mb)) {
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
	if (filter_string(buf, mb)
	    || filter_string(title, mb))
		return 2;
	else
		return 0;
}

int
search_filter(char *pat1, char *pat2, char *pat3)
{
	if (mmapfile(BADWORDS, &mf_badwords) < 0)
		return 0;
	if (filter_string(pat1, &mf_badwords)
	    || filter_string(pat2, &mf_badwords)
	    || filter_string(pat3, &mf_badwords)) {
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


/* ´ýÉ¾³ý£¬×Ö·û±àÂë×ª»»ÒÆ¶¯µ½ libythtbbs/misc.c ÏÂÃæ  */
/*
int gb2312_to_utf8(char *in, char *out, size_t size){
	iconv_t cd;
	cd = iconv_open("UTF-8", "GB2312");
	if ( cd == (iconv_t)(-1) ){
		perror("iconv_open failed");
		return 0;
	}

	size_t in_left = strlen(in) + 1;
	char *out_ptr;
	size_t res;

	out_ptr = out;
	res = iconv(cd, &in, &in_left, &out_ptr, &size);
	if ( res == (size_t)(-1) ){
		perror("iconv failed");
		return 0;
	}

	iconv_close(cd);
	return 1;
}

int utf8_to_gb2312(char *in, char *out, size_t size){
	iconv_t cd;
	cd = iconv_open("GB2312", "UTF-8");
	if ( cd == (iconv_t)(-1) ){
		perror("iconv_open failed");
		return 0;
	}

	size_t in_left = strlen(in) + 1;
	char *out_ptr;
	size_t res;

	out_ptr = out;
	res = iconv(cd, &in, &in_left, &out_ptr, &size);
	if ( res == (size_t)(-1) ){
		perror("iconv failed");
		return 0;
	}

	iconv_close(cd);
	return 1;
}
*/

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
			strsncpy(ansibuf, &s0[i + 2], m - (i + 2) + 1);
			i = m;
			if (s0[i] != 'm')
				continue;
			if (strlen(ansibuf) == 0) {
				bold = 0;
				//strnncpy2(s, &len, "</font><font class=b40><font class=c37>",
					  //39);//23
			}
			tmp = strtok(ansibuf, ";");
			while (tmp) {
				c = atoi(tmp);
				tmp = strtok(0, ";");
				if (c == 1)
					bold = 1;
				if (c == 0) {
					//strnncpy2(s, &len,
						  //"</font><font class=b40><font class=c37>",
						  //39);//23
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
						//sprintf(buf2,
							//"<font class=c%d>",
							//c);//</font>
						//strnncpy2(s, &len, buf2, 16);//23
					}
				}
				if (c >= 40 && c <= 47){
					//sprintf(buf2,
						//"<font class=b%d>",
						//c);//</font>
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

