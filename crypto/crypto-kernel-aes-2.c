//https://stackoverflow.com/questions/59931316/linux-kernel-crypto-api-for-ecb-aes
//https://www.kernel.org/doc/html/latest/crypto/userspace-if.html

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>
#include <string.h>

#ifndef SOL_ALG
#define SOL_ALG 279
#endif

void encrypt(char *key, int keylen, char *data, int datalen)
{
  struct sockaddr_alg sa = {
      .salg_family = AF_ALG,
      .salg_type = "skcipher",
      .salg_name = "ecb(aes)"};
  char cbuf[CMSG_SPACE(4)] = {0};

  int tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

  setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY,
             key,
             16);

  int opfd = accept(tfmfd, NULL, 0);

  struct msghdr msg = {};
  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof(cbuf);

  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_OP;
  cmsg->cmsg_len = CMSG_LEN(4);
  *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_ENCRYPT;

  struct iovec iov;
  iov.iov_base = data;
  iov.iov_len = 16;

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  sendmsg(opfd, &msg, 0);

  char buf[16];
  read(opfd, buf, 16);

  for (int i = 0; i < 16; i++)
  {
    printf("%02x", (unsigned char)buf[i]);
  }
  printf("\n");

  close(opfd);
  close(tfmfd);
}

void decrypt(char *key, int keylen, char *data, int datalen)
{
  struct sockaddr_alg sa = {
      .salg_family = AF_ALG,
      .salg_type = "skcipher",
      .salg_name = "ecb(aes)"};
  char cbuf[CMSG_SPACE(4)] = {0};

  int tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

  setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY, key, 16);

  int opfd = accept(tfmfd, NULL, 0);

  struct msghdr msg = {};
  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof(cbuf);

  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_OP;
  cmsg->cmsg_len = CMSG_LEN(4);
  *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_DECRYPT;

  struct iovec iov;
  iov.iov_base = data;
  iov.iov_len = 16;

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  sendmsg(opfd, &msg, 0);

  char buf[16];
  read(opfd, buf, 16);

  printf("%s\n", buf);

  close(opfd);
  close(tfmfd);
}

int main(void)
{

  char *msg1 = "hello world";
  char *msg2 = "Single block msg";
  char *msg3 = "Single block msg and longer";
  char *key1 = "123456";
  char *key2 = "\x06\xa9\x21\x40\x36\xb8\xa1\x5b\x51\x2e\x03\xd5\x34\x12\x00\x06";
  char *cyp1 = "\x3a\xe0\x0f\xbd\x31\xdf\xae\xed\x4d\xa6\xe4\x4f\xe2\xc1\x1b\x4f";

  encrypt(key2, sizeof(key2), msg2, sizeof(msg2));
  encrypt(key2, sizeof(key2), msg3, sizeof(msg3));
  encrypt(key1, sizeof(key1), msg1, sizeof(msg1));

  printf("decrypt\n");

  decrypt(key2, sizeof(key2), cyp1, sizeof(cyp1));

  return 0;
}
