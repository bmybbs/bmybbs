/*
Program: JIS.C
Version: 2.2
Date:    January 1, 1992
Author:  Ken R. Lunde, Adobe Systems Incorporated
  EMAIL: lunde@adobe.com
  MAIL : 1585 Charleston Road, P.O. Box 7900, Mountain View, CA 94039-7900
Type:    A utility for converting the kanji code of Japanese textfiles.
Code:    ANSI C (portable)
 
PORTABILITY:
This source code was written so that it would be portable on C compilers which
conform to the ANSI C standard. It has been tested on the following compilers:
THINK C (Macintosh), Vax C, Convex's C, Turbo C, and GNU C.
 
There are 4 lines which have been commented out. These lines of code are
necessary to develop a program which handles command-line arguments on a
Macintosh. I used THINK C as my development platform. I left these lines in
so that it would be easier to enhance/debug the program later. For those of
you who wish to use this program on the Macintosh, simply delete the comments
from those 4 lines of code, add the ANSI library to the THINK C project, and
then build the application. You then have a double-clickable application,
which when launched, will greet you with a Macintosh-style interface.
 
DISTRIBUTION AND RESTRICTIONS ON USAGE:
 1) Please give this source code away to your friends at no charge.
 2) Please try to compile this source code on various platforms to check for
    portablity, and please report back to me with any results be they good or
    bad. Suggestions are always welcome.
 3) Only use this program on a copy of a file -- do not use an original. This
    is just common sense.
 4) This source code or a compiled version may be bundled with commercial
    software as long as the author is notified beforehand. The author's name
    should also be mentioned in the credits.
 5) Feel free to use any of the algorithms for your own work. Many of them are
    being used in other programs I have written.
 6) The most current version can be obtained through FTP at ucdavis.edu
    (128.120.2.1) in the pub/JIS/C directory, or by requesting a copy
    directly from me.
 
DESCRIPTION:
 1) Supports Shift-JIS, EUC, New-JIS, Old-JIS, and NEC-JIS for both input and
    output.
 2) Automatically detects input file's kanji code. There is only one thing
    which will make this fail: a Shift-JIS file which only contains half-size
    katakana (remedy: insert a kanji anywhere in the file).
 3) The ability to convert half-size katakana in Shift-JIS and EUC into full-
    size equivalents. Note that half-size katakana includes other symbols such
    as a Japanese period, Japanese comma, center dot, etc.
 4) Supports conversion between the same code (i.e., EUC -> EUC, Shift-JIS ->
    Shift-JIS, etc.). This is useful as a filter for converting half-size
    katakana to their full-size equivalents.
 5) If the input file does not contain any Japanese, then the input file's
    contents are simply echoed to the output file. This adds reliability in
    case this program were used as a filter for incoming/outgoing mail.
 
OPERATION:
 1) The UNIX-style command-line is
 
    jis [-kanji-code] [-half-to-full] [infile] [outfile]
 
    The [-kanji-code] and [-half-to-full] flags can be in any order, but MUST
    come before the file names (if used). Note that [infile] and [outfile] can
    be replaced by redirecting stdin/stdout on UNIX systems.
 2) The [-kanji-code] flag, which determines the outfile's kanji code, can be
    1 of 5 possible values:
 
    -j    =    New-JIS (.new)
    -o    =    Old-JIS (.old)
    -n    =    NEC-JIS (.nec)
    -e    =    EUC (.euc)
    -s    =    Shift-JIS (.sjs)
 
    Upper- and lower-case are acceptable. The default kanji code flag (if the
    user doesn't select one) is -s (Shift-JIS).
 3) The [-half-to-full] flag, which determines whether to convert half-size
    katakana to their full-size counterparts, can be indicated by -f. The
    default is to NOT convert half-size katakana, but if the output file is
    in New-JIS, Old-JIS, or NEC-JIS, ALL half-size katakana will be converted
    to their full-size counterparts (necessary, since those 3 7-bit codes do
    not support half-size katakana). Again, upper- and lower-case are
    acceptable.
 4) The [infile] field is optional as one can redirect stdin and stdout.
 5) The [outfile] field is also optional. If no [outfile] field is specified,
    the program will semi-intelligently change the file's name. The program
    simply scans the [outfile] field, finds the last period in it, terminates
    the string at that point, and tacks on one of 5 possible extensions (they
    are listed under (2 above). Here are some example command lines, and the
    resulting outfile names:
 
    a) jis -e sig.jpn                     = sig.euc
    b) jis sig.jpn                        = sig.sjs (defaulted to Shift-JIS)
    c) jis -j sig.jpn.txt                 = sig.jpn.new
    d) jis -o sig                         = sig.old
 
    This is very useful for MS-DOS users since a filename such as sig.jpn.new
    will not result in converting a file called sig.jpn.
 
    Also note that if the outfile and infile have the same name, the program
    will not work, and data will be lost. I tried to build safe-guards against
    this. For example, note how my program will change the outfile name so
    that it does not overwrite the infile:
    
    a) jis -f sig.sjs                     = sig-.sjs
    b) jis -f sig.sjs sig.sjs             = sig-.sjs
    c) jis sig-.sjs                       = sig--.sjs
    
    If only the [infile] is given, a hyphen is inserted after the last period,
    and the extension is then reattached. If the outfile is specified by the
    user, then it will search for the last period (if any), attach a hyphen,
    and finally attach the proper extension). This sort of protection is NOT
    available from this program if stdin/stdout are used.
*/
 
