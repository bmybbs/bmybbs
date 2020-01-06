#include "bbs.h"

int
main(int argc,char *argv[])
{	
	FILE *fp;
	struct userec faint;
	if(argc!=3){
		printf("error,number of arguments!\n");
		return -1;
	}
	fp=fopen(MY_BBS_HOME "/.PASSWDS","r+");
	if(!fp){
		printf("error,can't open passwd file!\n");
		return -1;
	}
	if(!goodgbid(argv[1]))
		printf("warning,invalid userid %s\n",argv[1]);
	if(!goodgbid(argv[2]))
		printf("warning,invalid userid %s\n",argv[2]);
	while(fread(&faint,1,sizeof(faint),fp)==sizeof(faint)){
		if(strcmp(faint.userid,argv[1]))
			continue;
		strsncpy(faint.userid,argv[2],IDLEN+1);
		fseek(fp,- sizeof(faint),SEEK_CUR);
		fwrite(&faint,1,sizeof(faint),fp);
		fclose(fp);
		utime(MY_BBS_HOME "/.PASSFLUSH",NULL);
		return 0;
	}
	fclose(fp);
	printf("not found!\n");
	return 0;
}
