#include <stdio.h>
#include <stdlib.h>
#include "bmy/search.h"

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("usage: %s board query\n", argv[0]);
		return -1;
	}

	size_t search_size, i;
	struct fileheader_utf *articles = bmy_search_board(argv[1], argv[2], &search_size);
	if (articles) {
		for (i = 0; i < search_size; i++) {
			printf("%s %s %ld %ld\n", articles[i].boardname_en, articles[i].title, articles[i].thread, articles[i].filetime);
		}
		free(articles);
	}

	return 0;
}