/* #include <console.h>
#include <stdlib.h> */
#include <stdio.h>
#include <string.h>
 
#define NOT_SET     0
#define SET         1
#define NEW         1
#define OLD         2
#define NEC         3
#define EUC         4
#define SJIS        5
#define NEW_KI      "$B"
#define OLD_KI      "$@"
#define NEC_KI      "K"
#define NEW_KO      "(J"
#define OLD_KO      "(J"
#define NEC_KO      "H"
#define NUL         0
#define NL          10
#define FF          12
#define CR          13
#define ESC         27
#define TRUE        1
#define FALSE       0
#define PERIOD      '.'
#define SJIS1(A)    (((A >= 129) && (A <= 159)) || ((A >= 224) && (A <= 239)))
#define SJIS2(A)    ((A >= 64) && (A <= 252))
#define HANKATA(A)  ((A >= 161) && (A <= 223))
#define ISEUC(A)    ((A >= 161) && (A <= 254))
#define NOTEUC(A,B) (((A >= 129) && (A <= 159)) && ((B >= 64) && (B <= 160)))
#define ISMARU(A)   ((A >= 202) && (A <= 206))
#define ISNIGORI(A) (((A >= 182) && (A <= 196)) || ((A >= 202) && (A <= 206)))
 
void han2zen(FILE *in,int *ptr1,int *ptr2,int incode);
void SkipESCSeq(FILE *in,int data,int *ptr);
void sjis2jis(int *ptr1,int *ptr2);
void jis2sjis(int *ptr1,int *ptr2);
void echo2file(FILE *in,FILE *out);
void shift2seven(FILE *in,FILE *out,int outcode,int incode);
void shift2euc(FILE *in,FILE *out,int incode,int tofullsize);
void euc2seven(FILE *in,FILE *out,int outcode,int incode);
void euc2euc(FILE *in,FILE *out,int incode,int tofullsize);
void shift2shift(FILE *in,FILE *out,int incode,int tofullsize);
void euc2shift(FILE *in,FILE *out,int incode,int tofullsize);
void seven2shift(FILE *in,FILE *out);
void seven2euc(FILE *in,FILE *out);
void seven2seven(FILE *in,FILE *out,int outcode);
void KanjiIn(FILE *out,int data);
void KanjiOut(FILE *out,int data);
/* int ccommand(char ***p); */
int DetectCodeType(FILE *in);
 
