#ifndef __REGFORM_H
#define __REGFORM_H
/* regform.c */
#define NEWREGFILE MY_BBS_HOME "/new_register"
#define GETREGFILE MY_BBS_HOME "/new_register_getting"
#define SCANREGDIR MY_BBS_HOME "/scanregister_tmp/"
int getregforms(char *filename, int num, const char *userid);
#endif
