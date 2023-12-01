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
#ifndef _MAIN_COMMON_H
#define _MAIN_COMMON_H

#include "state.h"

#define DEF_SERVER_ADDR "127.0.0.1"
#define DEF_SERVER_PORT "19140"
#define DEF_CLIENT_PORT "19150"

/* print help message */
void print_help();

/*
  returns:
    player won  1
    lost       -1
    otherwise   0
 */
int win_or_lose(struct state *st);

int game_slowdown (int speed);

int get_options(int argc, char *argv[], struct basic_options *op, struct multi_options *mop);

int singleplayer_process_input (struct state *st, struct ui *ui, int c);

#endif
