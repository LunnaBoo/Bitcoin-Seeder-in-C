#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "protocol.h"
#include "utils.h"
#include <arpa/inet.h>


int pack_version_payload(unsigned char *buf, t_version_payload payload);
void pack_message(unsigned char *buf, t_message message, size_t payload_len);
int get_peer_ip(unsigned char *buf, int fd);


#endif
