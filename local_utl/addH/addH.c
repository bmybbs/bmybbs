// 给PASSWD文件中的所有用户添加自定义锁屏权限(H)
// author: leoncom@bmy

#include <stdio.h> 

int main() 
{ 
	FILE *fp,*fd; 
	if((fp=fopen(".PASSWDS","rb")) == NULL) 
		exit(1); 
	if((fd=fopen("result","wb")) == NULL) 
		exit(1); 
	unsigned c; 
	int count=0; 
	while(1) 
	{ 
		c=fgetc(fp); 
		if(feof(fp)) 
			exit(1); 
		count++; 
		if((count-224)%452 == 0) //读到的是后2字节权限位 
		{ 
			c=c | 0x80 ;         //添加H权限 
			//printf("%d\n",c); 
		} 
		fputc(c,fd); 
	} 
	fclose(fp); 
	fclose(fd); 
	return 0; 
} 
