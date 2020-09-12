#include <string.h>

int is4map6addr(char *s){
	return !strncasecmp(s,"::ffff:",7);
}

char *getv4addr(char *fromhost){
	char *addr;
	addr=rindex(fromhost,':');
	return ++addr;
}
