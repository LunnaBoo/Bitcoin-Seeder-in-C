#include "bitcoin.h"


void send_version(int fd, const int network, const unsigned char *peer_ip,
                  unsigned char *buf) {
  t_version_payload version_payload;
  t_message message;
  unsigned char serialized_payload[97];
  unsigned char serialized_message[121];
  size_t payload_len;

  version_payload_init(&version_payload, peer_ip);
  payload_len = pack_version_payload(serialized_payload, version_payload);
  message_init(&message, "version", network, serialized_payload, payload_len);
  pack_message(serialized_message, message, payload_len);
  send(fd, serialized_message, 121, 0);
  if (strlen((char *)buf) > 0)
    memcpy(buf, serialized_message, 121);
}


// void send_verack();
