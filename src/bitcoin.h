#ifndef BITCOIN_H
#define BITCOIN_H

#include "netbase.h"
#include "serialization.h"

void send_version(int fd, const int network, const unsigned char *peer_ip,
                  unsigned char *buf);
void send_headless(int fd, const char *cmd, const int network,
                   unsigned char *buf);
void send_ping(int fd, const int network, unsigned char *buf);

void send_pong(int fd, const int network, uint64_t nonce, unsigned char *buf);

#endif
