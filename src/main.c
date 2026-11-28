#include "bitcoin.h"
#include "database.h"
#include "debug.h"
#include "netbase.h"
#include "protocol.h"
#include "serialization.h"
#include <getopt.h>
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
      opts->port = atoi(optarg);
      break;
    }
    case 't': {
      // if (atoi(optarg) <= 0) {
      //
      //}
      opts->threads = atoi(optarg);
      break;
    }
    // case 'l': {
    //
    // break;
    //}
    case 'h': {
      show_help = true;
      break;
    }
    case '?': {
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
  unsigned char peer_ip[16];
  t_message msg;
  struct addrinfo *res;
  t_seeder_opts opts;
  t_addr_db database;

  database_init(&database);
  opts.port = 8333;
  opts.seed = "seed.bitcoin.sipa.be";
  // opts.logfile
  opts.threads = 96;

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

  get_peer_ip(peer_ip, socket_fd);

  printf("Connection established.\n\n");

  // SEND VERSION
  unsigned char serialized_message[350];
  send_version(socket_fd, main_net, peer_ip, opts.port, serialized_message);

  // Prints deserialized version header
  printf("--------------------\nMessage sent:");
  unpack_message(&msg, serialized_message, 121);
  print_message(&msg);

  memset(&msg, 0, sizeof msg); // clear msg struct
  //  Receives message
  if (full_read(socket_fd, &msg) == 1) {
    freeaddrinfo(res);
    return 1;
  }

  // Prints received messages deserialized headers
  printf("--------------------\nMessage received:");
  print_message(&msg);

  unsigned char serialized_verack[24]; // stored for printing/debug
  send_headless(socket_fd, "verack", main_net, serialized_verack);

  memset(&msg, 0, sizeof msg); // clear msg struct
  // Prints deserialized verack
  printf("--------------------\nMessage sent:");
  unpack_message(&msg, serialized_verack, 24);
  print_message(&msg);

  memset(&msg, 0, sizeof msg); // clear msg struct
  //  Receives message
  if (full_read(socket_fd, &msg) == 1) {
    freeaddrinfo(res);
    return 1;
  }

  // Prints received messages deserialized headers
  printf("--------------------\nMessage received:");
  print_message(&msg);
  // HANDSHAKE IS FINISHED \o/

  // Send get addr
  unsigned char serializized_getaddr[24];
  send_headless(socket_fd, "getaddr", main_net, serializized_getaddr);
  memset(&msg, 0, sizeof msg); // clear msg struct
  // Prints deserialized getaddr
  printf("--------------------\nMessage sent:");
  unpack_message(&msg, serializized_getaddr, 24);
  print_message(&msg);

  // Listen for new messages, and implement ping pong
  while (1) {
    size_t recv_bytes;
    memset(&msg, 0, sizeof msg); // clear msg struct
    recv_bytes = full_read(socket_fd, &msg);
    if (recv_bytes == 1)
      break;
    // DEBUG / LOGGING
    printf("--------------------\nMessage received:");
    print_message(&msg);
    // DEBUG / LOGGING

    static const unsigned char PING[12] = "ping";
    static const unsigned char ADDR[12] = "addr";
    if (memcmp(msg.msg_header.command, PING, 12) == 0) {
      t_ping_pong_payload ping_payload;
      unsigned char serialized_pong[PING_PONG_PAYLOAD_LEN + 24];
      unpack_ping_pong_payload(&ping_payload, msg.payload, msg.msg_header.size);
      send_pong(socket_fd, main_net, ping_payload.nonce, serialized_pong);

      // DEBUG / LOGGING
      printf("--------------------\nMessage sent:");
      memset(&msg, 0, sizeof msg); // clear msg struct
      unpack_message(&msg, serialized_pong, PING_PONG_PAYLOAD_LEN + 24);
      print_message(&msg);
      // DEBUG / LOGGING
    } else if (memcmp(msg.msg_header.command, ADDR, 12) == 0) {
      t_addr_payload addr_payload;

      if (unpack_addr_payload(&addr_payload, msg.payload,
                              msg.msg_header.size) == 1)
        continue;
      if (addr_payload.addr_count == 0)
        continue;
      for (size_t i = 0; i < addr_payload.addr_count; i++) {
        t_peer peer;
        size_t id;

        id = database.peer_storage->nb_entries;
        peer_init(&peer, addr_payload.addr_array[i]);
        peer_storage_set(database.peer_storage, peer, id);
        peer_map_set(database.peer_map, peer.addr, id);
        untried_stack_push(database.untried_stack, peer.addr);
      }
      // DEBUG / LOGGING
      printf("\nDB_NB_ENTRIES: %d\n", (int)database.peer_storage->nb_entries);
      for (int i = 0; i < 16; i++) {
        printf("%02x ", (unsigned int)database.peer_storage
                            ->storage[database.peer_storage->nb_entries - 1]
                            .addr.ip[i]);
      }
      printf("port=%u\n", (unsigned int)database.peer_storage
                              ->storage[database.peer_storage->nb_entries - 1]
                              .addr.port);
      // DEBUG / LOGGING
    }
  }

  // Connection was closed! Free and return
  printf("\nConnection closed.\n");
  database_destroy(&database);

  freeaddrinfo(res);
  return 0;
}
