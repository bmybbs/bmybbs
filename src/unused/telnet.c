#include "bbs.h"

extern int max_timeout;
queue_tl qneti, qneto;
char remotehost[64];
char exec_param[256];

int nspawn(int sock, const char *exepath, char *const *argv);

#ifndef TEST
int
main(int argc, char *argv[])
#else
int
tmain(int argc, const char *argv[])
#endif
{
	int ret;
	int socket;
	char *exe[8];
	// default value
	if (argc != 3) {
		printf("No comm file.\n");
		return 0;
	}
	queue_init(&qneti);
	queue_init(&qneto);

	if (!nload(argv[1])) {
		perror("load comm file.\n");
		return 0;
	}
	socket = atoi(argv[2]);

	ret = 0;
	argc = 0;
	while (ret < sizeof (exec_param) && exec_param[ret]) {
		if (argc >= 7)
			break;
		exe[argc] = &exec_param[ret];
		ret += strlen(exe[argc++]) + 1;
	}
	exe[argc] = NULL;

	ret = nspawn(socket, exec_param, exe);

	nsave(argv[1]);
	return ret;
}

#ifdef TEST
int cs;
void
abrt(int sig)
{
	close(cs);
	exit(0);
}

main()
{

	int ds, i;
	struct sockaddr_in sin;
	char buf[256];
	const char *argv[6];

	cs = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(8003);

	signal(SIGHUP, abrt);
	signal(SIGINT, abrt);

	i = 1;
	setsockopt(cs, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i));

	if (bind(cs, (struct sockaddr *) &sin, sizeof (sin))) {
		perror("bind");
		return 0;
	}
	listen(cs, 5);

	while (1) {
		i = sizeof (sin);
		ds = accept(cs, (struct sockaddr *) &sin, &i);
		if (ds <= 0)
			break;

		sprintf(buf, "try init %d\r\n", ds);
		write(ds, buf, strlen(buf));

		if (!tmachine_init(ds)) {
			sprintf(buf, "tmachine return %d\r\n", i);
			write(ds, buf, strlen(buf));
			close(ds);
			continue;
		}

		sprintf(buf, "try exec %d\r\n", ds);
		write(ds, buf, strlen(buf));

		memcpy(exec_param, "bash\0\0", 6);
		strcpy(remotehost, "test");
		strcpy(user_name, "bbs");

		sprintf(buf, "%d", ds);
		argv[0] = "ptyexec";
		argv[1] = "/tmp/telnet.dat";
		argv[2] = buf;
		argv[3] = NULL;

		nsave(argv[1]);

		i = tmain(3, argv);
		sprintf(buf, "tmain return %d\r\n", i);
		write(ds, buf, strlen(buf));

		shutdown(ds, 2);
		close(ds);
	}
	close(cs);
}

#endif
