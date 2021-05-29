#include "bbslib.h"

extern int showbinaryattach(char *filename);

int
bbscon1_main()
{
	char board[80], path[160], dir[200], file[80], filename[320], buf[80];
	struct fileheader *dirinfo = NULL;
	int type, num;
	char *ptr;
	struct mmapfile mf = { .ptr = NULL };
	changemode(READING);
	type = atoi(getparm("T"));
	switch (type) {
	case 4:
		ytht_strsncpy(buf, getparm("F"), 80);
		ptr = strrchr(buf, '/');
		if (NULL == ptr)
			http_fatal("错误的参数1");
		strcpy(file, ptr + 1);
		*ptr = '\0';
		sprintf(path, "boards/.1984/");
		strcat(path, buf);
		break;
	case 5:
		ytht_strsncpy(buf, getparm("F"), 80);
		ptr = strrchr(buf, '/');
		if (NULL == ptr)
			http_fatal("错误的参数1");
		strcpy(file, ptr + 1);
		*ptr = '\0';
		sprintf(path, "boards/.backnumbers/");
		strcat(path, buf);
		ptr = strchr(buf, '/');
		if (NULL == ptr)
			http_fatal("错误的参数2");
		*ptr = '\0';
		strcpy(board, buf);
		if (getboard(board) == NULL)
			http_fatal("错误的讨论区");
		break;
	default:
		http_fatal("错误的参数0");
		break;
	}
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		http_fatal("错误的参数1");
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("错误的参数2");
	sprintf(filename, "%s/%s", path, file);
	if (*getparm("attachname") == '/') {
		showbinaryattach(filename);
		return 0;
	}
	num = -1;
	sprintf(dir, "%s/.DIR", path);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("错误的讨论区");
		}
		dirinfo = findbarticle(&mf, file, &num , 1);
	}
	MMAP_CATCH {
		dirinfo = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (dirinfo == NULL)
		http_fatal("本文不存在或者已被删除");
	html_header(1);
	check_msg();
	printf("<body><center>\n");
	showcon(filename);
	printf("</center></body>\n");
	http_quit();
	return 0;
}
