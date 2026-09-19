#include "debug.h"
#include "serialization.h"


bool test_pack_int16(void) {
  int16_t i = INT16_MAX;
  unsigned char buf[2];
  unsigned char *verify;

  verify = check_bytes(&i);
  pack_int16(buf, (uint16_t)i);
  for (int n = 0; n < 2; n++)
    if (buf[n] != verify[n]) {
      printf("test_pack_int16: FAILED\n");
      return false;
    }
  printf("test_pack_int16: PASSED\n");
  return true;
}


bool test_pack_int32(void) {
  int32_t i = INT32_MAX;
  unsigned char buf[4];
  unsigned char *verify;

  verify = check_bytes(&i);
  pack_int32(buf, (uint32_t)i);
  for (int n = 0; n < 4; n++)
    if (buf[n] != verify[n]) {
      printf("test_pack_int32: FAILED\n");
      return false;
    }
  printf("test_pack_int32: PASSED\n");
  return true;
}


bool test_pack_int64(void) {
  int64_t i = INT64_MAX;
  unsigned char buf[8];
  unsigned char *verify;

  verify = check_bytes(&i);
  pack_int64(buf, (uint64_t)i);
  for (int n = 0; n < 8; n++)
    if (buf[n] != verify[n]) {
      printf("test_pack_int64: FAILED\n");
      return false;
    }
  printf("test_pack_int64: PASSED\n");
  return true;
}


bool test_unpack_int16(void) {
  int16_t i = INT16_MAX;
  int16_t i2 = INT16_MIN;
  int16_t n;
  int16_t n2;
  unsigned char buf[2];
  unsigned char buf2[2];

  pack_int16(buf, (uint16_t)i);
  pack_int16(buf2, (uint16_t)i2);
  n = unpack_int16(buf);
  n2 = unpack_int16(buf2);
  if ((n == i) && (n2 == i2)) {
    printf("test_unpack_int16: PASSED\n");
    return true;
  }
  printf("test_unpack_int16: FAILED\n");
  return false;
}


bool test_unpack_uint16(void) {
  uint16_t i = UINT16_MAX;
  uint16_t n;
  unsigned char buf[2];

  pack_int16(buf, i);
  n = unpack_uint16(buf);
  if (n == i) {
    printf("test_unpack_uint16: PASSED\n");
    return true;
  }
  printf("test_unpack_uint16: FAILED\n");
  return false;
}


bool test_unpack_int32(void) {
  int32_t i = INT32_MAX;
  int32_t i2 = INT32_MIN;
  int32_t n;
  int32_t n2;
  unsigned char buf[4];
  unsigned char buf2[4];

  pack_int32(buf, (uint32_t)i);
  pack_int32(buf2, (uint32_t)i2);
  n = unpack_int32(buf);
  n2 = unpack_int32(buf2);
  if ((n == i) && (n2 == i2)) {
    printf("test_unpack_int32: PASSED\n");
    return true;
  }
  printf("test_unpack_int32: FAILED\n");
  return false;
}


bool test_unpack_uint32(void) {
  uint32_t i = UINT32_MAX;
  uint32_t n;
  unsigned char buf[4];

  pack_int32(buf, i);
  n = unpack_uint32(buf);
  if (n == i) {
    printf("test_unpack_uint32: PASSED\n");
    return true;
  }
  printf("test_unpack_uint32: FAILED\n");
  return false;
}


bool test_unpack_int64(void) {
  int64_t i = INT64_MAX;
  int64_t i2 = INT64_MIN;
  int64_t n;
  int64_t n2;
  unsigned char buf[8];
  unsigned char buf2[8];

  pack_int64(buf, (uint64_t)i);
  pack_int64(buf2, (uint64_t)i2);
  n = unpack_int64(buf);
  n2 = unpack_int64(buf2);
  if ((n == i) && (n2 == i2)) {
    printf("test_unpack_int64: PASSED\n");
    return true;
  }
  printf("test_unpack_int64: FAILED\n");
  return false;
}


bool test_unpack_uint64(void) {
  uint64_t i = UINT64_MAX;
  uint64_t n;
  unsigned char buf[8];

  pack_int64(buf, i);
  n = unpack_uint64(buf);
  if (n == i) {
    printf("test_unpack_uint64: PASSED\n");
    return true;
  }
  printf("test_unpack_uint64: FAILED\n");
  return false;
}


int main(void) {
  printf("PACK:\n");
  test_pack_int16();
  test_pack_int32();
  test_pack_int64();
  printf("UNPACK:\n");
  test_unpack_int16();
  test_unpack_uint16();
  test_unpack_int32();
  test_unpack_uint32();
  test_unpack_int64();
  test_unpack_uint64();
}
