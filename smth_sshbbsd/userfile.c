/*

userfile.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jan 24 20:19:53 1996 ylo

*/

/*
 * $Log: userfile.c,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:44  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:49  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:13  bbsdev
 * bbs sshd
 *
 * Revision 1.19  1999/02/21 19:53:01  ylo
 * 	Intermediate commit of ssh1.2.27 stuff.
 * 	Main change is sprintf -> snprintf; however, there are also
 * 	many other changes.
 *
 * Revision 1.18  1998/05/23 20:28:26  kivinen
 *      Changed () -> (void).
 *
 * Revision 1.17  1998/04/17  00:43:08  kivinen
 *      Freebsd login capabilities support.
 *
 * Revision 1.16  1997/05/13 22:30:05  kivinen
 *      Added some casts.
 *
 * Revision 1.15  1997/03/26 07:10:04  kivinen
 *      Changed uid 0 to UID_ROOT.
 *
 * Revision 1.14  1997/03/25 05:47:00  kivinen
 *      Changed USERFILE_GET_DES_1_MAGIC_PHRASE to call
 *      userfile_get_des_1_magic_phrase.
 *
 * Revision 1.13  1997/03/19 17:53:46  kivinen
 *      Added USERFILE_GET_DES_1_MAGIC_PHRASE.
 *
 * Revision 1.12  1996/11/27 14:30:07  ttsalo
 *     Added group-writeability #ifdef
 *
 * Revision 1.11  1996/10/29 22:48:23  kivinen
 *      Removed USERFILE_LOCAL_SOCK and USERFILE_SEND.
 *
 * Revision 1.10  1996/10/07 11:40:20  ttsalo
 *      Configuring for hurd and a small fix to do_popen()
 *      from "Charles M. Hannum" <mycroft@gnu.ai.mit.edu> added.
 *
 * Revision 1.9  1996/10/04 01:01:49  kivinen
 *      Added printing of path to fatal calls in userfile_open and and
 *      userfile_local_socket_connect. Fixed userfile_open to
 *      userfile_local_socket_connect in calls to fatal in
 *      userfile_local_socket_connect.
 *
 * Revision 1.8  1996/09/27 17:18:06  ylo
 *      Fixed a typo.
 *
 * Revision 1.7  1996/09/08 17:21:08  ttsalo
 *      A lot of changes in agent-socket handling
 *
 * Revision 1.6  1996/09/04 12:39:59  ttsalo
 *      Added connecting to unix-domain socket
 *
 * Revision 1.5  1996/08/13 09:04:19  ttsalo
 *      Home directory, .ssh and .ssh/authorized_keys are now
 *      checked for wrong owner and group & world writeability.
 *
 * Revision 1.4  1996/05/29 07:37:31  ylo
 *      Do setgid and initgroups when initializing to read as user.
 *
 * Revision 1.3  1996/04/26 18:10:45  ylo
 *      Wrong file descriptors were closed in the forked child in
 *      do_popen.  This caused dup2 to fail on some machines, which in
 *      turn resulted in X11 authentication failing on some machines.
 *
 * Revision 1.2  1996/02/18 21:48:46  ylo
 *      Close pipes after popen fork.
 *      New function userfile_close_pipes.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 *      Imported ssh-1.2.13.
 *
 * $EndLog$
 */

/* Protocol for communication between the child and the parent: 

      Each message starts with a 32-bit length (msb first; includes
      type but not length itself), followed by one byte containing
      the packet type.

        1 USERFILE_OPEN
          string        file name
          int32         flags
          int32         mode

        2 USERFILE_OPEN_REPLY
          int32         handle (-1 if error)

        3 USERFILE_READ
          int32         handle
          int32         max_bytes

        4 USERFILE_READ_REPLY
          string        data      ;; empty data means EOF

        5 USERFILE_WRITE
          int32         handle
          string        data

        6 USERFILE_WRITE_REPLY
          int32         bytes_written  ;; != length of data means error
        
        7 USERFILE_CLOSE
          int32         handle

        8 USERFILE_CLOSE_REPLY
          int32         return value

        9 USERFILE_LSEEK
          int32         handle
          int32         offset
          int32         whence

       10 USERFILE_LSEEK_REPLY
          int32         returned_offset

       11 USERFILE_MKDIR
          string        path
          int32         mode

       12 USERFILE_MKDIR_REPLY
          int32         return value

       13 USERFILE_STAT
          string        path

       14 USERFILE_STAT_REPLY
          int32         return value
          sizeof(struct stat) binary bytes (in host order and layout)

       15 USERFILE_REMOVE
          string        path

       16 USERFILE_REMOVE_REPLY
          int32         return value

       17 USERFILE_POPEN
          string        command
          string        type

       18 USERFILE_POPEN_REPLY
          int32         handle (-1 if error)

       19 USERFILE_PCLOSE
          int32         handle

       20 USERFILE_PCLOSE_REPLY
          int32         return value

       21 USERFILE_GET_DES_1_MAGIC_PHRASE

       22 USERFILE_GET_DES_1_MAGIC_PHRASE_REPLY
          string        data
          
          */

