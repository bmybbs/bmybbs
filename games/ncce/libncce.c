//ecnegrevid 2001.7.20
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <newt.h>
#include <stdlib.h>
#include <sys/mman.h>
#define DICTPATH "/home/bbs/etc/ncce"

char *
mmapfile(char *fn)
{
	char *ptr;
	int fd;
	struct stat st;
	fd = open(fn, O_RDONLY);
	if (fd < 0)
		return NULL;
	if (fstat(fd, &st) < 0) {
		close(fd);
		return NULL;
	}
	if (!S_ISREG(st.st_mode)) {
		close(fd);
		return NULL;
	}
	if (st.st_size <= 0) {
		close(fd);
		return NULL;
	}
	ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	return ptr;
}

int
compare_word(char *word, char *item)
{
	char nword[100];
	int i, n = strlen(word);
	if (n >= 100)
		n = 99;
	for (i = 0; i < n && item[i] != 1; i++) ;
	n = i;
	strncpy(nword, item, n);
	nword[n] = 0;
	for (i = 0; i < n; i++)
		nword[i] += 30;
	return strncasecmp(word, nword, n);
}

void
cat_item(char *str, char *item, int maxlen)
{
	char nitem[100];
	int i, l = strlen(str);
	for (i = 0; i < 98; i++) {
		if (item[i] == 0) {
			nitem[i] = ' ';
		} else if (item[i] == 1) {
			break;
		} else
			nitem[i] = item[i] + 30;
	}
	nitem[i++] = '\n';
	nitem[i] = 0;
	if (l + i + 1 >= maxlen)
		return;
	strcat(str, nitem);
	return;
}

char *
search_dict(char *word)
{
	static char *ecidx = NULL, *eclib = NULL, *ceidx = NULL, *celib = NULL;
	char *idx, *lib;
	static char wordlist[2000];
	int total, nbegin, nend, i, retv;
	int *index;
	if (((unsigned char) *word) < 128) {
		if (ecidx == NULL) {
			ecidx = mmapfile(DICTPATH "/EC.IDX");
			eclib = mmapfile(DICTPATH "/EC.LIB");
		}
		idx = ecidx;
		lib = eclib;
	} else {
		if (ceidx == NULL) {
			ceidx = mmapfile(DICTPATH "/CE.IDX");
			celib = mmapfile(DICTPATH "/CE.LIB");
		}
		idx = ceidx;
		lib = celib;
	}
	wordlist[0] = 0;
	if (idx == NULL || lib == NULL) {
		strcpy(wordlist, "Can't read dict files.");
		return wordlist;
	}
	total = *(int *) idx;
	index = (int *) (idx + 4);
	nbegin = 0;
	nend = total - 1;
	while (nend - nbegin >= 50) {
		i = (nbegin + nend) / 2;
		retv = compare_word(word, lib + index[i]);
		if (retv > 0)
			nbegin = i;
		else
			nend = i;
	}
	for (i = 0; i < 200 && nbegin + i < total; i++) {
		retv = compare_word(word, lib + index[nbegin + i]);
		if (retv == 0)
			cat_item(wordlist, lib + index[nbegin + i],
				 sizeof (wordlist));
		else if (retv < 0)
			break;
	}
	if (wordlist[0] == 0)
		sprintf(wordlist, "没查到这个词: %.30s", word);
	return wordlist;
}
