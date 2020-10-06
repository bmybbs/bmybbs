/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef BMYBBS_COMMEND_H
#define BMYBBS_COMMEND_H
#include <time.h>
#include "config.h"

#define COMMENDFILE      MY_BBS_HOME"/.COMMEND"
#define COMMENDFILE2     MY_BBS_HOME"/.COMMEND2"
/**
 * @brief front page commend
 * modify by mintbaggio 040326
 */
struct commend{
	char board[24];         ///< the board that the article be commened is in
	char userid[14];        ///< the author of the article be commended
	char com_user[14];      ///< the user who commend this article
	char title[80];         ///< the title of the article be commended
	char filename[80];      ///< the filename of the article be commended
	time_t time;            ///< the time when the com_user commend this article
	unsigned int accessed;
};
#endif