#include "includes.h"
#include <gmp.h>
#include "userfile.h"
#include "getput.h"
#include "buffer.h"
#include "bufaux.h"
#include "xmalloc.h"
#include "ssh.h"




/* Protocol message types. */
#define USERFILE_OPEN           1
#define USERFILE_OPEN_REPLY     2
#define USERFILE_READ           3
#define USERFILE_READ_REPLY     4
#define USERFILE_WRITE          5
#define USERFILE_WRITE_REPLY    6
#define USERFILE_CLOSE          7
#define USERFILE_CLOSE_REPLY    8
#define USERFILE_LSEEK          9
#define USERFILE_LSEEK_REPLY   10
#define USERFILE_MKDIR         11
#define USERFILE_MKDIR_REPLY   12
#define USERFILE_STAT          13
#define USERFILE_STAT_REPLY    14
#define USERFILE_REMOVE        15
#define USERFILE_REMOVE_REPLY  16
#define USERFILE_POPEN         17
#define USERFILE_POPEN_REPLY   18
#define USERFILE_PCLOSE        19
#define USERFILE_PCLOSE_REPLY  20
#define USERFILE_GET_DES_1_MAGIC_PHRASE        21
#define USERFILE_GET_DES_1_MAGIC_PHRASE_REPLY  22


/* Flag indicating whether we have forked. */

/* Forks and execs the given command.  Returns a file descriptor for
   communicating with the program, or -1 on error.  The program will
   be run with empty environment to avoid LD_LIBRARY_PATH and similar
   attacks. */

int do_popen(const char *command, const char *type)
{
    int fds[2];
    int pid, i, j;
    char *args[100];
    char *env[100];
    extern char **environ;

    if (pipe(fds) < 0)
        fatal("pipe: %.100s", strerror(errno));

    pid = fork();
    if (pid < 0)
        fatal("fork: %.100s", strerror(errno));

    if (pid == 0) {             /* Child */


        /* Set up file descriptors. */
        if (type[0] == 'r') {
            if (dup2(fds[1], 1) < 0)
                perror("dup2 1");
        } else {
            if (dup2(fds[0], 0) < 0)
                perror("dup2 0");
        }
        close(fds[0]);
        close(fds[1]);

        /* Build argument vector. */
        i = 0;
        args[i++] = "/bin/sh";
        args[i++] = "-c";
        args[i++] = (char *) command;
        args[i++] = NULL;

        /* Prune environment to remove any potentially dangerous variables. */
        i = 0;
        for (j = 0; environ[j] && i < sizeof(env) / sizeof(env[0]) - 1; j++)
            if (strncmp(environ[j], "HOME=", 5) == 0 ||
                strncmp(environ[j], "USER=", 5) == 0 ||
                strncmp(environ[j], "HOME=", 5) == 0 ||
                strncmp(environ[j], "PATH=", 5) == 0 ||
                strncmp(environ[j], "LOGNAME=", 8) == 0 ||
                strncmp(environ[j], "TZ=", 3) == 0 ||
                strncmp(environ[j], "MAIL=", 5) == 0 ||
                strncmp(environ[j], "SHELL=", 6) == 0 ||
                strncmp(environ[j], "TERM=", 5) == 0 ||
                strncmp(environ[j], "DISPLAY=", 8) == 0 || strncmp(environ[j], "PRINTER=", 8) == 0 || strncmp(environ[j], "XAUTHORITY=", 11) == 0 || strncmp(environ[j], "TERMCAP=", 8) == 0)
                env[i++] = environ[j];
        env[i] = NULL;

        execve("/bin/sh", args, env);
        fatal("execv /bin/sh failed: %.100s", strerror(errno));
    }

    /* Parent. */
    if (type[0] == 'r') {       /* It is for reading. */
        close(fds[1]);
        return fds[0];
    } else {                    /* It is for writing. */
        close(fds[0]);
        return fds[1];
    }
}

/* Data structure for UserFiles. */

struct UserFile {
    enum { USERFILE_LOCAL, USERFILE_REMOTE } type;
    int handle;                 /* Local: file handle; remote: index to descriptor array. */
    unsigned char buf[512];
    unsigned int buf_first;
    unsigned int buf_last;
};

/* Allocates a UserFile handle and initializes it. */

static UserFile userfile_make_handle(int type, int handle)
{
    UserFile uf;

    uf = xmalloc(sizeof(*uf));
    uf->type = type;
    uf->handle = handle;
    uf->buf_first = 0;
    uf->buf_last = 0;
    return uf;
}

/* Opens a file using the given uid.  The uid must be either the current
   effective uid (in which case userfile_init need not have been called) or
   the uid passed to a previous call to userfile_init.  Returns a pointer
   to a structure, or NULL if an error occurred.  The flags and mode arguments
   are identical to open(). */

UserFile userfile_open(uid_t uid, const char *path, int flags, mode_t mode)
{
    int handle;

    if (uid == geteuid()) {
        handle = open(path, flags, mode);
        if (handle < 0)
            return NULL;
        return userfile_make_handle(USERFILE_LOCAL, handle);
    }

    return NULL;

}

/* Closes the userfile handle.  Returns >= 0 on success, and < 0 on error. */

