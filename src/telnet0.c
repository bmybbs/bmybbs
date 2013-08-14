#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/telnet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>
#define stty(fd, data) tcsetattr( fd, TCSANOW, data )
#define gtty(fd, data) tcgetattr( fd, data )
struct termios tty_state, tty_new;

get_tty()
{
	if (gtty(1, &tty_state) < 0)
		return 0;
	return 1;
}

init_tty()
{
	long vdisable;

	memcpy(&tty_new, &tty_state, sizeof (tty_new));
	tty_new.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ISIG);
	tty_new.c_cflag &= ~CSIZE;
	tty_new.c_cflag |= CS8;
	tty_new.c_cc[VMIN] = 1;
	tty_new.c_cc[VTIME] = 0;
	if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0) {
		tty_new.c_cc[VSTART] = vdisable;
		tty_new.c_cc[VSTOP] = vdisable;
		tty_new.c_cc[VLNEXT] = vdisable;
	}
	tcsetattr(1, TCSANOW, &tty_new);
}

reset_tty()
{
	stty(1, &tty_state);
}

static void
timeout()
{
	reset_tty();
	exit(0);
}

proc(char *server, int port)
{
	int fd;
	struct sockaddr_in blah;
	struct hostent *he;
	int result, lm = time(NULL);
	unsigned char buf[2048];
	fd_set readfds;
	struct timeval tv;
	signal(SIGALRM, timeout);
	alarm(20);
	bzero((char *) &blah, sizeof (blah));
	blah.sin_family = AF_INET;
	blah.sin_addr.s_addr = inet_addr(server);
	blah.sin_port = htons(port);
	fflush(stdout);
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((he = gethostbyname(server)) != NULL)
		bcopy(he->h_addr, (char *) &blah.sin_addr, he->h_length);
	else if ((blah.sin_addr.s_addr = inet_addr(server)) < 0) {
		return;
	}
	if (connect(fd, (struct sockaddr *) &blah, 16) < 0) {
		write(1, "没有连上噢 :(\n", strlen("没有连上噢 :(\n"));
		return;
	}
	signal(SIGALRM, SIG_IGN);
	if (0 >
	    write(1, "连接上了, 等待响应...\n",
		  strlen("连接上了, 等待响应...\n")))
		return;
	tv.tv_sec = 360;
	tv.tv_usec = 0;
	while (1) {
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		FD_SET(0, &readfds);

		result = select(fd + 1, &readfds, NULL, NULL, &tv);
		if (result < 0)
			break;
		if (result == 0) {
			if (time(NULL) - lm >= 360) {
				if (write(fd, "\033\133\101\033\133\102", 6) <
				    0)
					return;
				lm = time(NULL);
			}
			tv.tv_sec = 360;
			tv.tv_usec = 0;
			continue;
		}

		if (FD_ISSET(0, &readfds)) {
			result = read(0, buf, 2048);
			if (result <= 0)
				break;
			if (result == 1 && (buf[0] == 10 || buf[0] == 13)) {
				buf[0] = 13;
				buf[1] = 10;
				result = 2;
			}
			if (buf[0] == 29) {
				close(fd);
				return;
			}
			if (write(fd, buf, result) < 0)
				return;
			lm = time(NULL);
		} else {
			result = read(fd, buf, 2048);
			if (result <= 0)
				break;
			if (strchr(buf, 255)) {
				if (telnetopt(fd, buf, result) < 0)
					return;
			}
			else if (write(1, buf, result) < 0)
				return;
		}
	}
}

int
telnetopt(int fd, char *buf, int max)
{
	unsigned char c, d, e;
	int pp = 0, start = 0;
	unsigned char tmp[30];
	while (pp < max) {
		c = buf[pp++];
		if (c == 255) {
			if (pp - 1 > start)
				if (write(1, buf+start, pp-start-1) < 0)
					return -1;
			d = buf[pp++];
			e = buf[pp++];
			fflush(stdout);
			if ((d == 253) && (e == 3 || e == 24)) {
				tmp[0] = 255;
				tmp[1] = 251;
				tmp[2] = e;
				write(fd, tmp, 3);
				start = pp;
				continue;
			}
			if ((d == 251 || d == 252)
			    && (e == 1 || e == 3 || e == 24)) {
				tmp[0] = 255;
				tmp[1] = 253;
				tmp[2] = e;
				write(fd, tmp, 3);
				start = pp;
				continue;
			}
			if (d == 251 || d == 252) {
				tmp[0] = 255;
				tmp[1] = 254;
				tmp[2] = e;
				write(fd, tmp, 3);
				start = pp;
				continue;
			}
			if (d == 253 || d == 254) {
				tmp[0] = 255;
				tmp[1] = 252;
				tmp[2] = e;
				write(fd, tmp, 3);
				start = pp;
				continue;
			}
			if (d == 250) {
				while (e != 240 && pp < max)
					e = buf[pp++];
				tmp[0] = 255;
				tmp[1] = 250;
				tmp[2] = 24;
				tmp[3] = 0;
				tmp[4] = 65;
				tmp[5] = 78;
				tmp[6] = 83;
				tmp[7] = 73;
				tmp[8] = 255;
				tmp[9] = 240;
				write(fd, tmp, 10);
				start = pp;
				continue;
			}
			if (d == 255) {
				tmp[0] = d;
				tmp[1] = e;
				if (pp < max) {
					write(fd, tmp, 2);
				} else
					write(fd, tmp, 1);
			}
			start = pp;
		}
	}
	if (start < max)
		if (write(1, buf+start, max-start)<0)
			return -1;
	return 0;
}

main(int argn, char *argc[])
{
	char remotehost[256] = "localhost";
	int remoteport = 23;
	get_tty();
	init_tty();
	if (argn >= 2)
		strncpy(remotehost, argc[1], 255);
	remotehost[255] = 0;
	if (argn >= 3)
		remoteport = atoi(argc[2]);
	proc(remotehost, remoteport);
	reset_tty();
}
