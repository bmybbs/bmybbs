//ecnegrevid 2001.7.20
#ifndef items_h
#define items_h
struct item {
	char name[50];
	int value;
};
struct items {
	int n;
	int max;
	struct item *item;
};
struct items* creatitems(int max);
void freeitems(struct items *items);
int readitems(struct items *items, const char *filename);
int saveitems(struct items *items, const char *filename);
void additem(struct items *items, const char *name, int value);
int readvalue(struct items *items, const char *name, int *value);
int additemf(const char *filename, int max, const char *name, int value);
int readvaluef(const char *filename, int max, const char *name, int *value);
#endif
