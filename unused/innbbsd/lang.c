#include "stdio.h"
#include "lang.h"

char Big5FromTxt[]        ="祇獺: ";
char Big5BoardTxt[]       ="獺跋: ";
char Big5SubjectTxt[]     ="夹  肈: ";
char Big5OrganizationTxt[]="祇獺: ";
char Big5PathTxt[]        ="锣獺: ";
char Big5NCTUCSIETxt[]    ="ユ戈 News Server";
char Big5ModerationTxt[]  ="眤ゅ彻 \"%s\" 竒癳┕糵い. 叫单滦.\n";
char Big5bbslinkUsage1[]  ="セ祘Α璶タ盽磅︽ゲ斗盢郎竚 %s/innd :\n";
char Big5bbslinkUsage2[]  ="bbsname.bbs   砞﹚禥 BBS ID (叫鲸秖虏祏)\n";
char Big5bbslinkUsage3[]  ="nodelist.bbs  砞﹚呼隔 BBS  ID, Address ㎝ fullname\n";
char Big5bbslinkUsage4[]  ="newsfeeds.bbs 砞﹚呼隔獺ン newsgroup board nodelist ...\n";

char GBFromTxt[]        ="发信人: ";
char GBBoardTxt[]       ="信区: ";
char GBSubjectTxt[]     ="标  题: ";
char GBOrganizationTxt[]="发信站: ";
char GBPathTxt[]        ="转信站: ";
char GBNCTUCSIETxt[]    ="交大资工 News Server";
char GBModerationTxt[]  ="您的文章 \"%s\" 已经送往审核中. 请等待回覆.\n";
char GBbbslinkUsage1[]  ="本程式要正常执行必须将以下档案置於 %s/innd 下:\n";
char GBbbslinkUsage2[]  ="bbsname.bbs   设定贵站的 BBS ID (请尽量简短)\n";
char GBbbslinkUsage3[]  ="nodelist.bbs  设定网路各 BBS 站的 ID, Address 和 fullname\n";
char GBbbslinkUsage4[]  ="newsfeeds.bbs 设定网路信件的 newsgroup board nodelist ...\n";

char EFromTxt[]        ="From: ";
char EBoardTxt[]       ="Board: ";
char ESubjectTxt[]     ="Subject: ";
char EOrganizationTxt[]="Site: ";
char EPathTxt[]        ="Path: ";
char ENCTUCSIETxt[]    ="NCTU CSIE News Server";
char EModerationTxt[]  ="Your article \"%s\" has been sent to the moderator. Please wait for reply.\n";
char EbbslinkUsage1[]  ="To work normally, the following files should be put in %s/innd:\n";
char EbbslinkUsage2[]  ="bbsname.bbs   Your BBS ID (in short)\n";
char EbbslinkUsage3[]  ="nodelist.bbs  BBS ID, Address and fullname\n";
char EbbslinkUsage4[]  ="newsfeeds.bbs newsgroup board nodelist ...\n";

char *FromTxt        = Big5FromTxt;
char *BoardTxt       = Big5BoardTxt;
char *SubjectTxt     = Big5SubjectTxt;
char *OrganizationTxt= Big5OrganizationTxt;
char *PathTxt        = Big5PathTxt;
char *NCTUCSIETxt    = Big5NCTUCSIETxt;
char *ModerationTxt  = Big5ModerationTxt;
char *bbslinkUsage1  = Big5bbslinkUsage1;
char *bbslinkUsage2  = Big5bbslinkUsage2;
char *bbslinkUsage3  = Big5bbslinkUsage3;
char *bbslinkUsage4  = Big5bbslinkUsage4;


/*
typedef struct TxtClass {
  char *msgtxt;
  int size;
} TxtClass;
*/

TxtClass OrganizationTxtClass[]=
{
   {Big5OrganizationTxt, sizeof Big5OrganizationTxt},
   {GBOrganizationTxt,   sizeof GBOrganizationTxt},
   {EOrganizationTxt,    sizeof EOrganizationTxt},
   {NULL,0},
};

