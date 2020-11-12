#include <stdio.h>
#include <string.h>
#include "ytht/strlib.h"
#include "bbslib.h"
static struct allbrc allbrc;
static char allbrcuser[STRLEN];
static struct onebrc *pbrc, brc;

int
readuserallbrc(char *userid, int must)
{
	char buf[STRLEN];
	if (!userid)
		return 0;
	if (!loginok||isguest) {
		snprintf(buf, sizeof (buf), "guest.%s", fromhost);
		if (!must && !strncmp(allbrcuser, buf, sizeof (allbrcuser)))
			return 0;
		ytht_strsncpy(allbrcuser, buf, sizeof(allbrcuser));
		brc_init(&allbrc, allbrcuser, NULL);

	} else {
		if (!must && !strncmp(allbrcuser, userid, sizeof (allbrcuser)))
			return 0;
		sethomefile_s(buf, sizeof(buf), userid, "brc");
		ytht_strsncpy(allbrcuser, userid, sizeof(allbrcuser));
		brc_init(&allbrc, userid, buf);
	}
	return 0;
}

void
brc_update(char *userid)
{
	if (!pbrc->changed)
		return;
	readuserallbrc(userid, 0);
	brc_putboard(&allbrc, pbrc);
	if (isguest || !loginok) {
		char str[STRLEN];
		sprintf(str, "guest.%s", fromhost);
		brc_fini(&allbrc, str);
	} else
		brc_fini(&allbrc, userid);
}

int
brc_initial(char *userid, char *boardname)
{
	if (u_info)
		pbrc = &u_info->brc;
	else {
		pbrc = &brc;
		bzero(&brc, sizeof (brc));
	}
	if (boardname && !strncmp(boardname, pbrc->board, sizeof (pbrc->board)))
		return 0;
	readuserallbrc(userid, 1);
	if (boardname)
		brc_getboard(&allbrc, pbrc, boardname);
	return 0;
}

void
brc_add_read(struct fileheader *fh)
{
	SETREAD(fh, pbrc);
}

void
brc_add_readt(int t)
{
	brc_addlistt(pbrc, t);
}

int
brc_un_read(struct fileheader *fh)
{
	return UNREAD(fh, pbrc);
}

void
brc_clear()
{
	brc_clearto(pbrc, time(NULL));
}

int
brc_un_read_time(int ftime)
{
	return brc_unreadt(pbrc, ftime);
}

