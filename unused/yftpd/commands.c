#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <wait.h>
#ifdef HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif
#include <sys/file.h>

#include "options.h"
#include "mystring.h"
#include "login.h"
#include "logging.h"
#include "dirlist.h"
#include "commands.h"
#include "main.h"
#include "pathop.h"
#include "yftpdutmp.h"

int state = STATE_CONNECTED;
char user[USERLEN + 1];
struct sockaddr_in sa;
char pasv = 0;
int sock;
int pasvsock;
char philename[256];
int offset = 0;
short int xfertype = TYPE_BINARY;
int bytes_sent = 0, bytes_recvd = 0;
int epsvall = 0;

void
prints(char *format, ...)
{
	char buffer[256];
	va_list val;
	va_start(val, format);
	vsnprintf(buffer, sizeof (buffer), format, val);
	va_end(val);
	fprintf(stderr, "%s\r\n", buffer);
}

void
prepare_sock(int sock)
{
	int on = 1;
#ifdef TCP_NODELAY
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof (on));
#endif
#ifdef TCP_NOPUSH
	setsockopt(sock, IPPROTO_TCP, TCP_NOPUSH, (void *) &on, sizeof (on));
#endif
#ifdef SO_REUSEADDR
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof (on));
#endif
#ifdef SO_REUSEPORT
	setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void *) &on, sizeof (on));
#endif
#ifdef SO_SNDBUF
	on = 65536;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void *) &on, sizeof (on));
#endif
}

int
dataconn()
{
	struct sockaddr foo;
	struct sockaddr_in local;
	int namelen = sizeof (foo);

	memset(&foo, 0, sizeof (foo));
	memset(&local, 0, sizeof (local));

	if (pasv) {
		sock =
		    accept(pasvsock, (struct sockaddr *) &foo,
			   (int *) &namelen);
		if (sock == -1) {
			prints
			    ("425-Unable to accept data connection.\r\n425 %s.",
			     strerror(errno));
			return 1;
		}
		close(pasvsock);
		prepare_sock(sock);
	} else {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		prepare_sock(sock);
		local.sin_addr.s_addr = name.sin_addr.s_addr;
		local.sin_family = AF_INET;
		if (bind(sock, (struct sockaddr *) &local, sizeof (local)) < 0) {
			prints("425-Unable to bind data socket.\r\n425 %s.",
			       strerror(errno));
			return 1;
		}
		sa.sin_family = AF_INET;
		if (connect(sock, (struct sockaddr *) &sa, sizeof (sa)) == -1) {
			prints("425-Unable to establish data connection.\r\n"
			       "425 %s.", strerror(errno));
			return 1;
		}
	}
	prints("150 Data connection established.");
	return 0;
}

void
flow_control()
{
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(flow_semid, &sop, 1);
}

void
command_user(char *username)
{
	if (state) {
		prints("503 Username already given.");
		return;
	}
	mystrncpy(user, username, sizeof (user) - 1);
	state = STATE_USER;
	prints("331 Password please.");
}

void
command_pass(char *password)
{
	if (state > STATE_USER) {
		prints("503 Already logged in.");
		return;
	}
	if (yftpd_login(password)) {
		yftpd_log("Login as user '%s' failed.\n", user);
		prints("421 Login incorrect.");
		exit(0);
	}
}

void
command_pwd(char *params)
{
	char cwd[256];
	prints("257 \"%s\" is the current working directory.",
	       my_getcwd(cwd, sizeof (cwd) - 1));

}

void
command_type(char *params)
{
	if ((*params == 'A') || (*params == 'a')) {
		prints("200 OK");
		xfertype = TYPE_ASCII;
	} else if ((*params == 'I') || (*params == 'i')) {
		prints("200 OK");
		xfertype = TYPE_BINARY;
	} else
		prints("500 Type '%c' not supported.", *params);
}

