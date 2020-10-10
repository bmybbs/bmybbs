#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bmy/cookie.h"

static const char SEPRATOR = ';';

void bmy_cookie_parse(char *buf, struct cookie *cookie) {
	int i, j, k;
	cookie->userid = buf;

	if (buf == NULL || buf[0] == '\0')
		return;

	for (i = 0, j = strlen(buf), k = 0; i < j; i++) {
		if (buf[i] == SEPRATOR) {
			buf[i] = '\0';
			if (k == 0) {
				k++;
				cookie->sessid = buf + i + 1;
			} else {
				cookie->token = buf + i + 1;
				break;
			}
		}
	}
}

int bmy_cookie_gen(char *buf, size_t len, const struct cookie *cookie) {
	if (buf == NULL || len == 0)
		return -1;

	if (cookie == NULL) {
		sprintf(buf, "");
		return 0;
	}

	snprintf(buf, len, "%s%c%s%c%s",
		(cookie->userid == NULL) ? "" : cookie->userid,
		SEPRATOR,
		(cookie->sessid == NULL) ? "" : cookie->sessid,
		SEPRATOR,
		(cookie->token  == NULL) ? "" : cookie->token
	);
	buf[len - 1] = '\0';

	return strlen(buf);
}

