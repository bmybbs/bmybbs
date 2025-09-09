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
#include <sys/mman.h>
#include "bbs_global_vars.h"
#include "bbsinc.h"
#include "stuff.h"
#include "bcache.h"

#define BUFSIZE (8192)

char bigbuf[10240];
int numtowrite;
int bug_possible = 0;

#if 0
static int
saverecords(filename, size, pos)
char *filename;
int size, pos;
{
	int fd;
	if (!bug_possible)
		return 0;
	if ((fd = open(filename, O_RDONLY)) == -1)
		return -1;
	if (pos > 5)
		numtowrite = 5;
	else
		numtowrite = 4;
	lseek(fd, (pos - numtowrite - 1) * size, SEEK_SET);
	read(fd, bigbuf, numtowrite * size);
	return 0;
}

static int
restorerecords(filename, size, pos)
char *filename;
int size, pos;
{
	int fd;
	if (!bug_possible)
		return 0;
	if ((fd = open(filename, O_WRONLY)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	lseek(fd, (pos - numtowrite - 1) * size, SEEK_SET);
	safewrite(fd, bigbuf, numtowrite * size);
	errlog("post bug poison set out!");
	flock(fd, LOCK_UN);
	bigbuf[0] = '\0';
	close(fd);
	return 0;
}
#endif

long
get_num_records(char *filename, int size)
{
	struct stat st;

	if (stat(filename, &st) == -1)
		return 0;
	//add by hace 2003.05.05
	char *s,buf[64];
	int num=st.st_size;
	strcpy(buf,filename);
	s=strrchr(buf,'/')+1;
	strcpy(s,".TOPFILE");
	if(stat(buf,&st)==-1)
		return num/size;
	else
		return (num+st.st_size)/size;
	//end
}

/**
 * ���ز������õ׵�������
 * @author interma
 * @date 2005.11.21
 */
long get_num_records_excludeBottom(char *filename, int size) {
	struct stat st;

	if (stat(filename, &st) == -1)
		return 0;
	int num=st.st_size;
	return num/size;
}

int
apply_record(char *filename, int (*fptr) (void *), int size)
{
	char *buf;
	int fd, sizeread, n, i;

	if ((buf = malloc(size * NUMBUFFER)) == NULL) {
		return -1;
	}
	if ((fd = open(filename, O_RDONLY, 0)) == -1) {
		free(buf);
		return -1;
	}
	while ((sizeread = read(fd, buf, size * NUMBUFFER)) > 0) {
		n = sizeread / size;
		for (i = 0; i < n; i++) {
			if ((*fptr) (buf + i * size) == QUIT) {
				close(fd);
				free(buf);
				return QUIT;
			}
		}
		if (sizeread % size != 0) {
			close(fd);
			free(buf);
			return -1;
		}
	}
	close(fd);
	free(buf);
	return 0;
}



int
get_records(char *filename, void *rptr, int size, int id, int number)
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

	if (n < number*size) {//hace
		char *s, buf[64];
		struct stat st;
		strcpy(buf, filename);
		s = strrchr(buf, '/') + 1;
		strcpy(s, ".TOPFILE");
		if ((stat(buf, &st) != -1) && st.st_size > 0) {
			if ((fd = open(buf, O_RDONLY, 0)) >= 0) {
				int size_read;
				while ((size_read = read(fd, rptr + n, size)) > 0) {
					n += size_read;
					if (n >= number * size)
						break;
				}
				close(fd);
			}
		}
	}
	return (n /size);
}

int
insert_record(char *fpath, void *data, int size, int pos, int num)
{
	int fd;
	off_t off, len;
	struct stat st;

	if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
		return -1;

	flock(fd, LOCK_EX);

	fstat(fd, &st);
	len = st.st_size;

	/* lkchu.990428: ernie patch ��� len=0 & pos>0 (�ڸտ�������Ŀ¼��ȥ���ϣ�ѡ��һ��) ʱ��д������ */
	off = len ? size * pos : 0;
	lseek(fd, off, SEEK_SET);

	size *= num;
	len -= off;
	if (len > 0) {
		fpath = (char *) malloc(pos = len + size);
		memcpy(fpath, data, size);
		read(fd, fpath + size, len);
		lseek(fd, off, SEEK_SET);
		data = fpath;
		size = pos;
	}

	write(fd, data, size);

	flock(fd, LOCK_UN);

	close(fd);

	if (len > 0)
		free(data);

	return 0;
}

#ifndef EXT_UTL

int
delete_range(char *filename, int id1, int id2)
{
	struct fileheader fhdr;
	char tmpfile[STRLEN], deleted[STRLEN];
	int fdr, fdw;
	int count;
	tmpfilename(filename, tmpfile, deleted);
	if (digestmode == 4 || digestmode == 5) {
		tmpfile[strlen(tmpfile) - 1] = (digestmode == 4) ? 'D' : 'J';
		deleted[strlen(deleted) - 1] = (digestmode == 4) ? 'D' : 'J';
	}

	if ((fdr = open(filename, O_RDONLY, 0)) == -1) {
		return -2;
	}

	if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0660)) == -1) {
		close(fdr);
		return -3;
	}

	if (digestmode == 4 || digestmode == 5) {
		close(fdr);
		unlink(tmpfile);
		close(fdw);
		return 0;
	}
	flock(fdw, LOCK_EX);

	count = 1;
	while (read(fdr, &fhdr, sizeof (fhdr)) == sizeof (fhdr)) {
		if (!fhdr.filetime) {
			count++;
			continue;
		}
		fhdr.owner[sizeof(fhdr.owner) - 1] = 0;
		if ((id2 == -1 && !(fhdr.accessed & FH_DEL) && !(fhdr.accessed & FH_MINUSDEL)) || //modify by mintbaggio for minus-numposts delete
				(id2 != -1 && (count < id1 || count > id2 || fhdr.accessed & FH_MARKED || fhdr.accessed & FILE_TOP1))) {
			if ((safewrite(fdw, &fhdr, sizeof (fhdr)) == -1)) {
				unlink(tmpfile);
				flock(fdw, LOCK_UN);
				close(fdw);
				close(fdr);
				return -4;
			}
		} else {
/* add by KCN for delete ranger backup, modified by ylsdd */
#ifdef BACK_DELETE_RANGE
			if (uinfo.mode != RMAIL && (digestmode == 4 || digestmode == 5)) {
				char fullpath[STRLEN * 2];
				snprintf(fullpath, sizeof(fullpath), "boards/%s/%s", currboard, fh2fname(&fhdr));
				unlink(fullpath);
			} else {
				if (uinfo.mode == RMAIL) {
					char fullpath[STRLEN];
					setmailfile_s(fullpath, sizeof(fullpath), currentuser.userid, fh2fname(&fhdr));
					deltree(fullpath);
				} else{
					cancelpost(currboard, currentuser.userid, &fhdr, 0);
					if((fhdr.accessed & FH_MINUSDEL) && (strncasecmp(fhdr.title, "���ϼ���", 8))){        //add by mintbaggio for minus-postnums delete
						char usrid[STRLEN];
						int owned = 0, IScurrent = 0, posttime;
						ytht_strsncpy(usrid, fhdr.owner, sizeof(usrid));
						if ( !strstr(usrid, ".") )
							IScurrent = !strcmp(usrid, currentuser.userid);
						posttime = fhdr.filetime;
						if( !IScurrent ) {
							owned = getuser(usrid);
							if( owned!=0 )
								if(posttime < lookupuser.firstlogin)
									owned = 0;
						}
						else owned =  posttime > currentuser.firstlogin;

						if (owned && IScurrent) {
							set_safe_record();
							if (currentuser.numposts > 0 ) currentuser.numposts--;
							substitute_record(PASSFILE,&currentuser,sizeof(currentuser),usernum);
						}
						else if (owned) {
							if (lookupuser.numposts > 0) lookupuser.numposts--;
							substitute_record(PASSFILE,&lookupuser,sizeof(struct userec),owned);
						}
					}
				}
			}
#endif
		}
		count++;
	}
	close(fdr);
	if (rename(filename, deleted) == -1) {
		flock(fdw, LOCK_UN);
		close(fdw);
		return -6;
	}
	if (rename(tmpfile, filename) == -1) {
		flock(fdw, LOCK_UN);
		close(fdw);
		return -7;
	}
	flock(fdw, LOCK_UN);
	close(fdw);
	return 0;
}
#endif

int
update_file(char *dirname, int size, int ent, int (*filecheck) (void *), void (*fileupdate) (void *))
{
	char abuf[BUFSIZE];
	int fd;

	if (size > BUFSIZE) {
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
					errlog("update err");
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
				errlog("update err");
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
