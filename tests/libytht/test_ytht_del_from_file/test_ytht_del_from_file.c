#include <stdio.h>
#include "ytht/fileop.h"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("usage: ./a.out /path/to/file content_to_be_deleted");
		return -1;
	}

	return ytht_del_from_file(argv[1], argv[2]);
}
