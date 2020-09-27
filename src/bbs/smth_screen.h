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
#ifndef BMYBBS_BBS_SMTH_SCREEN
#define BMYBBS_BBS_SMTH_SCREEN
void resetcolor(void);
int num_noans_chr(char *str);
void clear(void);
void initscr(void);
void refresh(void);
void redoscr(void);
void move(int y, int x);
void getyx(int *y, int *x);
void clear_whole_line(int i);
void clrtoeol(void);
void clrtobot(void);
void outc(unsigned char c);
void outns(const char *str, int n);
void outs(const char *str);
void prints(char *format, ...);
void scroll(void);
void rscroll(void);
void saveline(int line, int mode, char *buffer);
void do_naws(int ln, int col);
void show_message(char msg[]);
#endif
