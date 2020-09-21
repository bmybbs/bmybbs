#include <string.h>
#include "ythtbbs.h"
int
qnyjzx(char *id)
{
	if (!strcmp(id, "shezhlA") || !strcmp(id, "shezhlB")
	    || !strcmp(id, "shezhlC")) {
		return 1;
	}
	return 0;
}

int
politics(char *board)
{
	if (!strcmp(board, "triangle") || !strcmp(board, "news")
	    || !strcmp(board, "auto_news"))
		return 1;
	return 0;
}
