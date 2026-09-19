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

/*
unsigned char *pack_version(t_msg_version msg) {
  //
  //
  //
  //
  //
  return NULL;
}
*/
