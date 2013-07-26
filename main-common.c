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

#include <stdio.h>
#include "common.h"
#include "state.h"

void print_help() {
  printf(
"                                 __                      \n"
"    ____                        /  ]                     \n"
"   / __ \\_ _ ___ ___ ___    __ _| |_  /\\      /\\___ ___  \n"
" _/ /  \\/ | |X _/ __/ __\\  /   \\   /  \\ \\ /\\ / /__ \\X _/ \n"
" \\ X    | | | | |__ | __X  | X || |    \\ V  V // _ | |   \n"
"  \\ \\__/\\ __X_| \\___/___/  \\___/| |     \\ /\\ / \\___X_|   \n"
"   \\____/                       |/       V  V            \n"
"\n"
"  Written by Alexey Nikolaev in 2013.\n"
"\n");
  
  printf("  Command line arguments:\n\n");
  printf(
    "-W width\n"
    "\tMap width (default is 21)\n\n"
    "-H height\n"
    "\tMap height (default is 21)\n\n"
    "-S [rhombus|rect|hex]\n"
    "\tMap shape (rectangle is default). Max number of countries N=4 for rhombus and rectangle, and N=6 for the hexagon.\n\n"
    "-l [2|3| ... N]\n"
    "\tSets L, the number of countries (default is N).\n\n"
    "-i [0|1|2|3|4]\n"
    "\tInequality between the countries (0 is the lowest, 4 in the highest).\n\n"
    "-q [1|2| ... L]\n"
    "\tChoose player's location by its quality (1 = the best available on the map, L = the worst). Only in the singleplayer mode.\n\n"
    "-r\n"
    "\tAbsolutely random initial conditions, overrides options -l, -i, and -q.\n\n"
    "-d [ee|e|n|h|hh]\n"
    "\tDifficulty level (AI) from the easiest to the hardest (default is normal).\n\n"
    "-s [p|sss|ss|s|n|f|ff|fff]\n"
    "\tGame speed from the slowest to the fastest (default is normal).\n\n"
    "-R seed\n"
    "\tSpecify a random seed (unsigned integer) for map generation.\n\n"
    "-T\n"
    "\tShow the timeline.\n\n"
    "-E [1|2| ... L]\n"
    "\tStart a server for not more than L clients.\n\n"
    "-e port\n"
    "\tServer's port (19140 is default).\n\n"
    "-C IP\n"
    "\tStart a client and connect to the provided server's IP-address.\n\n"
    "-c port\n"
    "\tClients's port (19150 is default).\n\n"
    "-h\n"
    "\tDisplay this help \n\n"
  );
}

int game_slowdown (int speed) {
  int slowdown = 20;
  switch (speed) {
    case sp_pause: slowdown = 1; break;
    case sp_slowest: slowdown = 160; break;
    case sp_slower: slowdown = 80; break;
    case sp_slow: slowdown = 40; break;
    case sp_normal: slowdown = 20; break;
    case sp_fast: slowdown = 10; break;
    case sp_faster: slowdown = 5; break;
    case sp_fastest: slowdown = 2; break;
  }
  return slowdown;
}


int singleplayer_process_input (struct state *st, struct ui *ui, char c) {
  int cursi = ui->cursor.i;
  int cursj = ui->cursor.j;
  switch (c) {
    case 'q': case 'Q':
      return 1;
    case 'f':
      st->prev_speed = st->speed;
      st->speed = faster(st->speed);
      break;
    case 's':
      st->prev_speed = st->speed;
      st->speed = slower(st->speed);
      break;
    case 'p':
      if (st->speed == sp_pause)
        st->speed = st->prev_speed;
      else {
        st->prev_speed = st->speed;
        st->speed = sp_pause;
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
        add_flag (&st->grid, &st->fg[st->controlled], ui->cursor.i, ui->cursor.j, FLAG_POWER);
      else
        remove_flag (&st->grid, &st->fg[st->controlled], ui->cursor.i, ui->cursor.j, FLAG_POWER);
      break;
    case 'x':
      remove_flags_with_prob (&st->grid, &st->fg[st->controlled], 1.0);
      break;
    case 'c':
      remove_flags_with_prob (&st->grid, &st->fg[st->controlled], 0.5);
      break;
    case 'r':
    case 'v':
      build (&st->grid, &st->country[st->controlled], st->controlled, ui->cursor.i, ui->cursor.j);
      break;
    
    case ESCAPE:
    case 91:
      break;
  }
  
  cursi = IN_SEGMENT(cursi, 0, st->grid.width-1);
  cursj = IN_SEGMENT(cursj, 0, st->grid.height-1);
  if ( is_visible(st->grid.tiles[cursi][cursj].cl) ) {
    ui->cursor.i = cursi;
    ui->cursor.j = cursj;
  }

  return 0;
}
