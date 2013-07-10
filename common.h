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

#ifndef _COMMON_H
#define _COMMON_H

#define MAX_PLAYER 8  /* number of players (countries) */
#define NEUTRAL 0     /* neutral player */
#define MAX_CLASS 1   /* classes of units. only one exists. */
#define MAX_WIDTH 40  /* max map width */
#define MAX_HEIGHT 29 /* max map height */
#define DIRECTIONS 6  /* number of neighbors on the grid */

#define MAX_POP 499   /* maximum polulation at a tile (for each player) */

#define MIN(x,y) (x<y)?(x):(y)
#define MAX(x,y) (x<y)?(y):(x)
#define IN_SEGMENT(x,l,r) (((x)<(l))?(l) : ( ((x)>(r))?(r):(x) ))

#define ESCAPE     '\033'
#define K_UP       65
#define K_DOWN     66
#define K_RIGHT    67
#define K_LEFT     68

/* game speed */
enum config_speed {sp_pause, sp_slowest, sp_slower, sp_slow, sp_normal, sp_fast, sp_faster, sp_fastest};

/* game difficulty */
enum config_dif {dif_easiest, dif_easy, dif_normal, dif_hard, dif_hardest};

#endif
