//https://stackoverflow.com/questions/17909401/linux-c-get-default-interfaces-ip-address

#include <stdio.h>
#include <unistd.h>
#include <string.h> /* For strncpy */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

int main()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want an IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "enp39s0", IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* Display result */
    printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    return 0;
}
