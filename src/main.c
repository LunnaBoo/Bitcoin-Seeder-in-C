#include "bitcoin.h"
#include "debug.h"
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
  t_message msg;
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

  // DEBUG / LOGGING
  t_message *my_msgs;
  size_t msg_count;
  // DEBUG / LOGGING

  // SEND VERSION
  unsigned char serialized_message[121];
  send_version(socket_fd, main_net, peer_ip, serialized_message);

  // Prints deserialized version header
  printf("--------------------\nMessage sent:");
  unpack_message(&my_msgs, &msg_count, serialized_message, 121);
  print_message(my_msgs, 1);

  //  Receives message
  full_read(socket_fd, &msg);

  // Prints received messages deserialized headers
  printf("--------------------\nMessage received:");
  print_message(&msg, 1);

  unsigned char serialized_verack[24]; // stored for printing/debug
  send_verack(socket_fd, main_net, serialized_verack);

  // Prints deserialized verack
  printf("--------------------\nMessage sent:");
  free(my_msgs);
  my_msgs = NULL;
  unpack_message(&my_msgs, &msg_count, serialized_verack, 24);
  print_message(my_msgs, msg_count);
  // HANDSHAKE IS FINISHED \o/

  // Listen for new messages, and implement ping pong
  while (1) {
    if (full_read(socket_fd, &msg) < 0) {
      return 1;
    }
    // DEBUG / LOGGING
    printf("--------------------\nMessage received:");
    print_message(&msg, 1);
    // DEBUG / LOGGING

    static const unsigned char PING[12] = "ping";
    if (memcmp(msg.msg_header.command, PING, 12) == 0) {
      t_ping_pong_payload ping_payload;
      unsigned char serialized_pong[PING_PONG_PAYLOAD_LEN + 24];
      unpack_ping_pong_payload(&ping_payload, msg.payload, msg.msg_header.size);
      send_pong(socket_fd, main_net, ping_payload.nonce, serialized_pong);

      // DEBUG / LOGGING
      printf("--------------------\nMessage sent:");
      free(my_msgs);
      my_msgs = NULL;
      unpack_message(&my_msgs, &msg_count, serialized_pong,
                     PING_PONG_PAYLOAD_LEN + 24);
      print_message(my_msgs, msg_count);
      // DEBUG / LOGGING
    }
  }

  // Connection was closed! Free and return
  printf("\nConnection closed.\n");
  freeaddrinfo(res);
  return 0;
}