main(int argc,char **argv)
{
  FILE *in,*out;
  char infilename[100],outfilename[100],extension[5];
  int c,incode,tofullsize = FALSE,outcode = NOT_SET;
 
/*  argc = ccommand(&argv); */
 
  while ((--argc > 0 ) && ((*++argv)[0] == '-')) {
    while (c = *++argv[0]) {
      switch (c) {
        case 'j' :
        case 'J' :
          outcode = NEW;
          strcpy(extension,".new");
          break;
        case 'o' :
        case 'O' :
          outcode = OLD;
          strcpy(extension,".old");
          break;
        case 'n' :
        case 'N' :
          outcode = NEC;
          strcpy(extension,".nec");
          break;
        case 'e' :
        case 'E' :
          outcode = EUC;
          strcpy(extension,".euc");
          break;
        case 's' :
        case 'S' :
          outcode = SJIS;
          strcpy(extension,".sjs");
          break;
        case 'f' :
        case 'F' :
          tofullsize = TRUE;
          break;
        default :
          break;
      }
    }
  }
  if (outcode == NOT_SET) {
    outcode = SJIS;
    strcpy(extension,".sjs");
  }
  if (argc == 0) {
    in = stdin;
    out = stdout;
  }
  else if (argc > 0) {
    if (argc == 1) {
      strcpy(infilename,*argv);
      if (strchr(*argv,PERIOD) != NULL)
        *strrchr(*argv,PERIOD) = '\0';
      strcpy(outfilename,*argv);
      strcat(outfilename,extension);
      if (!strcmp(infilename,outfilename)) {
        if (strchr(outfilename,PERIOD) != NULL)
          *strrchr(outfilename,PERIOD) = '\0';
        strcat(outfilename,"-");
        strcat(outfilename,extension);
      }
    }
    else if (argc > 1) {
      strcpy(infilename,*argv);
      strcpy(outfilename,*++argv);
      if (!strcmp(infilename,outfilename)) {
        if (strchr(outfilename,PERIOD) != NULL)
          *strrchr(outfilename,PERIOD) = '\0';
        strcat(outfilename,"-");
        strcat(outfilename,extension);
      }
    }
    if ((in = fopen(infilename,"r")) == NULL) {
      printf("\nCannot open %s\n",infilename);
      exit(1);
    }
    if ((out = fopen(outfilename,"w")) == NULL) {
      printf("\nCannot open %s\n",outfilename);
      exit(1);
    }
  }
  incode = DetectCodeType(in);
  rewind(in);
  switch (incode) {
    case NOT_SET :
      echo2file(in,out);
      break;
    case NEW :
    case OLD :
    case NEC :
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          seven2seven(in,out,outcode);
          break;
        case EUC :
          seven2euc(in,out);
          break;
        case SJIS :
          seven2shift(in,out);
          break;
      }
      break;
    case EUC :
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          euc2seven(in,out,outcode,incode);
          break;
        case EUC :
          euc2euc(in,out,incode,tofullsize);
          break;
        case SJIS :
          euc2shift(in,out,incode,tofullsize);
          break;
      }
      break;
    case SJIS :
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          shift2seven(in,out,outcode,incode);
          break;
        case EUC :
          shift2euc(in,out,incode,tofullsize);
          break;
        case SJIS :
          shift2shift(in,out,incode,tofullsize);
          break;
      }
      break;
  }
  exit(0);
}
 
void SkipESCSeq(FILE *in,int temp,int *shifted_in)
{
  if ((temp == '$') || (temp == '('))
    getc(in);
  if ((temp == 'K') || (temp == '$'))
    *shifted_in = TRUE;
  else
    *shifted_in = FALSE;
}
 