TxtClass FromTxtClass[]=
{
   {Big5FromTxt, sizeof Big5FromTxt},
   {GBFromTxt,   sizeof GBFromTxt},
   {EFromTxt,    sizeof EFromTxt},
   {NULL,0},
};

TxtClass SubjectTxtClass[]=
{
   {Big5SubjectTxt, sizeof Big5SubjectTxt},
   {GBSubjectTxt,   sizeof GBSubjectTxt},
   {ESubjectTxt,    sizeof ESubjectTxt},
   {NULL,0},
};

TxtClass PathTxtClass[]=
{
   {Big5PathTxt, sizeof Big5PathTxt},
   {GBPathTxt,   sizeof GBPathTxt},
   {EPathTxt,    sizeof EPathTxt},
   {NULL,0},
};

TxtClass BoardTxtClass[]=
{
   {Big5BoardTxt, sizeof Big5BoardTxt},
   {GBBoardTxt,   sizeof GBBoardTxt},
   {EBoardTxt,    sizeof EBoardTxt},
   {NULL,0},
};

isMsgTxt(txtclass,orgtxt)
TxtClass *txtclass;
char *orgtxt;
{
     TxtClass *ptr = txtclass;
     while (ptr && ptr->msgtxt && ptr->size > 0) {
       if (strncmp(ptr->msgtxt, orgtxt, ptr->size - 1)==0)
	 return 1;
       ptr++;
     }
     return 0;
}

initial_lang() 
{
	if (strcasecmp(LANG,"BIG5") == 0) {
	       lang_init( Big5Locale );
	} else if (strcasecmp(LANG,"GB") == 0) {
	       lang_init( GBLocale );
	} else if (strcasecmp(LANG,"ENGLISH") == 0) {
	       lang_init( EnglishLocale );
	}
}

lang_init(type)
int type;
{
  switch (type) {
  case Big5Locale:
	FromTxt        = Big5FromTxt;
	BoardTxt       = Big5BoardTxt;
	SubjectTxt     = Big5SubjectTxt;
	OrganizationTxt= Big5OrganizationTxt;
	PathTxt        = Big5PathTxt;
	NCTUCSIETxt    = Big5NCTUCSIETxt;
	ModerationTxt  = Big5ModerationTxt;
	bbslinkUsage1  = Big5bbslinkUsage1;
	bbslinkUsage2  = Big5bbslinkUsage2;
	bbslinkUsage3  = Big5bbslinkUsage3;
	bbslinkUsage4  = Big5bbslinkUsage4;
        break;
  case GBLocale:
	FromTxt        = GBFromTxt;
	BoardTxt       = GBBoardTxt;
	SubjectTxt     = GBSubjectTxt;
	OrganizationTxt= GBOrganizationTxt;
	PathTxt        = GBPathTxt;
	NCTUCSIETxt    = GBNCTUCSIETxt;
	ModerationTxt  = GBModerationTxt;
	bbslinkUsage1  = GBbbslinkUsage1;
	bbslinkUsage2  = GBbbslinkUsage2;
	bbslinkUsage3  = GBbbslinkUsage3;
	bbslinkUsage4  = GBbbslinkUsage4;
        break;
  case EnglishLocale:
	FromTxt        = EFromTxt;
	BoardTxt       = EBoardTxt;
	SubjectTxt     = ESubjectTxt;
	OrganizationTxt= EOrganizationTxt;
	PathTxt        = EPathTxt;
	NCTUCSIETxt    = ENCTUCSIETxt;
	ModerationTxt  = EModerationTxt;
	bbslinkUsage1  = EbbslinkUsage1;
	bbslinkUsage2  = EbbslinkUsage2;
	bbslinkUsage3  = EbbslinkUsage3;
	bbslinkUsage4  = EbbslinkUsage4;
	break;
  }
}
