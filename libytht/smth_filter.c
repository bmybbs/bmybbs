#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include "ytht/fileop.h"
#include "ytht/mgrep.h"

char *CurrentFileName;

int
ytht_smth_reload_badwords(char *wordlistf, char *imgf)
{
	int fp;
	struct pattern_image *pattern_buf;
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
	ytht_mgrep_prepf(fp, &pattern_buf, &pattern_imagesize);

	flock(fp, LOCK_UN);
	close(fp);
	fp = open(imgf, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	if (fp == -1) {
		ytht_mgrep_releasepf(pattern_buf);
		return -1;
	}
	write(fp, pattern_buf, pattern_imagesize);
	close(fp);
	ytht_mgrep_releasepf(pattern_buf);
	return 0;
}

int
ytht_smth_filter_file(char *checkfile, struct mmapfile *badword_img)
{
	int retv;
	struct mmapfile mf = { ptr:NULL };
	ytht_mgrep_default_setting();
	CurrentFileName = checkfile;
	if (mmapfile(checkfile, &mf) == -1) {
		return 0;
	}
	retv = ytht_mgrep_mgrep_str(mf.ptr, mf.size, (struct pattern_image *) badword_img->ptr);
	mmapfile(NULL, &mf);
	return retv;
}

int
ytht_smth_filter_string(char *string, struct mmapfile *badword_img)
{
	int retv;
	ytht_mgrep_default_setting();
	CurrentFileName = "";
	retv = ytht_mgrep_mgrep_str(string, strlen(string), (struct pattern_image *) badword_img->ptr);
	return retv;
}

int ytht_smth_filter_article(char *title, char *filename, struct mmapfile *badword_img){
	return (ytht_smth_filter_string(title, badword_img) || ytht_smth_filter_file(filename, badword_img));
}
