//https://www.beej.us/guide/bgnet/html/
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 500

int main(int argc, char *argv[])
{
    int sfd, s;
    char buf[BUF_SIZE];
    size_t len;
    ssize_t nread;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Obtain address(es) matching host/port. */

    memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_family = AF_INET; /* Allow IPv4 */
    // hints.ai_family = AF_INET6;     /* Allow IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    // hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; /* Any protocol */

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        char str[INET6_ADDRSTRLEN];
        if (rp->ai_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *p = (struct sockaddr_in *)rp->ai_addr;
            printf("res: %s\n", inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str)));
        }
        else if (rp->ai_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6 *p = (struct sockaddr_in6 *)rp->ai_addr;
            printf("res: %s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
        }
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        close(sfd);
    }

    freeaddrinfo(result); /* No longer needed */

    if (rp == NULL)
    { /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    /* Send remaining command-line arguments as separate
       datagrams, and read responses from server. */

    for (size_t j = 3; j < argc; j++)
    {
        len = strlen(argv[j]) + 1;
        /* +1 for terminating null byte */

        if (len > BUF_SIZE)
        {
            fprintf(stderr,
                    "Ignoring long message in argument %zu\n", j);
            continue;
        }

        if (write(sfd, argv[j], len) != len)
        {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }

        nread = read(sfd, buf, BUF_SIZE);
        if (nread == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        printf("Received %zd bytes: %s\n", nread, buf);
    }

    exit(EXIT_SUCCESS);
}
