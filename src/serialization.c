/* NOTES
 *1. little endian for everything BUT IP and PORT
 *2. IP and PORT are NETWORK byte order a.k.a big endian
 *3. Use memcpy to serialize unsigned char* and char*; they're raw byte vuffers
 *4. To write unsigned char* / char * to a buffer, don't write the '\0'
 * terminator.
 *5. In fixed length fields, if data is short just fill remaining bytes with
 * '\0's
 */

#include "serialization.h"
#include "protocol.h"
#include <stdlib.h>
#include <sys/types.h>

// Concatenate byte-by-byte of the value in little endian order
/*
static void pack_int16(unsigned char *buf, uint16_t i) {
  *(buf++) = i;
  *(buf++) = i >> 8;
}
*/
static void pack_int32(unsigned char *buf, uint32_t i) {
  *(buf++) = i;
  *(buf++) = i >> 8;
  *(buf++) = i >> 16;
  *(buf++) = i >> 24;
}

static void pack_int64(unsigned char *buf, uint64_t i) {
  *(buf++) = i;
  *(buf++) = i >> 8;
  *(buf++) = i >> 16;
  *(buf++) = i >> 24;
  *(buf++) = i >> 32;
  *(buf++) = i >> 40;
  *(buf++) = i >> 48;
  *(buf++) = i >> 56;
}

static int16_t unpack_int16(unsigned char *buf) {
  uint16_t i2;
  int i;

  i2 = ((uint16_t)buf[1] << 8) | buf[0];
  if (i2 <= 0x7fffu)
    i = i2;
  else
    i = -1 - (uint16_t)(0xffffu - i2);

  return i;
}

static uint16_t unpack_uint16(unsigned char *buf) {
  return ((uint16_t)buf[1] << 8) | buf[0];
}

static int32_t unpack_int32(unsigned char *buf) {
  uint32_t i2;
  int32_t i;

  i2 = buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) |
       ((uint32_t)buf[3] << 24);
  if (i2 <= 0x7fffffffu)
    i = i2;
  else
    i = -1 - (int32_t)(0xffffffffu - i2);

  return i;
}

static uint32_t unpack_uint32(unsigned char *buf) {
  return buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) |
         ((uint32_t)buf[3] << 24);
}

static int64_t unpack_int64(unsigned char *buf) {
  uint64_t i2;
  int64_t i;

  i2 = buf[0] | ((uint64_t)buf[1] << 8 | ((uint64_t)buf[2] << 16) |
                 ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 32) |
                 ((uint64_t)buf[5] << 40) | ((uint64_t)buf[6] << 48) |
                 ((uint64_t)buf[7] << 56));
  if (i2 <= 0x7fffffffffffffffu)
    i = i2;
  else
    i = -1 - (int64_t)(0xffffffffffffffffu - i2);

  return i;
}

static uint64_t unpack_uint64(unsigned char *buf) {
  return buf[0] | ((uint64_t)buf[1] << 8 | ((uint64_t)buf[2] << 16) |
                   ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 32) |
                   ((uint64_t)buf[5] << 40) | ((uint64_t)buf[6] << 48) |
                   ((uint64_t)buf[7] << 56));
}

static void pack_var_str(unsigned char *buf, t_var_str str) {
  // pack length
  // pack char[i] while i < length
  int i = 0;

  buf[i] = str.length;

  while (i < str.length) {
    buf[i + 1] = (unsigned char)str.string[i];
    i++;
  }
}

static void pack_port(unsigned char *buf, uint16_t port) {
  port = htons(port);
  memcpy(buf, &port, 2);
}

/*
static int pack_ip(unsigned char *buf, char *ip, bool is_ipv4) {
  // serializes IPs from string to network order
  // (this function was a mistake but now i'm embarassed of removing it)
  if (is_ipv4) {
    memset(buf, 0x00, 10);
    buf[10] = 0xFF;
    buf[11] = 0xFF;
    if (inet_pton(AF_INET, ip, buf + 12) <= 0)
      return -1;
  } else {
    if (inet_pton(AF_INET6, ip, buf) <= 0)
      return -1;
  }
  return 0;
}
*/

static void pack_net_addr(unsigned char *buf, t_net_addr net_addr,
                          bool has_time) {
  unsigned char time[4];
  unsigned char services[8];
  unsigned char port[2]; // big endian
  int offset = 0;

  pack_int32(time, net_addr.time);
  pack_int64(services, net_addr.services);
  pack_port(port, net_addr.port);

  if (has_time) {
    memcpy(buf, time, 4);
    offset += 4;
  }
  memcpy(buf + offset, services, 8);
  offset += 8;
  memcpy(buf + offset, net_addr.ip, 16); // already unsigned char
  offset += 16;
  memcpy(buf + offset, port, 2);
}

static void pack_header(unsigned char *buf, t_message_header header) {
  unsigned char size[4];
  int offset = 0;

  pack_int32(size, header.size);

  memcpy(buf, header.magic, magic_length);
  offset += magic_length;
  memcpy(buf + offset, header.command, command_length);
  offset += command_length;
  memcpy(buf + offset, size, 4);
  offset += 4;
  memcpy(buf + offset, header.checksum, checksum_length);
}

