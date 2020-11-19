/*
        一个将用户热键恢复到系统默认值的程序
        mintbaggio@BMY 2004.3.24
*/

#include "bbs.h"
#define Ctrl(c)         ( c & 037 )
#define KEY_TAB         9

int key[] = {
	'_', 'r', 'u', 'd', 'D', 'm', 't', 'n', 'E', 'Y', Ctrl('G'), Ctrl('T'), Ctrl('Y'), 
	'.', '>', 'g', 'L', 'T', 's', Ctrl('C'), Ctrl('P'), 'c', 'o', 'F', 'U', Ctrl('R'), 
	'i', 'I', 'R', 'v', 'M', 'W', Ctrl('W'), 'h', KEY_TAB, 'z', 'x', 'X', 'a', 'A', 
	'/', '?', '\'','\"',']', '[', Ctrl('D'), Ctrl('A'), '^', '\\','=', 'p', Ctrl('U'), 
	'b', '!', 'S', 'f', 'e', Ctrl('E'), Ctrl('K'), ',', 'K', ';', Ctrl('N'), 'C', Ctrl('S'), 
	Ctrl('X'), 'Z', 'B', 'l', '#', '\0'
};

int setdefaultkey();
char* setuserfile(char *buf, const char *userid, const char *filename);
int savekeys( char *name);

int main()
{
	setdefaultkey();
	return 0;
}

int setdefaultkey()
{
	FILE *pass_fp, *log_fp;
	char tempname[255];
	struct userec* check_user;
	
	chdir(MY_BBS_HOME);
	pass_fp = fopen(PASSFILE, "r");
	log_fp = fopen("setdefaultkey.log", "a");
	check_user = (struct userec*)malloc(sizeof(struct userec));
	while(fread(check_user, sizeof(struct userec), 1, pass_fp)==1){
	//	if(!strcmp(check_user->userid, "mint")){
		if(!check_user->userid[0])
			continue;
		if(!isalpha(check_user->userid[0]))
			continue;
		setuserfile(tempname, check_user->userid, "readkey");
		if(!savekeys(tempname))
			printf("fail");
		else{
			printf("change %s's readkey\n", check_user->userid);
			fprintf(log_fp, "chang %s's readkey\n", check_user->userid);
	//	}
		}
	}
	free(check_user);
	fclose(pass_fp);
	fclose(log_fp);
	return 0;
}

char* setuserfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}

int savekeys(char *name)
{
	int i;
	FILE *fp;
	fp = fopen(name, "w");
	if (fp == NULL)
		return 0;
	i = 0;
	while (key[i]!='\0') {
		fwrite(&(key[i]), sizeof (int), 1, fp);
		i++;
	}
	fclose(fp);
	return 1;
}

