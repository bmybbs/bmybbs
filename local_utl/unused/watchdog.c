#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int
main(int argc, const char *argv[])
{
	int pid;
	int fd = open("/dev/watchdog", O_WRONLY);

	if (fd == -1) {
		perror("watchdog");
		exit(1);
	}
	while (1) {
		pid = fork();
		if (pid)
			wait(NULL);
		else {
			write(fd, "\0", 1);
			printf("1\n");
			exit(0);
		}
		sleep(10);
	}
}
