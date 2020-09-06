#include <stdio.h>
#define RECORDFILE "etc/tt/tt.dat"
char *userid,*from;
char tt_id[20][20], tt_ip[20][30];
int tt_scr[20];

int tt_load_record() {
  int n;
  FILE *fp=fopen(RECORDFILE, "r");
  if(fp==0) {
    fp=fopen(RECORDFILE, "w");
    for(n=0; n<20; n++)
      fprintf(fp, "%s %s %d\n", "none", "0.0.0.0", 0);
    fclose(fp);
    fp=fopen(RECORDFILE, "r");
  }
  for(n=0; n<20; n++)
    fscanf(fp, "%s %s %d", tt_id[n], tt_ip[n], &tt_scr[n]);
  fclose(fp);
}

int tt_save_record() {
  int n, m1, m2;
  char id[20], ip[30];
  int scr;
  FILE *fp=fopen(RECORDFILE, "w");
  for(m1=0; m1<20; m1++)
  for(m2=m1+1; m2<20; m2++) 
    if(tt_scr[m1]<tt_scr[m2]) {
      strcpy(id, tt_id[m1]);
      strcpy(ip, tt_ip[m1]);
      scr=tt_scr[m1];
      strcpy(tt_id[m1], tt_id[m2]);
      strcpy(tt_ip[m1], tt_ip[m2]);
      tt_scr[m1]=tt_scr[m2];
      strcpy(tt_id[m2], id);
      strcpy(tt_ip[m2], ip);
      tt_scr[m2]=scr;
    }
  for(n=0; n<20; n++)
    fprintf(fp, "%s %s %d\n", tt_id[n], tt_ip[n], tt_scr[n]);
  fclose(fp);
}

int tt_check_record(int score) {
  int n;
  tt_load_record();
  for(n=0; n<20; n++)
    if(!strcasecmp(tt_id[n], userid)) {
      if(tt_scr[n]>score) return 0;
      tt_scr[n]=score;
      strncpy(tt_ip[n], from, 16);
      tt_ip[n][16]=0;
      tt_save_record();
      return 1;
    }
  if(tt_scr[19]<score) {
    tt_scr[19]=score;
    strcpy(tt_id[19], userid);
    strncpy(tt_ip[19], from, 16);
    tt_ip[19][16]=0;
    tt_save_record();
    return 1;
  }
  return 0;
}

main(int argn, char **argv)
{
	if(argn>=3) {
		userid=argv[1];
		from=argv[2];
	} else {
		userid="ÄäÃû";
		from="Î´Öª";
	}

	tt_game();
}

int tt_game() {
  char c[30], a, genbuf[60];
  char chars[]="£Á£Â£Ã£Ä£Å£Æ£Ç£È£É£Ê£Ë£Ì£Í£Î£Ï£Ð£Ñ£Ò£Ó£Ô£Õ£Ö£×£Ø£Ù£Ú";
  int m, n, t, score, retv;
  srand(getpid()+time(NULL));
  //randomize();
  //modify_user_mode(BBSNET);
  //report("tt game");
  tt_load_record();
  printf("[2J±¾Õ¾´ò×Ö¸ßÊÖÅÅÐÐ°ñ\r\n\r\n%4s %12.12s %24.24s %5s(WPMs)\r\n", "Ãû´Î", "ÕÊºÅ", "À´Ô´", "ËÙ¶È");
  for(n=0; n<20; n++)
    printf("%4d %12.12s %24.24s %5.2f\r\n", n+1, tt_id[n], tt_ip[n], tt_scr[n]/10.);
  fflush(stdout);
  read(0, genbuf, 32);
  printf("[2J[1;32m£Â£Â£Ó[m´ò×ÖÁ·Ï°³ÌÐò. (´óÐ¡Ð´¾ù¿É, ÊäÈëµÚÒ»×Ö·ûÇ°µÄµÈ´ý²»¼ÆÊ±. [1;32m^C[m or [1;32m^D[m ÍË³ö.)\r\n\r\n\r\n");

start:
  for(n=0; n<30; n++) {
    c[n]=rand()%26;
    printf("%c%c", chars[c[n]*2], chars[c[n]*2+1]);
  }
  printf("\r\n");
  fflush(stdout);
  m=0;
  t=times(0);
  while(m<30) {
    while((retv=read(0, genbuf, 32))>1);
    if(retv<=0) return 0;
    if(m==0&&abs(times(0)-t)>300) {
           printf("\r\n³¬Ê±! Äã±ØÐëÔÚ[1;32m3[mÃëÖÓÒÔÄÚ¿ªÊ¼!\r\n");
           goto start;
    }
    a=genbuf[0];
    if(a==c[m]+65 || a==c[m]+97) {
      printf("%c%c", chars[c[m]*2], chars[c[m]*2+1]);
      if(m==0) {
        t=times(0);
      }
      m++;
      fflush(stdout);
      usleep(60000);
    }
    if(genbuf[0]==3||genbuf[0]==4) return 0;
  }
  score=360000/(times(0)-t);
  printf("\r\n\r\nSpeed=%5.2f WPMs\r\n\r\n", score/10.);
  if(tt_check_record(score)) printf("[1;33m×£ºØ£¡ÄúË¢ÐÂÁË×Ô¼ºµÄ¼ÍÂ¼£¡[m\r\n\r\n");
  fflush(stdout);
  goto start;
}
