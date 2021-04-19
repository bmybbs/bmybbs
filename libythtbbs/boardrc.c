#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "ythtbbs/ythtbbs.h"

static void
shorter_brc(struct onebrc *brc)
{
	int i;
	if (brc->num < 2)
		return;
	if (brc->list[0] >= 2 && brc->list[brc->num - 1] == 1 && brc->list[brc->num - 2] == 2)
		brc->num--;
	for (i = brc->num - 1; i > 0; i--) {
		if (brc->list[i - 1] - brc->list[i] == 1)
			brc->num--;
		else
			break;
	}
}

static void
compress_brc(struct onebrc_c *brc_c, struct onebrc *brc)
{
	int i, diff;
	char *bits, *ptr = brc_c->data;
	bzero(brc_c, sizeof (*brc_c));
	strcpy(ptr, brc->board);
	ptr += strlen(brc->board) + 1;
	*ptr = brc->num;
	ptr++;
	*(unsigned *) ptr = brc->list[0];
	ptr += sizeof (unsigned);
	bits = ptr;
	ptr += ((brc->num - 1) * 2 + 7) / 8;
	for (i = 1; i < brc->num; i++) {
		diff = brc->list[i - 1] - brc->list[i];
		if (diff < 256) {
			*(unsigned char *) ptr = (unsigned char) diff;
			ptr += sizeof (unsigned char);
		} else if (diff < 256 * 256) {
			*(unsigned short *) ptr = (unsigned short) diff;
			ptr += sizeof (unsigned short);
			bits[(i - 1) / 4] |= 1 << ((i - 1) % 4);
		} else if (diff < 256 * 256 * 256) {
			ptr[0] = ((unsigned char *) &diff)[0];
			ptr[1] = ((unsigned char *) &diff)[1];
			ptr[2] = ((unsigned char *) &diff)[2];
			ptr += 3;
			bits[(i - 1) / 4] |= 1 << ((i - 1) % 4 + 4);
		} else {
			*(unsigned *) ptr = (unsigned) diff;
			ptr += sizeof (unsigned);
			bits[(i - 1) / 4] |= 1 << ((i - 1) % 4);
			bits[(i - 1) / 4] |= 1 << ((i - 1) % 4 + 4);
		}
	}
	if (brc->notetime) {
		*(int *) ptr = brc->notetime;
		ptr += sizeof (int);
	}
	brc_c->len = ptr - (char *) brc_c;
}

static void
uncompress_brc(struct onebrc *brc, struct onebrc_c *brc_c)
{
	int i, diff, bl, bh;
	char *ptr, *bits;
	brc->changed = 0;
	ptr = brc_c->data;
	ytht_strsncpy(brc->board, ptr, sizeof(brc->board));
	ptr += strlen(ptr) + 1;
	brc->num = *ptr & 0x7f;
	ptr++;
	brc->list[0] = *(unsigned *) ptr;
	ptr += sizeof (unsigned);
	bits = ptr;
	ptr += ((brc->num - 1) * 2 + 7) / 8;
	for (i = 1; i < brc->num; i++) {
		bl = bits[(i - 1) / 4] & (1 << ((i - 1) % 4));
		bh = bits[(i - 1) / 4] & (1 << (((i - 1) % 4) + 4));
		if (!bh) {
			if (!bl) {
				diff = *(unsigned char *) ptr;
				ptr += sizeof (unsigned char);
			} else {
				diff = *(unsigned short *) ptr;
				ptr += sizeof (unsigned short);
			}
		} else {
			if (!bl) {
				diff = 0;
				((unsigned char *) &diff)[0] = ptr[0];
				((unsigned char *) &diff)[1] = ptr[1];
				((unsigned char *) &diff)[2] = ptr[2];
				ptr += 3;
			} else {
				diff = *(unsigned *) ptr;
				ptr += sizeof (unsigned);
			}
		}
		brc->list[i] = brc->list[i - 1] - diff;
	}
	if (ptr - (char *) brc_c < brc_c->len)
		brc->notetime = *(int *) ptr;
	else
		brc->notetime = 0;
}

