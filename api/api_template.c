#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "apilib.h"

api_template_t api_template_create(const char *filename)
{
	char *p, *s;
	int fd;
	struct stat statbuf;

	fd = open(filename, O_RDONLY);
	if(fd == -1) {
		return NULL; // cannot open
	}

	if(fstat(fd, &statbuf) == -1) {
		close(fd);
		return NULL; // fstat error
	}

	if(!S_ISREG(statbuf.st_mode)) {
		close(fd);
		return NULL; // not a file
	}

	p = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	if(p == MAP_FAILED) {
		return NULL;
	}

	s = strdup(p);

	munmap(p, statbuf.st_size);
	return s;
}

void api_template_set(api_template_t *tpl, const char *key, char *fmt, ...)
{
	if(!tpl)
		return;

	va_list v;
	char *new_string, *old_string;

	va_start(v, fmt);
	vasprintf(&new_string, fmt, v);
	va_end(v);

	// old_string = "<% key %>"
	old_string = malloc(strlen(key) + 7);
	memset(old_string, 0, strlen(key)+7);
	sprintf(old_string, "<%% %s %%>", key);

	*tpl = string_replace(*tpl, old_string, new_string);

	free(new_string);
	free(old_string);
}

void api_template_free(api_template_t tpl)
{
	free(tpl);
}
