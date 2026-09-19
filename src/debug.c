#include "debug.h"


void print_addresses(struct addrinfo *res, char *hostname, int limit) {
  // prints all addresses found in a addrinfo linked list from a hostname
  int i = 0;
  struct addrinfo *p;

  printf("IP addresses for %s\n\n", hostname);
  // loop to get all IPs and print them
  for (p = res; p != NULL && i < limit; p = p->ai_next) {
    address_printer("", p);
    i++;
  }
  printf("\n");
}


void address_printer(char *message, struct addrinfo *p) {
  // prints ONE adddress found in a addrinfo struct
  void *addr;
  char *ipver;
  char ipstr[INET6_ADDRSTRLEN];
  struct sockaddr_in *ipv4;
  struct sockaddr_in6 *ipv6;

  // get the IP
  if (p->ai_family == AF_INET) // iPv4
  {
    ipv4 = (struct sockaddr_in *)p->ai_addr;
    addr = &(ipv4->sin_addr);
    ipver = "IPv4";
  } else {
    ipv6 = (struct sockaddr_in6 *)p->ai_addr;
    addr = &(ipv6->sin6_addr);
    ipver = "IPv6";
  }

  // print the IP
  inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
  if (message)
    printf("%s  %s:  %s\n", message, ipver, ipstr);
  else
    printf("  %s:  %s\n", ipver, ipstr);
}


unsigned char *print_bytes(void *value, unsigned int size) {
  unsigned char *p = (unsigned char *)value;

  for (unsigned int i = 0; i < size; i++)
    printf("%02x ", p[i]); // outputs the first byte of `foo`

  printf("\n");
  return p;
}


unsigned char *check_bytes(void *value) {
  unsigned char *p = (unsigned char *)value;
  return p;
}
