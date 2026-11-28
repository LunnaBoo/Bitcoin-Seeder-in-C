#include "database.h"
#include <iso646.h>

// hash_map implementation section

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

static uint64_t hash_key(t_peer_addr key);
static int peer_addrcmp(t_peer_addr s1, t_peer_addr s2);

t_peer_map *peer_map_create(void) {
  t_peer_map *peer_map;

  peer_map = malloc(sizeof(t_peer_map));
  if (!peer_map)
    return NULL;
  peer_map->nb_entries = 0;
  peer_map->capacity = INITIAL_CAPACITY;
  peer_map->entries = calloc(INITIAL_CAPACITY, sizeof(t_entry));
  if (!peer_map->entries) {
    free(peer_map);
    return NULL;
  }

  return peer_map;
}

void peer_map_destroy(t_peer_map *peer_map) {
  if (!peer_map)
    return;
  free(peer_map->entries);
  free(peer_map);
}

// look up: returns ID if found and -1 if not found
size_t peer_map_get(const t_peer_map *map, t_peer_addr key) {
  uint64_t hash;
  size_t index;

  hash = hash_key(key);
  index = (size_t)(hash & (uint64_t)(map->capacity - 1));

  while (map->entries[index].addr.port != 0) {
    if (peer_addrcmp(map->entries[index].addr, key) == 0)
      return map->entries[index].id;
    index++;
    if (index >= map->capacity)
      index = 0;
  }
  return -1;
}

static int peer_map_set_entry(t_entry *entries, size_t capacity,
                              size_t *nb_entries, t_peer_addr key,
                              size_t value) {
  uint64_t hash;
  size_t index;

  hash = hash_key(key);
  index = (size_t)(hash & (uint64_t)(capacity - 1));

  while (entries[index].addr.port != 0) {
    if (peer_addrcmp(entries[index].addr, key) == 0) {
      entries[index].id = value;
      return 1;
    }
    index++;
    if (index >= capacity)
      index = 0;
  }
  entries[index].addr = key;
  entries[index].id = value;
  if (nb_entries)
    (*nb_entries)++;

  return 0;
}

static bool peer_map_expand(t_peer_map *map) {
  size_t new_capacity;

  new_capacity = map->capacity * 2;
  if (new_capacity < map->capacity)
    return false; // overflow

  t_entry *new_entries = calloc(new_capacity, sizeof(t_entry));
  if (!new_entries)
    return false;

  for (size_t i = 0; i < map->capacity; i++) {
    t_entry tmp_entry = map->entries[i];
    if (tmp_entry.addr.port != 0)
      peer_map_set_entry(new_entries, new_capacity, NULL, tmp_entry.addr,
                         tmp_entry.id);
  }
  free(map->entries);
  map->entries = new_entries;
  map->capacity = new_capacity;

  return true;
}

int peer_map_set(t_peer_map *map, t_peer_addr key, size_t value) {
  if (map->nb_entries >= (map->capacity * 3 / 4)) {
    if (!peer_map_expand(map))
      return 1;
  }

  return peer_map_set_entry(map->entries, map->capacity, &map->nb_entries, key,
                            value);
}

static int peer_addrcmp(t_peer_addr s1, t_peer_addr s2) {
  if (memcmp(s1.ip, s2.ip, 16) == 0 && s1.port == s2.port)
    return 0;
  else
    return 1;
}

static uint64_t hash_key(t_peer_addr key) {
  uint64_t hash = FNV_OFFSET;
  const uint8_t *raw_port;

  for (size_t i = 0; i < 16; i++) {
    hash ^= key.ip[i];
    hash *= FNV_PRIME;
  }
  raw_port = (const uint8_t *)&key.port; // tells the compiler this addr should
                                         // be read as an array of bytes
  for (size_t i = 0; i < 2; i++) {
    hash ^= raw_port[i];
    hash *= FNV_PRIME;
  }

  return hash;
}

// hash_map implementation section

// circular_buff implementation section

t_circular_buff *retry_queue_create(void) {
  t_circular_buff *retry_queue;

  retry_queue = malloc(sizeof(t_circular_buff));
  if (!retry_queue)
    return NULL;
  retry_queue->capacity = INITIAL_CAPACITY;
  retry_queue->head = 0;
  retry_queue->tail = 0;
  retry_queue->nb_entries = 0;
  retry_queue->buffer = calloc(INITIAL_CAPACITY, sizeof(t_peer_addr));
  if (!retry_queue->buffer) {
    free(retry_queue);
    return NULL;
  }

  return retry_queue;
}

void retry_queue_destroy(t_circular_buff *retry_queue) {
  if (!retry_queue)
    return;
  free(retry_queue->buffer);
  free(retry_queue);
}

static bool retry_queue_expand(t_circular_buff *queue) {
  t_peer_addr *new_buffer;
  size_t new_capacity;
  size_t tail_to_end;
  size_t start_to_head;

  new_capacity = queue->capacity * 2;
  if (new_capacity < queue->capacity)
    return false;

  new_buffer = malloc(new_capacity * sizeof(t_peer_addr));
  if (!new_buffer)
    return false;
  tail_to_end = queue->capacity - queue->tail;

  if (queue->nb_entries <= tail_to_end) {
    memcpy(new_buffer, &queue->buffer[queue->tail],
           queue->nb_entries * sizeof(t_peer_addr));
  } else {
    start_to_head = queue->nb_entries - tail_to_end;
    memcpy(new_buffer, &queue->buffer[queue->tail],
           tail_to_end * sizeof(t_peer_addr));
    memcpy(new_buffer + tail_to_end, &queue->buffer[0],
           start_to_head * sizeof(t_peer_addr));
  }

  free(queue->buffer);
  queue->buffer = new_buffer;
  queue->capacity = new_capacity;
  queue->head = queue->nb_entries;
  queue->tail = 0;

  return true;
}

