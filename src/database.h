#ifndef DATABASE_H
#define DATABASE_H

#define INITIAL_CAPACITY 65536

#include "protocol.h"
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
  size_t head;     // holds next empty slot
  size_t tail;     // holds oldest element
  size_t capacity; // starts at 65536
  size_t nb_entries;
} t_circular_buff;

typedef struct s_stack_manager {
  size_t capacity;
  size_t top;
  t_peer_addr *stack;
} t_stack_manager;

typedef struct s_peer_storage {
  size_t capacity;
  size_t nb_entries; // counts peers and maps the next free index in storage
  t_peer *storage;
} t_peer_storage;

// Most important one
typedef struct s_addr_db {
  t_peer_map *peer_map;         // maps addresses to IDs. Used to access storage
  t_peer_storage *peer_storage; // stores all PEERS. (banned are wiped)
  t_circular_buff *retry_queue; // stores tried peer ADDRESSES for next retries
  t_stack_manager *untried_stack; // stores peer ADDRESSES that were never tried
  //  t_peer_addr *banned_peers;   // stores all BANNED peer ADDRESSES

  // Add an array of good nodes, maybe? Gotta implement good/bad logic first...
} t_addr_db;

t_peer_map *peer_map_create(void);
void peer_map_destroy(t_peer_map *peer_map);
size_t peer_map_get(const t_peer_map *map, t_peer_addr key);
int peer_map_set(t_peer_map *map, t_peer_addr key, size_t value);

t_circular_buff *retry_queue_create(void);
void retry_queue_destroy(t_circular_buff *retry_queue);
int retry_queue_push(t_circular_buff *queue, t_peer_addr data);
int retry_queue_pop(t_circular_buff *queue, t_peer_addr *data);

t_stack_manager *untried_stack_create(void);
void untried_stack_destroy(t_stack_manager *stack_man);
int untried_stack_push(t_stack_manager *stack_man, t_peer_addr data);
int untried_stack_pop(t_stack_manager *stack_man, t_peer_addr *data);

t_peer_storage *peer_storage_create(void);
void peer_init(t_peer *peer, t_net_addr net_addr);
int peer_storage_set(t_peer_storage *peer_storage, t_peer peer, size_t id);
void peer_storage_destroy(t_peer_storage *peer_storage);

void database_init(t_addr_db *addr_db);
void database_destroy(t_addr_db *db);

#endif
