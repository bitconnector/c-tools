#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/types.h>
#include <sys/ioctl.h>

void search_port()
{
    char *input_str = "192.168.7.1:7682";
    char *ret;

    printf("input: %s\n", input_str);

    ret = strchr(input_str, ':');
    if (!ret)
    {
        printf("no port given\n");
        return;
    }
    printf("locaton_ret: %p\n", ret);

    int port = atoi(ret + 1);
    printf("int_port: %i\n", port);

    int index = (int)(ret - input_str);
    printf("ip_char_size: %i\n", index);

    char col[15];
    strncpy(col, input_str, index);
    col[index] = '\0';
    printf("str_ip: %s\n", col);

    struct in_addr ip_bin;
    ip_bin.s_addr = inet_addr(col);
    //inet_pton(AF_INET, str, &ip_bin.s_addr);
    printf("inet_addr: %s\n", inet_ntoa(ip_bin));
}

void search_prefix(char *input_str)
{
    char ip_str[20];
    int index;

    char *ret = strchr(input_str, '/');
    if (ret)
    {
        int prefix = atoi(ret + 1);
        printf("int_prefix: %i\n", prefix);

        index = (int)(ret - input_str);
    }
    else
    {
        index = strlen(input_str);
        int prefix = 32;
    }
    strncpy(ip_str, input_str, index);
    ip_str[index] = '\0';

    struct in_addr ip_bin;
    ip_bin.s_addr = inet_addr(ip_str);
    printf("inet_addr: %s\n", inet_ntoa(ip_bin));
}

void get_ip_in_list()
{
    char *input_str = "192.168.7.1/24,192.168.8.1/24,192.168.178.4/32";
    //char *input_str = NULL;
    printf("input: %s\n", input_str);

    if (!input_str)
    {
        printf("no ip given\n");
        return;
    }
    char *ret = input_str - 1;
    do
    {
        ret++;
        printf("---got %p - %s\n", ret, ret);
        search_prefix(ret);
        ret = strchr(ret, ',');
    } while (ret);
    printf("list done\n");
}

struct in_addr genMask(int prefix)
{
    struct in_addr mask;
    if (prefix)
    {
        mask.s_addr = htonl(~((1 << (32 - prefix)) - 1));
    }
    else
    {
        mask.s_addr = htonl(0);
    }
    //printf("inet_mask: %s\n", inet_ntoa(mask));
    return mask;
}

//-----
typedef struct wg_allowedip
{
    uint16_t family;
    union
    {
        struct in_addr ip4;
        struct in6_addr ip6;
    };
    uint8_t cidr;
    struct wg_allowedip *next_allowedip;
} wg_allowedip;

struct in_addr setIpNet(struct in_addr ip, int prefix)
{
    struct in_addr mask = genMask(prefix);
    ip.s_addr &= mask.s_addr;
    return ip;
}

int checkIpNet(wg_allowedip *old, wg_allowedip new)
{
    wg_allowedip *ptr = old;
    while (old)
    {
        if (new.cidr < old->cidr) //new netmask is smaller
        {
            struct in_addr smallNet = setIpNet(old->ip4, new.cidr);
            if (memcmp(&smallNet, &new.ip4, sizeof(struct in_addr)) == 0) //same nets
            {
                wg_allowedip *next = old->next_allowedip;
                memcpy(old, &new, sizeof(wg_allowedip));
                old->next_allowedip = next;
                return 1;
            }
        }
        else
        {
            struct in_addr smallNet = setIpNet(new.ip4, old->cidr);
            if (memcmp(&smallNet, &old->ip4, sizeof(struct in_addr)) == 0) //ip already covered
            {
                return 2;
            }
        }
        old = old->next_allowedip;
    }
}

void getMasks()
{
    struct subnet
    {
        char ip_str[20];
        int prefix
    };
    struct subnet test[] = {{"192.168.8.1", 24}, {"10.30.0.1", 16}, {"10.30.0.1", 32}};

    wg_allowedip *first = NULL;
    wg_allowedip *ptr;

    for (int i = 0; i < sizeof(test) / sizeof(struct subnet); i++)
    {
        if (!first)
        {
            first = malloc(sizeof(wg_allowedip));
            first->ip4.s_addr = inet_addr(test[i].ip_str);
            first->cidr = test[i].prefix;
            ptr = first;
        }
        else
        {
            ptr->next_allowedip = malloc(sizeof(wg_allowedip));
        }
    }
}

int main()
{
    //search_prefix();
    //genMask(16);
    get_ip_in_list();
}