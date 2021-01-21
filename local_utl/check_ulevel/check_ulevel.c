/*
	一个权限检查程序，主要是因为BMY老站的用户都拥有PERM_SPECIAL1权限，不得不检查一下所有用户的
	权限，将不应该有这个权限的用户取消该权限。
	mintbaggio@BMY 2004.3.10
*/

#include "bbs.h"

#define LOG_FILE		"check_ulevel.log"
#define FILE_NAME		"check_ulevel.special"
#define SPECIAL_NUM		50
#define OPENFILE_ERROR		1
#define WRITEFILE_ERROR		2
//#define OPENFILE_ERROR2	2

char special_userid[SPECIAL_NUM][IDLEN+2];				//special userid

int get_userid();
int do_check();
int is_specialid(char* userid);

int main(int argc, char* argv[])
{
	if(get_userid())
		printf("unable to open file %s\n", FILE_NAME);
	switch(do_check()){
		case OPENFILE_ERROR:
			printf("unable to open file .PASSWDS\n");
			break;
		case WRITEFILE_ERROR:
			printf("unable to write to file .PASSWDS");
			break;
	}

	return 0;
}

int get_userid()
{
	FILE* fp;					//fp for file that do have PERM_SPECIAL1
	char buf[IDLEN+2];
	int i;

	fp = fopen(FILE_NAME, "r");
	if(fp == NULL)
		return OPENFILE_ERROR;

	for(i=0; i<SPECIAL_NUM; i++)
		bzero(special_userid[i], IDLEN+2);

	for(i=0; (i<SPECIAL_NUM)&&fgets(buf, IDLEN+1, fp); i++){
		buf[strlen(buf)-1] = 0;                  //get rid of '\n'
		strcpy(special_userid[i], buf);
	}
	fclose(fp);
	return 0;
}

int do_check()
{
	FILE* fp, *logfp;					//fp of .PASSWDS
	struct userec* check_user;

	chdir(MY_BBS_HOME);
	fp = fopen(PASSFILE, "r+");
	logfp = fopen(LOG_FILE, "a");
	if(fp == NULL || logfp == NULL) {
		if (fp) fclose(fp);
		if (logfp) fclose(logfp);
		return OPENFILE_ERROR;
	}
	check_user = (struct userec*)malloc(sizeof(struct userec));

	while(fread(check_user, sizeof(struct userec), 1, fp)==1){
		if(!check_user->userid[0])
			break;
		if(is_specialid(check_user->userid))
			continue;
		if(check_user->userlevel & PERM_SPECIAL1){
			printf("user %s has PERM_SPECIAL1\n", check_user->userid);
			fprintf(logfp, "user %s has PERM_SPECIAL1\n", check_user->userid);
			check_user->userlevel &= ~PERM_SPECIAL1;
			fseek(fp, -sizeof(struct userec), SEEK_CUR);
			if(fwrite(check_user, sizeof(struct userec), 1, fp)!=1)
				return WRITEFILE_ERROR;
			printf("user %s's level has been changed\n", check_user->userid);
			fprintf(logfp, "user %s's level has been changed(%s)\n", check_user->userid, ytht_ctime(time(NULL)));
		}
	}
	fclose(fp);
	fclose(logfp);
	return 0;
}

int is_specialid(char* userid)
{
	int i;

	for(i=0; i<SPECIAL_NUM; i++){
		if(!special_userid[i][0])
			break;
		if(!strcmp(userid, special_userid[i]))
			return 1;
	}
	return 0;
}

