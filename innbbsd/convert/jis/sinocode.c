/*
  Copyright (C) 1992 Per Hammarlund <perham@nada.kth.se>
*/

/*
  Written by Per Hammarlund <perham@nada.kth.se>
  Helpful remarks from Ken-Ichi Handa <handa@etl.go.jp>.
  The sinoco??.cod (translation) files are provided by Urs Widmer
  <a06g@alf.zfn.uni-bremen.de>.
*/

/*	
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.
	 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
	 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/ 

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#define VERSIONSTRING "Sinocode Version 0.11"

				/* make sure that you have a "/" at */
				/* the end of this string. */
#ifndef TRANSLATIONSFILESDIR
#define TRANSLATIONSFILESDIR "/home/sans/perham/src/new-sinocode/"
#endif


#define PROGNAME 0
#define HELPORVERSION 1
#define INFILECONV 1
#define INFILENAME 2
#define OUTFILECONV 3
#define OUTFILENAME 4

#define GB_2312_TO_BIG_5     0
#define GB_2312_TO_IBM_555   1
#define GB_2312_TO_JIS_EUC   2
#define BIG_5_TO_GB_2312     3
#define BIG_5_TO_IBM_555     4
#define BIG_5_TO_JIS_EUC     5
#define JIS_EUC_TO_GB_2312   6
#define JIS_EUC_TO_BIG_5     7
#define IBM_555_TO_BIG_5     8
#define IBM_555_TO_GB_2312   9

				/* this should be done with a malloc */
				/* depending on the size of the */
				/* translation file... */
#define NO_CHAR_IN_TABLE 16000
#define FIRST 0
#define SECOND 1

#define NULL 0
				/* numbering the coding */
#define NO_CODES     4
#define JIS_EUC_CODE 0
#define BIG5_CODE    1
#define GB2312_CODE  2
#define IBM555_CODE  3
				/* some strings for some useful */
				/* missing characters */
#define NO_NAMED_CHARACTERS 3
#define FAT_EQUAL  "fat_equal"
#define BLACK_BOX  "black_box"
#define SQUARE_BOX "square_box"

				/* the names of the missing chars */
				/* enviornment vars */
#define SINOCODE_DEFAULT_MISSING_CHAR "SINOMISSDEF"
#define JIS_EUC_CHAR                  "SINOJISEUC"
#define BIG5_CHAR                     "SINOBIG5"
#define GB2312_CHAR                   "SINOGB2312"
#define IBM555_CHAR                   "SINOIBM555"

typedef unsigned short u16;
typedef struct code_pair
{
  char first, second;
} code_pair;

typedef struct named_character 
{
  char * char_name;
  code_pair character_codes[NO_CODES];
} named_character;

named_character codes_for_named_characters[NO_NAMED_CHARACTERS] = 
{
  {FAT_EQUAL,
     {
       {(char) 0xA2, (char) 0xAE},
       {(char) 0x21, (char) 0x7E},
       {(char) 0xA1, (char) 0xFE},
       {(char) 0x22, (char) 0x2E}
     }},
  {BLACK_BOX,
     {
       {(char) 0xA2, (char) 0xA3},
       {(char) 0xA1, (char) 0xBD},
       {(char) 0xA1, (char) 0xF6},
       {(char) 0xFF, (char) 0xFF}
     }},
  {SQUARE_BOX,
     {
       {(char) 0xA2, (char) 0xA2},
       {(char) 0xA1, (char) 0xBC},
       {(char) 0xA1, (char) 0xF5},
       {(char) 0xFF, (char) 0xFF}
     }}};


  
char * env_vars_to_check[NO_CODES] = 
{
  JIS_EUC_CHAR,
  BIG5_CHAR,
  GB2312_CHAR,
  IBM555_CHAR
  };

char * default_missing_char_names[NO_CODES] =
  {
    FAT_EQUAL,
    FAT_EQUAL,
    FAT_EQUAL,
    FAT_EQUAL
  };

char default_missing_char_codes[NO_CODES][2];

static int translation_table[NO_CHAR_IN_TABLE][2];

static char * progname;
				/* file names of the translation files */
static char * translation_files[10] = 
{
  "sinocogb.cod",
  "sinocogb.cod",
  "sinocogj.cod",
  "sinocobg.cod",
  "",
  "sinocobj.cod",
  "sinocojg.cod",
  "sinocojb.cod",
  "",
  "sinocobg.cod"
  };

void print_help()
{
  fprintf( stdout, "There is no help so far!\n");
}



