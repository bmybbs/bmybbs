#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bmy/algorithms.h"


static const char DELIMITER = ',';

int main() {
	int array[] = { 1, 2, 3 };
	char *s = bmy_algo_join_int_array_to_string(array, 3, DELIMITER);
	const char *result = "1,2,3";
	size_t i;

	if (s == NULL) {
		printf("should not be NULL\n");
		return 0;
	}

	if (strlen(s) != strlen(result)) {
		printf("wrong length\n");
		free(s);
		return 0;
	}

	for (i = 0; i < strlen(s); i++) {
		if (s[i] != result[i]) {
			printf("wrong char at %zu, expected %c, got %c\n", i, result[i], s[i]);
			free(s);
			return 0;
		}
	}

	printf("test pass\n");
	return 0;
}