void
command_port(char *params)
{
	unsigned long a0, a1, a2, a3, p0, p1, addr;
	if (epsvall) {
		prints("500 EPSV ALL has been called.");
		return;
	}
	sscanf(params, "%lu,%lu,%lu,%lu,%lu,%lu", &a0, &a1, &a2, &a3, &p0, &p1);
	addr = htonl((a0 << 24) + (a1 << 16) + (a2 << 8) + a3);
	if ((addr != remotename.sin_addr.s_addr)
	    && !ALLOW_FXP) {
		prints("500 The given address is not yours.");
		return;
	}
	sa.sin_addr.s_addr = addr;
	sa.sin_port = htons((p0 << 8) + p1);
	if (pasv) {
		close(sock);
		pasv = 0;
	}
	prints("200 PORT %lu.%lu.%lu.%lu:%lu OK",
	       a0, a1, a2, a3, (p0 << 8) + p1);
}

void
command_eprt(char *params)
{
	char delim;
	int af;
	char addr[51];
	char foo[20];
	int port;
	if (epsvall) {
		prints("500 EPSV ALL has been called.");
		return;
	}
	if (strlen(params) < 5) {
		prints("501 Syntax error.");
		return;
	}
	delim = params[0];
	sprintf(foo, "%c%%i%c%%50[^%c]%c%%i%c", delim, delim, delim, delim,
		delim);
	if (sscanf(params, foo, &af, addr, &port) < 3) {
		prints("501 Syntax error.");
		return;
	}
	if (af != 1) {
		prints("522 Protocol unsupported, use (1)");
		return;
	}
	sa.sin_addr.s_addr = inet_addr(addr);
	if ((sa.sin_addr.s_addr != remotename.sin_addr.s_addr)
	    && !ALLOW_FXP) {
		prints("500 The given address is not yours.");
		return;
	}
	sa.sin_port = htons(port);
	if (pasv) {
		close(sock);
		pasv = 0;
	}
	prints("200 EPRT %s:%i OK", addr, port);
}

void
command_pasv(char *foo)
{
	int namelen, a1, a2, a3, a4;
	struct sockaddr_in localsock;
	if (epsvall) {
		prints("500 EPSV ALL has been called.");
		return;
	}
	pasvsock = socket(AF_INET, SOCK_STREAM, 0);
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_family = AF_INET;

	if (!strlen(PASSIVE_PORTS)) {
		/* bind to any port */
		sa.sin_port = 0;
		if (bind(pasvsock, (struct sockaddr *) &sa, sizeof (sa)) == -1) {
			prints
			    ("425-Error: Unable to bind data socket.\r\n425 %s",
			     strerror(errno));
			return;
		}
	} else {
		int i = 0, success = 0, port;
		for (;;) {
			port = int_from_list(PASSIVE_PORTS, i++);
			if (port < 0)
				break;
			sa.sin_port = htons(port);
			if (bind
			    (pasvsock, (struct sockaddr *) &sa,
			     sizeof (sa)) == 0) {
				success = 1;
#ifdef DEBUG
				yftpd_log
				    ("Passive mode: Successfully bound port %d\n",
				     port);
#endif
				break;
			}
		}
		if (!success) {
			prints("425 Error: Unable to bind data socket.");
			return;
		}
		prepare_sock(pasvsock);
	}
	if (listen(pasvsock, 1)) {
		prints("425-Error: Unable to make socket listen.\r\n425 %s",
		       strerror(errno));
		return;
	}
	namelen = sizeof (localsock);
	getsockname(pasvsock, (struct sockaddr *) &localsock, (int *) &namelen);
	sscanf((char *) inet_ntoa(name.sin_addr), "%i.%i.%i.%i", &a1, &a2,
	       &a3, &a4);
	prints("227 Entering Passive Mode (%i,%i,%i,%i,%i,%i)", a1, a2, a3, a4,
	       ntohs(localsock.sin_port) >> 8,
	       ntohs(localsock.sin_port) & 0xFF);
	pasv = 1;
}

