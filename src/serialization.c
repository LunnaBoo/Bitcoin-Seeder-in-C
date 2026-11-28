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

/*
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
*/

static uint16_t unpack_uint16(unsigned char *buf) {
  return ((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
  // return ((uint16_t)buf[1] << 8) | buf[0];
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

int pack_var_int(unsigned char *buf, t_var_int var_int) {
  uint64_t value = var_int.value;

  if (value < 253) {
    buf[0] = value;

    return 1;
  } else if (value >= 253 && value <= 65535) {
    *(buf++) = 0xFD;
    *(buf++) = value;
    *(buf++) = value >> 8;

    return 3;
  } else if (value > 65535 && value <= UINT32_MAX) {
    *(buf++) = 0xFE;
    *(buf++) = value;
    *(buf++) = value >> 8;
    *(buf++) = value >> 16;
    *(buf++) = value >> 24;

    return 5;
  } else {
    *(buf++) = 0xFF;
    *(buf++) = value;
    *(buf++) = value >> 8;
    *(buf++) = value >> 16;
    *(buf++) = value >> 24;
    *(buf++) = value >> 32;
    *(buf++) = value >> 40;
    *(buf++) = value >> 48;
    *(buf++) = value >> 56;

    return 9;
  }
}

int pack_user_agent(unsigned char *buf, t_var_str var_str) {
  size_t i;
  size_t byte_count;

  i = 0;
  byte_count = pack_var_int(buf, var_str.length);
  if (byte_count == 0)
    return 1;
  buf += byte_count;
  while (i < var_str.length.value) {
    buf[i] = (unsigned char)var_str.string[i];
    byte_count++;
    i++;
  }
  return byte_count;
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
  unsigned char user_agent[MY_SUBVERSION_SIZE];
  size_t user_agent_bytes;
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
  user_agent_bytes = pack_user_agent(user_agent, payload.user_agent);
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

  memcpy(buf + offset, user_agent, user_agent_bytes);
  offset += user_agent_bytes;
  payload_len += user_agent_bytes;

  memcpy(buf + offset, start_height, 4);
  offset += 4;
  memcpy(buf + offset, relay, 1);
  payload_len += 5;
  return payload_len;
}

void pack_ping_pong_payload(unsigned char *buf, t_ping_pong_payload payload) {
  pack_int64(buf, payload.nonce);
}

void pack_message(unsigned char *buf, t_message message, size_t payload_len) {
  pack_header(buf, message.msg_header);
  if (payload_len > 0)
    memcpy(buf + 24, message.payload, payload_len);
}
static int unpack_header(unsigned char *buf, t_message_header *header) {
  int offset = 0;

  memcpy(header->magic, buf, magic_length);
  offset += magic_length;
  memcpy(header->command, buf + offset, command_length);
  offset += command_length;
  header->size = unpack_uint32(buf + offset);
  offset += 4;
  memcpy(header->checksum, buf + offset, checksum_length);
  offset += checksum_length;

  return offset;
}

static size_t check_available_bytes(t_reader *reader) {
  if (reader->current_position >= reader->length)
    return 0;
  else
    return (reader->length - reader->current_position);
}

unsigned char *check_overflow(t_reader *reader, size_t bytes_to_write) {
  if (bytes_to_write <= 0)
    return NULL;
  if ((check_available_bytes(reader) < bytes_to_write))
    return NULL;
  unsigned char *p = reader->buf + reader->current_position;
  reader->current_position += bytes_to_write;
  return p;
}

// Designed to parse multiple messages at a time, which turned out to be
// pointless. It makes a lot more sense to get 1 message at a time, but I had no
// idea it was possible and thought I was supposed to read the whole recv()
// buffer at once. Now I see that implies having a 32mb buffer...
int unpack_message(t_message *msg, unsigned char *buf, const size_t buf_len) {
  t_message_header header;
  t_reader reader;
  unsigned char *buf_cursor;

  reader.buf = (unsigned char *)buf;
  reader.current_position = 0;
  reader.length = buf_len;

  if (buf_len < 24)
    return 1;

  unpack_header(buf, &header);
  reader.current_position += 24;
  msg->msg_header = header;

  if (header.size > MAX_MSG_SIZE)
    return 1;

  if (header.size > 0) {
    unsigned char payload_buf[header.size];

    buf_cursor = check_overflow(&reader, header.size);
    if (!buf_cursor)
      return 1;
    memcpy(payload_buf, buf_cursor, header.size);
    msg->payload = payload_buf;
  }

  return 0;
}

int unpack_ping_pong_payload(t_ping_pong_payload *payload,
                             const unsigned char *buf, const size_t buf_len) {
  t_reader reader;

  reader.buf = (unsigned char *)buf;
  reader.current_position = 0;
  reader.length = buf_len;

  if (check_overflow(&reader, buf_len) == NULL)
    return 1;
  payload->nonce = unpack_uint64((unsigned char *)buf);

  return 0;
}

static void unpack_net_addr(t_net_addr *net_addr, unsigned char *buf,
                            bool has_time) {
  unsigned int offset = 0;
  if (has_time) {
    net_addr->time = unpack_uint32(buf);
    offset += 4;
  }
  net_addr->services = unpack_uint64(buf + offset);
  offset += 8;
  memcpy(net_addr->ip, buf + offset, 16);
  offset += 16;
  net_addr->port = unpack_uint16(buf + offset);
}

static void unpack_var_int(unsigned char *buf, size_t buf_len,
                           t_var_int *var_int) {
  uint8_t two_bytes;
  uint8_t four_bytes;
  uint8_t eight_bytes;
  uint8_t first_byte;

  two_bytes = 0xFD;
  four_bytes = 0xFE;
  eight_bytes = 0xFF;

  if (buf_len < 1) {
    var_int->bytes_read = 0;
    var_int->value = 0;
    return;
  }
  first_byte = buf[0];
  if (first_byte < two_bytes && buf_len >= 1) {
    var_int->bytes_read = 1;
    var_int->value = first_byte;
  } else if (first_byte == two_bytes && buf_len >= 3) {
    var_int->bytes_read = 3;
    var_int->value = buf[1] | ((uint16_t)buf[2] << 8);
  } else if (first_byte == four_bytes && buf_len >= 5) {
    var_int->bytes_read = 5;
    var_int->value =
        buf[1] | ((uint32_t)buf[2] << 8 | ((uint32_t)buf[3] << 16) |
                  ((uint32_t)buf[4] << 24));
  } else if (first_byte == eight_bytes && buf_len >= 9) {
    var_int->bytes_read = 9;
    var_int->value =
        buf[1] | ((uint64_t)buf[2] << 8 | ((uint64_t)buf[3] << 16) |
                  ((uint64_t)buf[4] << 24) | ((uint64_t)buf[5] << 32) |
                  ((uint64_t)buf[6] << 40) | ((uint64_t)buf[7] << 48) |
                  ((uint64_t)buf[8] << 56));
  } else {
    var_int->bytes_read = 0;
    var_int->value = 0;
  }

  return;
}

static int unpack_var_str(t_var_str *var_str, unsigned char *buf,
                          size_t buf_len) {
  t_var_int var_int;
  size_t offset;
  size_t i;

  unpack_var_int(buf, buf_len, &var_int);
  offset = var_int.bytes_read;

  if (var_int.value == 0)
    return 0;
  if (var_int.value > 256)
    return 1;
  if (var_int.value + offset > buf_len)
    return 1;

  for (i = 0; i < var_int.value; i++) {
    var_str->string[i] = buf[i + offset];
  }
  var_str->string[i] = '\0';
  var_str->length = var_int;

  return 0;
}

// helper
static void apply_full_version_fields(t_version_payload *payload,
                                      int32_t version, uint64_t services,
                                      int64_t timestamp, t_net_addr addr_recv,
                                      t_net_addr addr_from, uint64_t nonce,
                                      t_var_str user_agent,
                                      int32_t start_height, bool relay) {
  payload->version = version;
  payload->services = services;
  payload->timestamp = timestamp;
  payload->addr_recv = addr_recv;
  payload->addr_from = addr_from;
  payload->nonce = nonce;
  payload->user_agent = user_agent;
  payload->start_height = start_height;
  payload->relay = relay;
}

// helper
static void apply_partial_version_fields(t_version_payload *payload,
                                         int32_t version, uint64_t services,
                                         int64_t timestamp,
                                         t_net_addr addr_recv) {
  payload->version = version;
  payload->services = services;
  payload->timestamp = timestamp;
  payload->addr_recv = addr_recv;
  payload->addr_from.port = 0;
  payload->addr_from.services = 0;
  payload->addr_from.time = 0;
  payload->nonce = 0;
  payload->user_agent.length.bytes_read = 0;
  payload->user_agent.length.value = 0;
  payload->start_height = -1;
  payload->relay = 0;
}

// TODO: TEST IT
int unpack_version_payload(t_version_payload *payload, unsigned char *buf,
                           const size_t buf_len) {
  t_reader reader;
  reader.buf = buf;
  reader.current_position = 0;
  reader.length = buf_len;
  unsigned int offset;
  unsigned char *buf_cursor;

  int32_t version;
  uint64_t services;
  int64_t timestamp;
  t_net_addr addr_recv;
  t_net_addr addr_from;
  uint64_t nonce;
  t_var_str user_agent;
  int32_t start_height;
  bool relay;

  if (buf_len < 46)
    return 1;

  offset = 0;
  version = unpack_int32(buf);
  offset += 4;
  services = unpack_uint64(buf + offset);
  offset += 8;
  timestamp = unpack_int64(buf + offset);
  offset += 8;
  unpack_net_addr(&addr_recv, buf + offset, 0);
  reader.current_position = 46;

  buf_cursor = check_overflow(&reader, 26);
  if (!buf_cursor) {
    apply_partial_version_fields(payload, version, services, timestamp,
                                 addr_recv);
    return 0;
  }
  unpack_net_addr(&addr_from, buf_cursor, false);

  buf_cursor = check_overflow(&reader, 8);
  if (!buf_cursor) {
    apply_partial_version_fields(payload, version, services, timestamp,
                                 addr_recv);
    return 0;
  }
  nonce = unpack_uint64(buf_cursor);

  buf_cursor = reader.buf + reader.current_position;
  unpack_var_int(buf_cursor, reader.length - reader.current_position,
                 &user_agent.length);
  if (user_agent.length.bytes_read == 0) {
    apply_partial_version_fields(payload, version, services, timestamp,
                                 addr_recv);
    return 0;
  }
  reader.current_position += user_agent.length.bytes_read;
  buf_cursor = check_overflow(&reader, user_agent.length.value);
  if (!buf_cursor) {
    apply_partial_version_fields(payload, version, services, timestamp,
                                 addr_recv);
    return 0;
  }
  unpack_var_str(&user_agent, buf_cursor,
                 reader.length -
                     (reader.current_position - user_agent.length.value));

  buf_cursor = check_overflow(&reader, 4);
  if (!buf_cursor) {
    apply_partial_version_fields(payload, version, services, timestamp,
                                 addr_recv);
    return 0;
  }
  start_height = unpack_int32(buf_cursor);

  buf_cursor = check_overflow(&reader, 1);
  if (buf_cursor && *buf_cursor != 0)
    relay = true;
  else if (buf_cursor && *buf_cursor == 0)
    relay = false;
  else {
    apply_partial_version_fields(payload, version, services, timestamp,
                                 addr_recv);
    return 0;
  }

  apply_full_version_fields(payload, version, services, timestamp, addr_recv,
                            addr_from, nonce, user_agent, start_height, relay);
  return 0;
}

int unpack_addr_payload(t_addr_payload *payload, unsigned char *buf,
                        const size_t buf_len) {
  t_reader reader;
  reader.buf = buf;
  reader.current_position = 0;
  reader.length = buf_len;
  unsigned char *buf_cursor;

  uint64_t addr_count;
  t_net_addr *addr_array;
  t_var_int var_int;

  if (buf_len < 1)
    return 1;

  unpack_var_int(buf, buf_len, &var_int);
  if (var_int.bytes_read == 0) {
    payload->addr_array = NULL;
    payload->addr_count = 0;
    return 1;
  }

  reader.current_position += var_int.bytes_read;

  addr_count = var_int.value;
  if (addr_count > MAX_ADDR_COUNT)
    return 1;
  if (addr_count == 0)
    return 1;

  size_t remaining = buf_len - var_int.bytes_read;
  if (addr_count > remaining / 30)
    return 1;

  addr_array = malloc(addr_count * sizeof(t_net_addr));
  if (!addr_array)
    return 1;
  for (size_t i = 0; i < addr_count; i++) {
    t_net_addr recv_addr;
    buf_cursor = check_overflow(&reader, 30);
    if (!buf_cursor) {
      free(addr_array);
      return 1;
    }
    unpack_net_addr(&recv_addr, buf_cursor, true);
    addr_array[i] = recv_addr;
  }

  payload->addr_array = addr_array;
  payload->addr_count = addr_count;

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

// TODO: Add validation method for the header and run it before trying to get
// the payload from the byte stream.
int full_read(int fd, t_message *msg) {
  unsigned char buf[MAX_MSG_SIZE + 24];
  uint32_t payload_size;
  size_t recv_bytes;
  ssize_t ret_value;

  ret_value = exact_read(fd, buf, 24);
  if (ret_value == -1)
    return 1;
  recv_bytes = ret_value;
  payload_size = unpack_uint32(buf + 16);
  if (payload_size > MAX_MSG_SIZE)
    return 1; // TODO: return -1 to signal bad peer
  if (payload_size > 0) {
    ret_value = exact_read(fd, buf + 24, payload_size);
    if (ret_value == -1)
      return 1;
    recv_bytes += ret_value;
  }
  if (unpack_message(msg, buf, recv_bytes) == 1)
    return 1;

  return recv_bytes;
}
