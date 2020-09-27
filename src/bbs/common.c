#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

static void invertl(char *lp);

static void
invertl(char *lp)
{
	char ch;
	ch = lp[0];
	lp[0] = lp[3];
	lp[3] = ch;
	ch = lp[1];
	lp[1] = lp[2];
	lp[2] = ch;
}

int
cmpIP(char *left, char *right)
{
	struct in_addr inaddra, inaddrb;
	if (inet_aton(left, &inaddra) != 0 && inet_aton(right, &inaddrb) != 0) {
		invertl((char *) (&inaddra.s_addr));
		invertl((char *) (&inaddrb.s_addr));
		return inaddra.s_addr - inaddrb.s_addr;
	} else
		return strcasecmp(left, right);
}