void KanjiIn(FILE *out,int outcode)
{
  switch (outcode) {
    case NEW :
      fprintf(out,"%c%s",ESC,NEW_KI);
      break;
    case OLD :
      fprintf(out,"%c%s",ESC,OLD_KI);
      break;
    case NEC :
      fprintf(out,"%c%s",ESC,NEC_KI);
      break;
  }
}
 
void KanjiOut(FILE *out,int outcode)
{
  switch (outcode) {
    case NEW :
      fprintf(out,"%c%s",ESC,NEW_KO);
      break;
    case OLD :
      fprintf(out,"%c%s",ESC,OLD_KO);
      break;
    case NEC :
      fprintf(out,"%c%s",ESC,NEC_KO);
      break;
  }
}
 
void sjis2jis(int *p1, int *p2)
{
	register unsigned char c1 = *p1;
	register unsigned char c2 = *p2;
	register int adjust = c2 < 159;
	register int rowOffset = c1 < 160 ? 112 : 176;
	register int cellOffset = adjust ? (31 + (c2 > 127)) : 126;
 
	*p1 = ((c1 - rowOffset) << 1) - adjust;
	*p2 -= cellOffset;
}
 
void jis2sjis(int *p1, int *p2)
{
	register unsigned char c1 = *p1;
	register unsigned char c2 = *p2;
	register int rowOffset = c1 < 95 ? 112 : 176;
	register int cellOffset = c1 % 2 ? 31 + (c2 > 95) : 126;
 
	*p1 = ((c1 + 1) >> 1) + rowOffset;
	*p2 = c2 + cellOffset;
}
 
void echo2file(FILE *in,FILE *out)
{
  int p1;
 
  while ((p1 = getc(in)) != EOF)
    fprintf(out,"%c",p1);
}
 
