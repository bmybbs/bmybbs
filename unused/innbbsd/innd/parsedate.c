#include <stdio.h>
#include <time.h>
main()
{
	char ptr1[256],*ptr=ptr1;
	time_t ti;

	fgets(ptr,255,stdin);
	if (!strncmp("Date: ",ptr,6))
		ptr+=6;
	ti = parsedate(ptr,0);
	printf("%s\n",ctime(&ti));
}
