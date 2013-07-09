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
#ifndef _MESSAGING_H
#define _MESSAGING_H

#include <inttypes.h>
#include "common.h"

/* messages from from a client to a server */
#define MSG_C_CONNECT  1

#define MSG_C_BUILD    20
#define MSG_C_FLAG_ON  21
#define MSG_C_FLAG_OFF 22
#define MSG_C_FLAG_OFF_ALL 23
#define MSG_C_FLAG_OFF_HALF 24
#define MSG_C_IS_ALIVE 30
#define MSG_C_PAUSE    40
#define MSG_C_UNPAUSE  41

/* message from a server to a client */
#define MSG_S_CONN_ACCEPTED 5
#define MSG_S_CONN_REJECTED 6

#define MSG_S_STATE        10

struct msg_c_data {
  uint8_t i;
  uint8_t j;
  uint8_t info;
};

struct msg_s_data {
  uint8_t control;        /* player you control */
  uint8_t pause_request;
  uint32_t gold [MAX_PLAYER];
  uint32_t time;
  uint8_t width;
  uint8_t height;
  uint8_t flag [MAX_WIDTH][MAX_HEIGHT];
  uint8_t owner [MAX_WIDTH][MAX_HEIGHT];
  uint16_t pop [MAX_WIDTH][MAX_HEIGHT];
  uint8_t tile [MAX_WIDTH][MAX_HEIGHT];
};

#endif
