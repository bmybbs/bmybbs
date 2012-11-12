#include "ythtlib.h"
char *
Ctime(time_t clock)
{
	char *tmp;
	char *ptr = ctime(&clock);
	tmp = strchr(ptr, '\n');
	if (NULL != tmp)
		*tmp = 0;
	return ptr;
}
