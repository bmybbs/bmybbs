#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int i, l;
	unsigned long j;

	for (i=1; i<argc; i++) {
		for (j=0, l=strlen(argv[i]); j<l; j++) {
			printf("\\x%X", argv[i][j] & 0xff);
		}
		printf("\n");
	}

	return 0;
}

