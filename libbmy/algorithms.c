#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *bmy_algo_join_int_array_to_string(const int array[], size_t num, char delimiter) {
	char buf[24], **buf_array, *result;
	size_t i, total_len;

	if (array == NULL || num == 0)
		return NULL;

	buf_array = calloc(num, sizeof(char *));  // apply 1
	if (buf_array == NULL)
		return NULL;

	for (i = 0, total_len = 0; i < num - 1; i++) {
		snprintf(buf, sizeof(buf), "%d%c", array[i], delimiter);
		buf_array[i] = strdup(buf);           // apply 2
		if (buf_array[i] == NULL)
			goto CLEANUP;
		total_len += strlen(buf);
	}
	snprintf(buf, sizeof(buf), "%d", array[num - 1]);
	buf_array[num - 1] = strdup(buf);         // apply 2
	if (buf_array[num - 1] == NULL)
		goto CLEANUP;
	total_len += strlen(buf);

	result = calloc(total_len + 1, sizeof(char));
	for (i = 0; i < num; i++) {
		strcat(result, buf_array[i]);
		free(buf_array[i]);                   // clean up 2
	}

	free(buf_array);                          // clean up 1
	return result;

CLEANUP:
	for (i = 0; i < num; i++) {
		if (buf_array[i])
			free(buf_array[i]);
	}
	free(buf_array);
	return NULL;
}

