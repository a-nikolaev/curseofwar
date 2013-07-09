#ifndef _NETWORK_H
#define _NETWORK_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MSG_BUF_SIZE 50000

/* get sockaddr, IPv4 or IPv6 (UGLY): */
void *get_in_addr(struct sockaddr *sa);

/* returns port number */
in_port_t get_in_port(struct sockaddr *sa);

/* Check if two sockaddr_storage structures match, 
    returns 1 if they do, or 0 otherwise */
int sa_match (struct sockaddr_storage *sa1, struct sockaddr_storage *sa2);

int server_init(int*, char*);
int client_init_session(int*, char*, struct addrinfo *p_addr, char*, char*);


#endif
