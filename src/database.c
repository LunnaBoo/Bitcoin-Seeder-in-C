#include "database.h"

// hash_map implementation section
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

static uint64_t hash_key(t_peer_addr key);
int peer_addrcmp(t_peer_addr s1, t_peer_addr s2);

t_peer_map *peer_map_create(void) {
  t_peer_map *peer_map;

  peer_map = malloc(sizeof(t_peer_map));
  if (!peer_map)
    return NULL;
  peer_map->nb_entries = 0;
  peer_map->capacity = INITIAL_CAPACITY;
  peer_map->entries = calloc(peer_map->capacity, sizeof(t_entry));
  if (!peer_map->entries) {
    free(peer_map);
    return NULL;
  }

  return 0;
}

void peer_map_destroy(t_peer_map *peer_map) {
  if (!peer_map)
    return;
  free(peer_map->entries);
  free(peer_map);
}

// look up
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

int peer_map_expand(t_peer_map *map) {
  //
}

int peer_map_set_entry(t_entry *entries, size_t capacity, t_peer_addr key,
                       size_t value, size_t nb_entries) {
  //
}

int peer_map_set(t_peer_map *map, t_peer_addr key, size_t value) {
  if (value < 0)
    return 1;

  if (map->nb_entries >= (map->capacity * 3 / 4)) {
    if (!peer_map_expand(map))
      return 1;
  }

  return peer_map_set_entry(map->entries, map->capacity, key, value,
                            map->nb_entries);
}

int peer_addrcmp(t_peer_addr s1, t_peer_addr s2) {
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
  for (size_t i = 0; i < 16; i++) {
    hash ^= raw_port[i];
    hash *= FNV_PRIME;
  }

  return hash;
}

// hash_map implementation section
