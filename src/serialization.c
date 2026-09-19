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

// Concatenate byte-by-byte of the value in little endian order

void pack_int16(unsigned char *buf, uint16_t i) {
  *(buf++) = i;
  *(buf++) = i >> 8;
}


void pack_int32(unsigned char *buf, uint32_t i) {
  *(buf++) = i;
  *(buf++) = i >> 8;
  *(buf++) = i >> 16;
  *(buf++) = i >> 24;
}


void pack_int64(unsigned char *buf, uint64_t i) {
  *(buf++) = i;
  *(buf++) = i >> 8;
  *(buf++) = i >> 16;
  *(buf++) = i >> 24;
  *(buf++) = i >> 32;
  *(buf++) = i >> 40;
  *(buf++) = i >> 48;
  *(buf++) = i >> 56;
}


int16_t unpack_int16(unsigned char *buf) {
  uint16_t i2;
  int i;

  i2 = ((uint16_t)buf[1] << 8) | buf[0];
  if (i2 <= 0x7fffu)
    i = i2;
  else
    i = -1 - (uint16_t)(0xffffu - i2);

  return i;
}


uint16_t unpack_uint16(unsigned char *buf) {
  return ((uint16_t)buf[1] << 8) | buf[0];
}


int32_t unpack_int32(unsigned char *buf) {
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


uint32_t unpack_uint32(unsigned char *buf) {
  return buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) |
         ((uint32_t)buf[3] << 24);
}


int64_t unpack_int64(unsigned char *buf) {
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


uint64_t unpack_uint64(unsigned char *buf) {
  return buf[0] | ((uint64_t)buf[1] << 8 | ((uint64_t)buf[2] << 16) |
                   ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 32) |
                   ((uint64_t)buf[5] << 40) | ((uint64_t)buf[6] << 48) |
                   ((uint64_t)buf[7] << 56));
}


void pack_var_str(unsigned char *buf, t_var_str str) {
  // pack length
  // pack char[i] while i < length
  int i = 0;

  buf[i] = str.length;

  while (i < str.length) {
    buf[i + 1] = (unsigned char)str.string[i];
    i++;
  }
}


void pack_port(unsigned char *buf, uint16_t port) {
  port = htons(port);
  memcpy(buf, &port, 2);
}


int pack_ip(unsigned char *buf, char *ip, bool is_ipv4) {
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


void pack_net_addr(unsigned char *buf, t_net_addr net_addr, bool is_ipv4,
                   bool has_time) {
  unsigned char time[4];
  unsigned char services[8];
  unsigned char ip[16];  // big endian
  unsigned char port[2]; // big endian

  pack_int16(time, net_addr.time);
  pack_int64(services, net_addr.services);
  pack_ip(ip, net_addr.ip, is_ipv4);
  pack_port(port, net_addr.port);

  if (has_time)
    memcpy(buf, time, 4);
  memcpy(buf, services, 8);
  memcpy(buf, ip, 16);
  memcpy(buf, port, 2);
}


void pack_header(unsigned char *buf, t_message_header header) {
  unsigned char size[4];

  pack_int32(size, header.size);

  memcpy(buf, header.magic, magic_length);
  memcpy(buf, header.command, command_length);
  memcpy(buf, size, 4);
  memcpy(buf, header.checksum, checksum_length);
}


void pack_version(unsigned char *buf, t_version_payload payload,
                  bool is_recv_ipv4, bool is_from_ipv4) {
  unsigned char version[4];
  unsigned char services[8];
  unsigned char timestamp[8];
  unsigned char addr_recv[26];
  unsigned char addr_from[26];
  unsigned char nonce[8];
  unsigned char user_agent[payload.user_agent.length + 1];
  unsigned char start_height[4];
  unsigned char relay[1];

  pack_int32(version, payload.version);
  pack_int64(services, payload.services);
  pack_int64(timestamp, payload.timestamp);
  pack_net_addr(addr_recv, payload.addr_recv, is_recv_ipv4, false);
  pack_net_addr(addr_from, payload.addr_from, is_from_ipv4, false);
  pack_int64(nonce, payload.nonce);
  pack_var_str(user_agent, payload.user_agent);
  pack_int32(start_height, payload.start_height);
  memcpy(relay, &payload.relay, 1);

  memcpy(buf, version, 4);
  memcpy(buf, services, 8);
  memcpy(buf, timestamp, 8);
  memcpy(buf, addr_recv, 26);
  memcpy(buf, addr_from, 26);
  memcpy(buf, nonce, 8);
  memcpy(buf, user_agent, payload.user_agent.length + 1);
  memcpy(buf, start_height, 4);
  memcpy(buf, relay, 1);
}
