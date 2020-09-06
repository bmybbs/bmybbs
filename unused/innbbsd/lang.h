#ifndef LANG_H
#define LANG_H

extern char *FromTxt        ; 
extern char *BoardTxt       ; 
extern char *SubjectTxt     ;
extern char *OrganizationTxt;
extern char *PathTxt        ;
extern char *NCTUCSIETxt    ;
extern char *ModerationTxt  ;
extern char *bbslinkUsage1  ;
extern char *bbslinkUsage2  ; 
extern char *bbslinkUsage3  ;
extern char *bbslinkUsage4  ;

enum MsgLocale {Big5Locale, GBLocale, EnglishLocale};

typedef struct TxtClass {
  char *msgtxt;
  int size;
} TxtClass;

extern TxtClass OrganizationTxtClass[];
extern TxtClass FromTxtClass[];
extern TxtClass BoardTxtClass[];
extern TxtClass SubjectTxtClass[];
extern TxtClass PathTxtClass[];

#endif
