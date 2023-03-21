/* send signals to the process (in order)
pkill a.out -INT
pkill a.out -USR1                                                        
pkill a.out
pkill a.out -9
pkill a.out -STOP
pkill a.out -CONT
https://www.computerhope.com/unix/signals.htm
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo)
{
    if (signo == SIGINT)
        printf("received SIGINT\n");
    else if (signo == SIGUSR1)
        printf("received SIGUSR1\n");
    else if (signo == SIGTERM)
        printf("received SIGTERM\n");
    else if (signo == SIGKILL)
        printf("received SIGKILL\n");
    else if (signo == SIGSTOP)
        printf("received SIGSTOP\n");
    else if (signo == SIGCONT)
        printf("received SIGCONT\n");
}

int main(void)
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGUSR1\n");
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGTERM\n");
    if (signal(SIGKILL, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGKILL\n");
    if (signal(SIGSTOP, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGSTOP\n");
    if (signal(SIGCONT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGCONT\n");
    // A long long wait so that we can easily issue a signal to this process
    while (1)
        sleep(1);
    return 0;
}
