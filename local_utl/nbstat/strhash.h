#ifndef BMYBBS_STRHASH_H
#define BMYBBS_STRHASH_H
#include <stdlib.h>

struct hword {
	char str[80];
	void *value;
	struct hword *next;
};

typedef struct hword *diction[26 * 26];

int getdic(diction dic, size_t size, void **mem);
struct hword *finddic(diction dic, char *key);
struct hword *insertdic(diction dic, struct hword *cell);
#endif //BMYBBS_STRHASH_H
