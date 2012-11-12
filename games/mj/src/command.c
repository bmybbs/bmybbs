#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "mjdef.h"
#include "qkmj.h"

#define TABLE 1
#define LIST 2
#define PLAYER 3
#define JOIN 4
#define SERV 5
#define QUIT 6
#define EXIT 7
#define WHO 8
#define SIT 9
#define NUM 10
#define NEW 11
#define SHOW 12
#define LEAVE 13
#define HELP 14
#define NOTE 15
#define STAT 16
#define LOGIN 17
#define PASSWD 18
#define preserve1 19
#define BROADCAST 20
#define KICK 21
#define KILL 22
#define INVITE 23
#define MSG 24
#define SHUTDOWN 25
#define LURKER 26
#define FIND 27
#define BEEP 28
#define EXEC 29
#define FREE 30
#define S_PLAYER 31
#define S_JOIN 32
#define S_HELP 33
#define S_WHO 34
#define S_SERV 35
#define S_LEAVE 36
#define S_TABLE 37
#define S_QUIT 38

int enable_kick=1;
int enable_exec=0;
char commands[100][10]
={"","TABLE","LIST","PLAYER","JOIN","SERV","QUIT","EXIT","WHO","SIT","NUM",
  "NEW","SHOW","LEAVE","HELP","NOTE","STAT","LOGIN","PASSWD","preserve1",
  "BROADCAST","KICK","KILL","INVITE","MSG","SHUTDOWN","LURKER","FIND","BEEP",
  "EXEC","FREE", "P", "J", "H", "W", "S", "L", "T", "Q"};

void Tokenize(strinput)
char *strinput;
{
  int k1, klast, Ltok, k1old;
  char *token;
  char str[255];
  narg=1;
  strcpy(str, strinput);
  token=strtok(str, " \n\t\r");
  if(token==NULL)
  {
    narg=0;
    return;
  }
  Ltok=strlen(token);
  strcpy(cmd_argv[narg], token);
  arglenv[narg]=Ltok;
  while(1)
  {
    token=strtok('\0', " \n\t\r");
    if(token==NULL)
      break;
    narg++;
    Ltok=strlen(token);
    strcpy(cmd_argv[narg], token);
    arglenv[narg]=Ltok;
  }
}

strupr(upper,org)
char *upper;
char *org;
{
  int i,len;
  len=strlen(org);
  for(i=0;i<len;i++)
    upper[i]=toupper(org[i]);
  upper[len]='\0';
}

command_mapper(cmd)
char *cmd;
{
  char cmd_upper[255];
  int i;

  strupr(cmd_upper,cmd);
  i=1;
  while(commands[i][0]!='\0')
  {
    if( strcmp(cmd_upper,commands[i]) == 0)
      return(i);
    i++;
  }
  return(0);

}

who(name)
char *name;
{
  char msg_buf[255];
  int i;

  if(name[0]!=0)
  {
    sprintf(msg_buf,"006%s",name);
    write_msg(gps_sockfd,msg_buf);
    return;
  }
  if(!in_join && !in_serv)
  {
    display_comment("你要查看那一桌?");
    return;
  }
  display_comment("----------------   此桌使用者   ------------------");
  msg_buf[0]=0;
  for(i=1;i<MAX_PLAYER;i++)
  {
    if(strlen(msg_buf)>49)
    {
      display_comment(msg_buf);
      msg_buf[0]=0;
    }
    if(player[i].in_table)
    {
      strcat(msg_buf,player[i].name);
      strcat(msg_buf,"  ");
    }
  }
  if(msg_buf[0]!=0)
    display_comment(msg_buf); 
  display_comment("--------------------------------------------------");
}

help()
{
  send_gps_line("-----------------   使用说明   -------------------");
  send_gps_line("/HELP          /H   查看使用说明");
  send_gps_line("/TABLE         /T   查看所有的桌");
  send_gps_line("/FREE               查看目前人数未满的桌");
  send_gps_line("/PLAYER        /P   查看目前在线上的使用者");
  send_gps_line("/LURKER             查看闲置的使用者");
  send_gps_line("/FIND <名称>        找寻此使用者");
  send_gps_line("/WHO <名称>    /W   查看此桌的使用者");
  send_gps_line("/SERV          /S   开桌");
  send_gps_line("/JOIN <名称>   /J   加入一桌");
  send_gps_line("/MSG <名称> <讯息>  送讯息给特定使用者");
  send_gps_line("/INVITE <名称>      邀请使用者到此桌");
  send_gps_line("/KICK <名称>        将使用者踢出此桌 (桌长才可用)");
  send_gps_line("/NOTE <附注>        更改此桌的附注");
  send_gps_line("/LEAVE         /L   离开一桌");
  send_gps_line("/STAT <名称>        看使用者状态");
  send_gps_line("/BEEP [ON|OFF]      设定声音开关");
  send_gps_line("/PASSWD             更改密码");
  send_gps_line("/QUIT          /Q   离开");
  send_gps_line("--------------------------------------------------");
}

