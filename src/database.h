#ifndef DATABASE_H
#define DATABASE_H

#define INITIAL_CAPACITY 65536

#include "utils.h"

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

// Will map key Addr -> Peer ID | peer map
typedef struct s_peer_map {
  size_t capacity;
  size_t nb_entries;
  t_entry *entries; // slots
} t_peer_map;

// Peer ID = Index to peer | retry queue
typedef struct s_circular_buff {
  t_peer_addr *buffer;
  size_t head;
  size_t tail;
  size_t capacity; // starts at 65536
} t_circular_buff;

typedef struct s_stack {
  size_t capacity;
  size_t nb_entries;
  t_peer_addr *stack;
} t_stack;

typedef struct s_peer_storage {
  size_t capacity;
  size_t nb_entries; // counts peers and maps the next free index in storage
  t_peer *peer_storage;
} t_peer_storage;

// Most important one
typedef struct s_addr_db {
  int total_ids;
  t_peer_map addr_to_id;       // maps addresses to IDs. Used to access storage
  t_peer_storage peer_storage; // stores all PEERS. (banned are wiped)
  t_circular_buff retry_queue; // stores tried peer ADDRESSES for future retries
  t_peer_addr *untried_stack;  // stores peer ADDRESSES that were never tried
  t_peer_addr *banned_peers;   // stores all BANNED peer ADDRESSES

  // Add an array of good nodes, maybe? Gotta implement good/bad logic first...
} t_addr_db;

#endif
