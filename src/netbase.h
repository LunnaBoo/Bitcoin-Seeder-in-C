#ifndef NETBASE_H
#define NETBASE_H

#define PORT "8333"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>      // getaddrinfo + addrinfo
#include <netinet/in.h> // sockaddr_in sockaddr_in6
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // close()


struct addrinfo *look_up_host(char *hostname);
void address_printer(char *message, struct addrinfo *p);
void print_addresses(struct addrinfo *res, char *hostname, int limit);
int connect_socket(struct addrinfo *res, int timeout);

#endif
