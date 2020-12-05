#include <stdio.h>
#include <string.h>

extern int count_threads_of_board(const char *boardname);

int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (strcmp(argv[1], "test_board") == 0) {
			count_threads_of_board(argv[2]);
		}
	} else {
		fprintf(stderr, "[usage] <program_name> <option>\noption:\n");
		fprintf(stderr, "\ttest_board <boardname>: 测试版面主题计数\n");
	}
	return 0;
}

