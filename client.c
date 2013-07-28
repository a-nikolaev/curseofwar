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
#include "client.h"
#include "state.h"

int client_init ();

int client_connect ();

int client_process_msg_s_state (struct state *st, struct msg_s_data *msg) {

  if (ntohl( msg->time ) <= st->time) return -1;

  int p, i, j;
  for (p=0; p<MAX_PLAYER; ++p) {
    st->country[p].gold = ntohl( msg->gold[p] );
      
    st->fg[p].width = msg->width;
    st->fg[p].height = msg->height;
  }

  st->time = ntohl( msg->time );

  st->controlled = msg->control;

  st->grid.width = msg->width;
  st->grid.height = msg->height;

  for (i=0; i<MAX_WIDTH; ++i) {
    for (j=0; j<MAX_HEIGHT; ++j) {
      st->grid.tiles[i][j].cl = msg->tile[i][j];

      for (p=0; p<MAX_PLAYER; ++p)
        st->grid.tiles[i][j].units[p][citizen] = 0;

      st->grid.tiles[i][j].pl = msg->owner[i][j];
      st->grid.tiles[i][j].units[msg->owner[i][j]][citizen] = ntohs( msg->pop[i][j] );

      for (p=0; p<MAX_PLAYER; ++p){
        st->fg[p].call[i][j] = 0;
        if ( (msg->flag[i][j] & (1<<p)) == 0)
          st->fg[p].flag[i][j] = 0;
        else
          st->fg[p].flag[i][j] = 1;
      }
    }
  }
  return 0;
}

int client_receive_msg_s (int sfd, struct state *st) {
  static uint8_t buf[MSG_BUF_SIZE];
  static struct msg_s_data msg_data;

  struct sockaddr_storage peer_addr; /* address you receive a message from */
  socklen_t peer_addr_len = sizeof(peer_addr);
  
  int nread = recvfrom(sfd, buf, MSG_BUF_SIZE-1, 0,
      (struct sockaddr *) &peer_addr, &peer_addr_len);
  if (nread == -1) return -1; /* Ignore failed request */
 
  uint8_t msg = 0;
  if (nread >= 1) {
    msg = buf[0];
    switch (msg) {
      case MSG_S_STATE:
        if (nread-1 >= sizeof(struct msg_s_data)) {
          memcpy(&msg_data, buf+1, sizeof(struct msg_s_data));
          client_process_msg_s_state (st, &msg_data);
        }
        else {return -1;}
        break;
    }
  }

  return msg;
}

int client_process_input (struct state *st, struct ui *ui, char c, int sfd, struct addrinfo *srv_addr) {
  int cursi = ui->cursor.i;
  int cursj = ui->cursor.j;

  switch (c) {
      case 'Q':
      case 'q': 
        return 1;                     /* quit program */
      /*    
      case 'f':
          st->prev_speed = st->speed;
          st->speed = faster(st->speed);
          break;
      case 's':
          st->prev_speed = st->speed;
          st->speed = slower(st->speed);
          break;
       */
      case 'p':
        if (st->speed == sp_pause)
          send_msg_c (sfd, srv_addr, MSG_C_PAUSE, 0, 0, 0);
        else {
          send_msg_c (sfd, srv_addr, MSG_C_UNPAUSE, 0, 0, 0);
        }
        break;
      case 'h': case K_LEFT:
        cursi--;
        break;
      case 'l': case K_RIGHT:
        cursi++;
        break;
      case 'k': case K_UP:
        cursj--;
        if (cursj % 2 == 1)
          cursi++;
        break;
      case 'j': case K_DOWN:
        cursj++;
        if (cursj % 2 == 0)
          cursi--;
        break;
      case ' ':
        if (st->fg[st->controlled].flag[ui->cursor.i][ui->cursor.j] == 0)
          send_msg_c (sfd, srv_addr, MSG_C_FLAG_ON, ui->cursor.i, ui->cursor.j, 0);
        else
          send_msg_c (sfd, srv_addr, MSG_C_FLAG_OFF, ui->cursor.i, ui->cursor.j, 0);
        break;
      case 'x':
        send_msg_c (sfd, srv_addr, MSG_C_FLAG_OFF_ALL, 0, 0, 0);
        break;
      case 'c':
        send_msg_c (sfd, srv_addr, MSG_C_FLAG_OFF_HALF, 0, 0, 0);
        break;
      case 'r':
      case 'v':
        send_msg_c (sfd, srv_addr, MSG_C_BUILD, ui->cursor.i, ui->cursor.j, 0);
        break;
      
      case ESCAPE:
      case 91:
        break;
      }


  adjust_cursor(st, ui, cursi, cursj);

  return 0; /* not finished */
}

void send_msg_c (int sfd, struct addrinfo *srv_addr, uint8_t msg, uint8_t i, uint8_t j, uint8_t info) {
  struct msg_c_data mcd = {i, j, info};
  static uint8_t buf[MSG_BUF_SIZE];
  buf[0] = msg;
  memcpy(buf+1, &mcd, sizeof(mcd));
  int nsent = sendto(sfd, buf, 1+sizeof(mcd), 0, srv_addr->ai_addr, srv_addr->ai_addrlen);
  if (nsent == -1) {
    perror("client: sendto");
  }
}