command_parser(msg)
char *msg;
{
  int i;
  int cmd_id;
  char sit;
  char table_upper[255];
  char msg_buf[255];
  char ans_buf[255];
  char ans_buf1[255];

  if(msg[0]=='/')
  {
    Tokenize(msg+1);
    cmd_id=command_mapper(cmd_argv[1]);
    switch(cmd_id)
    {
      case 0:
        send_gps_line("没有这个指令");
        break;
      case TABLE:
      case S_TABLE:
        write_msg(gps_sockfd,"003");
        break;
      case FREE:
        write_msg(gps_sockfd,"013");
        break;
      case LIST:
      case PLAYER:
      case S_PLAYER:
        write_msg(gps_sockfd,"002");
        break;
      case JOIN:
      case S_JOIN:
        if(!pass_login)
        {
          display_comment("请先 login 一个名称");
          break;
        }
        if(in_join || in_serv)
        {
          display_comment("请先离开此桌");
          break;
        }
        clear_variable();
        if(in_join)
        {
          close_join();
          write_msg(gps_sockfd,"205");
          init_global_screen();
        }
        if(in_serv)
        {
          close_serv();
          write_msg(gps_sockfd,"205");
          init_global_screen();
        }
        sprintf(msg_buf,"011%s",cmd_argv[2]);
        write_msg(gps_sockfd,msg_buf);
        /* Now wait for GPS's answer */
        break;
      case SERV:
      case S_SERV:
        if(!pass_login)
        {          
          display_comment("请先 login 一个名称");
          break;
        }
        if(in_join || in_serv)
        {
          display_comment("请先离开此桌");
          break;
        }
        clear_variable();
        if(in_join)
        {
          close_join();
          write_msg(gps_sockfd,"205");
          init_global_screen();
        }
        if(in_serv)
        {
          close_serv();
          write_msg(gps_sockfd,"205");
          init_global_screen();
        }
        init_serv_socket();
        sprintf(msg_buf,"012%d",SERV_PORT-1);
        write_msg(gps_sockfd,msg_buf);
        my_id=1;
        in_serv=1;
        on_seat=0;
        player_in_table=1;
        player[1].sit=1;
        player[1].money=my_money;
        player[1].id=my_gps_id;
        strcpy(player[1].name,my_name);
        my_sit=1;
        for(i=0;i<=4;i++)
          table[i]=0;
        table[1]=1;
        if(player_in_table==PLAYER_NUM)
        {
          init_playing_screen();
          opening();
          open_deal();
        }
        strcpy(player[1].name,my_name);
        player[1].in_table=1;
        send_gps_line("开桌");
        break;
      case QUIT:
      case S_QUIT:
      case EXIT:
        leave();
        break;
      case SHOW:
/*
        sprintf(msg_buf,"%d",pool[cmd_argv[2][0]-'0'].card[atoi(cmd_argv[3])]);
       display_comment(msg_buf);
show_allcard(cmd_argv[2][0]-'0');
*/
        break;
      case WHO:
      case S_WHO:
        if(narg==2)
          who(cmd_argv[2]);
        else
          who("");
        break;
      case NUM:
        i=cmd_argv[2][0]-'0';
        if(i>=1 && i<=4)
          PLAYER_NUM=i;
        break;
      case NEW:
        if(in_serv)
        {
          broadcast_msg(1,"290");
          opening();
          open_deal();
        }
        else
        {
          write_msg(table_sockfd,"290");
          opening();
        }
        break;
      case LEAVE:
      case S_LEAVE:
        if(in_join)
        {
          in_join=0;
          close_join();
          write_msg(gps_sockfd,"205");
          init_global_screen();
        }
        if(in_serv)
        {
          in_serv=0;
          close_serv();
          write_msg(gps_sockfd,"205");
          init_global_screen();
        }
        input_mode=TALK_MODE;
        break; 
      case HELP:
      case S_HELP:
        help();
        break;
      case NOTE:
        sprintf(msg_buf,"004%s",msg+6);
        write_msg(gps_sockfd,msg_buf);
        break;
      case STAT:
        if(narg<2)
          strcpy(cmd_argv[2],my_name);
        sprintf(msg_buf,"005%s",cmd_argv[2]);
        write_msg(gps_sockfd,msg_buf);
        break;
      case LOGIN:
        break;
      case BROADCAST:
        sprintf(msg_buf,"007%s",msg+11);
        write_msg(gps_sockfd,msg_buf);
        break;
      case MSG:
        if(narg<=2)
          break;
        if(in_join || in_serv)
        {
          for(i=1;i<=4;i++)
          {
            if(table[i] && strcmp(cmd_argv[2],player[table[i]].name)==0)
            {
              sprintf(msg_buf,"%s",msg+5+strlen(cmd_argv[2])+1);
              send_talk_line(msg_buf);
              goto finish_msg;
            }
          }
        }
        sprintf(msg_buf,"009%s",msg+5);
        write_msg(gps_sockfd,msg_buf);
        sprintf(msg_buf,"-> *%s* %s",cmd_argv[2],msg+5+strlen(cmd_argv[2])+1);
        msg_buf[talk_right]=0;
        display_comment(msg_buf);
        finish_msg:;
        break;
      case SHUTDOWN:
        write_msg(gps_sockfd,"500");
        break;
      case LURKER:
        write_msg(gps_sockfd,"010");
        break;
      case FIND:
        if(narg<2)
        {
          display_comment("你要找谁呢?");
          break;
        }
        sprintf(msg_buf,"021%s",cmd_argv[2]);
        write_msg(gps_sockfd,msg_buf);
        break;
      case EXEC:
        /*if(!enable_exec)
          break;
        nl();
        echo();
        nocbreak();
        system(msg+6);
        cbreak();
        nonl();
        noecho();
        wait_a_key("");
        redraw_screen();*/
        break;
      case BEEP:
        if(narg<2)
        {
          sprintf(msg_buf,"目前声音设定为%s",(set_beep==1) ? "开启":"关闭");
          display_comment(msg_buf);
        }
        else
        {
          strupr(ans_buf,cmd_argv[2]);
          if(strcmp(ans_buf,"ON")==0)
          {
            set_beep=1;
            display_comment("开启声音");
          }
          if(strcmp(ans_buf,"OFF")==0)
          {
            set_beep=0;
            display_comment("关闭声音");
          }
        }
        break;
      case PASSWD:
        ans_buf[0]=0;
        ask_question("请输入你原来的密码：",ans_buf,8,0);
        ans_buf[8]=0;
        if(strcmp(my_pass,ans_buf)!=0)
        {
          wait_a_key("密码错误,更改失败!");
          break;
        }
        ans_buf[0]=0;
        ask_question("请输入你要更改的密码：",ans_buf,8,0);
        ans_buf1[0]=0;
        ask_question("请再输入一次确认：",ans_buf1,8,0);
        ans_buf[8]=0;
        ans_buf1[8]=0;
        if(strcmp(ans_buf,ans_buf1)==0)
        {
          sprintf(msg_buf,"104%s",ans_buf);
          write_msg(gps_sockfd,msg_buf);
          strcpy(my_pass,ans_buf);
          wait_a_key("密码已更改!");
        }
        else
        {
          wait_a_key("两次密码不同,更改失败!");
        }
        break;
      case KICK:
        if(!enable_kick)
          break;
        if(in_serv)
        {
          if(narg<2)
          {
            display_comment("要把谁踢出去呢?");
          }
          else
          {
            if(strcmp(my_name,cmd_argv[2])==0)
            {
              display_comment("抱歉, 自己不能踢自己");
              break;
            }
            for(i=2;i<MAX_PLAYER;i++)
            {
              if(player[i].in_table && strcmp(player[i].name,cmd_argv[2])==0)
              {
                sprintf(msg_buf,"101%s 被踢出此桌",cmd_argv[2]);
                display_comment(msg_buf+3);
                broadcast_msg(1,msg_buf);
                write_msg(player[i].sockfd,"200");
                close_client(i);
                goto finish_kick;
              }
            }
            display_comment("此桌没有这个人");
          }
        }
        else
        {
          display_comment("此命令只有桌长可用");
        }
        finish_kick:;
        break;
      case KILL:
        if(narg>=2)
        {
          sprintf(msg_buf,"202%s",cmd_argv[2]);
          write_msg(gps_sockfd,msg_buf);
        }
        break;
      case INVITE:
        if(narg<2)
        {
          display_comment("你打算邀请谁?");
          break;
        }
        sprintf(msg_buf,"008%s",cmd_argv[2]);
        write_msg(gps_sockfd,msg_buf);
        sprintf(msg_buf,"邀请 %s 加入此桌",cmd_argv[2]);
        display_comment(msg_buf);
        break;
      default:
        err("Unknow command id");
        display_comment(msg);
        break;
    }
  }
  else
    send_talk_line(msg);
}

