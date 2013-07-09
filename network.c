#include "network.h"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* returns port number */
in_port_t get_in_port(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return (((struct sockaddr_in*)sa)->sin_port);
  }
  return (((struct sockaddr_in6*)sa)->sin6_port);
}

#define SIZE6 16
/* Equality of two arrays of size SIZE6 (two IPv6 addresses) */
int eq_6_addr (uint8_t s1[SIZE6], uint8_t s2[SIZE6]) {
  int i;
  for(i=0; i<SIZE6; ++i) {
    if (s1[i] != s2[i]) return 0;
  }
  return 1;
}

/* Equality of two addresses (both IP number and port number) */
int sa_match (struct sockaddr_storage *sa1, struct sockaddr_storage *sa2) {
  struct sockaddr *s1 = (struct sockaddr*) sa1;
  struct sockaddr *s2 = (struct sockaddr*) sa2;
  return (
      /* IPv4 equality */
      ( s1->sa_family == AF_INET && s2->sa_family == AF_INET &&
        ((struct sockaddr_in*)s1)->sin_addr.s_addr == ((struct sockaddr_in*)s2)->sin_addr.s_addr ) 
      ||
      /* IPv6 equality */
      ( s1->sa_family == AF_INET6 && s2->sa_family == AF_INET6 &&
        eq_6_addr (
          ((struct sockaddr_in6*)s1)->sin6_addr.s6_addr, 
          ((struct sockaddr_in6*)s2)->sin6_addr.s6_addr ) )
    )
    &&
    ( get_in_port(s1) == get_in_port(s2) );
}


/* Initialize server socket */
int server_init (int *p_sfd, char*str_port) { 
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(NULL, str_port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
  *  Try each address until we successfully bind(2).
  *  If socket(2) (or bind(2)) fails, we (close the socket
  *  and) try the next address. */

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    *p_sfd = socket(rp->ai_family, rp->ai_socktype,
    rp->ai_protocol);
    if (*p_sfd == -1)
      continue;

    if (bind(*p_sfd, rp->ai_addr, rp->ai_addrlen) == 0)
    break;                  /* Success */

    close(*p_sfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not bind\n");
    return -1;
  }

  freeaddrinfo(result);           /* No longer needed */
  return 0;
}

/* initialize client socket, and fill struct addrinfo *srv (split into two functions!) */
int client_init_session (int *p_sfd, char* str_my_port, struct addrinfo *srv, char *str_server_addr, char *str_server_port) { 
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(NULL, str_my_port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* getaddrinfo() returns a list of address structures.
  *  Try each address until we successfully bind(2).
  *  If socket(2) (or bind(2)) fails, we (close the socket
  *  and) try the next address. */

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    *p_sfd = socket(rp->ai_family, rp->ai_socktype,
    rp->ai_protocol);
    if (*p_sfd == -1)
      continue;

    if (bind(*p_sfd, rp->ai_addr, rp->ai_addrlen) == 0)
    break;                  /* Success */

    close(*p_sfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not bind\n");
    return -1;
  }

  freeaddrinfo(result);           /* No longer needed */


  /* Second half */

  /* get info about the server */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  s = getaddrinfo(str_server_addr, str_server_port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }
  /*
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    int tmp_df = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (tmp_df == -1) continue;
    close(tmp_df);
  }
  */
  rp = result;

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not connect\n");
    return -1;
  }
  *srv = *rp;

  return 0;
}