int pack_version_payload(unsigned char *buf, t_version_payload payload) {
  // return payload_len
  unsigned char version[4];
  unsigned char services[8];
  unsigned char timestamp[8];
  unsigned char addr_recv[26];
  unsigned char addr_from[26];
  unsigned char nonce[8];
  unsigned char user_agent[payload.user_agent.length + 1];
  unsigned char start_height[4];
  unsigned char relay[1];
  int payload_len;
  int offset = 0;

  pack_int32(version, payload.version);
  pack_int64(services, payload.services);
  pack_int64(timestamp, payload.timestamp);
  pack_net_addr(addr_recv, payload.addr_recv, false);
  pack_net_addr(addr_from, payload.addr_from, false);
  pack_int64(nonce, payload.nonce);
  pack_var_str(user_agent, payload.user_agent);
  pack_int32(start_height, payload.start_height);
  memcpy(relay, &payload.relay, 1);

  memcpy(buf, version, 4);
  offset += 4;
  memcpy(buf + offset, services, 8);
  offset += 8;
  memcpy(buf + offset, timestamp, 8);
  offset += 8;
  memcpy(buf + offset, addr_recv, 26);
  offset += 26;
  memcpy(buf + offset, addr_from, 26);
  offset += 26;
  memcpy(buf + offset, nonce, 8);
  offset += 8;
  payload_len = 80;
  memcpy(buf + offset, user_agent, payload.user_agent.length + 1);
  offset += payload.user_agent.length + 1;
  payload_len += payload.user_agent.length + 1;
  memcpy(buf + offset, start_height, 4);
  offset += 4;
  memcpy(buf + offset, relay, 1);
  payload_len += 5;
  return payload_len;
}

void pack_message(unsigned char *buf, t_message message, size_t payload_len) {
  pack_header(buf, message.msg_header);
  memcpy(buf + 24, message.payload, payload_len);
}

static int unpack_header(unsigned char *buf, t_message_header *header) {
  int offset = 0;
  unsigned char magic[magic_length];
  unsigned char command[command_length];
  uint32_t size;
  unsigned char checksum[checksum_length];

  memcpy(magic, buf, magic_length);
  offset += magic_length;
  memcpy(command, buf + offset, command_length);
  offset += command_length;
  size = unpack_uint32(buf + offset);
  offset += 4;
  memcpy(checksum, buf + offset, checksum_length);
  offset += checksum_length;

  for (int i = 0; i < magic_length; i++)
    header->magic[i] = magic[i];
  for (int i = 0; i < command_length; i++)
    header->command[i] = command[i];
  header->size = size;
  for (int i = 0; i < checksum_length; i++)
    header->checksum[i] = checksum[i];

  return offset;
}

static size_t check_available_bytes(t_reader *reader) {
  if (reader->current_position >= reader->length)
    return 0;
  else
    return (reader->length - reader->current_position);
}

unsigned char *check_overflow(t_reader *reader, size_t bytes_to_write) {
  if ((check_available_bytes(reader) < bytes_to_write))
    return NULL;
  unsigned char *p = reader->buf + reader->current_position;
  reader->current_position += bytes_to_write;
  return p;
}

int unpack_message(t_message **msgs, size_t *msg_count,
                   const unsigned char *buf, const size_t buf_len) {
  t_message_header header;
  t_reader reader;
  unsigned char *buf_cursor;
  unsigned char payload_buf[buf_len];
  unsigned int array_limit = 5;

  reader.buf = (unsigned char *)buf;
  reader.current_position = 0;
  reader.length = buf_len;

  *msgs = malloc(array_limit * sizeof(t_message));
  if (*msgs == NULL)
    return 1;
  *msg_count = 0;

  while (reader.current_position < buf_len) {
    if (*msg_count >= array_limit) {
      array_limit *= 2;
      t_message *tmp = realloc(*msgs, array_limit * sizeof(t_message));
      if (tmp == NULL)
        return 1;
      *msgs = tmp;
    }
    // get header
    buf_cursor = check_overflow(&reader, 24);
    if (buf_cursor == NULL)
      return 1;
    unpack_header(buf_cursor, &header);
    (*msgs)[*msg_count].msg_header = header;

    // get payload
    buf_cursor = check_overflow(&reader, header.size);
    if (buf_cursor == NULL)
      return 1;
    memcpy(payload_buf, buf_cursor, header.size);
    (*msgs)[*msg_count].payload = payload_buf;

    *msg_count += 1;
  }
  if (*msg_count < array_limit) {
    t_message *tmp = realloc(*msgs, *msg_count * sizeof(t_message));
    if (tmp == NULL)
      return 1;
    *msgs = tmp;
  }
  return 0;
}

int test_unpack_message(unsigned char *buf, size_t buf_len) {
  t_message *msgs;
  size_t msg_count;
  int n = 0;

  unpack_message(&msgs, &msg_count, buf, buf_len);
  while ((size_t)n < msg_count) {
    printf("magic: ");
    for (int i = 0; i < magic_length; i++)
      printf("%02hhx ", msgs[n].msg_header.magic[i]);
    printf("\ncommand: ");
    for (int i = 0; i < command_length; i++)
      printf("%c ", msgs[n].msg_header.command[i]);
    printf("\nsize: %d\n", msgs[n].msg_header.size);
    printf("checksum: ");
    for (int i = 0; i < checksum_length; i++)
      printf("%02hhx ", msgs[n].msg_header.checksum[i]);
    printf("\n");
    n++;
  }
  // payload

  return 0;
}

int get_peer_ip(unsigned char *buf, int fd) {
  // Writes peer ip to a buffer (already serialized)
  struct sockaddr_storage addr;
  socklen_t len = sizeof(addr);

  if (getpeername(fd, (struct sockaddr *)&addr, &len) == 0) {
    if (addr.ss_family == AF_INET) {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;

      memset(buf, 0x00, 10);
      buf[10] = 0xFF;
      buf[11] = 0xFF;
      memcpy(buf + 12, &s->sin_addr, 4);
    } else {
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      memcpy(buf, &s->sin6_addr, 16); // UNTESTED - MAY NOT WORK
    }
  } else {
    return 1;
  }
  return 0;
}

// void pack_addr(unsigned char *buf, t_addr_payload)
