#include <time.h>
#include <sys/resource.h>
#include <unistd.h>

static int load_limit = 200;	// it means the cpu limit is 1/200=0.5%
void
set_cpu_limit(int limit)
{
	if (limit > 5000)
		limit = 5000;
	else if (limit < 2)
		limit = 2;
	load_limit = limit;
}

int
limit_cpu(void)
{
	static time_t start = 0;
	time_t now;
	struct rusage u;
	static time_t sc, sc_u, t = 0;
	time_t cost, cost_u, d;
	int ret = 0;
	if (!start) {
		start = time(0);
		getrusage(RUSAGE_SELF, &u);
		sc = u.ru_utime.tv_sec + u.ru_stime.tv_sec;
		sc_u = u.ru_utime.tv_usec + u.ru_stime.tv_usec;
		return ret;
	}
	if ((t++) % 20)
		return ret;
	now = time(0);
	getrusage(RUSAGE_SELF, &u);
	cost = u.ru_utime.tv_sec + u.ru_stime.tv_sec;
	cost_u = u.ru_utime.tv_usec + u.ru_stime.tv_usec;
	d = load_limit * ((cost - sc) + (cost_u - sc_u) / 1000000) - (now - start);
	if (d > 10)
		d = 10;
	if (d > 0) {
		sleep(d);
		ret = 1;
	}
	if (now - start > 300) {
		start = now;
		sc = cost;
		sc_u = cost_u;
	}
	return ret;
}
