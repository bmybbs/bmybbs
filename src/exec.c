#include "bbs.h"
#include <pwd.h>
#include <grp.h>

#define BUF_SIZE  256
#define SCPYN(x,y) strncpy(x,y,sizeof(x)); x[sizeof(x)-1]=0;

char *big2gb(char *s, int *plen, int inst);
char *gb2big(char *s, int *plen, int inst);
void conv_init();

/*
char * convertin(char * p,int * i)
{
	return p;
}
char * convertout(char *p,int *i)
{
	return p;
}
*/

static int spawn_break;
static queue_tl qptyo;
char user_name[64];
int net;
int pty;
int term_convert;

extern int tmachine_server_recv_and_check_option(int net);

static void
ncleanup(int sig)
{
	if (sig == SIGCHLD)
		while (waitpid(-1, NULL, WNOHANG) >= 0)
			sleep(1);
	spawn_break = 1;
}

int
nspawn(int sock, const char *exepath, char *const *argvs)
{
	int ret, pid;
	struct timeval tm;
	char line[BUF_SIZE];
	struct passwd *pwd;
	struct group *grp;

	conv_init();

	pty = 0;
	net = sock;

	queue_init(&qptyo);

	pwd = getpwnam(user_name);
	endpwent();

	grp = getgrnam("tty");
	endgrent();

	if (!pwd)
		return -100;
	if (!grp)
		return -101;

#ifdef BBS_UID
	if (pwd->pw_uid != BBS_UID)
		return -100;
	if (pwd->pw_gid != BBS_GID)
		return -100;
#else
	if (pwd->pw_uid < 99)
		return -100;
	if (pwd->pw_gid < 99)
		return -100;
#endif

	// create pty
	pty = getpty(line);
	if (pty <= 0)
		return -102;

	ret = 1;
	setsockopt(net, SOL_SOCKET, SO_OOBINLINE, &ret, sizeof ret);
	ioctl(net, FIONBIO, (char *) &ret);
	ioctl(pty, FIONBIO, (char *) &ret);
	ioctl(pty, TIOCPKT, (char *) &ret);	// Turn on packet mode

	spawn_break = 0;
	signal(SIGHUP, ncleanup);
	signal(SIGTERM, ncleanup);
	signal(SIGINT, ncleanup);
	signal(SIGCHLD, ncleanup);
	signal(SIGPIPE, ncleanup);

	pid = 0;
	ret = fork();
	if (ret > 0) {		// do pty side 
		// write wtmp
		pid = ret;

		wlogin(&line[5], user_name, remotehost);

		{
			struct winsize ws;
			ws.ws_col = term_cols;
			ws.ws_row = term_lines;
			ioctl(pty, TIOCSWINSZ, (char *) &ws);
		}

#ifdef TEST
		{
			char buf[256];
			sprintf(buf, "line is %s\r\n", line);
			write(net, buf, strlen(buf));
		}
#endif

		while (1) {
			fd_set ibits, obits, xbits;
			int hifd, c;

			tm.tv_sec = max_timeout;
			tm.tv_usec = 0;

			hifd = 0;
			FD_ZERO(&ibits);
			FD_ZERO(&obits);
			FD_ZERO(&xbits);

			if (queue_free_size(&qneti)) {
				FD_SET(net, &ibits);
				if (net >= hifd)
					hifd = net + 1;
			}
			if (queue_free_size(&qneto) > BUF_SIZE) {
				FD_SET(pty, &ibits);
				if (pty >= hifd)
					hifd = pty + 1;
			}
			if (queue_data_size(&qneto)) {
				FD_SET(net, &obits);
				if (net >= hifd)
					hifd = net + 1;
			}
			if (queue_data_size(&qptyo)) {
				FD_SET(pty, &obits);
				if (pty >= hifd)
					hifd = pty + 1;
			}
			FD_SET(net, &xbits);
			if (net >= hifd)
				hifd = net + 1;

			if (spawn_break) {
				c = 0;
				ioctl(pty, FIONREAD, &c);
				if (!c) {
					ret = -1;
					break;
				}
			}
			if (max_timeout)
				c = select(hifd, &ibits, &obits, &xbits, &tm);
			else
				c = select(hifd, &ibits, &obits, &xbits, NULL);
			if (FD_ISSET(net, &xbits)) {
				ret = -2;
				break;
			}

			if (c < 0 && errno == EINTR)
				continue;
			if (c == 0) {
				if (!max_timeout)
					continue;
				ret = 0;	// timeout
				break;
			}

			if (FD_ISSET(net, &ibits)) {
				char nbuf[QUEUE_BUF_SIZE];
				char *pbuf;
				int i;

				if (tmachine_server_recv_and_check_option(net) <
				    0) {
					if (errno != EWOULDBLOCK
					    && errno != EIO) {
						ret = -4;
						break;
					}
				}
				c = queue_read(&qneti, nbuf + 1,
					       sizeof (nbuf) - 1);
				if (term_convert)
					pbuf = big2gb(nbuf + 1, &c, 0);
				else
					pbuf = nbuf + 1;

				for (i = 0; i < c; i++) {
					static int lastcrlf;
					if (options[TELOPT_BINARY] == WONT) {
						if (pbuf[i] != '\r'
						    && pbuf[i] != '\n')
							lastcrlf = 0;
						else if ((lastcrlf == '\n')
							 && (pbuf[i] == '\r')) {
							lastcrlf = 0;
							continue;
						} else if ((lastcrlf == '\r')
							   && (pbuf[i] == '\n')) {
							lastcrlf = 0;
							continue;
						} else {
							queue_write_char(&qptyo,
									 '\r');
							lastcrlf = pbuf[i];
							continue;
						}
					}
					queue_write_char(&qptyo, pbuf[i]);
				}
			}
			if (FD_ISSET(pty, &ibits)) {
				unsigned char ptyibuf[BUF_SIZE];
				char *pbuf;
				int i, pcc;

				pcc = read(pty, ptyibuf, BUF_SIZE);

				if (pcc < 0
				    && (errno == EWOULDBLOCK || errno == EIO)) {
					pcc = 0;
				} else {
					if (pcc < 0) {
						ret = -5;
						break;
					}

					if (ptyibuf[0] & TIOCPKT_FLUSHWRITE) {
						queue_write_char(&qneto,
								 (char) IAC);
						queue_write_char(&qneto,
								 (char) DM);
					}

					if (options[TELOPT_LFLOW] == DO &&
					    (ptyibuf[0] &
					     (TIOCPKT_NOSTOP | TIOCPKT_DOSTOP)))
					{
						unsigned char tlflow[] =
						    { IAC, SB, TELOPT_LFLOW, 0,
							IAC, SE
						};
						if (ptyibuf[0] & TIOCPKT_DOSTOP)
							tlflow[3] = 1;
						queue_write(&qneto,
							    (char *) tlflow,
							    sizeof (tlflow));
					}
					pcc--;
					if (term_convert)
						pbuf =
						    gb2big((char *) &ptyibuf[1],
							   &pcc, 0);
					else
						pbuf = ptyibuf + 1;
					if (pbuf) {
						int lastcr;
						lastcr = 0;
						for (i = 0; i < pcc; i++) {
							static int lastcrlf;
							if (pbuf[i] ==
							    (char) IAC)
								queue_write_char
								    (&qneto,
								     IAC);
/* wrong
							if(options[TELOPT_BINARY]==WONT){
								if( pbuf[i]!='\r' && pbuf[i]!='\n' )
									lastcrlf=0;
								else if( lastcrlf ){
									lastcrlf=0;
									continue;
								} else {
									queue_write_char(&qneto,'\r');
									queue_write_char(&qneto,'\n');
									lastcrlf=1;
									continue;
								}
							}
*/
							queue_write_char(&qneto,
									 (char)
									 pbuf
									 [i]);
						}
					}
					queue_send(&qneto, net);
				}
			}
			if (FD_ISSET(net, &obits)) {
				queue_send(&qneto, net);
			}
			if (FD_ISSET(pty, &obits)) {
				if (!spawn_break)
					queue_send(&qptyo, pty);
			}
		}
	} else if (ret == 0) {	// start client

		close(net);
		close(pty);

		signal(SIGHUP, SIG_IGN);

		ret = open(_PATH_TTY, O_RDWR);
		if (ret >= 0) {
			ioctl(ret, TIOCNOTTY, (char *) 0);
			close(ret);
		}

		setsid();	// no ctty

		chown(line, pwd->pw_uid, grp->gr_gid);

		chmod(line, 0620);

		ret = open(line, O_RDWR | O_NOCTTY);

		ioctl(ret, TIOCSCTTY, (char *) 0);

		login_tty(ret);

		if (ret != 0)
			dup2(ret, 0);
		if (ret != 1)
			dup2(ret, 1);
		if (ret != 2)
			dup2(ret, 2);
		if (ret > 2)
			close(ret);

		setpgrp();

		setgroups(1, &pwd->pw_gid);
		setregid(pwd->pw_gid, pwd->pw_gid);
		setreuid(pwd->pw_uid, pwd->pw_uid);

		execvp(exepath, argvs);
		exit(0);
	}

	if (pid) {
		int i;

		queue_send(&qneto, net);

#ifdef TEST
		printf("wait client %d, return code %d\n", pid, ret);
#endif
		i = 0;
		while (1) {
			if (waitpid(-pid, NULL, WNOHANG) < 0) {
				if (errno == ECHILD)
					break;
			}
#ifdef TEST
			printf(".");
			fflush(NULL);
#endif
			i++;
			if (i == 1)
				kill(-pid, SIGINT);
			if (i == 2)
				kill(-pid, SIGTERM);
			if (i >= 3)
				kill(-pid, SIGHUP);
			if (i >= 5)
				kill(-pid, SIGKILL);
			sleep(1);
		}
		// cleanup wtmp
#ifdef TEST
		printf("logout %d\n", pid);
#endif

		ret = wlogout(&line[5]);
	}
	// close pty
	closepty(pty);

	return ret;
}
