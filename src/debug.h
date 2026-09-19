#ifndef DEBUG_H
#define DEBUG_H

#include "netbase.h"
#include "utils.h"


void print_addresses(struct addrinfo *res, char *hostname, int limit);
void address_printer(char *message, struct addrinfo *p);
unsigned char *print_bytes(void *value, unsigned int size);
unsigned char *check_bytes(void *value);

#endif
