/*

userfile.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jan 24 19:53:02 1996 ylo

Functions for reading files as the real user from a program running as root.

This works by forking a separate process to do the reading.

*/

/*
 * $Log: userfile.h,v $
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
 * Revision 1.9  1997/03/26 05:36:21  kivinen
 * 	Fixed prototypes.
 *
 * Revision 1.8  1997/03/19 17:53:56  kivinen
 * 	Added USERFILE_GET_DES_1_MAGIC_PHRASE.
 *
 * Revision 1.7  1996/10/29 22:48:43  kivinen
 * 	Removed userfile_local_socket_connect and userfile_send
 * 	prototypes.
 *
 * Revision 1.6  1996/09/08 17:21:07  ttsalo
 * 	A lot of changes in agent-socket handling
 *
 * Revision 1.5  1996/09/04 12:40:00  ttsalo
 * 	Added connecting to unix-domain socket
 *
 * Revision 1.4  1996/08/13 09:04:20  ttsalo
 * 	Home directory, .ssh and .ssh/authorized_keys are now
 * 	checked for wrong owner and group & world writeability.
 *
 * Revision 1.3  1996/05/29 07:42:02  ylo
 * 	Updated prototype of userfile_init.
 *
 * Revision 1.2  1996/02/18 21:49:10  ylo
 * 	Added userfile_close_pipes.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:10  ylo
 * 	Imported ssh-1.2.13.
 *
 * $EndLog$
 */

#ifndef USERFILE_H
#define USERFILE_H

typedef struct UserFile *UserFile;

/* Initializes reading as a user.  Before calling this, I/O may only be
   performed as the user that is running the current program (current
   effective uid).  SIGPIPE should be set to ignored before this call.
   The cleanup callback will be called in the child before switching to the
   user's uid.  The callback may be NULL. */
void userfile_init(const char *username, uid_t uid, gid_t gid, void (*cleanup_callback) (void *), void *context);

/* Stops reading files as an ordinary user.  It is not an error to call this
   even if userfile_init has not been called. */
void userfile_uninit(void);

/* Closes any pipes the userfile might have open.  This should be called after
   every fork. */
void userfile_close_pipes(void);

/* Opens a file using the given uid.  The uid must be either the current
   effective uid (in which case userfile_init need not have been called) or
   the uid passed to a previous call to userfile_init.  Returns a pointer
   to a structure, or NULL if an error occurred.  The flags and mode arguments
   are identical to open(). */
UserFile userfile_open(uid_t uid, const char *path, int flags, mode_t mode);

/* Closes the userfile handle.  Returns >= 0 on success, and < 0 on error. */
int userfile_close(UserFile f);

/* Returns the next character from the file (as an unsigned integer) or -1
   if an error is encountered. */
int userfile_getc(UserFile f);

/* Reads data from the file.  Returns as much data as is the buffer
   size, unless end of file is encountered.  Returns the number of bytes
   read, 0 on EOF, and -1 on error. */
int userfile_read(UserFile f, void *buf, unsigned int len);

/* Writes data to the file.  Writes all data, unless an error is encountered.
   Returns the number of bytes actually written; -1 indicates error. */
int userfile_write(UserFile f, const void *buf, unsigned int len);

/* Reads a line from the file.  The line will be null-terminated, and
   will include the newline.  Returns a pointer to the given buffer,
   or NULL if no more data was available.  If a line is too long,
   reads as much as the buffer can accommodate (and null-terminates
   it).  If the last line of the file does not terminate with a
   newline, returns the line, null-terminated, but without a
   newline. */
char *userfile_gets(char *buf, unsigned int size, UserFile f);

/* Performs lseek() on the given file. */
off_t userfile_lseek(UserFile uf, off_t offset, int whence);

/* Creates a directory using the given uid. */
int userfile_mkdir(uid_t uid, const char *path, mode_t mode);

/* Performs stat() using the given uid. */
int userfile_stat(uid_t uid, const char *path, struct stat *st);

/* Performs remove() using the given uid. */
int userfile_remove(uid_t uid, const char *path);

/* Performs popen() on the given uid; returns a file from where the output
   of the command can be read (type == "r") or to where data can be written
   (type == "w"). */
UserFile userfile_popen(uid_t uid, const char *command, const char *type);

/* Performs pclose() on the given uid.  Returns <0 if an error occurs. */
int userfile_pclose(UserFile uf);

/* Check owner and permissions of a given file/directory.
   Permissions ----w--w- must not exist and owner must be either
   pw->pw_uid or root. Return value: 0 = not ok, 1 = ok */
int userfile_check_owner_permissions(struct passwd *pw, const char *path);

/* Encapsulate a normal file descriptor inside a struct UserFile */
UserFile userfile_encapsulate_fd(int fd);

/* Get sun des 1 magic phrase, return NULL if not found */
char *userfile_get_des_1_magic_phrase(uid_t uid);

#endif                          /* USERFILE_H */
