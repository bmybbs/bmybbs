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
    
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
void
show_help(fname)
char *fname;
{
	ansimore(fname, YEA);
	clear();
}

int
mainreadhelp()
{
	show_help("help/mainreadhelp");
	return FULLUPDATE;
}

int
mailreadhelp()
{
	show_help("help/mailreadhelp");
	return FULLUPDATE;
}

int
selbacknumberhelp()
{
	show_help("help/selbacknumberhelp");
	return FULLUPDATE;
}

int
backnumberhelp()
{
	show_help("help/backnumberhelp");
	return FULLUPDATE;
}
