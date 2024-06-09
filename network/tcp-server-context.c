#define MAX_EVENTS 5
#define PORT 8080

#include <stdio.h> // for fprintf()
#include <stdlib.h>
#include <string.h>    // for strncmp
#include <unistd.h>    // for close(), read()
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

struct SessionData
{
    int fd;
    struct sockaddr_in addr;
    int x;
};

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int yes = 1;
    int no = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // open port after restart with pending connections

    // int flen = 120;
    // setsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &flen, sizeof(flen)); // restrict the maximum packet size

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
    event.data.ptr = &sockfd;

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
            // printf("Reading file descriptor '%d' -- ", events[i].data.ptr);
            if (events[i].data.ptr == &sockfd)
            {
                struct sockaddr_in cli;
                int len = sizeof(cli);
                int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
                if (connfd < 0)
                {
                    printf("server accept failed...\n");
                    exit(0);
                }
                setsockopt(connfd, IPPROTO_TCP, TCP_QUICKACK, &no, sizeof(int)); // disable automatic empty ack reply
                struct SessionData *session = malloc(sizeof(struct SessionData));
                event.data.ptr = session;
                event.events = EPOLLIN;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event))
                {
                    printf("Failed to add file descriptor to epoll\n");
                    exit(0);
                }

                session->fd = connfd;
                memcpy(&session->addr, &cli, len);
                session->x = 0;

                char buffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli.sin_addr.s_addr, buffer, sizeof(buffer));
                printf("new Client %s:%i\n", buffer, cli.sin_port);
            }
            else
            {
                struct SessionData *session = events[i].data.ptr;
                int connfd = session->fd;
                char buff[20];
                memset(buff, 0, 20);

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
                    free(session);
                    continue;
                }
                if (!strncmp(buff, "stop", 4))
                {
                    printf("Client stop\n");
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connfd, 0))
                    {
                        printf("Failed to delete file descriptor in epoll\n");
                        exit(0);
                    }
                    close(connfd);
                    free(session);
                    continue;
                }

                printf("From client: %s\n", buff);
                write(connfd, "back\n", 5);
            }
        }
    }
    return 0;
}