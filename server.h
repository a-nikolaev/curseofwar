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
#ifndef _SERVER_H
#define _SERVER_H

/*
#include <sys/socket.h>
*/
#include <netinet/in.h>
#include "network.h"
#include "common.h"
#include "state.h"

#define MAX_CLIENT MAX_PLAYER

struct client_record {
  int pl;
  int id;
  char* name;
  struct sockaddr_storage sa;
};

/* multiplayer modes: 
    multi_lobby = waiting for clients, 
    multi_play = playing, 
 */
enum server_mode {server_mode_lobby, server_mode_play};

void server_send_msg_s_state (int sfd, struct client_record cl[], int cl_num, struct state *st);

int server_process_msg_c (uint8_t buf[MSG_BUF_SIZE], int nread, struct state *st, int pl);

int server_get_msg (uint8_t buf[MSG_BUF_SIZE], int nread);

#endif