void shift2seven(FILE *in,FILE *out,int outcode,int incode)
{
  int shifted_in,p1,p2;
  
  shifted_in = FALSE;
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case NUL :
      case FF :
        break;
      case CR :
      case NL :
        if (shifted_in) {
          shifted_in = FALSE;
          KanjiOut(out,outcode);
        }
        fprintf(out,"%c",NL);
        break;
      default :
        if SJIS1(p1) {
          p2 = getc(in);
          if SJIS2(p2) {
            sjis2jis(&p1,&p2);
            if (!shifted_in) {
              shifted_in = TRUE;
              KanjiIn(out,outcode);
            }
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else if HANKATA(p1) {
          han2zen(in,&p1,&p2,incode);
          sjis2jis(&p1,&p2);
          if (!shifted_in) {
            shifted_in = TRUE;
            KanjiIn(out,outcode);
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else {
          if (shifted_in) {
            shifted_in = FALSE;
            KanjiOut(out,outcode);
          }
          fprintf(out,"%c",p1);
        }
        break;
    }
  }
  if (shifted_in)
    KanjiOut(out,outcode);
}
 
void shift2euc(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;
  while ((p1 = getc(in)) != EOF) {
printf("%c",p1);
    switch (p1) {
      case CR :
      case NL :
        fprintf(out,"%c",NL);
        break;
      case NUL :
      case FF :
        break;
      default :
        if SJIS1(p1) {
          p2 = getc(in);
          if SJIS2(p2) {
            sjis2jis(&p1,&p2);
            p1 += 128;
            p2 += 128;
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else if HANKATA(p1) {
          if (tofullsize) {
            han2zen(in,&p1,&p2,incode);
            sjis2jis(&p1,&p2);
            p1 += 128;
            p2 += 128;
          }
          else {
            p2 = p1;
            p1 = 142;
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else {
          fprintf(out,"%c",p1);
        }
        break;
    }
  }
}
 
void euc2seven(FILE *in,FILE *out,int outcode,int incode)
{
  int shifted_in,p1,p2;
 
  shifted_in = FALSE;
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case NL :
        if (shifted_in) {
          shifted_in = FALSE;
          KanjiOut(out,outcode);
        }
        fprintf(out,"%c",p1);
        break;
      case FF :
        break;
      default :
        if ISEUC(p1) {
          p2 = getc(in);
          if ISEUC(p2) {
            p1 -= 128;
            p2 -= 128;
            if (!shifted_in) {
              shifted_in = TRUE;
              KanjiIn(out,outcode);
            }
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else if (p1 == 142) {
          p2 = getc(in);
          if HANKATA(p2) {
            p1 = p2;
            han2zen(in,&p1,&p2,incode);
            sjis2jis(&p1,&p2);
            if (!shifted_in) {
              shifted_in = TRUE;
              KanjiIn(out,outcode);
            }
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else {
          if (shifted_in) {
            shifted_in = FALSE;
            KanjiOut(out,outcode);
          }
          fprintf(out,"%c",p1);
        }
        break;
    }
  }
  if (shifted_in)
    KanjiOut(out,outcode);
}
 
void euc2shift(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;
 
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case FF :
        break;
      default :
        if ISEUC(p1) {
          p2 = getc(in);
          if ISEUC(p2) {
            p1 -= 128;
            p2 -= 128;
            jis2sjis(&p1,&p2);
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else if (p1 == 142) {
          p2 = getc(in);
          if HANKATA(p2) {
            if (tofullsize) {
              p1 = p2;
              han2zen(in,&p1,&p2,incode);
              fprintf(out,"%c%c",p1,p2);
            }
            else {
              p1 = p2;
              fprintf(out,"%c",p1);
            }
          }
          else
            fprintf(out,"%c%c",p1,p2);
        }
        else
          fprintf(out,"%c",p1);
        break;
    }
  }
}
 
void seven2shift(FILE *in,FILE *out)
{
  int shifted_in,temp,p1,p2;
 
  shifted_in = FALSE;
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case ESC :
        temp = getc(in);
        SkipESCSeq(in,temp,&shifted_in);
        break;
      case FF :
        break;
      default :
        if (shifted_in) {
          p2 = getc(in);
          jis2sjis(&p1,&p2);
          fprintf(out,"%c%c",p1,p2);
        }
        else
          fprintf(out,"%c",p1);
        break;
    }
  }
}
  
void seven2euc(FILE *in,FILE *out)
{
  int shifted_in,temp,p1,p2;
 
  shifted_in = FALSE;
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case ESC :
        temp = getc(in);
        SkipESCSeq(in,temp,&shifted_in);
        break;
      case NL :
        if (shifted_in) {
          shifted_in = FALSE;
        }
        fprintf(out,"%c",p1);
        break;
      case FF :
        break;
      default :
        if (shifted_in) {
          p2 = getc(in);
          p1 += 128;
          p2 += 128;
          fprintf(out,"%c%c",p1,p2);
        }
        else
          fprintf(out,"%c",p1);
        break;
    }
  }
}
 
void seven2seven(FILE *in,FILE *out,int outcode)
{
  int shifted_in,temp,p1,p2;
 
  shifted_in = FALSE;
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case ESC :
        temp = getc(in);
        SkipESCSeq(in,temp,&shifted_in);
        if (shifted_in)
          KanjiIn(out,outcode);
        else
          KanjiOut(out,outcode);
        break;
      case NL :
        if (shifted_in) {
          shifted_in = FALSE;
          KanjiOut(out,outcode);
        }
        fprintf(out,"%c",p1);
        break;
      case FF :
        break;
      default :
        if (shifted_in) {
          p2 = getc(in);
          fprintf(out,"%c%c",p1,p2);
        }
        else
          fprintf(out,"%c",p1);
        break;
    }
  }
  if (shifted_in)
    KanjiOut(out,outcode);
}
 
void euc2euc(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;
 
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case FF :
        break;
      default :
        if ISEUC(p1) {
          p2 = getc(in);
          if ISEUC(p2)
            fprintf(out,"%c%c",p1,p2);
        }
        else if (p1 == 142) {
          p2 = getc(in);
          if (HANKATA(p2) && (tofullsize)) {
            p1 = p2;
            han2zen(in,&p1,&p2,incode);
            sjis2jis(&p1,&p2);
            p1 += 128;
            p2 += 128;
          }
          fprintf(out,"%c%c",p1,p2);
        }
        else
          fprintf(out,"%c",p1);
        break;
    }
  }
}
 
