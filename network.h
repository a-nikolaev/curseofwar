/******************************************************************************

  Curse of War -- Real Time Strategy Game for Linux.
  Copyright (C) 2013 Alexey Nikolaev.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
******************************************************************************/

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
