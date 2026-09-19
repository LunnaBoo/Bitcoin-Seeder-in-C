#include "netbase.h"
#include <stdio.h>

int main(void) {
  int socket_fd;
  char buffer[1000];
  struct addrinfo *res;
  char *hostname = "seed.bitcoin.sipa.be";

  // DNS look up
  res = look_up_host(hostname);
  if (res == NULL)
    return 1;

  // print IPs found
  print_addresses(res, hostname, 3);
  if ((socket_fd = connect_socket(res, 3)) == -1) {
    printf("ERROR: connect_socket() failed.\n");
    return 1;
  } else {
    printf("Connection established.\n");
  }

  // Prints incoming message and warns if connection was closed
  int byte_num = recv(socket_fd, buffer, sizeof buffer, 0);
  for (int i = 0; i < byte_num; i++)
    printf("%02x ", (unsigned char)buffer[i]);
  printf("\n");
  if (byte_num == 0)
    printf("Connection closed.");
  // printf("recv errno = %d (%s)\n", errno, strerror(errno));

  freeaddrinfo(res);
  return 0;
}