void
command_epsv(char *params)
{
	struct sockaddr_in localsock;
	int namelen;
	int af;
	if (params[0]) {
		if (!strncasecmp(params, "ALL", 3))
			epsvall = 1;
		else {
			if (sscanf(params, "%i", &af) < 1) {
				prints("501 Syntax error.");
				return;
			} else {
				if (af != 1) {
					prints
					    ("522 Protocol unsupported, use (1)");
					return;
				}
			}
		}
	}
	pasvsock = socket(AF_INET, SOCK_STREAM, 0);
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = 0;
	sa.sin_family = AF_INET;
	if (bind(pasvsock, (struct sockaddr *) &sa, sizeof (sa)) == -1) {
		prints("500-Error: Unable to bind data socket.\r\n425 %s",
		       strerror(errno));
		return;
	}
	if (listen(pasvsock, 1)) {
		prints("500-Error: Unable to make socket listen.\r\n425 %s",
		       strerror(errno));
		return;
	}
	namelen = sizeof (localsock);
	getsockname(pasvsock, (struct sockaddr *) &localsock, (int *) &namelen);
	prints("229 Entering extended passive mode (|||%i|)",
	       ntohs(localsock.sin_port));
	pasv = 1;
}

char
test_abort(char selectbefore, int file, int sock)
{
	char str[256];
	fd_set rfds;
	struct timeval tv;
	if (selectbefore) {
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(fileno(stdin), &rfds);
		if (!select(fileno(stdin) + 1, &rfds, NULL, NULL, &tv))
			return 0;
	}
	fgets(str, sizeof (str), stdin);
	if (strstr(str, "ABOR")) {
		prints("426 Transfer aborted.");
		close(file);
		close(sock);
		prints("226 Aborted.");
		yftpd_log("Client aborted file transmission.\n");
		alarm(CONTROL_TIMEOUT);
		return 1;
	}
	return 0;
}

void
command_allo(char *foo)
{
	command_noop(foo);
}

void
do_stor(char *filename, int flags)
{
	char *buffer;
	int fd, i, max;
	fd_set rfds;
	struct timeval tv;
	char *p, *pp;
	int quota_type;
	quota_type = my_quota_type(filename);
	if (quota_type < 0) {
		prints("Error: invalid file name");
		return;
	}
	if (quota->q[quota_type] <= 0) {
		yftpd_log
		    ("Error: quota limits reached while trying to store file '%s'.\n",
		     filename);
		prints("553 Error: quota limits reached.");
		return;
	}
	fd = my_open(filename, flags, 0666);
	if (fd == -1) {
		yftpd_log("Error: '%s' while trying to store file '%s'.\n",
			  strerror(errno), filename);
		prints("553 Error: %s.", strerror(errno));
		return;
	}
	yftpd_log("Client is storing file '%s'.\n", filename);
	if (dataconn()) {
		close(fd);
		return;
	}
	alarm(0);
	buffer = malloc(XFER_BUFSIZE);
	lseek(fd, offset, SEEK_SET);
	offset = 0;
	/* Do not use the whole buffer, because a null byte has to be
	 * written after the string in ASCII mode. */
	max = (sock > fileno(stdin) ? sock : fileno(stdin)) + 1;
	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		FD_SET(fileno(stdin), &rfds);
		tv.tv_sec = DATA_TIMEOUT;
		tv.tv_usec = 0;
		if (!select(max, &rfds, NULL, NULL, &tv)) {
			close(sock);
			close(fd);
			prints("426 Kicked due to data transmission timeout.");
			yftpd_log("Kicked due to data transmission timeout.\n");
			exit(0);
		}
		if (FD_ISSET(fileno(stdin), &rfds)) {
			test_abort(0, fd, sock);
			free(buffer);
			return;
		}
		if (!((i = recv(sock, buffer, XFER_BUFSIZE - 1, 0))))
			break;
		bytes_recvd += i;
		if (xfertype == TYPE_ASCII) {
			buffer[i] = '\0';
			p = pp = buffer;
			while (*p) {
				if ((unsigned char) *p == 13)
					p++;
				else
					*pp++ = *p++;
			}
			*pp++ = 0;
			i = strlen(buffer);
		}
		if (quota->q[quota_type] < i) {
			close(sock);
			close(fd);
			free(buffer);
			alarm(CONTROL_TIMEOUT);
			prints("553 Error: quota limits reached.");
			return;
		}
		write(fd, buffer, i);
		quota->q[quota_type] -= i;
		quota->changed++;
		flow_control();
	}
	close(fd);
	close(sock);
	alarm(CONTROL_TIMEOUT);
	free(buffer);
	prints("226 File transmission successful.");
	yftpd_log("File transmission successful.\n");
}

