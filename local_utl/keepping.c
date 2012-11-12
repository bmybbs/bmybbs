#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
/*
main()
{
	pid_t child_pid;
	signal(SIGCHLD, SIG_DFL);
	while(1) {
		if((child_pid=fork())==0) {
			system("ping 162.105.31.0 -b >/dev/null 2>/dev/null");
			exit(0);
		} else if(child_pid>0) {
			sleep(2);
			while(wait(NULL)!=child_pid)
				sleep(1);
		}
	}
}*/
main()
{
	while (1) {
		system("ping 162.105.31.0 -b -i 3 >/dev/null 2>/dev/null");
	}
}
