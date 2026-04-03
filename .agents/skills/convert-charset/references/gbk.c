#include <stdio.h>

char* a[] = {
	"ÖĞ",
};

int main() {
	printf("%s%s%s\n", "ÖĞ", "ÎÄ", "abc"); // ×¢ÊÍ
	return 0;
}
