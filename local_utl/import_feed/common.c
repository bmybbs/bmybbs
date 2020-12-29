#include <stdbool.h>
#include <string.h>
#include "ythtbbs/misc.h"
#include "common.h"

bool is_valid_username(const char *s) {
	if (s == NULL)
		return false;

	if (s[0] == 0)
		return false;

	return ((s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z'));
}

