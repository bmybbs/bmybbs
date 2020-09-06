#include "bbs.h"
#define MAXDEPTH 6
int Name_ok=0;
char tmp[256], buf[256];

main() {
	FILE *fp, *fp2;
	chdir(MY_BBS_HOME);
	fp=fopen("0Announce/.Search", "r");
	while(fscanf(fp, "%s %s", tmp, buf)>0) {
		if(strchr(tmp, '*')) continue;
		sprintf(tmp, "%s/0Announce/%s", MY_BBS_HOME, buf);
		if(chdir(tmp)==-1) continue;
		fp2=fopen("index","w");
		if(fp2!=0)
		{
			Name_ok=0;
			add_index(fp2);
			fclose(fp2);
		}
	}
	fclose(fp);
}
add_index(FILE *fp2) {
	do_index("[[1;32m±¾°æ¾«»ªÇøË÷Òý[m]", ".", 1,fp2);
	if(!Name_ok) {
		FILE *fp;
		fp=fopen(".Names", "a");
		if(fp!=0) {
			fprintf(fp, "Name=±¾°æ¾«»ªÇøË÷Òý\nPath=~/index\n");
			fclose(fp);
		}
	}
}

do_index(char *title, char *path, int dep,FILE *fp2) {
	int m;
	int HIDE=0;
	struct stat buf;
	FILE *fp;
	unsigned char buf1[256], title1[256], path1[256], *flag="";
	if (dep >= MAXDEPTH)
		return;
	if(lstat(path, &buf)==-1) return;
	if ((!S_ISDIR(buf.st_mode)) && (!S_ISREG(buf.st_mode)))
		return;
	for(m=1; m<dep-1; m++) fprintf(fp2,"©¦");
	if(strcmp(path, ".")) fprintf(fp2,"©À");
	if(abs(time(0)-buf.st_mtime)<7*86400) flag=" [1;31mnew[1;33m![m"; 
	if(buf.st_mode & S_IFDIR) {
	//	fprintf(fp2,"©¦\n");
		fprintf(fp2,"©Ð[[1;37mÄ¿Â¼¿ªÊ¼[m] %s%s\n", title, flag);
		sprintf(buf1, "%s/.Names", path);
		fp=fopen(buf1, "r");
		if(fp!=0) { 
			while(fgets(buf1, 80, fp)>0) {
				if(!strncmp(buf1, "Name=", 5)) {
					sprintf(title1, "%s", buf1+5);
					if(strstr(title1 + 38,"(BM: SYSOPS)") ||
						strstr(title1 + 38,"(BM: BMS)")||
						!strncmp(title1,"<HIDE>",6))
						HIDE=1;
					else
						HIDE=0;
					title1[38]=0;
					for(m=0; m<strlen(title1); m++) if(title1[m]<27) title1[m]=0;
					fgets(buf1, 256, fp);
					if(!strncmp("Path=~/", buf1, 6)) {
						if(HIDE) continue;
						if(!strncmp(buf1, "Path=~/index", 11) && dep==1) Name_ok=1;
						sprintf(path1, "%s/%s", path, buf1+7);
						for(m=0; m<strlen(path1); m++) if (path1[m]<27) path1[m]=0;
						do_index(title1, path1, dep+1,fp2);
					}
				}
			}
			fclose(fp);
		}
		for(m=1; m<dep; m++) fprintf(fp2,"©¦");
		fprintf(fp2,"©¸[[1;37mÄ¿Â¼½áÊø[m] %s\n", title);
	} else
		fprintf(fp2,"[[1;36mÎÄ¼þ[m] %s%s\n", title, flag);
}
