#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bmy/cookie.h"

static const char SEPRATOR = '-';

void bmy_cookie_parse(char *buf, struct bmy_cookie *cookie) {
	int i, j, k;
	cookie->userid = buf;

	if (buf == NULL || buf[0] == '\0')
		return;

	for (i = 0, j = strlen(buf), k = 0; i < j; i++) {
		if (buf[i] == SEPRATOR) {
			buf[i] = '\0';
			switch (k) {
				case 0:
					cookie->sessid = buf + i + 1;
					break;
				case 1:
					cookie->token = buf + i + 1;
					break;
				case 2:
					cookie->extraparam = buf + i + 1;
					break;
			}

			if (k == 2) break;
			else        k++;
		}
	}
}

int bmy_cookie_gen(char *buf, size_t len, const struct bmy_cookie *cookie) {
	if (buf == NULL || len == 0)
		return -1;

	if (cookie == NULL) {
		sprintf(buf, "");
		return 0;
	}

	snprintf(buf, len, "%s%c%s%c%s%c%s",
		(cookie->userid == NULL) ? "" : cookie->userid,
		SEPRATOR,
		(cookie->sessid == NULL) ? "" : cookie->sessid,
		SEPRATOR,
		(cookie->token  == NULL) ? "" : cookie->token,
		SEPRATOR,
		(cookie->extraparam == NULL) ? "" : cookie->extraparam
	);
	buf[len - 1] = '\0';

	return strlen(buf);
}

