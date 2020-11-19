/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
#include "smth_screen.h"
#include "goodbye.h"
#include "bbs_global_vars.h"

typedef struct {
	char *match;
	char *replace;
} tag_logout;

int countlogouts(char *filename) {
	FILE *fp;
	char buf[256];
	int count = 0;

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;

	while (fgets(buf, 255, fp) != NULL) {
		if (strstr(buf, "@logout@") || strstr(buf, "@login@"))
			count++;
	}
	fclose(fp);
	return count + 1;
}

void user_display(char *filename, int number, int mode) {
	FILE *fp;
	char buf[256];
	int count = 1;

	clear();
	move(1, 0);
	if ((fp = fopen(filename, "r")) == NULL)
		return;
	while (fgets(buf, 255, fp) != NULL) {
		if (strstr(buf, "@logout@") || strstr(buf, "@login@")) {
			count++;
			continue;
		}
		if (count == number) {
			if (mode == YEA)
				showstuff(buf);
			else {
				prints("%s", buf);
			}
		} else if (count > number)
			break;
		else
			continue;
	}
	fclose(fp);
	return;
}

void showstuff(char *buf) {
	extern time_t login_start_time;
	int frg, i, matchfrg, strlength, cnt, tmpnum;
	static char numlogins[10], numposts[10], rgtday[35], lasttime[35],
	    thistime[35], lastlogout[35], stay[10], alltime[20], ccperf[20],
	    perf[10], exp[10], ccexp[20];
	char buf2[STRLEN], *ptr, *ptr2;
	time_t now;

	static const tag_logout loglst[] = {
		{"userid", currentuser.userid},
		{"username", currentuser.username},
		//{"realname",     currentuser.realname},
		{"address", currentuser.address},
		{"email", currentuser.email},
		{"ip", currentuser.ip},
		{"realemail", currentuser.realmail},
		//{"ident",        currentuser.ident},
		{"rgtday", rgtday},
		{"log", numlogins},
		{"pst", numposts},
		{"lastlogin", lasttime},
		{"lasthost", currentuser.lasthost},
		{"lastlogout", lastlogout},
		{"now", thistime},
		{"bbsname", MY_BBS_NAME},
		{"stay", stay},
		{"alltime", alltime},
		{"exp", exp},
		{"cexp", ccexp},
		{"perf", perf},
		{"cperf", ccperf},
		{NULL, NULL}
	};
	if (!strchr(buf, '$')) {
		prints("%s", buf);
		return;
	}
	now = time(0);
	tmpnum = countexp(&currentuser);
	sprintf(exp, "%d", tmpnum);
	strcpy(ccexp, charexp(tmpnum));
	tmpnum = countperf(&currentuser);
	sprintf(perf, "%d", tmpnum);
	strcpy(ccperf, cperf(tmpnum));
	// %ld小时%ldx分钟
	sprintf(alltime, "%ld\xD0\xA1\xCA\xB1%ld\xB7\xD6\xD6\xD3", (long int) (currentuser.stay / 3600),
		(long int) ((currentuser.stay / 60) % 60));
	sprintf(rgtday, "%24.24s", ctime(&currentuser.firstlogin));
	sprintf(lasttime, "%24.24s", ctime(&currentuser.lastlogin));
	sprintf(lastlogout, "%24.24s", ctime(&currentuser.lastlogout));
	sprintf(thistime, "%24.24s", ctime(&now));
	sprintf(stay, "%ld", (long int) ((time(0) - login_start_time) / 60));
	sprintf(numlogins, "%d", currentuser.numlogins);
	sprintf(numposts, "%d", currentuser.numposts);

	frg = 1;
	ptr2 = buf;
	do {
		if ((ptr = strchr(ptr2, '$'))) {
			matchfrg = 0;
			*ptr = '\0';
			prints("%s", ptr2);
			ptr += 1;
			for (i = 0; loglst[i].match != NULL; i++) {
				if (strstr(ptr, loglst[i].match) == ptr) {
					strlength = strlen(loglst[i].match);
					ptr2 = ptr + strlength;
					for (cnt = 0; *(ptr2 + cnt) == ' ';
					     cnt++) ;
					sprintf(buf2, "%-*.*s",
						cnt ? strlength +
						cnt : strlength + 1,
						strlength + cnt,
						loglst[i].replace);
					prints("%s", buf2);
					ptr2 += (cnt ? (cnt - 1) : cnt);
					matchfrg = 1;
					break;
				}
			}
			if (!matchfrg) {
				prints("$");
				ptr2 = ptr;
			}
		} else {
			prints("%s", ptr2);
			frg = 0;
		}
	}
	while (frg);
	return;
}