void shift2shift(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;
  
  while ((p1 = getc(in)) != EOF) {
    switch (p1) {
      case CR :
      case NL :
        fprintf(out,"%c",NL);
        break;
      case NUL :
      case FF :
        break;
      default :
        if SJIS1(p1) {
          p2 = getc(in);
          if SJIS2(p2)
            fprintf(out,"%c%c",p1,p2);
        }
        else if (HANKATA(p1) && (tofullsize)) {
          han2zen(in,&p1,&p2,incode);
          fprintf(out,"%c%c",p1,p2);
        }
        else
          fprintf(out,"%c",p1);
        break;
    }
  }
}
 
int DetectCodeType(FILE *in)
{
  int p1,p2,p3,whatcode;
 
  whatcode = NOT_SET;
  while (((p1 = getc(in)) != EOF) &&
  ((whatcode == NOT_SET) || (whatcode == EUC))) {
    if (p1 == ESC) {
      p2 = getc(in);
      if (p2 == '$') {
        p3 = getc(in);
        if (p3 == 'B')
          whatcode = NEW;
        else if (p3 == '@')
          whatcode = OLD;
      }
      else if (p2 == 'K')
        whatcode = NEC;
    }
    else if ((p1 >= 129) && (p1 <= 254)) {
      p2 = getc(in);
      if NOTEUC(p1,p2)
        whatcode = SJIS;
      else if (ISEUC(p1) && ISEUC(p2))
        whatcode = EUC;
      else if (((p1 == 142)) && HANKATA(p2))
        whatcode = EUC;
    }
  }
  return whatcode;
}
 
