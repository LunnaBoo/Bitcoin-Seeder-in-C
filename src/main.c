#include "bitcoin.h"
#include "netbase.h"
#include "protocol.h"
#include "serialization.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>


int get_peer_ip(unsigned char *buf, int fd);

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
  }

  unsigned char peer_ip[16];
  get_peer_ip(peer_ip, socket_fd);

  printf("Connection established.\n\n");

  // SEND VERSION
  unsigned char serialized_message[121];
  send_version(socket_fd, main_net, peer_ip, serialized_message);
  printf("Message sent:\n");
  for (int i = 0; i < 121; i++)
    printf("%02x ", (unsigned char)serialized_message[i]);
  printf("\n\n");

  //  Prints incoming message and warns if connection was closed
  int byte_num = recv(socket_fd, buffer, sizeof buffer, 0);
  printf("Message received:\n");
  for (int i = 0; i < byte_num; i++)
    printf("%02x ", (unsigned char)buffer[i]);
  printf("\n");

  t_message *msgs;
  size_t msg_count;

  unpack_message(&msgs, &msg_count, (unsigned char *)buffer, byte_num);
  // test_unpack_message();

  if (byte_num == 0)
    printf("\nConnection closed.\n");
  // printf("recv errno = %d (%s)\n", errno, strerror(errno));

  //  deserialize(&buff, byte_num)

  freeaddrinfo(res);
  return 0;
}
