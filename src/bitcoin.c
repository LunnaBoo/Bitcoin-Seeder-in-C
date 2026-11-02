#include "bitcoin.h"
#include "protocol.h"
#include "serialization.h"

// Removed payload_len var and used VERSION_PAYLOAD_LEN everywhere instead
void send_version(int fd, const int network, const unsigned char *peer_ip,
                  unsigned char *buf) {
  t_version_payload version_payload;
  t_message message;
  unsigned char serialized_payload[VERSION_PAYLOAD_LEN];
  unsigned char serialized_message[VERSION_PAYLOAD_LEN + 24];

  version_payload_init(&version_payload, peer_ip);
  pack_version_payload(serialized_payload, version_payload);
  message_init(&message, "version", network, serialized_payload,
               VERSION_PAYLOAD_LEN);
  pack_message(serialized_message, message, VERSION_PAYLOAD_LEN);
  send(fd, serialized_message, VERSION_PAYLOAD_LEN + 24, 0);
  if (buf != NULL)
    memcpy(buf, serialized_message, VERSION_PAYLOAD_LEN + 24);
}

void send_verack(int fd, const int network, unsigned char *buf) {
  t_message msg;
  unsigned char serialized_msg[24];

  message_init(&msg, "verack", network, NULL, 0);
  pack_message(serialized_msg, msg, 0);
  send(fd, serialized_msg, 24, 0);
  if (buf != NULL)
    memcpy(buf, serialized_msg, 24);
}

void send_ping(int fd, const int network, unsigned char *buf) {
  t_ping_pong_payload ping_payload;
  t_message message;
  unsigned char serialized_payload[PING_PONG_PAYLOAD_LEN];
  unsigned char serialized_message[PING_PONG_PAYLOAD_LEN + 24];

  ping_payload_init(&ping_payload);
  pack_ping_pong_payload(serialized_payload, ping_payload);
  message_init(&message, "ping", network, serialized_payload,
               PING_PONG_PAYLOAD_LEN);
  pack_message(serialized_message, message, PING_PONG_PAYLOAD_LEN);
  send(fd, serialized_message, PING_PONG_PAYLOAD_LEN + 24, 0);
  if (buf != NULL)
    memcpy(buf, serialized_message, PING_PONG_PAYLOAD_LEN + 24);
}

void send_pong(int fd, const int network, uint64_t nonce, unsigned char *buf) {
  t_ping_pong_payload pong_payload;
  t_message message;
  unsigned char serialized_payload[PING_PONG_PAYLOAD_LEN];
  unsigned char serialized_message[PING_PONG_PAYLOAD_LEN + 24];

  pong_payload_init(&pong_payload, nonce);
  pack_ping_pong_payload(serialized_payload, pong_payload);
  message_init(&message, "pong", network, serialized_payload,
               PING_PONG_PAYLOAD_LEN);
  pack_message(serialized_message, message, PING_PONG_PAYLOAD_LEN);
  send(fd, serialized_message, PING_PONG_PAYLOAD_LEN + 24, 0);
  if (buf != NULL)
    memcpy(buf, serialized_message, PING_PONG_PAYLOAD_LEN + 24);
}
