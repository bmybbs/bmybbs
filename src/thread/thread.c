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
	Copyright (C) 1999, Zhou Lin, kcn@cic.tsinghua.edu.cn

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

char tname[STRLEN];
char fname[STRLEN];

int
generate_title(char *fname, char *tname)
{
	struct fileheader mkpost, *ptr1, *ptr2;
	struct flock ldata;
	int fd, size = sizeof (struct fileheader), total, i, j, count = 0, hasht;
	char *t;
	struct mmapfile mf = { .ptr = NULL };
	struct hashstruct {
		int index, data;
	} *hashtable;
	int *index, *next;

	if ((fd = open(tname, O_WRONLY | O_CREAT, 0664)) == -1) {
		return -1;	/* 创建文件发生错误 */
	}
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = 0;
	ldata.l_start = 0;
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		close(fd);
		return -1;	/* lock error */
	}

	index = NULL;
	hashtable = NULL;
	next = NULL;
	MMAP_TRY {
		if (mmapfile(fname, &mf) == -1) {
			ldata.l_type = F_UNLCK;
			fcntl(fd, F_SETLKW, &ldata);
			close(fd);
			MMAP_RETURN(-1);
		}
		total = mf.size / size;
		hasht = total * 8 / 5;
		hashtable = (struct hashstruct *) malloc(sizeof (*hashtable) * hasht);
		index = (int *) malloc(sizeof (int) * total);
		next = (int *) malloc(sizeof (int) * total);
		memset(hashtable, 0xFF, sizeof (*hashtable) * hasht);
		memset(index, 0, sizeof (int) * total);
		ptr1 = (struct fileheader *) (mf.ptr);
		for (i = 0; i < total; i++, ptr1++) {
			int l = 0, m;

			if (ptr1->thread == ptr1->filetime)
				l = i;
			else {
				l = ptr1->thread % hasht;
				while (hashtable[l].index != ptr1->thread && hashtable[l].index != -1) {
					l++;
					if (l >= hasht)
						l = 0;
				}
				if (hashtable[l].index == -1)
					l = i;
				else
					l = hashtable[l].data;
			}
			if (l == i) {
				l = ptr1->thread % hasht;
				while (hashtable[l].index != -1) {
					l++;
					if (l >= hasht)
						l = 0;
				}
				hashtable[l].index = ptr1->thread;
				hashtable[l].data = i;
				index[i] = i;
				next[i] = 0;
			} else {
				m = index[l];
				next[m] = i;
				next[i] = 0;
				index[l] = i;
				index[i] = -1;
			}
		}
		ptr1 = (struct fileheader *) (mf.ptr);
		for (i = 0; i < total; i++, ptr1++)
			if (index[i] != -1) {

				write(fd, ptr1, size);
				count++;
				j = next[i];
				while (j != 0) {
					ptr2 = (struct fileheader *) (mf.ptr + j * size);
					memcpy(&mkpost, ptr2, sizeof (mkpost));
					t = ptr2->title;
					if (!strncmp(t, "Re:", 3))
						t += 4;
					sprintf(mkpost.title, "Re: %s", t);
					write(fd, &mkpost, size);
					count++;
					j = next[j];
				}
			}

		free(index);
		free(next);
		free(hashtable);
	}
	MMAP_CATCH {
		ldata.l_type = F_UNLCK;
		fcntl(fd, F_SETLKW, &ldata);
		close(fd);
		mmapfile(NULL, &mf);
		if (index)
			free(index);
		if (next)
			free(next);
		if (hashtable)
			free(hashtable);
		MMAP_RETURN(-1);
	}
	MMAP_END mmapfile(NULL, &mf);
	ftruncate(fd, count * size);
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &ldata);	/* 退出互斥区域 */
	close(fd);
	return 0;
}

int main(int argc, char *argv[]) {
	char dname[STRLEN];
	char buf[256];
	struct stat st1, st2;

	if (argc < 2)
		return 0;

	umask(007);
	snprintf(dname, sizeof(dname), "boards/%s/%s", argv[1], DOT_DIR);
	snprintf(fname, sizeof(fname), "boards/%s/%s2", argv[1], DOT_DIR);
	snprintf(tname, sizeof(tname), "boards/%s/%s", argv[1], THREAD_DIR);

	if (stat(dname, &st1) == -1)
		return -1;
	if (stat(tname, &st2) != -1) {
		if (st2.st_mtime >= st1.st_mtime)
			return -1;
	}

	unlink(tname);
	sprintf(buf, "cp %s %s 1>/dev/null 2>/dev/null", dname, fname);
	system(buf);
	generate_title(fname, tname);
	unlink(fname);
	return 0;
}

