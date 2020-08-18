// by ecnegrevid 2001.7.18
#include <sys/time.h>
#include <unistd.h>
#include <grp.h>
#include <signal.h>
#include <termios.h>
#include <pty.h>
#include <string.h>

void pipedata(int fdr, int fdw);

void
dosetpty()
{
	struct termios ti;
	tcgetattr(0, &ti);
	ti.c_iflag |= IGNBRK;
	ti.c_lflag &= ~(ISIG | TOSTOP);
	ti.c_lflag |= ICANON | ECHO | ECHOE | ECHONL;
	ti.c_cc[VEOL] = 0;
	tcsetattr(0, TCSADRAIN, &ti);
}

int
main(int argc, char *argv[])
{
	int fdm, n;
	pid_t pid;
	char slave_name[20];
	char buf[256];
	setsid();
	pid = forkpty(&fdm, slave_name, NULL, NULL);
	if (pid < 0)
		return -1;
	else if (pid == 0) {	/* child */
		dosetpty();
		if (execvp(argv[1], &argv[1]) < 0)
			return kill(getppid(), SIGTERM);
	}
	pid = fork();
	if (pid < 0)
		return -1;
	else if (pid == 0) {
		pipedata(0, fdm);
		kill(getppid(), 9);
	} else {
		pipedata(fdm, 1);
		kill(pid, 9);
	}

	return 0;
}

void
pipedata(int fdr, int fdw)
{
	char ch;
	//while(read(fdr, &ch, 1)==1)
	//      if(write(fdw, &ch, 1)!=1)
	//              return;
	char buf[256];
	int i = 0, retv, n;
	fd_set fsr, fsw;
	FD_ZERO(&fsr);
	FD_ZERO(&fsw);
	while (1) {
		if (i < 256)
			FD_SET(fdr, &fsr);
		if (i)
			FD_SET(fdw, &fsw);
		retv =
		    select((fdr > fdw) ? (1 + fdr) : (1 + fdw), &fsr, &fsw,
			   NULL, NULL);
		if (retv < 0)
			return;
		if (FD_ISSET(fdr, &fsr)) {
			n = read(fdr, buf + i, sizeof (buf) - i);
			if (n <= 0)
				return;
			i += n;
			FD_CLR(fdr, &fsr);
		}
		if (FD_ISSET(fdw, &fsw)) {
			n = write(fdw, buf, i);
			if (n <= 0)
				return;
			i -= n;
			memmove(buf, buf + n, i);
			FD_CLR(fdw, &fsw);
		}
	}
}
