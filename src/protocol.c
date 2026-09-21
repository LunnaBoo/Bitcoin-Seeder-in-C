#include "protocol.h"
#include <openssl/evp.h>
#include <time.h>


// implement message creation function / struct initialization
// leave actual operations (test node; send message; etc) to bitcoin.c/h)

static void write_magic(t_message_header *header, const int network);
static void dsha256(unsigned char *buf, const unsigned char *payload,
                    const size_t payload_len);

void message_init(t_message *msg, const char *cmd, int network,
                  const unsigned char *payload, const size_t payload_len) {
  t_message_header *header = &msg->msg_header;
  unsigned char payload_hash[256];

  write_magic(header, network); // magic
  memset(header->command, 0, command_length);
  memcpy(header->command, cmd, strlen(cmd)); // command
  header->size = payload_len;                // size
  dsha256(payload_hash, payload, payload_len);
  memcpy(header->checksum, payload_hash, 4); // checksum

  msg->payload = (unsigned char *)payload;
}

void version_payload_init(t_version_payload *payload,
                          const unsigned char *recv_ip) {
  t_net_addr *addr_recv = &payload->addr_recv;
  t_net_addr *addr_from = &payload->addr_from;

  payload->version = VERSION;
  payload->services = SERVICES;
  payload->timestamp = time(NULL);

  addr_recv->services = 0;
  for (int i = 0; i < 16; i++)
    addr_recv->ip[i] = recv_ip[i];
  addr_recv->port = PORT;

  addr_from->services = SERVICES;
  for (int i = 0; i < 16; i++)
    addr_from->ip[i] = 0;
  addr_from->port = PORT;

  payload->nonce = NONCE;
  payload->user_agent.string = USER_AGENT;
  payload->user_agent.length = strlen(USER_AGENT);
  payload->start_height = -1; // MUST CHANGE LATER
  payload->relay = true;
}


static void write_magic(t_message_header *header, const int network) {
  switch (network) {
  case 1:
    header->magic[0] = 0xF9;
    header->magic[1] = 0xBE;
    header->magic[2] = 0xB4;
    header->magic[3] = 0xD9;
    break;
  case 2:
    header->magic[0] = 0xFA;
    header->magic[1] = 0xBF;
    header->magic[2] = 0xB5;
    header->magic[3] = 0xDA;
    break;
  case 3:
    header->magic[0] = 0x0B;
    header->magic[1] = 0x11;
    header->magic[2] = 0x09;
    header->magic[3] = 0x07;
    break;
  case 4:
    header->magic[0] = 0x0A;
    header->magic[1] = 0x03;
    header->magic[2] = 0xCF;
    header->magic[3] = 0x40;
    break;
  case 5:
    header->magic[0] = 0xF9;
    header->magic[1] = 0xBE;
    header->magic[2] = 0xB4;
    header->magic[3] = 0xFE;
    break;
  }
}

static void dsha256(unsigned char *buf, const unsigned char *payload,
                    const size_t payload_len) {
  unsigned char hash1[32]; // 256 bits!
  size_t size1;
  size_t size2;

  EVP_Q_digest(NULL, "SHA256", NULL, payload, payload_len, hash1, &size1);
  EVP_Q_digest(NULL, "SHA256", NULL, hash1, size1, buf, &size2);
}
