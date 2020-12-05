#include <stdio.h>
#include <string.h>

extern int count_threads_of_board(const char *boardname);
extern int import_board(void);

int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (strcmp(argv[1], "test_board") == 0) {
			count_threads_of_board(argv[2]);
		}
		if (strcmp(argv[1], "import_board") == 0) {
			import_board();
		}
	} else {
		fprintf(stderr, "[usage] <program_name> <option>\noption:\n");
		fprintf(stderr, "\ttest_board <boardname>: 测试版面主题计数\n");
		fprintf(stderr, "\timport_board: 导入版面信息到 t_boards 表\n");
	}
	return 0;
}

