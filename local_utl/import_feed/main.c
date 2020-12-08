#include <stdio.h>
#include <string.h>

extern int count_threads_of_board(const char *boardname);
extern int import_board(void);
extern int import_user(void);
extern int import_thread(void);
extern int delete_user(void);

int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (strcmp(argv[1], "test_board") == 0) {
			count_threads_of_board(argv[2]);
		}
		else if (strcmp(argv[1], "import_board") == 0) {
			import_board();
		}
		else if (strcmp(argv[1], "import_user") == 0) {
			import_user();
		}
		else if (strcmp(argv[1], "import_thread") == 0) {
			import_thread();
		}
		else if (strcmp(argv[1], "delete_user") == 0) {
			delete_user();
		}
	} else {
		fprintf(stderr, "[usage] <program_name> <option>\noption:\n");
		fprintf(stderr, "\ttest_board <boardname>: 测试版面主题计数\n");
		fprintf(stderr, "\timport_board: 导入版面信息到 t_boards 表，并创建版面视图\n");
		fprintf(stderr, "\timport_user: 导入用户信息到 t_users 表，并初始化订阅元信息，构建对应视图\n");
		fprintf(stderr, "\timport_thread: 读取各个版面的主题贴，统计讨论数，并最终合并为一个按时间排列的列表导入到 t_threads 表中\n");
	}
	return 0;
}

