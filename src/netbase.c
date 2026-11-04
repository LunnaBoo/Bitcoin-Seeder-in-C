#include "netbase.h"
#include "serialization.h"
#include "utils.h"
#include <sys/types.h>

#define ADDRINFO_PORT "8333"

struct addrinfo *look_up_host(char *hostname) {
  // Does DNS look up; finds IPs from a hostname
  struct addrinfo hints; // actual struct
  struct addrinfo *res;  // pointer to list head
  int status;

  memset(&hints, 0, sizeof hints); // gotta make sure the struct's empty
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // service field MUST be port 8333
  status = getaddrinfo(hostname, ADDRINFO_PORT, &hints,
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

int exact_read(int fd, unsigned char *buf, size_t bytes_to_read) {
  size_t bytes_read = 0;
  ssize_t current_read = 0;

  while (bytes_read < bytes_to_read) {
    current_read = recv(fd, buf + bytes_read, bytes_to_read - bytes_read, 0);
    if (current_read < 0)
      return -1;
    else if (current_read == 0)
      return 1;
    bytes_read += current_read;
  }

  return 0;
}
