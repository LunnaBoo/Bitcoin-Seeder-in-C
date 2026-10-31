#include "protocol.h"
#include "serialization.h"
#include <inttypes.h>

// checks if ping nonce is correctly serialized
// should print 376508076491343909 (in case the nonce is still the hardcoded
// one)
void test_ping_serialization() {
  unsigned char serialized_ping[32];
  unsigned char serialized_ping_payload[8];
  t_ping_pong_payload ping_payload;
  t_message ping;
  t_message *msgs;
  size_t msg_count;

  send_ping(1, main_net, serialized_ping);
  unpack_message(&msgs, &msg_count, serialized_ping, 32);
  unpack_ping_pong_payload(&ping_payload, msgs[0].payload, 8);
  printf("%" PRIu64 "\n", ping_payload.nonce);
}
