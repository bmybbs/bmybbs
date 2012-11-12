//ecnegrevid 2001.7.20
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "items.h"
struct items* creatitems(int max)
{
	struct items *is;
	struct item *i;
	is=malloc(sizeof(*is));
	if(is==NULL) return NULL;
	i=malloc(sizeof(*i)*max);
	if(i==NULL) {
		free(is);
		return NULL;
	}
	is->item=i;
	is->n=0;
	is->max=max;
	return is;
}
void freeitems(struct items *items)
{
	free(items->item);
	free(items);
}

int readitems(struct items *items, const char *filename)
{
	char fmt[20];
	FILE *fp=fopen(filename, "r");
	items->n=0;
	if(fp==NULL)
		return -1;
	sprintf(fmt, "%%%ds%%d", sizeof(items->item[0].name)-1);
	while(items->n<items->max) {
		if(fscanf(fp,"%s%d", items->item[items->n].name,
					&items->item[items->n].value)!=2)
			break;
		items->n++;
	}
	fclose(fp);
	return items->n;
}

int saveitems(struct items *items, const char *filename)
{
	int i;
	FILE *fp=fopen(filename, "w+");
	if(fp==NULL) {
		return -1;
	}
	for(i=0;i<items->n;i++)
		fprintf(fp, "%s %d\n", items->item[i].name, items->item[i].value);
	fclose(fp);
	return 0;
}

void additem(struct items *items, const char *name, int value)
{	int i;
	struct item item, tmp;
	strncpy(item.name, name, sizeof(item.name)-1);
	item.name[sizeof(item.name)-1]=0;
	item.value=value;
	for(i=0;i<items->n;i++) {
		tmp=items->item[i];
		items->item[i]=item;
		item=tmp;
		if(i!=0&&strcmp(item.name,items->item[0].name)==0)
			break;
	}
	if(i==items->n&&items->n<items->max)
		items->item[items->n++]=item;
}

int readvalue(struct items *items, const char *name, int *value)
{
	int i;
	for(i=0;i<items->n;i++) {
		if(strncmp(items->item[i].name, name, sizeof(items->item[i].name))==0)
			break;
	}
	if(i==items->n)
		return -1;
	*value=items->item[i].value;
	return 0;
}
int additemf(const char *filename, int max, const char *name, int value)
{
        struct items *its;
        its=creatitems(max);
        if(its==NULL) return -1;
        readitems(its, filename);
	additem(its, name, value);
	if(saveitems(its,filename)>=0) {
		freeitems(its);
		return 0;
	}
        freeitems(its);
        return -1;
}
int readvaluef(const char *filename, int max, const char *name, int *value)
{
        struct items *its;
        its=creatitems(max);
        if(its==NULL) return -1;
        if(readitems(its, filename)<0||readvalue(its,name,value)<0) {
                freeitems(its);
                return -1;
        }
        freeitems(its);
        return 0;
}
