#include "bitcoin.h"
#include "protocol.h"
#include "serialization.h"

// TODO: VERSION_PAYLOAD_LEN for the serialized_payload and message is messed up
// because of the user_agent. Gotta fix the user_agent length problem so
// serialized buffers here hold true size, without stack smashing.
int send_version(int fd, const int network, const unsigned char *peer_ip,
                 int my_port, unsigned char *buf) {
  t_version_payload version_payload;
  t_message message;
  unsigned char serialized_payload[VERSION_PAYLOAD_LEN];
  unsigned char serialized_message[VERSION_PAYLOAD_LEN + 24];
  ssize_t currently_sent = 0;
  size_t bytes_sent = 0;

  version_payload_init(&version_payload, peer_ip, my_port);

  pack_version_payload(serialized_payload, version_payload);
  message_init(&message, "version", network, serialized_payload,
               VERSION_PAYLOAD_LEN);
  pack_message(serialized_message, message, VERSION_PAYLOAD_LEN);
  while (bytes_sent < VERSION_PAYLOAD_LEN + 24) {
    currently_sent = send(fd, serialized_message, VERSION_PAYLOAD_LEN + 24, 0);
    if (currently_sent < 0)
      return -1;
    bytes_sent += currently_sent;
  }
  if (buf != NULL)
    memcpy(buf, serialized_message, VERSION_PAYLOAD_LEN + 24);

  return 0;
}

// Works for any headless message, like verack and getaddr
int send_headless(int fd, const char *cmd, const int network,
                  unsigned char *buf) {
  t_message msg;
  unsigned char serialized_msg[24];
  ssize_t currently_sent = 0;
  size_t bytes_sent = 0;

  message_init(&msg, cmd, network, NULL, 0);
  pack_message(serialized_msg, msg, 0);
  while (bytes_sent < 24) {
    currently_sent = send(fd, serialized_msg, 24, 0);
    if (currently_sent < 0)
      return -1;
    bytes_sent += currently_sent;
  }
  if (buf != NULL)
    memcpy(buf, serialized_msg, 24);

  return 0;
}

int send_ping(int fd, const int network, unsigned char *buf) {
  t_ping_pong_payload ping_payload;
  t_message message;
  unsigned char serialized_payload[PING_PONG_PAYLOAD_LEN];
  unsigned char serialized_message[PING_PONG_PAYLOAD_LEN + 24];
  ssize_t currently_sent = 0;
  size_t bytes_sent = 0;

  ping_payload_init(&ping_payload);
  pack_ping_pong_payload(serialized_payload, ping_payload);
  message_init(&message, "ping", network, serialized_payload,
               PING_PONG_PAYLOAD_LEN);
  pack_message(serialized_message, message, PING_PONG_PAYLOAD_LEN);
  while (bytes_sent < PING_PONG_PAYLOAD_LEN + 24) {
    currently_sent =
        send(fd, serialized_message, PING_PONG_PAYLOAD_LEN + 24, 0);
    if (currently_sent < 0)
      return -1;
    bytes_sent += currently_sent;
  }
  if (buf != NULL)
    memcpy(buf, serialized_message, PING_PONG_PAYLOAD_LEN + 24);

  return 0;
}

int send_pong(int fd, const int network, uint64_t nonce, unsigned char *buf) {
  t_ping_pong_payload pong_payload;
  t_message message;
  unsigned char serialized_payload[PING_PONG_PAYLOAD_LEN];
  unsigned char serialized_message[PING_PONG_PAYLOAD_LEN + 24];
  ssize_t currently_sent = 0;
  size_t bytes_sent = 0;

  pong_payload_init(&pong_payload, nonce);
  pack_ping_pong_payload(serialized_payload, pong_payload);
  message_init(&message, "pong", network, serialized_payload,
               PING_PONG_PAYLOAD_LEN);
  pack_message(serialized_message, message, PING_PONG_PAYLOAD_LEN);
  while (bytes_sent < PING_PONG_PAYLOAD_LEN + 24) {
    currently_sent =
        send(fd, serialized_message, PING_PONG_PAYLOAD_LEN + 24, 0);
    if (currently_sent < 0)
      return -1;
    bytes_sent += currently_sent;
  }
  if (buf != NULL)
    memcpy(buf, serialized_message, PING_PONG_PAYLOAD_LEN + 24);

  return 0;
}
