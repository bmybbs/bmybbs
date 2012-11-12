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
#include "ythtbbs.h"

#define BUFSIZE (8192)

long
get_num_records(filename, size)
char *filename;
int size;
{
	struct stat st;

	if (stat(filename, &st) == -1)
		return 0;
	return (st.st_size / size);
}

void
toobigmesg()
{
/*
    prints( "record size too big!!\n" );
    oflush();
*/
}

int
get_record(filename, rptr, size, id)
char *filename;
char *rptr;
int size, id;
{
	int fd;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -2;
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) {
		close(fd);
		return -3;
	}
	if (read(fd, rptr, size) != size) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int
get_records(filename, rptr, size, id, number)
char *filename;
char *rptr;
int size, id, number;
{
	int fd;
	int n;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) {
		close(fd);
		return 0;
	}
	if ((n = read(fd, rptr, size * number)) == -1) {
		close(fd);
		return -1;
	}
	close(fd);
	return (n / size);
}

int
substitute_record(filename, rptr, size, id)
char *filename;
char *rptr;
int size, id;
{
#ifdef LINUX
	struct flock ldata;
	int retval;
#endif
	int fd;
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0660)) == -1)
		return -1;
#ifdef LINUX
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (id - 1);
	if ((retval = fcntl(fd, F_SETLKW, &ldata)) == -1) {
		report("reclock error");
		return -1;
	}
#else
	flock(fd, LOCK_EX);
#endif
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1)
		report("subrec seek err");
	if (safewrite(fd, rptr, size) != size)
		report("subrec write err");
#ifdef LINUX
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &ldata);
#else
	flock(fd, LOCK_UN);
#endif
	close(fd);
	return 0;
}

int
update_file(dirname, size, ent, filecheck, fileupdate)
char *dirname;
int size, ent;
int (*filecheck) ();
void (*fileupdate) ();
{
	char abuf[BUFSIZE];
	int fd;

	if (size > BUFSIZE) {
		toobigmesg();
		return -1;
	}
	if ((fd = open(dirname, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	if (lseek(fd, size * (ent - 1), SEEK_SET) != -1) {
		if (read(fd, abuf, size) == size)
			if ((*filecheck) (abuf)) {
				lseek(fd, -size, SEEK_CUR);
				(*fileupdate) (abuf);
				if (safewrite(fd, abuf, size) != size) {
					report("update err");
					flock(fd, LOCK_UN);
					close(fd);
					return -1;
				}
				flock(fd, LOCK_UN);
				close(fd);
				return 0;
			}
	}
	lseek(fd, 0, SEEK_SET);
	while (read(fd, abuf, size) == size) {
		if ((*filecheck) (abuf)) {
			lseek(fd, -size, SEEK_CUR);
			(*fileupdate) (abuf);
			if (safewrite(fd, abuf, size) != size) {
				report("update err");
				flock(fd, LOCK_UN);
				close(fd);
				return -1;
			}
			flock(fd, LOCK_UN);
			close(fd);
			return 0;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
	return -1;
}
