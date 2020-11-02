//ylsdd Nov 05, 2002
#include <dirent.h>
#include "bbs.h"
#include "ythtbbs/ythtbbs.h"
#include "bmy/cookie.h"
#include "ythtbbs/session.h"

static char *FileName;		/* The filename, as selected by the user. */
static char *ContentStart;	/* Pointer to the file content. */
static int ContentLength;	/* Bytecount of the content. */
char userattachpath[256];
int attachtotalsize;

static char *
getsenv(char *s)
{
	char *t = getenv(s);
	if (t)
		return t;
	return "";
}

static char *
getreqstr()
{
	static char str[100] = { 0 }, *ptr;
	if (str[0])
		return str;
	ytht_strsncpy(str, getsenv("SCRIPT_URL"), sizeof(str));
	if ((ptr = strchr(str, '&')))
		*ptr = 0;
	return str;
}

int
getpathsize(char *path, int showlist)
{
	DIR *pdir;
	struct dirent *pdent;
	char fname[1024];
	int totalsize = 0, size;
	if (showlist)
		printf("已经上载的附件有:<br>");
	pdir = opendir(path);
	if (!pdir)
		return -1;
	while ((pdent = readdir(pdir))) {
		if (!strcmp(pdent->d_name, "..") || !strcmp(pdent->d_name, "."))
			continue;
		if (strlen(pdent->d_name) + strlen(path) >= sizeof (fname)) {
			totalsize = -1;
			break;
		}
		sprintf(fname, "%s/%s", path, pdent->d_name);
		size = ytht_file_size_s(fname);
		printf("<ul>\n");
		if (showlist) {
			printf("<li> <b>%s</b> (<i>%d字节</i>) ", pdent->d_name, size);
			printf("<a href='%s&%s'>删除</a></li>\n", getreqstr(), pdent->d_name);
		}
		printf("</ul>\n");
		if (size < 0) {
			totalsize = -1;
			break;
		}
		totalsize += size;
	}
	closedir(pdir);
	if (showlist) {
		printf("<br>大小总计 %d 字节 (最大 %d 字节)<br>", totalsize, MAXATTACHSIZE);
	}
	return totalsize;
}

static void
html_header()
{
	printf("Content-type: text/html; charset=gb2312\n\n\n");
	printf("<!DOCTYPE html><HTML><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=gbk\"/><title>upload-BMYBBS</title></head><body bgcolor=#f0f4f0>\n");
}

static void
http_fatal(char *str)
{
	printf("错误! %s!<br>", str);
	printf("<a href=javascript:history.go(-1)>快速返回</a></body></html>");
	exit(0);
}

/**
 * Skip a line in the input stream.
 * @param Input        Pointer into the incoming stream.
 * @param InputLength  Bytes left in the incoming stream.
 */
static void SkipLine(char **Input, int *InputLength) {
	while ((**Input != '\0') && (**Input != '\r') && (**Input != '\n')) {
		*Input = *Input + 1;
		*InputLength = *InputLength - 1;
	}
	if (**Input == '\r') {
		*Input = *Input + 1;
		*InputLength = *InputLength - 1;
	}
	if (**Input == '\n') {
		*Input = *Input + 1;
		*InputLength = *InputLength - 1;
	}
}

/**
 * @brief TODO
 * @param Input        Pointer into the incoming stream.
 * @param InputLength  TODO
 * @param len          TODO
 */
static void GoAhead(char **Input, int *InputLength, int len) {
	*Input += len;
	*InputLength -= len;
}

/**
 * Accept a single segment from the incoming mime stream. Each field in the
 * form will generate a mime segment. Return a pointer to the beginning of
 * the Boundary, or NULL if the stream is exhausted.
 * @param Input        Pointer into the incoming stream.
 * @param InputLength  Bytes left in the incoming stream.
 * @param Boundary     Character string that delimits segments.
 */
