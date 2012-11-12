#include "ythtbbs.h"
#include <sys/file.h>
#include <sys/mman.h>
#include <signal.h>

int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
int ONLYCOUNT, num_of_matched, total_line;
char *CurrentFileName;

extern int prepf(int fp, void **patternbuf, size_t * patt_image_len);
extern int mgrep(int fp, void *patternbuf);
extern int mgrep_str(char *data, int len, void *patternbuf);
extern void releasepf(void *patternbuf);

int
reload_badwords(char *wordlistf, char *imgf)
{
	int fp;
	void *pattern_buf;
	size_t pattern_imagesize;
	fp = open(wordlistf, O_RDONLY);
	if (fp == -1)
		return -1;
	flock(fp, LOCK_EX);
/*	if (file_isfile(imgf)) {
		flock(fp, LOCK_UN);
		close(fp);
		return 0;
	}*/
	prepf(fp, &pattern_buf, &pattern_imagesize);

	flock(fp, LOCK_UN);
	close(fp);
	fp = open(imgf, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	if (fp == -1) {
		releasepf(pattern_buf);
		return -1;
	}
	write(fp, pattern_buf, pattern_imagesize);
	close(fp);
	releasepf(pattern_buf);
	return 0;
}

static void
default_setting()
{
	WHOLELINE = 0;
	NOUPPER = 1;
	INVERSE = 0;
	FILENAMEONLY = 1;
	WORDBOUND = 0;
	SILENT = 1;
	FNAME = 1;
	ONLYCOUNT = 0;

	num_of_matched = 0;
}

int
filter_file(char *checkfile, struct mmapfile *badword_img)
{
	int retv;
	struct mmapfile mf = { ptr:NULL };
	default_setting();
	CurrentFileName = checkfile;
	if (mmapfile(checkfile, &mf) == -1) {
		return 0;
	}
	retv = mgrep_str(mf.ptr, mf.size, badword_img->ptr);
	mmapfile(NULL, &mf);
	return retv;
}

int
filter_string(char *string, struct mmapfile *badword_img)
{
	int retv;
	default_setting();
	CurrentFileName = "";
	retv = mgrep_str(string, strlen(string), badword_img->ptr);
	return retv;
}

int filter_article(char *title, char *filename, struct mmapfile *badword_img){
	return (filter_string(title, badword_img) || filter_file(filename, badword_img)); 
}
