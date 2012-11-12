//usage: lynx http://serve.cei.gov.cn/sl/sl01/l01index.htm -dump|/home/bbs/bin/tianqi
#include <stdio.h>
#include <string.h>
#include "../include/config.h"
#define MAXLINES 30

main()
{
	char linea[400], lineb[400], *prefix = "\033[1;44m[ÍøÓÑÍÆ¼ö]", *ptr;
	char lines[MAXLINES][400];
	char board[18], author[13], title[40];
	int i, j, nlines;
	FILE *fp0, *fp1;
	fgets(linea, sizeof (linea), stdin);
	fgets(lineb, sizeof (lineb), stdin);
	for (i = 0; i < MAXLINES; i++) {
		if (fgets(linea, sizeof (linea), stdin) == NULL)
			break;
		if (fgets(lineb, sizeof (lineb), stdin) == NULL)
			break;
		sscanf(linea + 45, "%17s", board);
		sscanf(linea + 119, "%s", author);
		title[sizeof (title) - 1] = '\0';
		memmove(title, lineb + 27, sizeof (title) - 1);
		sprintf(lines[i],
			"%s\033[32m%-12.12s\033[36m%38.38s\033[35m%17.17s°æ",
			prefix, author, title, board);
		strcat(lines[i], "\033[m");
		//printf("\n%d%s\n", i, lines[0]);
		//if(!strcmp("sex", board)) i--;
	}
	nlines = i;
	if (nlines == 0)
		return;
	fp0 = fopen(MY_BBS_HOME "/etc/endline", "r");
	if (fp0 == NULL)
		return;
	fp1 = fopen(MY_BBS_HOME "/etc/endline.new", "w");
	//fp1=fopen("endlinttest.txt","w");
	if (fp1 == NULL)
		return;
	i = 0;
	j = 0;
	//printf("###%s\n", lines[0]);
	while (fgets(linea, sizeof (linea), fp0) != NULL) {
		if (strncmp(linea, prefix, strlen(prefix)) == 0) {
			fprintf(fp1, "%s\n", lines[j]);
			//printf("%s\n",lines[j]);
			j++;
			if (j >= nlines)
				j = 0;
		} else
			fprintf(fp1, "%s", linea);
	}
	fclose(fp0);
	fclose(fp1);
	rename(MY_BBS_HOME "/etc/endline.new", MY_BBS_HOME "/etc/endline");
}