static void AcceptSegment(char **Input, int *InputLength, char *Boundary) {
	char *FieldName;	/* Name of the variable from the form. */
	char *ContentEnd;
	char *contentstr, *ptr;
	/* The input stream should begin with a Boundary line. Error-exit if not found. */
	if (strncmp(*Input, Boundary, strlen(Boundary)) != 0)
		http_fatal("文件传送错误 10");
	/* Skip the Boundary line. */
	GoAhead(Input, InputLength, strlen(Boundary));
	SkipLine(Input, InputLength);
	/* Return NULL if the stream is exhausted (no more segments). */
	if ((**Input == '\0') || (strncmp(*Input, "--", 2) == 0))
		http_fatal("文件传送错误 11");
	// The first line of a segment must be a "Content-Disposition" line. It
	// contains the fieldname, and optionally the original filename. Error-exit
	// if the line is not recognised.
	contentstr = "content-disposition: form-data; name=\"";
	if (strncasecmp(*Input, contentstr, strlen(contentstr)))
		http_fatal("文件传送错误 12");
	GoAhead(Input, InputLength, strlen(contentstr));
	FieldName = *Input;
	ptr = strchr(*Input, '\"');
	if (!ptr)
		http_fatal("文件传送错误 13");
	*ptr = 0;
	ptr++;
	while (*ptr && !isalpha(*ptr))
		ptr++;
	GoAhead(Input, InputLength, ptr - *Input);
	if (strncasecmp(*Input, "filename=\"", strlen("filename=\"")))
		http_fatal("文件传送错误 14");
	GoAhead(Input, InputLength, strlen("filename=\""));
	FileName = *Input;
	ptr = strchr(*Input, '\"');
	if (!ptr)
		http_fatal("文件传送错误 15");
	*ptr = 0;
	ptr++;
	GoAhead(Input, InputLength, ptr - *Input);
	// Skip the Disposition line and one or more mime lines, until an empty
	// line is found.
	SkipLine(Input, InputLength);
	while ((**Input != '\r') && (**Input != '\n'))
		SkipLine(Input, InputLength);
	SkipLine(Input, InputLength);
	// The following data in the stream is binary. The Boundary string is the
	// end of the data. There may be a CRLF just before the Boundary, which
	// must be stripped.
	ContentStart = *Input;
	ContentLength = 0;
	while (*InputLength > 0 && memcmp(*Input, Boundary, strlen(Boundary))) {
		GoAhead(Input, InputLength, 1);
		ContentLength++;
	}
	ContentEnd = *Input - 1;
	if ((ContentLength > 0) && (*ContentEnd == '\n')) {
		ContentEnd--;
		ContentLength--;
	}
	if ((ContentLength > 0) && (*ContentEnd == '\r')) {
		ContentEnd--;
		ContentLength--;
	}
}

static int
myatoi(char *a)
{
	int i = 0;
	while ((unsigned char)*a)
		i = i * 26 + (*(a++) - 'A');
	return i;
}

int
save_attach()
{
	char *ptr, *p0, filename[1024];
	char *pSuffix;//add by wsf
	int suffixLen;//add by wsf
	FILE *fp;


	p0 = FileName;
	ptr = strrchr(p0, '/');
	if (ptr) {
		*ptr = 0;
		ptr++;
	} else
		ptr = p0;
	p0 = ptr;
	ptr = strrchr(p0, '\\');
	if (ptr) {
		*ptr = 0;
		ptr++;
	} else
		ptr = p0;
	p0 = ptr;
	//修改开始 add by wsf
	if (strlen(p0) > 40){//文件名过长截断时保留后缀名
		pSuffix = strrchr(p0,'.');
		suffixLen = strlen(pSuffix);

		int pAscii=40-suffixLen;
		while((unsigned int)p0[pAscii]>0xA0)
			--pAscii;
		pAscii = (pAscii+1) & 1;
		strcpy(&p0[40-suffixLen-pAscii],pSuffix);
		printf("<script language=\"JavaScript\">alert('文件名超过40个字符长度，已经进行了截取');</script>\n");
	}

	if (checkfilename(p0)){
		printf("<script language=\"JavaScript\">\n"
				" alert(\"文件名中不能包含空格或者下面的非法字符\\r\\n\\/~`!@#$%%^&*()|{}[];:\\\"'<>,?\");\n"
				"</script>\n");
		http_fatal("无效的文件名");
	}
	//修改结束 add by wsf
	sprintf(filename, "%s/%s", userattachpath, p0);
	fp = fopen(filename, "w");
	fwrite(ContentStart, 1, ContentLength, fp);
	fclose(fp);
	printf("文件 %s (%d字节) 上载成功<br>", p0, ContentLength);
	return 0;
}

void
do_del()
{
	char str[1024], *p0, *ptr, str_gbk[1024];
	char filename[1024];
	ytht_strsncpy(str, getsenv("PATH_INFO"), sizeof(str));
	if (!(p0 = strchr(str, '&')))
		return;
	p0++;
	strcpy(str, p0);
	if(is_utf(str, strlen(str))){
		printf("<span style=\"color: red\">utf8</span><br />");
		u2g(str,strlen(str),str_gbk,sizeof(str_gbk));
		p0=str_gbk;
	}
	else
		p0=str;

	ptr = strsep(&p0, "&");

	if (checkfilename(ptr))
		http_fatal("无效的文件名");
	sprintf(filename, "%s/%s", userattachpath, ptr);

	if( access(filename, F_OK) != -1){
		printf("<span style=\"color: red\">已删除: %s</span><br />", ptr);
		unlink(filename);
	}
	else{
		printf("<span style=\"color: red\">错误: %s 不存在</span><br />", ptr);
	}

}

