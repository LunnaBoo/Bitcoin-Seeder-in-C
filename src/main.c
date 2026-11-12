#include "bitcoin.h"
#include "debug.h"
#include "netbase.h"
#include "protocol.h"
#include "serialization.h"
#include <getopt.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

typedef struct s_seeder_opts {
  int threads; // coming soon
  int port;
  const char *seed;
  const char *logfile; // path to a future logfile
} t_seeder_opts;

void cli_parsing(t_seeder_opts *opts, int argc, char **argv) {
  static const char *help =
      "Bitcoin-seeder (In C)\n"
      "Options:\n"
      "-s <seed>      Seed node to collect peers from (replaces default)\n"
      "-t <threads>   Number of crawlers to run in parallel (default 96)\n"
      "-l <path>      Path to logfile\n"
      "-?, --help     Shows this text\n"
      "\n";

  bool show_help = false;

  while (1) {
    static struct option long_options[] = {
        {"seed", required_argument, 0, 's'},
        {"port", required_argument, 0, 'p'},
        {"threads", required_argument, 0, 't'},
        {"logfile", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "s:p:t:l:h", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 's': {
      opts->seed = optarg;
      break;
    }
    case 'p': {
      break;
    }
    case 't': {
      //
      break;
    }
    case 'l': {
      //
      break;
    }
    case 'h': {
      show_help = true;
      break;
    }
    }
    if (show_help) {
      printf("%s", help);
      exit(0);
    }
  }
}

int get_peer_ip(unsigned char *buf, int fd);

int main(int argc, char **argv) {
  int socket_fd;
  t_message msg;
  struct addrinfo *res;
  t_seeder_opts opts;
  opts.port = PORT;
  opts.seed = "seed.bitcoin.sipa.be";
  // opts.logfile
  // opts.threads = 96;

  cli_parsing(&opts, argc, argv);

  // DNS look up
  res = look_up_host((char *)opts.seed);
  if (res == NULL)
    return 1;

  // print IPs found
  print_addresses(res, (char *)opts.seed, 3);

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
  send_headless(socket_fd, "verack", main_net, serialized_verack);

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