void
command_stor(char *filename)
{
	do_stor(filename, O_CREAT | O_WRONLY | O_TRUNC);
}

void
command_appe(char *filename)
{
	do_stor(filename, O_CREAT | O_WRONLY | O_APPEND);
}

void
command_retr(char *filename)
{
	char *buffer;
	int fd, i;
	struct stat statbuf;
#ifdef HAVE_SYS_SENDFILE_H
	off_t sendfile_offset;
#endif
	if (((fd = my_open(filename, O_RDONLY, 0))) == -1) {
		yftpd_log("Error: '%s' while trying to receive file '%s'.\n",
			  strerror(errno), filename);
		prints("553 Error: %s.", strerror(errno));
		return;
	}			/* No else, the file remains open so that it needn't be opened again */
	my_stat(filename, (struct stat *) &statbuf);
	yftpd_log("Client is receiving file '%s'.\n", filename);

	if (dataconn()) {
		close(fd);
		return;
	}
	alarm(DATA_TIMEOUT);
#ifdef HAVE_SYS_SENDFILE_H
	sendfile_offset = offset;
	if (xfertype != TYPE_ASCII) {
		alarm_type = fd;
		while (sendfile(sock, fd, &sendfile_offset, XFER_BUFSIZE)) {
			alarm(DATA_TIMEOUT);
			if (test_abort(1, fd, sock))
				return;
			flow_control();
		}
	} else {
#endif
		lseek(fd, offset, SEEK_SET);
		offset = 0;
		buffer = malloc(XFER_BUFSIZE * 2 + 1);
		alarm_type = fd;
		while ((i = read(fd, buffer, XFER_BUFSIZE))) {
			alarm(DATA_TIMEOUT);
			if (test_abort(1, fd, sock)) {
				free(buffer);
				return;
			}
#ifndef HAVE_SYS_SENDFILE_H
			if (xfertype == TYPE_ASCII) {
#endif
				buffer[i] = '\0';
				i += replace(buffer, "\n", "\r\n");
#ifndef HAVE_SYS_SENDFILE_H
			}
#endif
			send(sock, buffer, i, 0);
			flow_control();
			bytes_sent += i;
		}
		alarm_type = 0;
		free(buffer);
#ifdef HAVE_SYS_SENDFILE_H
	}
#endif
	close(fd);
	close(sock);
	alarm_type = 0;
	alarm(CONTROL_TIMEOUT);
	prints("226 File transmission successful.");
	yftpd_log("File transmission successful.\n");
}

void
do_dirlist(char *dirname, char verbose)
{
	FILE *datastream;
	if (dirname[0] != '\0') {
		/* skip arguments */
		if (dirname[0] == '-') {
			while ((dirname[0] != ' ') && (dirname[0] != '\0'))
				dirname++;
			if (dirname[0] != '\0')
				dirname++;
		}
	}
	if (dataconn())
		return;
	alarm(0);
	datastream = fdopen(sock, "w");
	if (dirname[0] == '\0')
		dirlist("*", datastream, verbose);
	else
		dirlist(dirname, datastream, verbose);
	fclose(datastream);
	alarm(CONTROL_TIMEOUT);
	prints("226 Directory list has been submitted.");
}

void
command_list(char *dirname)
{
	do_dirlist(dirname, 1);
}

void
command_nlst(char *dirname)
{
	do_dirlist(dirname, 0);
}

void
command_syst(char *params)
{
	prints("215 UNIX Type: L8");
}

void
command_mdtm(char *filename)
{
	struct stat statbuf;
	struct tm *filetime;
	if (!my_stat(filename, (struct stat *) &statbuf)) {
		filetime = gmtime((time_t *) & statbuf.st_mtime);
		prints("213 %04i%02i%02i%02i%02i%02i",
		       filetime->tm_year + 1900, filetime->tm_mon + 1,
		       filetime->tm_mday, filetime->tm_hour,
		       filetime->tm_min, filetime->tm_sec);
	} else {
		prints("550 Error while determining the modification time: %s",
		       strerror(errno));
	}
}