void
printuploadform()
{
	char *req = getreqstr();
	printf("<hr>"
			"<form name=frmUpload action='%s' enctype='multipart/form-data' method=post>"
			"上载附件: <input type=file name=file>"
			"<input type=submit value=上载 "
			"onclick=\"this.value='附件上载中，请稍候...';this.disabled=true;frmUpload.submit();\">"
			"</form> "
			"在这里可以为文章附加点小图片小程序啥的, 不要贴太大的东西哦, 文件名里面也不要有括号问号什么的, 否则会粘贴失败哦.<br>"
			"<b>可以在文章中任意定位附件</b>，只需要在文章编辑框中预期位置上顶头写上“#attach 1.jpg”就可以了(别忘了将 1.jpg 换成所上载的文件名 :) )<br>"
			"还要注意，如果是图片的话，<b>单个图片文件最大大小为 %d byte</b>."
			"<center><input type=button value='刷新' onclick=\"location='%s';\">&nbsp; &nbsp;"
			"<input type=button value='完成' onclick='window.close();'></center>",
			req, MAXPICSIZE, req);
}

int
main(int argc, char *argv[], char *environment[])
{
	char *ptr, *buf;
	char Boundary[1024] = "--";
	int len, i;
	const struct user_info *ptr_info;
	char cookie_buf[128];
	struct bmy_cookie cookie;

	html_header();
	seteuid(BBSUID);

	if (geteuid() != BBSUID)
		http_fatal("内部错误 0");
	ythtbbs_cache_utmp_resolve();

	ytht_strsncpy(cookie_buf, getsenv("HTTP_COOKIE"), sizeof(cookie_buf));
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(cookie_buf, &cookie);

	i = ythtbbs_session_get_utmp_idx(cookie.sessid, cookie.userid);
	if (i < 0 || i > USHM_SIZE)
		http_fatal("请先登录 2");
	ptr_info = ythtbbs_cache_utmp_get_by_idx(i);
	if (!ptr_info->active)
		http_fatal("请先登录 31");
	if (!(ptr_info->userlevel & PERM_POST))
		http_fatal("缺乏 POST 权限");
	snprintf(userattachpath, sizeof (userattachpath), PATHUSERATTACH "/%s", ptr_info->userid);
	mkdir(userattachpath, 0760);
	//clearpath(userattachpath);
	/* Test if the program was started by a METHOD=POST form. */
	if (strcasecmp(getsenv("REQUEST_METHOD"), "post")) {
		do_del();
		attachtotalsize = getpathsize(userattachpath, 1);
		if (attachtotalsize < MAXATTACHSIZE)
			printuploadform();
		printf("</body></html>");
		return 0;
	}
	attachtotalsize = getpathsize(userattachpath, 0);
	if (attachtotalsize < 0)
		http_fatal("无法检测目录大小");
	/* Test if the program was started with ENCTYPE="multipart/form-data". */
	ptr = getsenv("CONTENT_TYPE");
	if (strncasecmp(ptr, "multipart/form-data; boundary=", 30))
		http_fatal("文件传送错误 2");
	// Determine the Boundary, the string that separates the segments in the
	// stream. The boundary is available from the CONTENT_TYPE environment
	// variable.
	ptr = strchr(ptr, '=');
	if (!ptr)
		http_fatal("文件传送错误 3");
	ptr++;
	ytht_strsncpy(Boundary + 2, ptr, sizeof(Boundary) - 2);
	// Get the total number of bytes in the input stream from the
	// CONTENT_LENGTH environment variable.
	len = atoi(getsenv("CONTENT_LENGTH"));
	if (len <= 0 || len > 5000000)
		http_fatal("文件传送错误 4");
	buf = malloc(len + 1);
	if (!buf)
		http_fatal("文件传送错误 5");
	len = fread(buf, 1, len, stdin);
	buf[len] = 0;
	ptr = buf;
	AcceptSegment(&ptr, &len, Boundary);

	// 图片大小限制
	ptr = FileName+strlen(FileName);
	if (!strcasecmp(ptr - 4, ".gif") || !strcasecmp(ptr - 4, ".jpg") ||
			!strcasecmp(ptr - 4, ".bmp") || !strcasecmp(ptr - 4, ".png")) {
		if (ContentLength > MAXPICSIZE) {
			free(buf);
			http_fatal("图片附件太大, 超过限额");
		}
	}

	if (ContentLength + attachtotalsize > MAXATTACHSIZE) {
		free(buf);
		http_fatal("附件太大, 超过限额");
	}
	save_attach();
	/* Cleanup. */
	free(buf);
	attachtotalsize = getpathsize(userattachpath, 1);
	if (attachtotalsize < MAXATTACHSIZE)
		printuploadform();
	return (0);
}

