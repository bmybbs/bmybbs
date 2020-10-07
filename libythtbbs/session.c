#include <stddef.h>
#include "ythtbbs/misc.h"

static const char SESSION_DICT[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789";
static const int SESSION_DICT_LEN = 62;

void ythtbbs_session_generate(char *buf, size_t len) {
	size_t i;
	ythtbbs_get_random_buf(buf, len);

	for (i = 0; i < len; i++) {
		buf[i] = SESSION_DICT[buf[i] % SESSION_DICT_LEN];
	}

	buf[len - 1] = '\0';
}

