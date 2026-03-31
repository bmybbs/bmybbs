#include <stdio.h>

char* a[] = {
	// 中
	"\xD6\xD0",
};

int main() {
	// 中
	// 文
	printf("%s%s%s\n", "\xD6\xD0", "\xCE\xC4", "abc"); // 注释
	return 0;
}