static int
brc_c_unreadt(struct onebrc_c *brc_c, int t)
{
	int i, diff, bl, bh, num, thist;
	char *ptr, *bits;

	ptr = brc_c->data;
	ptr += strlen(ptr) + 1;
	num = *ptr & 0x7f;
	ptr++;
	thist = *(unsigned *) ptr;
	if (t > thist)
		return 1;
	else if (t == thist)
		return 0;
	ptr += sizeof (unsigned);
	bits = ptr;
	ptr += ((num - 1) * 2 + 7) / 8;
	if (num > 4)
		num = 4;
	for (i = 1; i < num; i++) {
		bl = bits[(i - 1) / 4] & (1 << ((i - 1) % 4));
		bh = bits[(i - 1) / 4] & (1 << (((i - 1) % 4) + 4));
		if (!bh) {
			if (!bl) {
				diff = *(unsigned char *) ptr;
				ptr += sizeof (unsigned char);
			} else {
				diff = *(unsigned short *) ptr;
				ptr += sizeof (unsigned short);
			}
		} else {
			if (!bl) {
				diff = 0;
				((unsigned char *) &diff)[0] = ptr[0];
				((unsigned char *) &diff)[1] = ptr[1];
				((unsigned char *) &diff)[2] = ptr[2];
				ptr += 3;
			} else {
				diff = *(unsigned *) ptr;
				ptr += sizeof (unsigned);
			}
		}
		thist -= diff;
		if (t > thist)
			return 1;
		else if (t == thist)
			return 0;
	}
	return 0;
}

static void
settmpbrc(char *filename, char *userid)
{
	sprintf(filename, "%s/%s", PATHTMPBRC, userid);
}

void
brc_init(struct allbrc *allbrc, char *userid, char *filename)
{
	int fd;
	char filename1[80];
	allbrc->changed = 0;
	settmpbrc(filename1, userid);
	if ((fd = open(filename1, O_RDONLY)) < 0) {
		if (filename == NULL || (fd = open(filename, O_RDONLY)) < 0)
			return;
	}
	allbrc->size = read(fd, allbrc->brc_c, BRC_MAXSIZE);
	close(fd);
	if (allbrc->size < 0)
		allbrc->size = 0;
}

void
brc_fini(struct allbrc *allbrc, char *userid)
{
	int fd;
	char filename1[80];
	char tmpfile[80];
	if (!allbrc->changed)
		return;
	sprintf(filename1, "%s.tmp", userid);
	settmpbrc(tmpfile, filename1);
	if ((fd = open(tmpfile, O_WRONLY | O_CREAT, 0660)) < 0)
		return;
	write(fd, allbrc->brc_c, allbrc->size);
	close(fd);
	settmpbrc(filename1, userid);
	rename(tmpfile, filename1);
	allbrc->changed = 0;
}

void
brc_getboard(struct allbrc *allbrc, struct onebrc *brc, char *board)
{
	char *ptr, *ptr0;
	ptr0 = allbrc->brc_c + allbrc->size;
	for (ptr = allbrc->brc_c; ptr < ptr0;) {
		if (!((struct onebrc_c *) ptr)->len)
			break;
		if (ptr + ((struct onebrc_c *) ptr)->len > ptr0)
			break;
		if (!strncmp(((struct onebrc_c *) ptr)->data, board, BRC_STRLEN - 1)) {
			uncompress_brc(brc, (struct onebrc_c *) ptr);
			return;
		}
		ptr += ((struct onebrc_c *) ptr)->len;
	}
	strncpy(brc->board, board, BRC_STRLEN - 1);
	brc->changed = 0;
	brc->board[BRC_STRLEN - 1] = 0;
	brc->num = 1;
	brc->cur = 0;
	brc->list[0] = 1;
	brc->notetime = 0;
}

