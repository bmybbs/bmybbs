#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#define STRLEN  256
#define LINEMAX  128
#define PATHLEN 1024

int toupper(char *str)
{
   int i=0;
   int length=strlen(str);
   for(i=0;i<length;i++)
	if(str[i] > 'a' && str[i] < 'z' )
	{
		str[i]=str[i] - 32;
	}
   return 1;
}

void a_article_search(char *path,char *key,char *location)
{
   FILE *fp;
   struct   stat  buf;
   char index_file[]="/.Names";		
   char titlebuf[LINEMAX];
   char pathbuf[LINEMAX];
   char index_path[PATHLEN];
   char filename[256];							//记录当前目录完整路径
   strcpy(index_path,path);
   strcat(index_path,index_file);				//.Names完整路径
   if((fp=fopen(index_path,"r"))==NULL)
   {
     //printf("open error!,%s\n",index_path);
     return;
   }
   while(!feof(fp))
   {
	 bzero(filename,80);
	 strcpy(filename,path);
     fgets(titlebuf,LINEMAX,fp);
     if(strncmp(titlebuf,"Name=",5)==0)
     {
     	fgets(pathbuf,LINEMAX,fp);					//如果是隐藏/权限目录或文件
		if(strstr(titlebuf+43,"(BM: BMS)") || strstr(titlebuf+43,"(BM: SYSOPS)") ||
			strstr(titlebuf+5,"<HIDE>"))
		continue;
		char temppath[80];
        if(strncmp(pathbuf,"Path=",5)==0)           //截出目录选项
        {
         if(strncmp(pathbuf,"Path=~/",7)==0)
		  strncpy(temppath,pathbuf+7,80);
		 else
		  strncpy(temppath,pathbuf+5,80);
		 temppath[79]='\0';
		}
		strcat(filename,"/");
		strcat(filename,temppath);
		filename[strlen(filename)-1]='\0';
		lstat(filename,&buf);
		if(S_ISDIR(buf.st_mode))					//是目录则递归
		{
		  char linebuf[20];
		  fgets(linebuf,20,fp);
		  if(strncmp(linebuf,"Numb=",5)==0)
			 linebuf[strlen(linebuf)-1]='\0';
		  char nextlocate[20];
		  sprintf(nextlocate,"%s%s-",location,linebuf+5);
		  a_article_search(filename,key,nextlocate);
		}
		else if(S_ISREG(buf.st_mode))				//是文件则判断是否含有关键字
		{
			char temptitle[64];
			bzero(temptitle,sizeof(temptitle));
			strncpy(temptitle,titlebuf+5,38);
			char tempupper[64];
			strcpy(tempupper,temptitle);
			char keyupper[50];
			strcpy(keyupper,key);
			toupper(tempupper);
			toupper(keyupper);
			if(strstr(tempupper,keyupper) != NULL)
			{
				char linebuf[20];
				fgets(linebuf,20,fp);
		        if(strncmp(linebuf,"Numb=",5)==0)
			         linebuf[strlen(linebuf)-1]='\0';
		        char nextlocate[32];
		            sprintf(nextlocate,"%s%s",location,linebuf+5);
				temptitle[strlen(temptitle)-7]='\0';
				char *filenamenum;
				char tempresult[256];
				strcpy(tempresult,filename+19);
				filenamenum=strrchr(tempresult,'/');
				char temptitleresult[30];
				strcpy(temptitleresult,filenamenum);
				tempresult[strlen(tempresult)-strlen(filenamenum)]='\0';
				temptitle[strlen(temptitle)-1]='\0';	
         		printf("%s,%s,%s,%s \n",nextlocate,tempresult,temptitleresult,temptitle);
			}	
			else
			{
				titlebuf[0]='\0';
				pathbuf[0]='\0';
			}
		}
		else
		{
			continue;                //��������������
		}
     }
	else
		titlebuf[0]='\0';
   }
	fclose(fp);
	return;
}

int
main(int argc,char *argv[])
{
	char path[256]="/home/bbs/0Announce/";
	strcat(path,argv[1]);
	a_article_search(path,argv[2],"x-");
	return 0;
}
