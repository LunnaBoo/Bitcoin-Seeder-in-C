#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>      // getaddrinfo + addrinfo
#include <netinet/in.h> // sockaddr_in sockaddr_in6
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // memset
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


struct addrinfo *look_up_host(char *hostname);
void address_printer(char *message, struct addrinfo *p);
void print_addresses(struct addrinfo *res, char *hostname, int limit);
int connect_socket(struct addrinfo *res, int timeout);


int main(void) {
  int socket_fd;
  char buffer[1000];
  struct addrinfo *res;
  char *hostname = "seed.bitcoin.sipa.be";

  res = look_up_host(hostname);
  if (res == NULL)
    return 1;

  print_addresses(res, hostname, 3);
  if ((socket_fd = connect_socket(res, 3)) == false) {
    printf("ERROR: connect_socket() failed.\n");
    return 1;
  } else {
    printf("Connection established.\n");
  }

  int byte_num = recv(socket_fd, buffer, sizeof buffer, 0);
  for (int i = 0; i < byte_num; i++)
    printf("%02x ", (unsigned char)buffer[i]);
  printf("\n");
  if (byte_num == 0)
    printf("Connection closed.");
  // printf("recv errno = %d (%s)\n", errno, strerror(errno));

  freeaddrinfo(res);
  return 0;
}


struct addrinfo *look_up_host(char *hostname) {
  // Does DNS look up; finds IPs from a hostname
  struct addrinfo hints; // actual struct
  struct addrinfo *res;  // pointer to list head
  struct addrinfo *p;    // pointer for list navigation
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints); // gotta make sure the struct's empty
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // service field MUST be port 8333
  status = getaddrinfo(hostname, "8333", &hints,
                       &res); // call getaddrinfo to fill addrinfo
  if (status != 0) {
    fprintf(stderr, "getaddrinfo: %s\n",
            gai_strerror(status)); // error handling
    return NULL;
  }
  return res;
}


int connect_socket(struct addrinfo *res, int timeout) {
  // iterates through addrinfo linked list and tries to connect to an IP
  int status;
  int socket_fd;
  struct addrinfo *p;


  for (p = res; p != NULL; p = p->ai_next) {
    if ((socket_fd =
             socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
      continue;

    // Make the socket non-blocking
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
      continue;

    // Try to connect
    if ((status = connect(socket_fd, p->ai_addr, p->ai_addrlen)) == 0)
      break;
    if (status == 0) {
      // fcntl to change the socket back to blocking
      fcntl(socket_fd, F_SETFL, flags & ~O_NONBLOCK);
      return socket_fd; // immediately connect
    } else {
      if (errno == EINPROGRESS)
        address_printer("Connecting to:", p);
      else {
        printf("Failed :<\n");
        close(socket_fd);
      }

      // use select() to monitor multiple scokets
      fd_set set;
      FD_ZERO(&set);
      FD_SET(socket_fd, &set);
      struct timeval tv = {timeout, 0};
      if (select(socket_fd + 1, NULL, &set, NULL, &tv) > 0) {
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &err, &len);

        if (err == 0) {
          // fcntl to change the socket back to blocking
          fcntl(socket_fd, F_SETFL, flags & ~O_NONBLOCK);
          return socket_fd;
          break;
        }
        close(socket_fd);
      }
    }
  }
  return -1;
}


void print_addresses(struct addrinfo *res, char *hostname, int limit) {
  // prints all addresses found in a addrinfo struct from a hostname
  int i = 0;
  struct addrinfo *p;
  char ipstr[INET6_ADDRSTRLEN];

  printf("IP addresses for %s\n\n", hostname);
  // loop to get all IPs and print them
  for (p = res; p != NULL && i < limit; p = p->ai_next) {
    address_printer("", p);
    i++;
  }
  printf("\n");
}


void address_printer(char *message, struct addrinfo *p) {
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
