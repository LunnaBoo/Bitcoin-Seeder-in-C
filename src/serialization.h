#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "protocol.h"
#include "utils.h"
#include <arpa/inet.h>


typedef struct s_reader {
  unsigned char *buf;
  size_t current_position;
  size_t length;
} t_reader;

int pack_version_payload(unsigned char *buf, t_version_payload payload);
void pack_message(unsigned char *buf, t_message message, size_t payload_len);
int unpack_message(t_message **msgs, size_t *msg_count,
                   const unsigned char *buf, const size_t buf_len);
int get_peer_ip(unsigned char *buf, int fd);

unsigned char *check_overflow(t_reader *reader, size_t bytes_to_write);

int test_unpack_message(unsigned char *buf, size_t buf_len);


#endif