int userfile_close(UserFile uf)
{
    int ret;

    switch (uf->type) {
    case USERFILE_LOCAL:
        ret = close(uf->handle);
        xfree(uf);
        return ret;

    default:
        fatal("userfile_close: type %d", uf->type);
         /*NOTREACHED*/ return -1;
    }
}

/* Get more data from the child into the buffer.  Returns false if no more
   data is available (EOF). */

static int userfile_fill(UserFile uf)
{
    unsigned int len;
    char *cp;
    int ret;

    if (uf->buf_first < uf->buf_last)
        fatal("userfile_fill: buffer not empty");

    switch (uf->type) {
    case USERFILE_LOCAL:
        ret = read(uf->handle, uf->buf, sizeof(uf->buf));
        if (ret <= 0)
            return 0;
        uf->buf_first = 0;
        uf->buf_last = ret;
        break;

    default:
        fatal("userfile_fill: type %d", uf->type);
    }

    return 1;
}

/* Returns the next character from the file (as an unsigned integer) or -1
   if an error is encountered. */

int userfile_getc(UserFile uf)
{
    if (uf->buf_first >= uf->buf_last) {
        if (!userfile_fill(uf))
            return -1;

        if (uf->buf_first >= uf->buf_last)
            fatal("userfile_getc/fill error");
    }

    return uf->buf[uf->buf_first++];
}

/* Reads data from the file.  Returns as much data as is the buffer
   size, unless end of file is encountered.  Returns the number of bytes
   read, 0 on EOF, and -1 on error. */

int userfile_read(UserFile uf, void *buf, unsigned int len)
{
    unsigned int i;
    int ch;
    unsigned char *ucp;

    ucp = buf;
    for (i = 0; i < len; i++) {
        ch = userfile_getc(uf);
        if (ch == -1)
            break;
        ucp[i] = ch;
    }

    return i;
}

/* Writes data to the file.  Writes all data, unless an error is encountered.
   Returns the number of bytes actually written; -1 indicates error. */

int userfile_write(UserFile uf, const void *buf, unsigned int len)
{
    unsigned int chunk_len, offset;
    int ret;
    const unsigned char *ucp;

    switch (uf->type) {
    case USERFILE_LOCAL:
        return write(uf->handle, buf, len);

    default:
        fatal("userfile_write: type %d", uf->type);
         /*NOTREACHED*/ return 0;
    }
}

/* Reads a line from the file.  The line will be null-terminated, and
   will include the newline.  Returns a pointer to the given buffer,
   or NULL if no more data was available.  If a line is too long,
   reads as much as the buffer can accommodate (and null-terminates
   it).  If the last line of the file does not terminate with a
   newline, returns the line, null-terminated, but without a
   newline. */

char *userfile_gets(char *buf, unsigned int size, UserFile uf)
{
    unsigned int i;
    int ch;

    for (i = 0; i < size - 1;) {
        ch = userfile_getc(uf);
        if (ch == -1)
            break;
        buf[i++] = ch;
        if (ch == '\n')
            break;
    }
    if (i == 0)
        return NULL;

    buf[i] = '\0';

    return buf;
}

/* Performs lseek() on the given file. */

off_t userfile_lseek(UserFile uf, off_t offset, int whence)
{
    switch (uf->type) {
    case USERFILE_LOCAL:
        return lseek(uf->handle, offset, whence);


    default:
        fatal("userfile_lseek: type %d", uf->type);
         /*NOTREACHED*/ return 0;
    }
}

/* Creates a directory using the given uid. */

int userfile_mkdir(uid_t uid, const char *path, mode_t mode)
{
    /* Perform directly if with current effective uid. */
    if (uid == geteuid())
        return mkdir(path, mode);
    return -1;
}

/* Performs stat() using the given uid. */

int userfile_stat(uid_t uid, const char *path, struct stat *st)
{
    int ret;

    /* Perform directly if with current effective uid. */
    if (uid == geteuid())
        return stat(path, st);
    return -1;
}

/* Performs remove() using the given uid. */

int userfile_remove(uid_t uid, const char *path)
{
    /* Perform directly if with current effective uid. */
    if (uid == geteuid())
        return remove(path);

    return -1;
}

/* Performs popen() on the given uid; returns a file from where the output
   of the command can be read (type == "r") or to where data can be written
   (type == "w"). */

UserFile userfile_popen(uid_t uid, const char *command, const char *type)
{
    int handle;

    if (uid == geteuid()) {
        handle = do_popen(command, type);
        if (handle < 0)
            return NULL;
        return userfile_make_handle(USERFILE_LOCAL, handle);
    }

    return NULL;
}

/* Performs pclose() on the given uid.  Returns <0 if an error occurs. */

int userfile_pclose(UserFile uf)
{
    int ret, ret2;

    switch (uf->type) {
    case USERFILE_LOCAL:
        ret = close(uf->handle);
        ret2 = wait(NULL);
        if (ret >= 0)
            ret = ret2;
        xfree(uf);
        return ret;

    default:
        fatal("userfile_close: type %d", uf->type);
         /*NOTREACHED*/ return -1;
    }
}
