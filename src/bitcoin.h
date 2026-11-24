#ifndef BITCOIN_H
#define BITCOIN_H

#include "netbase.h"
#include "serialization.h"

int send_version(int fd, const int network, const unsigned char *peer_ip,
                 int my_port, unsigned char *buf);
int send_headless(int fd, const char *cmd, const int network,
                  unsigned char *buf);
int send_ping(int fd, const int network, unsigned char *buf);

int send_pong(int fd, const int network, uint64_t nonce, unsigned char *buf);

#endif
