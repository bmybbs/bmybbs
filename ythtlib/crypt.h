/* crypt.c */
#ifndef __CRYPT_H
#define __CRYPT_H
char *genpasswd(char *pw);
char *crypt1(const char *buf, const char *salt);
int checkpasswd(const char *pw_crypted, const char *pw_try);
#endif
