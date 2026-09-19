#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "protocol.h"
#include <stdint.h>

void pack_int16(unsigned char *buf, uint16_t i);
void pack_int32(unsigned char *buf, uint32_t i);
void pack_int64(unsigned char *buf, uint64_t i);
int16_t unpack_int16(unsigned char *buf);
uint16_t unpack_uint16(unsigned char *buf);
int32_t unpack_int32(unsigned char *buf);
uint32_t unpack_uint32(unsigned char *buf);
int64_t unpack_int64(unsigned char *buf);
uint64_t unpack_uint64(unsigned char *buf);

#endif
