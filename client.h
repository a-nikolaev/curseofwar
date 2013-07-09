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
#ifndef _CLIENT_H
#define _CLIENT_H

#include "network.h"
#include "messaging.h"
#include "state.h"

int client_process_input (struct state *st, struct ui *ui, char c, int sfd, struct addrinfo *srv_addr);
int client_process_msg_s_state (struct state *st, struct msg_s_data *msg);

int client_receive_msg_s (int sfd, struct state *st);

void send_msg_c (int sfd, struct addrinfo *srv_addr, uint8_t msg, uint8_t i, uint8_t j, uint8_t info);

#endif