void han2zen(FILE *in,int *one,int *two,int incode)
{
  int junk,maru,nigori;
 
  maru = nigori = FALSE;
  if (incode == SJIS) {
    *two = getc(in);
    if (*two == 222) {
      if (ISNIGORI(*one) || (*one == 179))
        nigori = TRUE;
      else
        ungetc(*two,in);
    }
    else if (*two == 223) {
      if ISMARU(*one)
        maru = TRUE;
      else
        ungetc(*two,in);
    }
    else
      ungetc(*two,in);
  }
  else if (incode == EUC) {
    junk = getc(in);
    if (junk == 142) {
      *two = getc(in);
      if (*two == 222) {
        if (ISNIGORI(*one) || (*one == 179))
          nigori = TRUE;
        else {
          ungetc(*two,in);
          ungetc(junk,in);
        }
      }
      else if (*two == 223) {
        if ISMARU(*one)
          maru = TRUE;
        else {
          ungetc(*two,in);
          ungetc(junk,in);
        }
      }
      else {
        ungetc(*two,in);
        ungetc(junk,in);
      }
    }
    else
      ungetc(junk,in);
  }
  switch (*one) {
    case 161 :
      *one = 129;
      *two = 66;
      break;
    case 162 :
      *one = 129;
      *two = 117;
      break;
    case 163 :
      *one = 129;
      *two = 118;
      break;
    case 164 :
      *one = 129;
      *two = 65;
      break;
    case 165 :
      *one = 129;
      *two = 69;
      break;
    case 166 :
      *one = 131;
      *two = 146;
      break;
    case 167 :
      *one = 131;
      *two = 64;
      break;
    case 168 :
      *one = 131;
      *two = 66;
      break;
    case 169 :
      *one = 131;
      *two = 68;
      break;
    case 170 :
      *one = 131;
      *two = 70;
      break;
    case 171 :
      *one = 131;
      *two = 72;
      break;
    case 172 :
      *one = 131;
      *two = 131;
      break;
    case 173 :
      *one = 131;
      *two = 133;
      break;
    case 174 :
      *one = 131;
      *two = 135;
      break;
    case 175 :
      *one = 131;
      *two = 98;
      break;
    case 176 :
      *one = 129;
      *two = 91;
      break;
    case 177 :
      *one = 131;
      *two = 65;
      break;
    case 178 :
      *one = 131;
      *two = 67;
      break;
    case 179 :
      *one = 131;
      *two = 69;
      break;
    case 180 :
      *one = 131;
      *two = 71;
      break;
    case 181 :
      *one = 131;
      *two = 73;
      break;
    case 182 :
      *one = 131;
      *two = 74;
      break;
    case 183 :
      *one = 131;
      *two = 76;
      break;
    case 184 :
      *one = 131;
      *two = 78;
      break;
    case 185 :
      *one = 131;
      *two = 80;
      break;
    case 186 :
      *one = 131;
      *two = 82;
      break;
    case 187 :
      *one = 131;
      *two = 84;
      break;
    case 188 :
      *one = 131;
      *two = 86;
      break;
    case 189 :
      *one = 131;
      *two = 88;
      break;
    case 190 :
      *one = 131;
      *two = 90;
      break;
    case 191 :
      *one = 131;
      *two = 92;
      break;
    case 192 :
      *one = 131;
      *two = 94;
      break;
    case 193 :
      *one = 131;
      *two = 96;
      break;
    case 194 :
      *one = 131;
      *two = 99;
      break;
    case 195 :
      *one = 131;
      *two = 101;
      break;
    case 196 :
      *one = 131;
      *two = 103;
      break;
    case 197 :
      *one = 131;
      *two = 105;
      break;
    case 198 :
      *one = 131;
      *two = 106;
      break;
    case 199 :
      *one = 131;
      *two = 107;
      break;
    case 200 :
      *one = 131;
      *two = 108;
      break;
    case 201 :
      *one = 131;
      *two = 109;
      break;
    case 202 :
      *one = 131;
      *two = 110;
      break;
    case 203 :
      *one = 131;
      *two = 113;
      break;
    case 204 :
      *one = 131;
      *two = 116;
      break;
    case 205 :
      *one = 131;
      *two = 119;
      break;
    case 206 :
      *one = 131;
      *two = 122;
      break;
    case 207 :
      *one = 131;
      *two = 125;
      break;
    case 208 :
      *one = 131;
      *two = 126;
      break;
    case 209 :
      *one = 131;
      *two = 128;
      break;
    case 210 :
      *one = 131;
      *two = 129;
      break;
    case 211 :
      *one = 131;
      *two = 130;
      break;
    case 212 :
      *one = 131;
      *two = 132;
      break;
    case 213 :
      *one = 131;
      *two = 134;
      break;
    case 214 :
      *one = 131;
      *two = 136;
      break;
    case 215 :
      *one = 131;
      *two = 137;
      break;
    case 216 :
      *one = 131;
      *two = 138;
      break;
    case 217 :
      *one = 131;
      *two = 139;
      break;
    case 218 :
      *one = 131;
      *two = 140;
      break;
    case 219 :
      *one = 131;
      *two = 141;
      break;
    case 220 :
      *one = 131;
      *two = 143;
      break;
    case 221 :
      *one = 131;
      *two = 147;
      break;
    case 222 :
      *one = 129;
      *two = 74;
      break;
    case 223 :
      *one = 129;
      *two = 75;
      break;
  }
  if (nigori) {
    if (((*two >= 74) && (*two <= 103)) || ((*two >= 110) && (*two <= 122)))
      (*two)++;
    else if ((*one == 131) && (*two == 69))
      *two = 148;
  }
  else if ((maru) && ((*two >= 110) && (*two <= 122)))
    *two += 2;
}