void setup_missing_chars_to_use(void)
{
  char * env_ptr;
  int i, j;
				/* first check the default env var */
  if( (env_ptr = getenv( SINOCODE_DEFAULT_MISSING_CHAR )) != NULL)
    for( i = 0; i < NO_CODES; i++)
      default_missing_char_names[i] = env_ptr;
  
				/* Then check the others */
  for( i = 0; i < NO_CODES; i++)
    if( (env_ptr = getenv(env_vars_to_check[i])) != NULL )
      default_missing_char_names[i] = env_ptr;
  
				/* Then set the codes up from the */
				/* names they have been given */
  for( i = 0; i < NO_CODES; i++)
    {
				/* possibly loop through all the */
				/* indexes */
      for( j = 0; j < NO_NAMED_CHARACTERS; j++)
	if( ! strcmp(default_missing_char_names[i],
		     codes_for_named_characters[j].char_name))
	  break;

				/* now we have either found a coding */
				/* or we have to use the two byte */
				/* string supplied by the user */
      if( j != NO_NAMED_CHARACTERS )
	{
	  default_missing_char_codes[i][0] =
	    codes_for_named_characters[j].character_codes[i].first;
	  default_missing_char_codes[i][1] =
	    codes_for_named_characters[j].character_codes[i].second;
	}
      else
				/* use what the user has supplied */
	if( 2 == strlen( default_missing_char_names[i] ))
	  {
	    default_missing_char_codes[i][0] =
	      default_missing_char_names[i][0];
	    default_missing_char_codes[i][1] =
	      default_missing_char_names[i][1];
	  }
	else
	  {
	    fprintf( stderr,
		    "%s: Your environment variables \"%s\" or \"%s\" are\nnot correct.\n",
		    progname,
		    SINOCODE_DEFAULT_MISSING_CHAR,
		    env_vars_to_check[i]);
	    fprintf(stderr, "Either they should contain one of the following strings:\n");
	    for( i = 0; i < NO_NAMED_CHARACTERS; i++)
	      fprintf( stderr, "\"%s\"\n",
		      codes_for_named_characters[i].char_name );
	    fprintf( stderr, "Or be a two byte code for a characters\n");
	    exit(1);
	  }
    }
}				/* setup_missing_chars_to_use */

		  
void read_translation_table( char *filename )
{
  char *realpath;
  FILE *tranfp;
  int i, junk;
  
				/* allocate memory for the real */
				/* filename */
  if( ! (realpath = (char *) malloc(strlen(TRANSLATIONSFILESDIR) +
				    strlen(filename) +
				    1)))
    {
      fprintf( stderr, "%s: Couldn't get enough memory.\n",
	      progname );
      exit( 1 );
    }
  
  strcpy( realpath, TRANSLATIONSFILESDIR);
  strcat( realpath, filename );

  if( !(tranfp = fopen( realpath, "r" )))
    {
      fprintf( stderr, "%s: Couldn't open translation file: \"%s\"\n",
	      progname, realpath );
      exit( 1 );
    }
  
  for( i = 1; 1; i++ )
    {
      translation_table[i][FIRST] = getc( tranfp );
      translation_table[i][SECOND] = getc( tranfp );
      junk = getc( tranfp );
      if( junk == EOF )
	break;
    }	
  return;
}

void find_gb_big_code(int ch1, int ch2, int * outch1, int * outch2)
{
  int index;
  
  index = ((ch1 - 161) * 94)+ ch2 - 160;

  *outch1 = translation_table[index][FIRST];
  *outch2 = translation_table[index][SECOND];

  return;
}

void find_big_gb_code( int ch1, int ch2, int * outch1, int * outch2 )
{
  int index;

  index = ((ch1-161)*157)+ch2-63-((ch2 < 161)?0:34);

  *outch1 = translation_table[index][FIRST];
  *outch2 = translation_table[index][SECOND];

  return;
}				/* find_big_gb_code */

void find_big( int char1, int char2,
	      int * outchar1, int * outchar2 )
{
  int rno, tmp1, tmp2;
  rno = (char1-137)*188 + char2 - 63 - (char2>126)?1:0;
  rno -= (rno>408)?93:0;
  tmp1 = rno%157+161;
  tmp2 = (rno - ((rno % 157) * 157))+63;
  tmp2 += (tmp2>126)?34:0;
  *outchar1 = tmp1;
  *outchar2 = tmp2;
  return;  
}				/* find_big */


