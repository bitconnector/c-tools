// https://manpath.be/f33/2/signalfd#L22
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    sigset_t mask;
    int sfd;
    struct signalfd_siginfo fdsi;
    ssize_t s;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    /* Block signals so that they aren't handled
       according to their default dispositions */

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
    {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
    {
        perror("signalfd");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
        if (s != sizeof(struct signalfd_siginfo))
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (fdsi.ssi_signo == SIGINT)
        {
            printf("Got SIGINT\n");
        }
        else if (fdsi.ssi_signo == SIGQUIT)
        {
            printf("Got SIGQUIT\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Read unexpected signal\n");
        }
    }
}
