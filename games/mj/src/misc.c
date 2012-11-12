#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "curses.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "mjdef.h"
#include "qkmj.h"
#define NO_SUN_HP 1

float thinktime()
{
  float t;
  char msg_buf[80];

  gettimeofday(&after, (struct timezone *) 0);
  after.tv_sec-=before.tv_sec;
  after.tv_usec-=before.tv_usec;
  if(after.tv_usec<0)
  {
    after.tv_sec--;
    after.tv_usec+=1000000;
  }
  t=(float) after.tv_sec+(float) after.tv_usec/1000000; 
  return(t);
}

beep1()
{
  if(set_beep)
    beep();
}

beep()
{
  putchar('\007');
  fflush(stdout);
}

mvwgetstring(win,y,x,max_len,str_buf,mode)
WINDOW *win;
int y;
int x;
int max_len;
unsigned char *str_buf;
int mode;
{
  int ch;
  unsigned char ch_buf[2];
  int org_x,org_y;
  int i;

  keypad(win,TRUE);
  meta(win,TRUE);
  org_y=y;
  org_x=x;
  wmvaddstr(win,y,x,str_buf);
  wrefresh(win);
  x=org_x+strlen(str_buf);
  while(1)
  {
    ch=my_getch();
    switch(ch)
    {
      case KEY_UP:
      case KEY_DOWN:
      case KEY_LEFT:
      case KEY_RIGHT:
        break;
      case BACKSPACE:
      case CTRL_H:
        if(x>org_x)
        {
          x--;
          str_buf[x-org_x]=0;
          mvwaddch(win,y,x,' ');
          wmove(win,y,x); 
          wrefresh(win);
        }
        break;
      case CTRL_U:
        wmove(win,y,org_x);
        for(i=0;i<x-org_x;i++)
          waddch(win,' ');
        wmove(win,y,org_x);
        str_buf[0]=0;
        x=org_x;
        wrefresh(win);
        break;
      case KEY_ENTER: 
      case ENTER:
        return;  
        break;
      default:
        if(x-org_x>=max_len)
          break;
        str_buf[x-org_x]=ch;
        str_buf[x+1-org_x]=0;
        if(mode==0)
          mvwaddstr(win,y,x++,"*");
        else 
        {
          ch_buf[0]=ch;
          ch_buf[1]=0; 
          mvwaddstr(win,y,x++,ch_buf);
        }
        wrefresh(win);
        break;
    }
  }
}

/*
attron(mode)
int mode;
{
  set_mode(mode);
}
attroff(mode)
int mode;
{
  set_mode(0);
}
*/
