#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "utils.h"
enum { checksum_length = 4, command_length = 12, magic_length = 4 };


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
  char ip[16];
  uint16_t port;
} t_net_addr;

// variable length string (it is needed to store length
// of all variable fields for (de)serialization to work).
typedef struct s_var_str {
  uint8_t length; // technically it should be varint type
  char *string;
} t_var_str;

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


#endif