int
brc_unreadt_quick(struct allbrc *allbrc, char *board, int t)
{
	char *ptr, *ptr0;
	ptr0 = allbrc->brc_c + allbrc->size;
	for (ptr = allbrc->brc_c; ptr < ptr0;) {
		if (!((struct onebrc_c *) ptr)->len)
			break;
		if (ptr + ((struct onebrc_c *) ptr)->len > ptr0)
			break;
		if (!strncmp(((struct onebrc_c *) ptr)->data, board, BRC_STRLEN - 1)) {
			return brc_c_unreadt((struct onebrc_c *) ptr, t);
		}
		ptr += ((struct onebrc_c *) ptr)->len;
	}
	return 1;
}

void
brc_putboard(struct allbrc *allbrc, struct onebrc *brc)
{
	struct allbrc *tmpallbrc;
	char *ptr, *ptr1, *ptr0, *ptr10;
	int len;
	if (!brc->changed)
		return;
	brc->changed = 0;
	tmpallbrc = malloc(sizeof (struct allbrc));
	ptr1 = tmpallbrc->brc_c;
	ptr10 = tmpallbrc->brc_c + sizeof (tmpallbrc->brc_c);
	shorter_brc(brc);
	compress_brc((struct onebrc_c *) ptr1, brc);
	ptr1 += ((struct onebrc_c *) ptr1)->len;

	ptr0 = allbrc->brc_c + allbrc->size;
	for (ptr = allbrc->brc_c; ptr < ptr0;) {
		len = ((struct onebrc_c *) ptr)->len;
		if (len <= 0)
			break;
		if (ptr + len > ptr0)
			break;
		if (ptr1 + len > ptr10)
			break;
		if (!strncmp(((struct onebrc_c *) ptr)->data, brc->board, BRC_STRLEN - 1)) {
			ptr += len;
			continue;
		}
		memcpy(ptr1, ptr, len);
		ptr += len;
		ptr1 += len;
	}
	tmpallbrc->size = ptr1 - tmpallbrc->brc_c;
	tmpallbrc->changed = 1;
	memcpy(allbrc, tmpallbrc, sizeof (struct allbrc));
	free(tmpallbrc);
}

static int
brc_locate(struct onebrc *brc, int t)
{
	if (brc->num == 0) {
		brc->cur = 0;
		return 0;
	}
	if (brc->cur >= brc->num)
		brc->cur = brc->num - 1;
	if (t <= brc->list[brc->cur]) {
		while (brc->cur < brc->num) {
			if (t == brc->list[brc->cur])
				return 1;
			if (t > brc->list[brc->cur])
				return 0;
			brc->cur++;
		}
		return 0;
	}
	while (brc->cur > 0) {
		if (t < brc->list[brc->cur - 1])
			return 0;
		brc->cur--;
		if (t == brc->list[brc->cur])
			return 1;
	}
	return 0;
}

static void
brc_insert(struct onebrc *brc, int t)
{
	if (brc->num < BRC_MAXNUM)
		brc->num++;
	if (brc->cur >= brc->num)
		return;
	brc->changed = 1;
	memmove(&brc->list[brc->cur + 1], &brc->list[brc->cur],
		sizeof (brc->list[0]) * (brc->num - brc->cur - 1));
	brc->list[brc->cur] = t;
}

static void
brc_set(struct onebrc *brc, int t)
{
	if (brc->num && brc->cur >= brc->num)
		return;
	brc->changed = 1;
	brc->list[brc->cur] = t;
	brc->num = brc->cur + 1;
}

void
brc_addlistt(struct onebrc *brc, int t)
{
	if (brc_unreadt(brc, t)) {
		brc_insert(brc, t);
	}
}

int
brc_unreadt(struct onebrc *brc, int t)
{
	if (brc_locate(brc, t))
		return 0;
	if (brc->num <= 0)
		return 1;
	if (brc->cur < brc->num)
		return 1;
	return 0;
}

void
brc_clearto(struct onebrc *brc, int t)
{
	brc_locate(brc, t);
	brc_set(brc, t);
}

int UNREAD(const struct fileheader *fh, struct onebrc *brc) {
	return brc_unreadt(brc, fh->edittime ? fh->edittime : fh->filetime);
}

void SETREAD(const struct fileheader *fh, struct onebrc *brc) {
	brc_addlistt(brc, fh->edittime ? fh->edittime : fh->filetime);
}