int retry_queue_push(t_circular_buff *queue, t_peer_addr data) {
  if (queue->nb_entries >= queue->capacity) {
    if (!retry_queue_expand(queue))
      return 1;
  }
  queue->buffer[queue->head] = data;
  queue->nb_entries++;
  queue->head++;
  if (queue->head >= queue->capacity)
    queue->head = 0;

  return 0;
}

int retry_queue_pop(t_circular_buff *queue, t_peer_addr *data) {
  size_t next;

  if (queue->nb_entries == 0)
    return 1;

  next = queue->tail + 1;
  if (next >= queue->capacity)
    next = 0;

  *data = queue->buffer[queue->tail];
  queue->tail = next;
  queue->nb_entries--;

  return 0;
}

// circular_buff implementation section

// stack implementation section

t_stack_manager *untried_stack_create(void) {
  t_stack_manager *untried_stack;

  untried_stack = malloc(sizeof(t_stack_manager));
  if (!untried_stack)
    return NULL;
  untried_stack->capacity = INITIAL_CAPACITY;
  untried_stack->stack = malloc(INITIAL_CAPACITY * sizeof(t_peer_addr));
  if (!untried_stack->stack) {
    free(untried_stack);
    return NULL;
  }
  untried_stack->top = 0;

  return untried_stack;
}

void untried_stack_destroy(t_stack_manager *stack_man) {
  if (!stack_man)
    return;
  free(stack_man->stack);
  free(stack_man);
}

static bool untried_stack_expand(t_stack_manager *stack_man) {
  size_t new_capacity;
  t_peer_addr *new_stack;

  new_capacity = stack_man->capacity * 2;
  if (new_capacity < stack_man->capacity)
    return false;

  new_stack = realloc(stack_man->stack, new_capacity * sizeof(t_peer_addr));
  if (!new_stack)
    return false;

  free(stack_man->stack);
  stack_man->stack = new_stack;
  stack_man->capacity = new_capacity;

  return true;
}

int untried_stack_push(t_stack_manager *stack_man, t_peer_addr data) {
  if (stack_man->top >= stack_man->capacity) {
    if (!untried_stack_expand(stack_man))
      return 1;
  }
  stack_man->stack[stack_man->top++] = data;

  return 0;
}

int untried_stack_pop(t_stack_manager *stack_man, t_peer_addr *data) {
  if (stack_man->top == 0)
    return 1;
  *data = stack_man->stack[--stack_man->top];

  return 0;
}

// stack implementation section

// init

t_peer_storage *peer_storage_create(void) {
  t_peer_storage *peer_storage;

  peer_storage = malloc(sizeof(t_peer_storage));
  if (!peer_storage)
    return NULL;
  peer_storage->capacity = INITIAL_CAPACITY;
  peer_storage->nb_entries = 0;
  peer_storage->storage = malloc(INITIAL_CAPACITY * sizeof(t_peer));
  if (!peer_storage->storage)
    return NULL;

  return peer_storage;
}

void peer_storage_destroy(t_peer_storage *peer_storage) {
  if (!peer_storage)
    return;
  free(peer_storage->storage);
  free(peer_storage);
}

static bool peer_storage_expand(t_peer_storage *peer_storage) {
  size_t new_capacity;
  t_peer *new_storage;

  new_capacity = peer_storage->capacity * 2;
  if (new_capacity < peer_storage->capacity)
    return false;

  new_storage = realloc(peer_storage->storage, new_capacity * sizeof(t_peer));
  if (!new_storage)
    return false;

  free(peer_storage->storage);
  peer_storage->storage = new_storage;
  peer_storage->capacity = new_capacity;

  return true;
}

int peer_storage_set(t_peer_storage *peer_storage, t_peer peer, size_t id) {
  if (peer_storage->nb_entries >= peer_storage->capacity)
    if (!peer_storage_expand(peer_storage))
      return 1;

  peer_storage->storage[id] = peer;
  peer_storage->nb_entries++;

  return 0;
}

void database_destroy(t_addr_db *db) {
  if (!db)
    return;
  peer_map_destroy(db->peer_map);
  retry_queue_destroy(db->retry_queue);
  untried_stack_destroy(db->untried_stack);
  peer_storage_destroy(db->peer_storage);
}

void database_init(t_addr_db *addr_db) {
  addr_db->peer_map = peer_map_create();
  addr_db->retry_queue = retry_queue_create();
  addr_db->untried_stack = untried_stack_create();
  addr_db->peer_storage = peer_storage_create();
}

void peer_init(t_peer *peer, t_net_addr net_addr) {
  memcpy(peer->addr.ip, net_addr.ip, 16);
  peer->addr.port = net_addr.port;
  peer->services = net_addr.services;
  peer->banned = 0;
  peer->blocks = 0;
  peer->client_version = 0;
  peer->last_success = 0;
  peer->last_tried = 0;
  peer->total_successes = 0;
  peer->total_attempts = 0;
}

// init

// persistent memory section
