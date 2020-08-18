#include "bbs.h"
#include "stdio.h"

int silentmode = 0;
int id_boards(char *id);

int main(int argn, char *(argv[]))
{
	char str[250], *id;
	if (argn > 1) {
		if (strchr(argv[1], 's') != NULL)
			silentmode = 1;
	}
	while (1) {
		if (!silentmode)
			printf("请输入所要查询的ID：");
		if (fgets(str, 250, stdin) == NULL)
			break;
		id = strtok(str, ",: ;|&()\r\n\0");
		if (id == NULL)
			continue;
		id_boards(id);
	}

	return 0;
}

int
id_boards(char *id)
{
	FILE *fd1;
	struct boardheader rec;
	int size1 = sizeof (rec), i = 0;
	if (!silentmode)
		printf("%s担任版主/版副的版面：", id);
	else
		printf("%-15s ", id);
	fd1 = fopen(MY_BBS_HOME "/.BOARDS", "r");
	while (fread(&rec, size1, 1, fd1) == 1) {
		if (!(rec.level & PERM_POSTMASK) && !(rec.level & PERM_NOZAP)
		    && rec.level != 0)
			continue;
		if (rec.clubnum != 0 && (!(rec.flag & CLUBTYPE_FLAG)))
			continue;

		if (chk_BM_id(id, &rec)) {
			printf("%s%s", i == 0 ? "" : " ", rec.filename);
			i++;
		}
	}
	printf(i == 0 ? "一个都没有。\n" : "\n");
	fclose(fd1);
	return 0;
}
