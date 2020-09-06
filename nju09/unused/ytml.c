#include "bbslib.h"

static char *
getstr(char *buf, int len)
{
	char *ptr, *tmp;
	ptr = strndup(buf, len);
	if (NULL == ptr) {
		errlog("no mem!");
		exit(7);
	}
	tmp = strtrim(ptr);
	strcpy(ptr, tmp);
	return ptr;
}

#if defined(ENABLE_GHTHASH) && defined(ENABLE_FASTCGI)
#include <ght_hash_table.h>
static struct func_applet *
get_func_applet(char *name)
{
	static ght_hash_table_t *p_table = NULL;
	struct func_applet *a;

	if (p_table == NULL) {
		int i;
		a = funcs;
		p_table = ght_create(250, NULL, 0);
		while (a->func != NULL) {
			a->count = 0;
			a->utime = 0;
			a->stime = 0;
			for (i = 0; a->name[i] != NULL; i++)
				ght_insert(p_table, (void *) a,
					   strlen(a->name[i]),
					   (void *) a->name[i]);
			a++;
		}
	}
	return ght_get(p_table, strlen(name), name);
}
#else
static struct func_applet *
get_func_applet(char *name)
{
	struct func_applet *a;
	int i;
	a = funcs;
	while (a->func != NULL) {
		for (i = 0; a->name[i] != NULL; i++)
			if (!strcmp(name, a->name[i]))
				return a;
		a++;
	}
	return NULL;
}
#endif

static function
get_func(char *name)
{
	struct func_applet *tmp;
	tmp = get_func_applet(name);
	if (NULL == tmp)
		return NULL;
	tmp->count++;
	return tmp->func;
}

static void
do_parse_arg(char *buf, int len, char *argv[], int *argc)
{
	char *ptr;
	if ((*argc) >= ARGC_MAX)
		return;
	ptr = memchr(buf, ',', len);
	if (NULL == ptr) {
		argv[*argc] = getstr(buf, len);
		(*argc)++;
		return;
	}
	argv[*argc] = getstr(buf, ptr - buf);
	(*argc)++;
	if (ptr < buf + len - 1)
		do_parse_arg(ptr + 1, len - (ptr + 1 - buf), argv, argc);
	return;
}

static void
do_func(char *buf, int len)
{
	char *ptr;
	ptr = memchr(buf, '(', len);
	if (ptr) {
		char *func, *argv[ARGC_MAX], *end;
		function f;
		int i, argc;
		func = getstr(buf, ptr - buf);
		f = get_func(func);
		if (NULL == f) {
			printf("不支持的功能调用 %s!\n", func);
			free(func);
			return;
		}
		argv[0] = func;
		argc = 1;
		end = memchr(ptr, ')', len - (ptr - buf));
		if (NULL == end) {
			printf("语法错误,找不到参数结束标志右括号)!\n");
			free(func);
			return;
		}
		if (end > ptr + 1)
			do_parse_arg(ptr + 1, end - ptr - 1, argv, &argc);
		(*f) (argc, argv);
		for (i = 0; i < argc; i++)
			free(argv[i]);
		if (end < buf + len - 1) {
			char *st;
			st = memchr(end + 1, ';', len - (end + 1 - buf));
			if (NULL != st && st < buf + len - 1)
				do_func(st + 1, len - (st + 1 - buf));
		}
	} else
		printf("语法错误,找不到参数开始的标志左括号(!\n");
}
static void
do_process(char *buf, int len)
{
	char *ptr, *end;
	ptr = strnstr(buf, "<ytht ", len);
	if (!ptr) {
		fwrite(buf, len, 1, stdout);
		return;
	}
	if (ptr > buf)
		fwrite(buf, ptr - buf, 1, stdout);
	end = memchr(ptr, '>', len + buf - ptr);
	if (!end) {
		printf("语法错误,找不到ytht标签的结束!\n");
		return;
	}
	if (end > ptr + 6)
		do_func(ptr + 6, end - ptr - 6);
	if (end < buf + len - 1)
		do_process(end + 1, buf + len - 1 - end);
}

int
process_ytml(char *filename)
{
	char *buf = NULL, *ptr;
	int len, fd;

	struct stat ml;
	len = strlen(filename);
	if (len < 6 || strcmp(filename + len - 5, ".ytml"))
		return -1;
	fd = open(filename, O_RDONLY);
	if (-1 == fd)
		return -2;
	if (fstat(fd, &ml))
		goto ERROR;
	MMAP_TRY {
		if ((void *) -1 ==
		    (buf = mmap(0, ml.st_size, PROT_READ, MAP_SHARED, fd, 0))) {
			close(fd);
			MMAP_RETURN(-3);
		}
		ptr = memchr(buf, 0, ml.st_size);
		if (ptr) {
			close(fd);
			munmap(buf, ml.st_size);
			MMAP_RETURN(-3);
		}
		do_process(buf, ml.st_size);
		close(fd);
	}
	MMAP_CATCH {
		close(fd);
	}
	MMAP_END munmap(buf, ml.st_size);
	return 0;
      ERROR:close(fd);
	return -3;
}