void
command_cwd(char *dir)
{
	if (dir[0] == '\0')
		strcpy(dir, "/");
	if (dir[0] == '~') {
		strcpy(dir, "/");
	}
	if (my_chdir(dir)) {
		yftpd_log("Error: '%s' while changing directory to '%s'.\n",
			  strerror(errno), dir);
		prints("451 Error: %s.", strerror(errno));
	} else {
		yftpd_log("Changed directory to '%s'.\n", dir);
		prints("250 OK");
	}
}

void
command_cdup(char *params)
{
	yftpd_log("Changed directory to '..'.\n");
	my_chdir("..");
	prints("250 OK");
}

void
command_dele(char *filename)
{
	if (my_unlink(filename)) {
		yftpd_log("Error: '%s' while trying to delete file '%s'.\n",
			  strerror(errno), filename);
		prints("451 Error: %s.", strerror(errno));
	} else {
		yftpd_log("Deleted file '%s'.\n", filename);
		prints("200 OK");
	}
}

void
command_mkd(char *dirname)
{
	if (my_mkdir(dirname, 0755)) {
		yftpd_log
		    ("Error: '%s' while trying to create directory '%s'.\n",
		     strerror(errno), dirname);
		prints("451 Error: %s.", strerror(errno));
	} else {
		yftpd_log("Created directory '%s'.\n", dirname);
		prints("257 \"%s\" has been created.", dirname);
	}
}

void
command_rmd(char *dirname)
{
	if (my_rmdir(dirname)) {
		yftpd_log
		    ("Error: '%s' while trying to remove directory '%s'.\n",
		     strerror(errno), dirname);
		prints("451 Error: %s.", strerror(errno));
	} else {
		yftpd_log("Removed directory '%s'.\n", dirname);
		prints("250 OK");
	}
}

void
command_noop(char *params)
{
	prints("200 OK");
}

void
command_rnfr(char *oldname)
{
	FILE *phile;
	if ((phile = my_fopen(oldname, "r"))) {
		fclose(phile);
		mystrncpy(philename, oldname, sizeof (philename) - 1);
		state = STATE_RENAME;
		prints("350 OK. New file name wanted.");
	} else {
		prints("451 Error: %s.", strerror(errno));
	}
}

void
command_rnto(char *newname)
{
	if (my_rename(philename, newname)) {
		yftpd_log("Error: '%s' while trying to rename '%s' to '%s'.\n",
			  strerror(errno), philename, newname);
		prints("451 Error: %s.", strerror(errno));
	} else {
		yftpd_log("Successfully renamed '%s' to '%s'.\n", philename,
			  newname);
		prints("250 OK. Successfully renamed.");
		state = STATE_AUTHENTICATED;
	}
}

void
command_rest(char *params)
{
	offset = strtoul(params, NULL, 10);
	prints("350 Restarting at offset %i.", offset);
}

void
command_size(char *filename)
{
	struct stat statbuf;
	if (!my_stat(filename, &statbuf)) {
		prints("213 %i", (int) statbuf.st_size);
	} else {
		prints("550 Error: %s.", strerror(errno));
	}
}

void
command_quit(char *params)
{
	prints("221 %s", QUIT_MSG);
	exit(0);
}

void
command_stat(char *filename)
{
	prints("213-Status of %s:", filename);
	yftpd_stat(filename, stderr, 1);
	prints("213 End of Status.");
}

/* SITE commands */
void
command_adminwho(char *params)
{
	struct yftpdutmp tmp;
	fprintf(stderr, "200-User listing follows.\n");
	fprintf(stderr,
		"200-PID       User           Host                Login time\n");
	rewind(yftpdutmp);
	while (fread((void *) &tmp, sizeof (tmp), 1, yftpdutmp)) {
		if (!tmp.bu_type)
			continue;
		fprintf(stderr, "200-%-10i%-15s%-20s%s", tmp.bu_pid,
			tmp.bu_name, tmp.bu_host, ctime(&(tmp.bu_time)));
	}
	fprintf(stderr, "200 User listing finished.\n");
}

