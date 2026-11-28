#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "netbase.h"
#include "utils.h"
#include <stdint.h>
enum { checksum_length = 4, command_length = 12, magic_length = 4 };

enum { main_net = 1, test_net = 2, test_net3 = 3, sig_net = 4, namecoin = 5 };

#define DEFAULT_PORT "8333"
#define SERVICES 1
#define VERSION 70015

// must update MY_SUBVERSION_SIZE after changing SUBVERSION
#define SUBVERSION "/boo:0.1.0/"
#define MY_SUBVERSION_SIZE 11

#define NONCE 0x0539a019ca550825ULL

#define VERSION_PAYLOAD_LEN 85 // ignores SUBVERSION
#define PING_PONG_PAYLOAD_LEN 8
#define MAX_MSG_SIZE (256 * 1024) // 256 KB

typedef struct s_message_header {
  unsigned char magic[magic_length];       // 4-byte network identifier
  unsigned char command[command_length];   // 12-byte command name
  uint32_t size;                           // size of message payload
  unsigned char checksum[checksum_length]; // used to verify message payload
} t_message_header;

typedef struct s_message {
  t_message_header msg_header;
  unsigned char *payload; // actual contents of the message; message body
} t_message;

// contains information about network addresses
typedef struct s_net_addr {
  uint32_t time;
  uint64_t services;
  unsigned char ip[16];
  uint16_t port;
} t_net_addr;

typedef struct s_var_int {
  uint8_t bytes_read;
  uint64_t value;
} t_var_int;

// variable length string (it is needed to store length
// of all variable fields for (de)serialization to work).
typedef struct s_var_str {
  t_var_int length;
  char string[257];
} t_var_str;

// netaddr DOES NOT have time
// verack has no payload
typedef struct s_version_payload {
  int32_t version;
  uint64_t services;
  int64_t timestamp;
  t_net_addr addr_recv;
  t_net_addr addr_from;
  uint64_t nonce;
  t_var_str user_agent;
  int32_t start_height;
  bool relay;
} t_version_payload;

// addr SENDS addresses to others
// getaddr REQUESTS addresses and HAS NO PAYLOAD.
// netaddr HAS time
typedef struct s_addr_payload {
  uint64_t addr_count; // NOT uint64_t; It should be CompactSize
  t_net_addr *addr_array;
} t_addr_payload;

typedef struct s_ping_pong_payload {
  uint64_t nonce; // ping: random nonce
                  // pong: should match received ping's nonce
} t_ping_pong_payload;

void message_init(t_message *msg, const char *cmd, const int network,
                  const unsigned char *payload, const size_t payload_len);
void version_payload_init(t_version_payload *payload,
                          const unsigned char *recv_ip, int my_port);
void ping_payload_init(t_ping_pong_payload *payload);
void pong_payload_init(t_ping_pong_payload *payload, uint64_t nonce);

#endif
