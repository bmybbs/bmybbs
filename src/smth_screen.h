/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

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


/* Maximum Screen width in chars */
/*#define LINELEN (220) */
#include "bbstelnet.h"
//#define SCREEN_MODIFIED 1
#define SCREEN_BRIGHT 2
#define SCREEN_LINE 4
#define SCREEN_BLINK 8
#define SCREEN_BACK 16
//#define SCREEN_NOTMOD 30
//#define SCREEN_ALL 31

struct screenline {
    unsigned char data[LINELEN];
    unsigned char mode[LINELEN];
    unsigned char color[LINELEN];
    unsigned char changed[LINELEN];
    int lchanged;
};
