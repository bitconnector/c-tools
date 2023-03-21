//https://stackoverflow.com/questions/7158528/is-there-any-code-for-bitwise-and-ipv6-address-and-network-mask-prefix
//./a.out fe80::2c6:6aaf:7321:5760/100

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <arpa/inet.h>

#if defined __GNUC__ && __GNUC__ >= 2
#include <byteswap.h>
#else
#error "Sorry, you need GNU for this"
#endif

struct split
{
  uint64_t start;
  uint64_t end;
};

void ipv6_prefix (unsigned char *masked, unsigned char *packed, int prefix)
{
  struct split parts;
  uint64_t mask = 0;
  unsigned char *p = masked;

  memset(masked, 0, sizeof(struct in6_addr));
  memcpy(&parts, packed, sizeof(parts));

  if (prefix <= 64)
  {
    mask = bswap_64(bswap_64(parts.start) & ((uint64_t) (~0) << (64 - prefix)));
    memcpy(masked, &mask, sizeof(uint64_t));
    return;
  }

  prefix -= 64;

  memcpy(masked, &(parts.start), sizeof(uint64_t));
  p += sizeof(uint64_t);
  mask = bswap_64(bswap_64(parts.end) & (uint64_t) (~0) << (64 - prefix));
  memcpy(p, &mask, sizeof(uint64_t));
}

int main (int argc, char **argv)
{
  unsigned char packed[sizeof(struct in6_addr)];
  unsigned char masked[sizeof(struct in6_addr)];
  char buf[INET6_ADDRSTRLEN], *p;
  int prefix = 56;

  if (argc < 2)
    return 1;

  if ((p = strchr(argv[1], '/')))
  {
    *p++ = '\0';
    prefix = atoi(p);
  }

  inet_pton(AF_INET6, argv[1], packed);

  ipv6_prefix(masked, packed, prefix);

  inet_ntop(AF_INET6, masked, buf, INET6_ADDRSTRLEN);
  printf("prefix = %s/%d\n", buf, prefix);
  return 0;
}
