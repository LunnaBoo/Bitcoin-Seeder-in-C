#ifndef BITCOIN_H
#define BITCOIN_H

#include "netbase.h"
#include "serialization.h"

typedef struct s_peer_addr {
  unsigned char ip[16];
  uint16_t port;
} t_peer_addr;

typedef struct s_peer {
  t_peer_addr addr;
  uint64_t services;      // services it provides (ex.: addr or addrv2)
  int32_t client_version; // >= 70001 = good | <= 31900 = bad (should ban)
  int64_t last_success;   // UNIX timestamp of last successfull connection
  int64_t last_tried;     // UNIX timestamp of last connection attempt
  int32_t blocks;         // >= 350.000 = good
  int32_t total_attempts;
  int32_t total_successes;
  uint8_t banned;
} t_peer;

typedef struct s_entry {
  t_peer_addr addr;
  size_t id;
} t_entry;

typedef struct s_peer_map {
  size_t max_size;
  size_t current_size;
  t_entry *entries; // slots
} t_peer_map;

int send_version(int fd, const int network, const unsigned char *peer_ip,
                 int my_port, unsigned char *buf);
int send_headless(int fd, const char *cmd, const int network,
                  unsigned char *buf);
int send_ping(int fd, const int network, unsigned char *buf);

int send_pong(int fd, const int network, uint64_t nonce, unsigned char *buf);

#endif
