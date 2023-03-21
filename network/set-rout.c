#include <stdio.h>

#include <arpa/inet.h>

#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <net/route.h>

void main()
{
    int sockfd;
    struct rtentry rt;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed\n");
        return;
    }

    struct sockaddr_in *sockinfo = (struct sockaddr_in *)&rt.rt_gateway;
    sockinfo->sin_family = AF_INET;
    sockinfo->sin_addr.s_addr = inet_addr("Your Address");

    sockinfo = (struct sockaddr_in *)&rt.rt_dst;
    sockinfo->sin_family = AF_INET;
    sockinfo->sin_addr.s_addr = INADDR_ANY;

    sockinfo = (struct sockaddr_in *)&rt.rt_genmask;
    sockinfo->sin_family = AF_INET;
    sockinfo->sin_addr.s_addr = INADDR_ANY;

    rt.rt_flags = RTF_UP | RTF_GATEWAY;
    rt.rt_dev = "eth0";

    if (ioctl(sockfd, SIOCADDRT, &rt) < 0)
        perror("ioctl");

    return;
}
