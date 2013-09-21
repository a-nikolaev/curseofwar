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
#include <string.h>
#include "common.h"
#include "state.h"
#include "main-common.h"

#ifdef WIN32
# ifndef MINGW32
#  define NOGETOPT 1
# endif
#endif

#ifndef NOGETOPT // For now, we deactivate command line options parsing for windows versions
  #include <getopt.h>
#endif

#ifndef VERSION
# define VERSION ""
#endif

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
    "-v\n"
    "\tDisplay the version number\n\n"
    "-h\n"
    "\tDisplay this help \n\n"
  );
}

int win_or_lose(struct state *st) {
  int i, j, p;
  int pop[MAX_PLAYER];
  for (p=0; p<MAX_PLAYER; ++p) pop[p] = 0;

  for(i=0; i<st->grid.width; ++i){
    for(j=0; j<st->grid.height; ++j){
      if(is_inhabitable(st->grid.tiles[i][j].cl)) {
        for(p=0; p<MAX_PLAYER; ++p){
          pop[p] += st->grid.tiles[i][j].units[p][citizen];
        }
      }
    }
  }

  int win = 1;
  int lose = 0;
  int best = 0;
  for(p=0; p<MAX_PLAYER; ++p){
    if(pop[best] < pop[p]) best = p;
    if(p!=st->controlled && pop[p]>0) win = 0;
  }
  if(pop[st->controlled] == 0) lose = 1;

  if (win) return 1;
  else if (lose) return -1;
  else return 0;
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

int get_options(int argc, char *argv[], struct basic_options *op, struct multi_options *mop) {
  
  op->keep_random_flag = 0; // random
  op->dif = dif_normal; // diffiulty
  op->speed = sp_normal; // speed
  op->w = 21; // width
  op->h = 21; // height
  op->loc_num = 0;  // the number of starting locations
  op->map_seed = rand();
  op->conditions = 0;
  op->timeline_flag = 0;

  op->inequality = RANDOM_INEQUALITY;
  op->shape = st_rect;

  /* multiplayer option */
  mop->clients_num = 1;
  mop->multiplayer_flag = 0;
  mop->server_flag = 1;
  
  mop->val_client_port = strdup(DEF_CLIENT_PORT);
  mop->val_server_addr = strdup(DEF_SERVER_ADDR);
  mop->val_server_port = strdup(DEF_SERVER_PORT);
  
  int conditions_were_set = 0;
#ifndef NOGETOPT
  opterr = 0;
  int c;
  while ((c = getopt (argc, argv, "hvrTW:H:i:l:q:d:s:R:S:E:e:C:c:")) != -1){
    switch(c){
      case 'r': op->keep_random_flag = 1; break;
      case 'T': op->timeline_flag = 1; break;
      case 'W': { char* endptr = NULL;
                  op->w = MAX(14, strtol(optarg, &endptr, 10));
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'H': { char* endptr = NULL;
                  op->h = MAX(14, strtol(optarg, &endptr, 10));
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'i': { char* endptr = NULL;
                  op->inequality = strtol(optarg, &endptr, 10);
                  if (*endptr != '\0' || op->inequality < 0 || op->inequality > 4) {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'l': { char* endptr = NULL;
                  op->loc_num = strtol(optarg, &endptr, 10);
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'q': { char* endptr = NULL;
                  op->conditions = strtol(optarg, &endptr, 10);
                  conditions_were_set = 1;
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'R': { char* endptr = NULL;
                  op->map_seed = abs(strtol(optarg, &endptr, 10));
                  if (*endptr != '\0' || *optarg == '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'd': if (strcmp(optarg, "n") == 0) op->dif = dif_normal;
                else if (strcmp(optarg, "e") == 0) op->dif = dif_easy;
                else if (strcmp(optarg, "e1") == 0) op->dif = dif_easy;
                else if (strcmp(optarg, "ee") == 0) op->dif = dif_easiest;
                else if (strcmp(optarg, "e2") == 0) op->dif = dif_easiest;
                else if (strcmp(optarg, "h") == 0) op->dif = dif_hard;
                else if (strcmp(optarg, "h1") == 0) op->dif = dif_hard;
                else if (strcmp(optarg, "hh") == 0) op->dif = dif_hardest;
                else if (strcmp(optarg, "h2") == 0) op->dif = dif_hardest;
                else {
                  print_help();
                  return 1;
                }
                break;
      case 's': if (strcmp(optarg, "n") == 0) op->speed = sp_normal;
                else if (strcmp(optarg, "s") == 0 || strcmp(optarg, "s1") == 0) op->speed = sp_slow;
                else if (strcmp(optarg, "ss") == 0 || strcmp(optarg, "s2") == 0) op->speed = sp_slower;
                else if (strcmp(optarg, "sss") == 0 || strcmp(optarg, "s3") == 0) op->speed = sp_slowest;
                else if (strcmp(optarg, "f") == 0 || strcmp(optarg, "f1") == 0) op->speed = sp_fast;
                else if (strcmp(optarg, "ff") == 0 || strcmp(optarg, "f2") == 0) op->speed = sp_faster;
                else if (strcmp(optarg, "fff") == 0 || strcmp(optarg, "f3") == 0) op->speed = sp_fastest;
                else if (strcmp(optarg, "p") == 0) op->speed = sp_pause;
                else {
                  print_help();
                  return 1;
                }
                break;
      case 'S': if (strcmp(optarg, "rhombus") == 0) op->shape = st_rhombus;
                else if (strcmp(optarg, "rect") == 0) op->shape = st_rect;
                else if (strcmp(optarg, "hex") == 0) op->shape = st_hex;
                else {
                  print_help();
                  return 1;
                }
                break;

                /* multiplayer-related options */
      case 'E': { char* endptr = NULL;
                  mop->clients_num = strtol(optarg, &endptr, 10);
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                  mop->multiplayer_flag = 1;
                  mop->server_flag = 1; 
                }
                break;
      case 'e':
                free(mop->val_server_port);
                mop->val_server_port = strdup(optarg);
                break;
      case 'C': 
                mop->multiplayer_flag = 1;
                mop->server_flag = 0; 
                free(mop->val_server_addr);
                mop->val_server_addr = strdup(optarg);
                break;
      case 'c':
                free(mop->val_client_port);
                mop->val_client_port = strdup(optarg);
                break;
      case 'v':
          printf("%s\n", VERSION);
          return 1;
      case '?': case 'h':
          print_help();
          return 1;
      default: 
          return 1;
    }
  }
#endif
  /* Adjust l_val and conditions_val */
  int avlbl_loc_num = stencil_avlbl_loc_num (op->shape);
  if(op->loc_num == 0) op->loc_num = avlbl_loc_num;

  if (op->loc_num < 2 || op->loc_num > avlbl_loc_num) {
    print_help();
    return 1;
  }
  if (conditions_were_set && (op->conditions<1 || op->conditions>op->loc_num)) {
    print_help();
    return 1;
  }

  if (mop->clients_num < 1 || mop->clients_num > op->loc_num) {
    print_help();
    return 1;
  }

  if (op->shape == st_rect) {
    op->w = MIN(MAX_WIDTH-1, op->w + (op->h + 1)/2);
  }
  return 0;
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
  
  adjust_cursor(st, ui, cursi, cursj);

  return 0;
}
