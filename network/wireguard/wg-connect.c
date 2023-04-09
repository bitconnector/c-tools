// gcc wg-connect.c lib/wireguard.c lib/wireguard.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h> //sleep

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>

#include <net/route.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "lib/wireguard.h"

bool exit_program = false;

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("received SIGINT\n");
        exit_program = true;
    }
}

void wait_until_sigint()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
    while (!exit_program)
    {
        sleep(1);
    }
}

wg_device *generate_wg_dev()
{
    wg_peer *peer = malloc(sizeof(wg_peer));

    const char *p_key = "b4Ft1F7vDxwtzIo/jGtoZXfUzFeAhW86ZSo+u7kl9zw=";
    wg_key_from_base64(peer->public_key, p_key);
    peer->flags |= WGPEER_HAS_PUBLIC_KEY;

    const char *_end = "10.38.230.11";
    inet_pton(AF_INET, _end, &peer->endpoint.addr4.sin_addr.s_addr);
    peer->endpoint.addr4.sin_family = AF_INET;
    peer->endpoint.addr4.sin_port = htons(2321);

    peer->persistent_keepalive_interval = 12;
    peer->flags |= WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL;

    peer->first_allowedip = malloc(sizeof(wg_allowedip));
    const char *_allow = "192.168.7.0";
    inet_pton(AF_INET, _allow, &peer->first_allowedip->ip4.s_addr);
    peer->first_allowedip->cidr = 24;
    peer->first_allowedip->family = AF_INET;
    peer->flags |= WGPEER_REPLACE_ALLOWEDIPS;
    peer->last_allowedip = peer->first_allowedip;

    //-----
    wg_device *dev = malloc(sizeof(wg_device));

    const char *device_name = "wgtest1";
    sprintf(dev->name, "%s", device_name);

    const char *key = "QAymzm0pe/6o1LR8a8mKwJwIsCE0iDh69RgmYXlhKkc=";
    wg_key_from_base64(dev->private_key, key);
    dev->flags |= WGDEVICE_HAS_PRIVATE_KEY;

    dev->listen_port = htons(7974);
    dev->flags |= WGDEVICE_HAS_LISTEN_PORT;

    dev->first_peer = peer;
    dev->last_peer = peer;

    return dev;
}

void init_wg_dev(wg_device *dev)
{
    if (wg_add_device(dev->name) < 0)
    {
        perror("Unable to add device");
        exit(1);
    }
    if (wg_set_device(dev) < 0)
    {
        perror("Unable to set device");
        exit(1);
    }
}

void remove_wg_dev()
{
    if (wg_del_device("wgtest1") < 0)
    {
        perror("Unable to delete device");
        exit(1);
    }
}

void addRoute()
{
    int sockfd;
    struct rtentry rt;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket creation failed\n");
        return;
    }

    struct sockaddr_in *sockinfo = (struct sockaddr_in *)&rt.rt_dst;
    sockinfo->sin_family = AF_INET;
    sockinfo->sin_addr.s_addr = inet_addr("192.168.7.0");

    sockinfo = (struct sockaddr_in *)&rt.rt_genmask;
    sockinfo->sin_family = AF_INET;
    sockinfo->sin_addr.s_addr = inet_addr("255.255.255.0");

    rt.rt_metric = 200 + 1;
    rt.rt_flags = RTF_UP;
    rt.rt_dev = "wgtest1";

    if (ioctl(sockfd, SIOCADDRT, &rt) < 0)
        perror("ioctl");

    return;
}

void add_ip()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    sprintf(ifr.ifr_name, "%s", "wgtest1");
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr("192.168.7.100");

    ioctl(sockfd, SIOCSIFADDR, &ifr);
    ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    ifr.ifr_flags = IFF_UP | IFF_RUNNING | IFF_NOARP;

    ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    close(sockfd);
}

int main()
{
    init_wg_dev(generate_wg_dev());

    add_ip();
    addRoute();

    wait_until_sigint();

    remove_wg_dev();

    return 0;
}
