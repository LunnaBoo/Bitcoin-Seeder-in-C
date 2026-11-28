#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "protocol.h"
#include "utils.h"
#include <arpa/inet.h>

#define MAX_ADDR_COUNT 1000

typedef struct s_reader {
  unsigned char *buf;
  size_t current_position;
  size_t length;
} t_reader;

int pack_version_payload(unsigned char *buf, t_version_payload payload);
void pack_message(unsigned char *buf, t_message message, size_t payload_len);
void pack_ping_pong_payload(unsigned char *buf, t_ping_pong_payload payload);

int unpack_message(t_message *msg, unsigned char *buf, const size_t buf_len);
int unpack_ping_pong_payload(t_ping_pong_payload *payload,
                             const unsigned char *buf, const size_t buf_len);
int unpack_version_payload(t_version_payload *payload, unsigned char *buf,
                           const size_t buf_len);
int unpack_addr_payload(t_addr_payload *payload, unsigned char *buf,
                        const size_t buf_len);

int get_peer_ip(unsigned char *buf, int fd);
unsigned char *check_overflow(t_reader *reader, size_t bytes_to_write);
int full_read(int fd, t_message *msg);

#endif
