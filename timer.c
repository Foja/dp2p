#include <signal.h>
#include <stdio.h>
void alarm_handler()
{
	printf("timeout scaduto\n");
	return;
}
int main()
{

struct sigaction alarmaction = {.sa_handler = alarm_handler};
int retcode = sigaction(SIGALRM, &alarmaction, NULL);
    if (retcode == -1) {
    	perror("bad sigaction()\n");
		return 1;
    	}
alarm(5);
sleep(5);


}