void find_ibm( int char1, int char2,
	      int * outchar1, int * outchar2 )
{
  int rno, tmp1, tmp2;
  
  rno = (char1 - 161) * 157 + char2 - 63-(char2>126)?34:0;
  rno += (rno>408)?93:0;
  tmp1 = (rno % 188) + 137;
  tmp2 = (rno - ((rno % 188) * 188))+63;
  if( tmp2 > 126 )
    tmp2++;
  *outchar1 = tmp1;
  *outchar1 = tmp2;
  return;
}				/* find_ibm */


				/* 1 */
void gb_2312_to_big_5( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  
  read_translation_table( translation_files[GB_2312_TO_BIG_5]);
  
  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_gb_big_code( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[BIG5_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[BIG5_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  
  return;
}
				/* 2 */
void gb_2312_to_ibm_555( FILE *infilefp, FILE *outfilefp )
{ 
  int char1, char2;
  int outchar1, outchar2;
  
  read_translation_table( translation_files[GB_2312_TO_IBM_555]);
  
  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_gb_big_code( char1, char2, & outchar1, & outchar2 );
				/* convert to IBM */
	  if ( outchar1 < 161 )
	    outchar1 = (int) ' ';
	  else
	    find_ibm( outchar1, outchar2, &outchar1, &outchar2 );

	  if ( outchar1 == (int) ' ' )
	    {
	      putc( default_missing_char_codes[IBM555_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[IBM555_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  return;
}

				/* 3 */
void gb_2312_to_jis_euc( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[GB_2312_TO_JIS_EUC]);

  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_gb_big_code( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[JIS_EUC_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[JIS_EUC_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  return;
}

				/* 4 */
void big_5_to_gb_2312( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[BIG_5_TO_GB_2312]);

  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_big_gb_code( char1, char2, & outchar1, &outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[GB2312_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[GB2312_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  
  return;
}

				/* 5 */
void big_5_to_ibm_555( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[BIG_5_TO_IBM_555]);

  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_ibm( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[IBM555_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[IBM555_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }

  return;
}

				/* 6 */
void big_5_to_jis_euc( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[BIG_5_TO_JIS_EUC]);

  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_big_gb_code( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (int) ' ' )
	    {
	      putc( default_missing_char_codes[JIS_EUC_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[JIS_EUC_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  return;
}

				/* 7 */
void jis_euc_to_gb_2312( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[JIS_EUC_TO_GB_2312]);
  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_gb_big_code( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[GB2312_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[GB2312_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }

  return;
}

				/* 8 */
void jis_euc_to_big_5( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[JIS_EUC_TO_BIG_5]);

  while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 160 && char1 <= 255 )
	{
	  char2 = getc( infilefp );
	  find_gb_big_code( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[BIG5_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[BIG5_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) outchar1, outfilefp);
	      putc( (char) outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  return;
}

				/* 9 */
void ibm_555_to_big_5( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;

  return;

#ifdef IBM
  read_translation_table( translation_files[ibm_555_to_big_5]);
    while ( (char1 = getc( infilefp )) != EOF )
    {
      if ( char1 >= 134 && char <= 255 )
	{
	  char2 = getc( infilefp );
	  find_gb_big_code( char1, char2, & outchar1, & outchar2 );
	  if ( outchar1 == (char) ' ' )
	    {
	      putc( default_missing_char_codes[BIG5_CODE][FIRST], outfilefp);
	      putc( default_missing_char_codes[BIG5_CODE][SECOND], outfilefp);
	    }
	  else
	    {
	      putc( (char) * outchar1, outfilefp);
	      putc( (char) * outchar2, outfilefp);
	    }
	}
      else
	putc( (char) char1, outfilefp );
    }
  return;
#endif
}

				/* 10 */
void ibm_555_to_gb_2312( FILE *infilefp, FILE *outfilefp )
{
  int char1, char2;
  int outchar1, outchar2;
  read_translation_table( translation_files[IBM_555_TO_GB_2312]);

  find_big( char1, char2, & outchar1, & outchar2 );
  find_big_gb_code( outchar1, outchar2, &outchar1, &outchar2 );
  if ( outchar1 == (char) ' ' )
    {
      putc( default_missing_char_codes[GB2312_CODE][FIRST], outfilefp);
      putc( default_missing_char_codes[GB2312_CODE][SECOND], outfilefp);
    }
  else
    {
      putc( (char) outchar1, outfilefp);
      putc( (char) outchar2, outfilefp);
    }
  return;
}


void main(argc, argv)
  int argc;
  char **argv;
{
  FILE *infilefp, *outfilefp;
  
  progname = argv[0];
				/* make sure we use the correct */
				/* missing chars */
  setup_missing_chars_to_use();
  
				/* this should aways be called with */
				/* either one of four arguments. */
  if( argc == 2)
    {
      if( !strcmp(argv[HELPORVERSION], "-version"))
	{
	  fprintf( stdout, "%s: %s\n", argv[PROGNAME], VERSIONSTRING );
	  exit( 0 );
	}
      else
	if( ! strcmp(argv[HELPORVERSION], "-help"))
	  {
	    print_help();
	    exit( 0 );
	  }
    }
  else
    if( argc == 5 )
      {
				/* first check the two files. */
	if( ! strcmp( argv[INFILENAME], "-"))
	  infilefp = stdin;
	else
	  if( ! (infilefp = fopen( argv[INFILENAME], "r")))
	    {
	      fprintf(stderr, "%s: Couldn't open input file: \"%s\"\n",
		      argv[PROGNAME], argv[INFILENAME]);
	      exit( 1 );
	    }

	if( ! strcmp( argv[OUTFILENAME], "-"))
	  outfilefp = stdout;
	else
				/* perhaps we should stat here to make */
				/* sure that we do not overwrite */
				/* something. perham: future */
	  if( ! (outfilefp = fopen( argv[OUTFILENAME], "w")))
	    {
	      fprintf(stderr, "%s: Couldn't open output file: \"%s\"\n",
		      argv[PROGNAME], argv[INFILENAME]);
	      exit( 1 );
	    }
				/* then parse the switches and */
				/* directly jump to the conversion */

	/* GB-2312   -->  BIG-5  */
	if((0 == strcmp( argv[INFILECONV], "-gb")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-big")))
	  {
	    gb_2312_to_big_5( infilefp, outfilefp );
	    goto out;
	  }

	/* GB-2312   -->  IBM-555  */
	if((0 == strcmp( argv[INFILECONV], "-gb")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-ibm")))
	  {
	    ( infilefp, outfilefp );
	    goto out;
	  }

	/* GB-2312   -->  JIS (EUC)  */
	if((0 == strcmp( argv[INFILECONV], "-gb")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-jis")))
	  {
	    gb_2312_to_jis_euc( infilefp, outfilefp );
	    goto out;
	  }

	/* BIG-5     -->  GB-2312  */
	if((0 == strcmp( argv[INFILECONV], "-big")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-gb")))
	  {
	    big_5_to_gb_2312( infilefp, outfilefp );
	    goto out;
	  }

	/* BIG-5     -->  IBM-555  */
	if((0 == strcmp( argv[INFILECONV], "-big")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-ibm")))
	  {
	    big_5_to_ibm_555( infilefp, outfilefp );
	    goto out;
	  }

	/* BIG-5     -->  JIS (EUC)  */
	if((0 == strcmp( argv[INFILECONV], "-big")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-jis")))
	  {
	    big_5_to_jis_euc( infilefp, outfilefp );
	    goto out;
	  }

	/* JIS (EUC) -->  GB-2312  */
	if((0 == strcmp( argv[INFILECONV], "-jis")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-gb")))
	  {
	    jis_euc_to_gb_2312( infilefp, outfilefp );
	    goto out;
	  }

	/* JIS (EUC) -->  BIG-5  */
	if((0 == strcmp( argv[INFILECONV], "-jis")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-big")))
	  {
	    jis_euc_to_big_5( infilefp, outfilefp );
	    goto out;
	  }

	/* IBM-555   -->  BIG-5  */
	if((0 == strcmp( argv[INFILECONV], "-ibm")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-big")))
	  {
	    ibm_555_to_big_5( infilefp, outfilefp );
	    goto out;
	  }

	/* IBM-555   -->  GB-2312  */
	if((0 == strcmp( argv[INFILECONV], "-ibm")) &&
	   (0 == strcmp( argv[OUTFILECONV], "-gb")))
	  {
	    ibm_555_to_gb_2312( infilefp, outfilefp );
	    goto out;
	  }
	
				/* If we arrive here, then we have not */
				/* found a coding scheme */
	fprintf(stderr, "%s: Unknown coding scheme(s)\n",
		argv[PROGNAME]);
				/* we do use goto! :-) */
      out:;
      }				/* four arguments */
    else
      {
	fprintf( stderr, "%s: %s\n", argv[0], "Wrong number of arguments");
	exit( 1 );
      }
				/* Just a plain exit. */
  exit( 0 );
}				/* main */







