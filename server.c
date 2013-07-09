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
#include "server.h"
#include "messaging.h"

void server_send_msg_s_state (int sfd, struct client_record cl[], int cl_num, struct state *st) {
  static uint8_t buf[MSG_BUF_SIZE];
  static struct msg_s_data msg_data;

  int p, i, j;

  for (p=0; p<MAX_PLAYER; ++p) {
    msg_data.gold[p] = htonl( st->country[p].gold );
      
    msg_data.width = st->fg[p].width;
    msg_data.height =st->fg[p].height;
  }

  msg_data.time = htonl( st->time );

  msg_data.control = 0; /* specify for every client */
  msg_data.width = st->grid.width;
  msg_data.height = st->grid.height;
  for (i=0; i<MAX_WIDTH; ++i) {
    for (j=0; j<MAX_HEIGHT; ++j) {
      msg_data.tile[i][j] = st->grid.tiles[i][j].cl;

      int owner = st->grid.tiles[i][j].pl;
      msg_data.owner[i][j] = owner;
      msg_data.pop[i][j] = htons( st->grid.tiles[i][j].units[owner][citizen] );

      msg_data.flag[i][j] = 0;
      for (p=0; p<MAX_PLAYER; ++p){
        if (st->fg[p].flag[i][j])
          msg_data.flag[i][j] |= (1<<p);
      }
    }
  }


  buf[0] = MSG_S_STATE;
  int size = sizeof(msg_data);
  memcpy(buf+1, &msg_data, size); 
  int n_to_send = size+1;

  /* send */
  socklen_t peer_addr_len;
  for (i=0; i<cl_num; ++i) {
    peer_addr_len = sizeof (cl[i].sa);
    buf[1] = cl[i].pl;
    sendto(sfd, buf, n_to_send, 0, (struct sockaddr *) &cl[i].sa, peer_addr_len);
  }
}

void process_msg_c (struct state *st, int pl, uint8_t msg, struct msg_c_data *cm) {
  switch (msg) {
    case MSG_C_BUILD:
      build (&st->grid, &st->country[pl], pl, cm->i, cm->j);
      break;
    case MSG_C_FLAG_ON:
      add_flag (&st->grid, &st->fg[pl], cm->i, cm->j, FLAG_POWER);
      break;
    case MSG_C_FLAG_OFF:
      remove_flag (&st->grid, &st->fg[pl], cm->i, cm->j, FLAG_POWER);
      break;
    case MSG_C_FLAG_OFF_ALL:
      remove_flags_with_prob (&st->grid, &st->fg[pl], 1.0);
      break;
    case MSG_C_FLAG_OFF_HALF:
      remove_flags_with_prob (&st->grid, &st->fg[pl], 0.5);
      break;
    case MSG_C_IS_ALIVE:
      break;
    case MSG_C_PAUSE:
    case MSG_C_UNPAUSE:
    default:
      break;
  }
}

int server_process_msg_c (uint8_t buf[MSG_BUF_SIZE], int nread, struct state *st, int pl) {
  uint8_t msg;
  static struct msg_c_data msg_data;
  if (nread >= 1 + sizeof(struct msg_c_data)) {
    msg = buf[0];
    memcpy(&msg_data, buf+1, sizeof(struct msg_c_data));
    process_msg_c(st, pl, msg, &msg_data);
  }
  else {return -1;}

  return msg;
}

int server_get_msg (uint8_t buf[MSG_BUF_SIZE], int nread) {
  if (nread>=1) return buf[0]; else return 0;
}
