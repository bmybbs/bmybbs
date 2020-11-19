#include <stdio.h>
#include <string.h>
#include "ytht/fileop.h"

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("usage: ./a.out /path/to/file content_to_be_deleted true_or_false\n");
		return -1;
	}

	return ytht_del_from_file(argv[1], argv[2], !strcasecmp(argv[3], "true"));
}
