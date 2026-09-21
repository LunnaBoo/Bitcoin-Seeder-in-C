#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "netbase.h"
#include "utils.h"
enum { checksum_length = 4, command_length = 12, magic_length = 4 };

enum { main_net = 1, test_net = 2, test_net3 = 3, sig_net = 4, namecoin = 5 };

#define PORT 8333
#define SERVICES 1
#define VERSION 70015
#define USER_AGENT "/boo:0.1.0/"
#define NONCE 0x0539a019ca550825ULL

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

// variable length string (it is needed to store length
// of all variable fields for (de)serialization to work).
typedef struct s_var_str {
  uint8_t length; // technically it should be varint type
  char *string;
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
  uint8_t count; // should be varint
  t_net_addr *addr_array;
} t_addr_payload;


void message_init(t_message *msg, const char *cmd, const int network,
                  const unsigned char *payload, const size_t payload_len);
void version_payload_init(t_version_payload *payload,
                          const unsigned char *recv_ip);

#endif
