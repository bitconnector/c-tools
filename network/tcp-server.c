#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_EVENTS 5
#define MAX 80

int main(int argc, char *argv[])
{

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }

    if ((listen(sockfd, MAX_EVENTS)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }

    struct epoll_event event;
    int epoll_fd = epoll_create1(0);

    event.events = EPOLLIN;
    event.data.fd = 0; // context data for event

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event))
    {
        printf("Failed to add file descriptor to epoll\n");
        exit(0);
    }

    struct epoll_event events[MAX_EVENTS];
    while (1)
    {
        printf("\nPolling for input...\n");
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
        printf("%d ready events\n", event_count);
        for (int i = 0; i < event_count; i++)
        {
            printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            if (events[i].data.fd == 0)
            {
                struct sockaddr_in cli;
                int len = sizeof(cli);
                int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
                if (connfd < 0)
                {
                    printf("server accept failed...\n");
                    exit(0);
                }
                event.data.fd = connfd;
                event.events = EPOLLIN;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event))
                {
                    printf("Failed to add file descriptor to epoll\n");
                    exit(0);
                }
                char buffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli.sin_addr.s_addr, buffer, sizeof(buffer));
                printf("new Client %s:%i\n", buffer, cli.sin_port);
            }
            else
            {
                int connfd = events[i].data.fd;
                char buff[MAX];
                for (int i = 0; i < MAX; i++)
                    buff[i] = 0;

                int size = read(connfd, buff, sizeof(buff));
                if (size == 0)
                {
                    printf("Client disconnected\n");
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connfd, 0))
                    {
                        printf("Failed to delete file descriptor in epoll\n");
                        exit(0);
                    }
                    close(connfd);
                    continue;
                }

                printf("From client: %s\n", buff);
                write(connfd, "back\n", 5);
            }
        }
    }
    return 0;
}