void
command_adminkick(char *strpid)
{
	int pid = strtoul(strpid, NULL, 10);

	if (!HAS_PERM(PERM_SYSOP)) {
		prints("500 Error: only the administrator can kick.");
		return;
	}
	if (!pid)
		prints("500 Error: Given PID is not valid.");
	else if (yftpdutmp_pidexists(pid)) {
		if (kill(pid, SIGTERM))
			prints("500 Error: %s.", strerror(errno));
		else
			prints("200 OK");
	} else
		prints("500 Error: The given PID does not belong to yftpd.");
}

void
command_adminquota(char *params)
{
	int i;
	prints("200-Quota listing follows.");
	prints("200-Quotaid size     description");
	for (i = 0; i < NUM_QUOTA_TYPE; i++)
		prints("200-%-7d %-7dK %s", i, (int) (quota->q[i] / 1024),
		       quota->d[i]);
	prints("200 Quota listing finished.");
}

void
command_adminsetquota(char *params)
{
	int quotaid, sizek;
	if (!HAS_PERM(PERM_SYSOP)) {
		prints("500 Error: only the administrator can change quota.");
		return;
	}
	if (2 != sscanf(params, "%d %d", &quotaid, &sizek)) {
		prints
		    ("200 Sytax error: setquota <quotaid> <newsize in kbytes>.");
		return;
	}
	if (quotaid >= NUM_QUOTA_TYPE || quotaid < 0) {
		prints("200 Invalid quotaid number.");
		return;
	}
	if (sizek * 1024 < 0) {
		prints("200 Invalid quota size.");
		return;
	}
	quota->q[quotaid] = sizek * 1024;
	do_savequota(1);
	prints("200 quota size of %d is set to %dbytes.", quotaid,
	       sizek * 1024);
}

void
command_site(char *str)
{
	const struct command subcmds[] = {
		{"who", NULL, command_adminwho, STATE_AUTHENTICATED},
		{"kick", NULL, command_adminkick, STATE_AUTHENTICATED},
		{"quota", NULL, command_adminquota, STATE_AUTHENTICATED},
		{"setquota", NULL, command_adminsetquota, STATE_AUTHENTICATED},
		{NULL, NULL, 0},
	};
	int i;
	if (!ENABLE_SITE) {
		prints("550 SITE commands are disabled.");
		return;
	}
	for (i = 0; subcmds[i].name; i++) {
		if (!strncasecmp(str, subcmds[i].name, strlen(subcmds[i].name))) {
			cutto(str, strlen(subcmds[i].name));
			subcmds[i].function(str);
			return;
		}
	}
	prints("550 Unknown command: 'SITE %s'.", str);
}

