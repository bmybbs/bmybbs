#ifndef __COMMANDS_H
#define __COMMANDS_H

#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

enum {
	STATE_CONNECTED, STATE_USER, STATE_AUTHENTICATED, STATE_RENAME
};

enum {
	TYPE_ASCII, TYPE_BINARY
};

#define USERLEN 30
#define MAXCMD 255

extern int state;
extern char user[USERLEN + 1];
extern struct sockaddr_in sa;
extern char pasv;
extern int sock;
extern int transferring;
extern int pasvsock;
extern char philename[256];
extern int offset;

void prints(char *format, ...);

void new_umask();
int parsecmd(char *);
int dataconn();
void command_user(char *);
void command_pass(char *);
void command_pwd(char *);
void command_type(char *);
void command_port(char *);
void command_stor(char *);
void command_retr(char *);
void command_list(char *);
void command_syst(char *);
void command_cwd(char *);
void command_cdup(char *);
void command_dele(char *);
void command_mkd(char *);
void command_rmd(char *);
void command_noop(char *);
void command_rnfr(char *);
void command_rnto(char *);
void command_rest(char *);
void command_abor(char *);
void command_quit(char *);
void command_help(char *);
void command_stat(char *);
void command_feat(char *);

struct command {
	char *name;
	char *syntax;
	void (*function) (char *);
	char state_needed;
	char showinfeat;
};

#endif
