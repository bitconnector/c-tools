// https://suchprogramming.com/epoll-in-3-easy-steps/
// https://stackoverflow.com/questions/19078360/how-to-assign-a-context-to-a-socket-or-an-epoll-event
#define MAX_EVENTS 5
#define READ_SIZE 10
#include <stdio.h>     // for fprintf()
#include <unistd.h>    // for close(), read()
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <string.h>    // for strncmp
#include <sys/signalfd.h>
#include <signal.h>
#include <stdlib.h>

int main()
{
    int running = 1, event_count, i;
    size_t bytes_read;
    char read_buffer[READ_SIZE + 1];
    struct epoll_event event, events[MAX_EVENTS];
    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor\n");
        return 1;
    }

    event.events = EPOLLIN;
    event.data.fd = 42; // context data for event

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event)) // 0 -> stdin file descriptor
    {
        fprintf(stderr, "Failed to add file descriptor to epoll\n");
        close(epoll_fd);
        return 1;
    }

    sigset_t mask;
    sigemptyset(&mask);
    // sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    int r = sigprocmask(SIG_BLOCK, &mask, 0);
    if (r == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor2\n");
        return 1;
    }
    int signal_fd = signalfd(-1, &mask, 0);
    if (signal_fd == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor3\n");
        return 1;
    }

    // epoll_event event;
    event.data.fd = 1;
    event.events = EPOLLIN;
    r = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &event);
    if (r == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor4\n");
        return 1;
    }

    while (running)
    {
        printf("\nPolling for input...\n");
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
        printf("%d ready events\n", event_count);
        for (i = 0; i < event_count; i++)
        {
            printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            if (events[i].data.fd == 1)
            {
                struct signalfd_siginfo fdsi;
                ssize_t s = read(signal_fd, &fdsi, sizeof(struct signalfd_siginfo));
                if (s != sizeof(struct signalfd_siginfo))
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if (fdsi.ssi_signo == SIGINT)
                {
                    printf("Got SIGINT\n");
                }
                else
                {
                    printf("Read unexpected signal\n");
                }
                running = 0;
                break;
            }
            bytes_read = read(0, read_buffer, READ_SIZE);
            printf("%zd bytes read.\n", bytes_read);
            read_buffer[bytes_read] = '\0';
            printf("Read '%s'\n", read_buffer);

            if (!strncmp(read_buffer, "stop\n", 5))
                running = 0;
        }
    }

    if (close(epoll_fd))
    {
        fprintf(stderr, "Failed to close epoll file descriptor\n");
        return 1;
    }

    return 0;
}
