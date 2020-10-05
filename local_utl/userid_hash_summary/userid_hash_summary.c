#include <stdio.h>
#include <string.h>
#include "ythtbbs/user.h"
#include "ythtbbs/cache.h"

static const unsigned int TOTAL = 26*26;

int main(int argc, char *argv[]) {
	unsigned int aux[TOTAL], i, hash;
	FILE *fp;
	struct userec ue;

	fp = fopen(".PASSWDS", "r");
	if (fp == NULL)
		return -1;

	memset(&aux, 0, sizeof(aux));

	while(fread(&ue, sizeof(ue), 1, fp) == 1) {
		if (ue.userid[0] == '\0') continue;
		hash = ythtbbs_cache_User_hash(ue.userid);
		if (hash >= TOTAL) {
			fprintf(stderr, "out of range for user[%s]\n", ue.userid);
			return -2;
		}
		aux[hash]++;
		printf("%s:%d\n", ue.userid, hash);
	}

	printf("\n========\n");

	for (i=0; i<TOTAL; i++) {
		printf("slot-%d,%d\n", i, aux[i]);
	}

	return 0;
}