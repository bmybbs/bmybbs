#include <string.h>
#include "strhash.h"
// from src/bcache.c:useridhash by ylsdd. perhaps it is no good enough.
int
strhash(char *id)
{
	int n1 = 0;
	int n2 = 0;
	while (*id) {
		n1 += ((unsigned char) *id) % 26;
		id++;
		if (!*id)
			break;
		n2 += ((unsigned char) *id) % 26;
		id++;
	}
	n1 %= 26;
	n2 %= 26;
	return n1 * 26 + n2;
}

int
getdic(diction dic, size_t size, void **mem)
{
	struct hword *tmp;
	int i, count;
	void *m;
	count = 0;
	for (i = 0; i < 26 * 26; i++) {
		tmp = dic[i];
		while (tmp != NULL) {
			count++;
			tmp = tmp->next;
		}
	}
	m = malloc(size * count);
	if (m == NULL)
		return -1;
	count = 0;
	for (i = 0; i < 26 * 26; i++) {
		tmp = dic[i];
		while (tmp != NULL) {
			memcpy(m + size * count, tmp->value, size);
			count++;
			tmp = tmp->next;
		}
	}
	*mem = m;
	return count;
}

struct hword *
finddic(diction dic, char *key)
{
	int hashkey;
	struct hword *tmp;
	hashkey = strhash(key);
	tmp = dic[hashkey];
	while (tmp != NULL) {
		if (strcmp(tmp->str, key)) {
			tmp = tmp->next;
			continue;
		}
		break;
	}
	return tmp;
}

struct hword *
insertdic(diction dic, struct hword *w)
{
	int hashkey;
	hashkey = strhash(w->str);
	w->next = dic[hashkey];
	dic[hashkey] = w;
	return w;
}
