#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "protocol.h"
#include "utils.h"
#include <arpa/inet.h>

void pack_int16(unsigned char *buf, uint16_t i);
void pack_int32(unsigned char *buf, uint32_t i);
void pack_int64(unsigned char *buf, uint64_t i);
void pack_var_str(unsigned char *buf, t_var_str str);
void pack_port(unsigned char *buf, uint16_t port);
int pack_ip(unsigned char *buf, char *ip, bool is_ipv4);
void pack_net_addr(unsigned char *buf, t_net_addr net_addr, bool is_ipv4,
                   bool has_time);
void pack_header(unsigned char *buf, t_message_header header);
void pack_version(unsigned char *buf, t_version_payload payload,
                  bool is_recv_ipv4, bool is_from_ipv4);
int16_t unpack_int16(unsigned char *buf);
uint16_t unpack_uint16(unsigned char *buf);
int32_t unpack_int32(unsigned char *buf);
uint32_t unpack_uint32(unsigned char *buf);
int64_t unpack_int64(unsigned char *buf);
uint64_t unpack_uint64(unsigned char *buf);

#endif