/* Command parsing */
const struct command commands[] = {
	{"USER", "<sp> username", command_user, STATE_CONNECTED, 0},
	{"PASS", "<sp> password", command_pass, STATE_USER, 0},
	{"XPWD", "(returns cwd)", command_pwd, STATE_AUTHENTICATED, 1},
	{"PWD", "(returns cwd)", command_pwd, STATE_AUTHENTICATED, 0},
	{"TYPE", "<sp> type-code (A or I)", command_type, STATE_AUTHENTICATED, 0},
	{"PORT", "<sp> h1,h2,h3,h4,p1,p2", command_port, STATE_AUTHENTICATED, 0},

	{"EPRT", "<sp><d><net-prt><d><ip><d><tcp-prt><d>", command_eprt,
	 STATE_AUTHENTICATED, 1},
	{"PASV", "(returns address/port)", command_pasv, STATE_AUTHENTICATED,
	 0},
	{"EPSV", "(returns address/post)", command_epsv, STATE_AUTHENTICATED,
	 1},
	{"ALLO", "<sp> size", command_allo, STATE_AUTHENTICATED, 1},
	{"STOR", "<sp> pathname", command_stor, STATE_AUTHENTICATED, 0},
	{"APPE", "<sp> pathname", command_appe, STATE_AUTHENTICATED, 1},
	{"RETR", "<sp> pathname", command_retr, STATE_AUTHENTICATED, 0},
	{"LIST", "[<sp> pathname]", command_list, STATE_AUTHENTICATED, 0},
	{"NLST", "[<sp> pathname]", command_nlst, STATE_AUTHENTICATED, 0},
	{"SYST", "(returns system type)", command_syst, STATE_CONNECTED, 0},
	{"MDTM", "<sp> pathname", command_mdtm, STATE_AUTHENTICATED, 1},
	{"XCWD", "<sp> pathname", command_cwd, STATE_AUTHENTICATED, 1},
	{"CWD", "<sp> pathname", command_cwd, STATE_AUTHENTICATED, 0},
	{"XCUP", "(up one directory)", command_cdup, STATE_AUTHENTICATED, 1},
	{"CDUP", "(up one directory)", command_cdup, STATE_AUTHENTICATED, 0},
	{"DELE", "<sp> pathname", command_dele, STATE_AUTHENTICATED, 0},
	{"XMKD", "<sp> pathname", command_mkd, STATE_AUTHENTICATED, 1},
	{"MKD", "<sp> pathname", command_mkd, STATE_AUTHENTICATED, 0},
	{"XRMD", "<sp> pathname", command_rmd, STATE_AUTHENTICATED, 1},
	{"RMD", "<sp> pathname", command_rmd, STATE_AUTHENTICATED, 0},
	{"NOOP", "(no operation)", command_noop, STATE_AUTHENTICATED, 0},
	{"RNFR", "<sp> pathname", command_rnfr, STATE_AUTHENTICATED, 0},
	{"RNTO", "<sp> pathname", command_rnto, STATE_RENAME, 0},
	{"REST", "<sp> byte-count", command_rest, STATE_AUTHENTICATED, 1},
	{"SIZE", "<sp> pathname", command_size, STATE_AUTHENTICATED, 1},
	{"QUIT", "(close control connection)", command_quit, STATE_CONNECTED, 0},
	{"HELP", "[<sp> command]", command_help, STATE_AUTHENTICATED, 0},
	{"STAT", "<sp> pathname", command_stat, STATE_AUTHENTICATED, 0},
	{"SITE", "<sp> string", command_site, STATE_AUTHENTICATED, 0},

	{"FEAT", "(returns list of extensions)", command_feat,
	 STATE_AUTHENTICATED, 0},
	{NULL, NULL, NULL, 0, 0}
};

void
command_feat(char *params)
{
	int i;
	prints("211-Extensions supported:");
	for (i = 0; commands[i].name; i++)
		if (commands[i].showinfeat)
			prints(" %s", commands[i].name);
	prints("211 End");
}

void
command_help(char *params)
{
	int i;
	if (params[0] == '\0') {
		prints("214-The following commands are recognized.");
		for (i = 0; commands[i].name; i++)
			prints("214-%s", commands[i].name);
	} else {
		for (i = 0; commands[i].name; i++)
			if (!strcasecmp(params, commands[i].name))
				prints("214-Syntax: %s", commands[i].syntax);
	}
	prints("214 End of help");
}

int
parsecmd(char *str)
{
	int i;
	char *p, *pp;
	p = pp = str;		/* Remove garbage in the string */
	while (*p)
		if ((unsigned char) *p < 32)
			p++;
		else
			*pp++ = *p++;
	*pp++ = 0;
	for (i = 0; commands[i].name; i++) {	/* Parse command */
		if (strncasecmp
		    (str, commands[i].name, strlen(commands[i].name))) continue;
		cutto(str, strlen(commands[i].name));
		p = str;
		while ((*p) && ((*p == ' ') || (*p == '\t')))
			p++;
		memmove(str, p, strlen(str) - (p - str) + 1);
		if (state >= commands[i].state_needed) {
			readclubrights();
			commands[i].function(str);
			return 0;
		}
		switch (state) {
		case STATE_CONNECTED:
			prints("503 USER expected.");
			return 1;
		case STATE_USER:
			prints("503 PASS expected.");
			return 1;
		case STATE_AUTHENTICATED:
			prints("503 RNFR before RNTO expected.");
			return 1;
		}
	}
	prints("500 Unknown command: \"%s\"", str);
	return 0;
}
