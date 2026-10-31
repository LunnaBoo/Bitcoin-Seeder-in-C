#include "bitcoin.h"
#include "netbase.h"
#include "protocol.h"
#include "serialization.h"
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

  // Connect to socket
  if ((socket_fd = connect_socket(res, 3)) == -1) {
    printf("ERROR: connect_socket() failed.\n");
    return 1;
  }

  unsigned char peer_ip[16];
  get_peer_ip(peer_ip, socket_fd);

  printf("Connection established.\n\n");

  t_message *msgs;
  size_t msg_count;

  // SEND VERSION
  unsigned char serialized_message[121];
  send_version(socket_fd, main_net, peer_ip, serialized_message);

  // Prints deserialized version header
  printf("--------------------\nMessage sent:");
  unpack_message(&msgs, &msg_count, serialized_message, 121);
  print_message(msgs, msg_count);

  //  Receives message
  int byte_num = recv(socket_fd, buffer, sizeof buffer, 0);

  // Prints received messages deserialized headers
  printf("--------------------\nMessage received:");
  free(msgs);
  msgs = NULL;
  unpack_message(&msgs, &msg_count, (unsigned char *)buffer, byte_num);
  print_message(msgs, msg_count);

  unsigned char serialized_verack[24];
  send_verack(socket_fd, main_net, peer_ip, serialized_verack);

  // Prints deserialized verack
  printf("--------------------\nMessage sent:");
  free(msgs);
  msgs = NULL;
  unpack_message(&msgs, &msg_count, serialized_verack, 24);
  print_message(msgs, msg_count);
  // HANDSHAKE IS FINISHED \o/

  // Listen for new messages, and implement ping pong
  while (byte_num > 0) {
    byte_num = recv(socket_fd, buffer, sizeof buffer, 0);
  }

  // Connection was closed! Free and return
  printf("\nConnection closed.\n");
  freeaddrinfo(res);
  return 0;
}
