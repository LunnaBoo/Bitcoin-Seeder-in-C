#ifndef BITCOIN_H
#define BITCOIN_H


#include "netbase.h"
#include "serialization.h"

void send_version(int fd, const int network, const unsigned char *peer_ip,
                  unsigned char *buf);

#endif
