/*
    功能     :  列出站内使用者资料
    注意事项 :  须要目前的 bbs.h

    Compile  :  gcc -o bfinger bfinger.c modetype.c

    使用方式 :  bfinger ~bbs/.UTMP.bbs i n f M
                (列出使用者的 ID,  nick, from host, mode )

    Note: you may want to ' ln -s bfinger rfinger ', because
          bfinger is used on BBS hosts   --> gather more precise info
          rfinger is used on hosts outside bbs -> less precise info
*/

#define  FIRSTLOGIN
#define  BBS_PASSFILE   MY_BBS_HOME "/.PASSWDS"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "ythtbbs.h"

int in_bbs;
struct user_info aman;
char field_str[20][128];
char field_idx[] = "uftmMinx";
int field_count = 0;
int field_lst_no[20];
int field_lst_size[20];
int field_default_size[20] = {
	12, 16, 12, 4, 10,
	12, 18, 8, 12, 24,
	30, 10, 8, 16, 20,
	30, 0, 0, 0, 0
};

char *field_name[] = {
	"UID",
	"From",
	"TTY",
	"Mode",
	"Mode",
	"ID",
	"Nick",
	"Idle",
	NULL
};

char *MYUTMPFILE;

set_opt(argc, argv)
int argc;
char *argv[];
{
	int i, flag, field, size;
	int *p;
	char *ptr, *field_ptr;

	field_count = 0;

	for (i = 2; i < argc; i++) {
		field_ptr = (char *) strchr(field_idx, argv[i][0]);
		if (field_ptr == NULL)
			continue;
		else
			field = field_ptr - field_idx;

		size = atoi(argv[i] + 1);

		field_lst_no[field_count] = field;
		field_lst_size[field_count] = (size == 0) ?
		    field_default_size[field] : size;
		field_count++;
	}

}

char *
repeat(ch, n)
int ch, n;
{
	char *p;
	int i;
	static char buf[256];

	p = buf;
	for (i = 0; i < n; i++)
		*(p++) = ch;
	*p = '\0';
	return buf;
}

print_head()
{
	int i, field, size;

	for (i = 0; i < field_count; i++) {
		field = field_lst_no[i];
		size = field_lst_size[i];
		printf("%-*.*s ", size, size, field_name[field]);
	}
	printf("\n");
	for (i = 0; i < field_count; i++) {
		field = field_lst_no[i];
		size = field_lst_size[i];
		printf("%-*.*s ", size, size, repeat('=', size));
	}
	printf("\n");
}

print_record()
{
	int i, field, size;

	for (i = 0; i < field_count; i++) {
		field = field_lst_no[i];
		size = field_lst_size[i];
		printf("%-*.*s ", size, size, field_str[field]);
	}
	printf("\n");
}

char *
idle_str(tty)
char *tty;
{

	struct stat buf;
	static char hh_mm_ss[80];
	time_t now, diff;
	int hh, mm;

	if ((stat(tty, &buf) != 0) || (strstr(tty, "tty") == NULL)) {
		strcpy(hh_mm_ss, "不详");
		return hh_mm_ss;
	};

	now = time(0);

	diff = now - buf.st_atime;

	hh = diff / 3600;
	mm = (diff / 60) % 60;

	if (hh > 0)
		sprintf(hh_mm_ss, "%d:%02d", hh, mm);
	else if (mm > 0)
		sprintf(hh_mm_ss, "%d", mm);
	else
		sprintf(hh_mm_ss, "   ");

	return hh_mm_ss;

}

char *
my_ctime(t)
time_t *t;
{
	static char time_str[80];
	strcpy(time_str, (char *) ctime(t));
	time_str[strlen(time_str) - 1] = '\0';
	return time_str;
}

report()
{
};

dump_record(serial_no, p)
int serial_no;
struct user_info *p;
{
	int i = 0, j;
	int pat;

	sprintf(field_str[i++], "%d", p->pid);
	sprintf(field_str[i++], "%s", p->from);
//    sprintf( field_str[ i++ ], "%s", p->tty );
	sprintf(field_str[i++], "%d", p->mode);
	sprintf(field_str[i++], "%s", ModeType(p->mode));
	sprintf(field_str[i++], "%s", p->userid);
	sprintf(field_str[i++], "%s", p->username);

/*
if (in_bbs) {
    sprintf( field_str[ i++ ], "%s", idle_str( p->tty ) );
} else {
    sprintf( field_str[ i++ ], "%s", "NA" );
}
*/

}

usage(prog_name)
char *prog_name;
{
	int i;

	printf("Usage: %s %s\n", prog_name, "utmp_file [XN] ....");
	printf("Example: %s %s\n", prog_name, "d3 i12 e30");
	printf("N is field width, X is one of the following char :\n");

	for (i = 0; field_name[i]; i++) {
		printf("\t%c -> %20.20s (default size = %2d)\n",
		       field_idx[i], field_name[i], field_default_size[i]);
	}
}

char *default_argv[] = {
	"bfinger", MY_BBS_HOME "/.UTMP.bbs", "i", "n", "f", "M"
};

main(argc, argv)
int argc;
char *argv[];
{
	FILE *inf;
	int i, user_num = 0;
	char *p;

	in_bbs = (strstr(argv[0], "bfinger") == NULL) ? 0 : 1;

	if (argc < 2) {
		set_opt(sizeof (default_argv) / sizeof (default_argv[0]),
			default_argv);
		MYUTMPFILE = default_argv[1];
	} else {
		set_opt(argc, argv);
		MYUTMPFILE = argv[1];
	}

	inf = fopen(MYUTMPFILE, "rb");
	if (inf == NULL) {
		printf("Error open %s\n", MYUTMPFILE);
		usage(argv[0]);
		exit(0);
	}

	if (strcmp(argv[2], "users") != 0) {
		print_head();
	}
	for (i = 0;; i++) {
		if (fread(&aman, sizeof (aman), 1, inf) <= 0)
			break;
		if (aman.active && (in_bbs ? (kill(aman.pid, 0) != -1) : 1)) {
			dump_record(i, &aman);
			if (strcmp(argv[2], "users") != 0) {
				print_record();
			}
			user_num++;
		}
	}
	if (strcmp(argv[2], "users") != 0) {
		printf("===================\n");
		printf("total users = %d\n", user_num);
	} else
		printf("%d\n", user_num);

	fclose(inf);
}
