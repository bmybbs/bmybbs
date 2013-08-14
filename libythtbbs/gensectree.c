#include <string.h>
#include <stdio.h>
#include "ythtbbs.h"
#define MAXPAIRS 300
struct {
	int used;
	char seccode[20];
	char title[30];
} pairs[MAXPAIRS];

int npairs;
struct sectree rootsec = { title:MY_BBS_NAME, parent:NULL };

int
readseclist(char *filename)
{
	FILE *fp;
	char buf[256], *ptr;
	int i;
	fp = fopen(filename, "rt");
	for (i = 0; i < MAXPAIRS; i++) {
		if (fgets(buf, sizeof (buf), fp) == NULL)
			break;
		if (buf[0] == '%')
			continue;
		ptr = strtok(buf, " \t\r\n");
		if (!ptr)
			continue;
		strsncpy(pairs[i].seccode, ptr, sizeof (pairs[i].seccode));
		ptr = strtok(NULL, "\t\r\n");
		if (!ptr)
			continue;
		strsncpy(pairs[i].title, ptr, sizeof (pairs[i].title));
		pairs[i].used = 0;
	}
	npairs = i;
	return 0;
}

char *
getintrostr(char *basestr)
{
	char buf[80];
	int i;
	sprintf(buf, "*%s", basestr);
	for (i = 0; i < npairs; i++) {
		if (!strcmp(buf, pairs[i].seccode))
			return pairs[i].title;
	}
	return "";
}

char *
getdes(char *basestr)
{
	char buf[80];
	int i;
	sprintf(buf, "@%s", basestr);
	for (i = 0; i < npairs; i++) {
		if (!strcmp(buf, pairs[i].seccode))
			return pairs[i].title;
	}
	return "";
}

int
gentree(char *basestr, struct sectree *tree)
{
	int i, len;
	struct sectree *subsec;
	len = strlen(basestr);
	strsncpy(tree->basestr, basestr, sizeof (tree->basestr));
	strsncpy(tree->introstr, getintrostr(basestr), sizeof (tree->introstr));
	strsncpy(tree->des, getdes(basestr), sizeof (tree->des));
	tree->nsubsec = 0;
	bzero(tree->seccodes, sizeof (tree->seccodes));
	for (i = 0; i < npairs && tree->nsubsec <= MAXSUBSEC; i++) {
		if (pairs[i].seccode[0] == '*')
			continue;
		if (strlen(pairs[i].seccode) != len + 1)
			continue;
		if (strncmp(pairs[i].seccode, basestr, len))
			continue;
		subsec = malloc(sizeof (struct sectree));
		subsec->parent = tree;
		strsncpy(subsec->title, pairs[i].title, sizeof (subsec->title));
		gentree(pairs[i].seccode, subsec);
		tree->subsec[tree->nsubsec] = subsec;
		tree->seccodes[tree->nsubsec] = pairs[i].seccode[len];
		tree->nsubsec++;
	}
	return 0;
}

int
printtree(char *name0, const struct sectree *sec, int mode)
{
	char name[20];
	int i;
	if (mode)
		printf("/*------ %s -------*/\n", name0);
	if (sec->basestr[0])
		printf("static ");
	printf("const struct sectree %s%s\n", name0, mode ? " = {" : ";");
	if (mode) {
		if (sec->parent == NULL)
			printf("\tparent: NULL,\n");
		else {
			strncpy(name, name0, 20);
			name[strlen(name) - 1] = 0;
			printf("\tparent: &%s,\n", name);
		}
		printf("\ttitle: \"%s\",\n", sec->title);
		printf("\tbasestr: \"%s\",\n", sec->basestr);
		printf("\tseccodes: \"%s\",\n", sec->seccodes);
		printf("\tintrostr: \"%s\",\n", sec->introstr);
		printf("\tdes: \"%s\",\n", sec->des);
		printf("\tnsubsec: %d,\n", sec->nsubsec);
		printf("\tsubsec: {\n");
		for (i = 0; i < sec->nsubsec; i++) {
			printf("\t\t&%s%c,\n", name0, 'A' + i);
		}
		printf("\t}\n};\n");
	}
	for (i = 0; i < sec->nsubsec; i++) {
		sprintf(name, "%s%c", name0, 'A' + i);
		printtree(name, sec->subsec[i], mode);
	}
	return 0;
}

int
main()
{
	readseclist("seclist.local");
	gentree("", &rootsec);
	printf("#include <stdio.h>\n");
	printf("#include \"sectree.h\"\n");
	printtree("sectree", &rootsec, 0);
	printtree("sectree", &rootsec, 1);
	return 0;
}
