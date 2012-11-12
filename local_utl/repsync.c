#include <time.h>
#include <unistd.h>
int
main()
{
	int i = 0;
	time_t t;
	struct tm *tm;
	while (1) {
		i++;
		sync();
		sleep(2);
		if (i % 30)
			continue;
		t = time(NULL);
		tm = localtime(&t);
		if (tm->tm_hour > 8)
			break;
	}
	return 0;
}
