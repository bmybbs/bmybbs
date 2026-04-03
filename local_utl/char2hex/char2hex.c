#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	unsigned long i, l;
	unsigned long j;

	for (i = 1; i < (unsigned long) argc; i++) {
		for (j = 0, l = strlen(argv[i]); j < l; j++) {
			printf("\\x%X", argv[i][j] & 0xff);
		}
		printf("\n");
	}

	return 0;
}

