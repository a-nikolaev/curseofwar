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
#ifndef _GRID_H
#define _GRID_H

#include <stdlib.h>
#include "common.h"

#define FLAG_ON 1
#define FLAG_OFF 0
#define FLAG_POWER 8

/* enum unit_class
 *
 * Units/Creatures that can be controlled by the players:
 *  Only citizens are available. */
enum unit_class { citizen=0 };


/* enum tile_class
 *
 * Territory classes:
 *  mountain is a natural barrier
 *  mine is a source of gold
 *  grassland is a habitable territory that does not have cities
 *  village, town, and castle are three kinds of cities with different population growth rate
 *    (castles have the highest rate, while villages have the lowest).
 *
 * */
enum tile_class { mountain, mine, grassland, village, town, castle };

/* is_a_city(t)
    returns 1 for village, town, or castle */
int is_a_city(enum tile_class t);

/* is_inhabitable(t)
    returns 1 for grassland, village, town, or castle */
int is_inhabitable(enum tile_class t);


/* struct tile
 * 
 * Tiles are the smallest pieces of the map.
 *
 * Components:
 *  cl is the tile's territory class,
 *  pl is the id if the player, owner of the tile
 *  units is the array that contains information about the population of the tile
 *    (info for all players, and for all unit classes)
 *
 * */
struct tile {
  enum tile_class cl;
  int pl;
  int units[MAX_PLAYER][MAX_CLASS];
};

/* struct loc 
 * 
 * Location. 
 *
 * Components:
 *  i (horizontal axis)
 *  j (vertical axis)
 * */
struct loc {
  int i;
  int j;
};

/* There are 6 possible directions to move from a tile. Hexagonal geometry. */
const struct loc dirs[DIRECTIONS];

/* struct grid
 *
 * 2D Array of tiles + width and height information.
 * The map is stored in this structure.
 */
struct grid {
  int width;
  int height;
  struct tile tiles[MAX_WIDTH][MAX_HEIGHT];
};

/* grid_init (&g, w, h)
 *
 *  Initialize the grid g. Set its width to w and height to h.
 *  It also generates the tiles: Random mountains, mines and cities 
 */
void grid_init(struct grid *g, int w, int h);

/* conflict (&g, players, players_num, human_player)
 *
 *  Enhances an already initialized grid.
 *  Places at most 4 players at the corners of the map, gives them a castle and 2 mines nearby.
 *  One of those players is always controlled by a human player.
 *
 *  players is the array of the ids of the possible opponents (represented by integers, usually 1 < i < MAX_PLAYER),
 *  players_num is the size of the players array
 *
 *  locations_num is the number of starting locations (can be equal to 2, 3, or 4)
 *  human_player is the id of the human player (usually = 1) 
 *
 */
void conflict (struct grid *g, int players[], int players_num, int locations_num, int human_player);

/* struct flag_grid
 *
 *  Similar to the struct grid, but stores information about player's flags.
 *  Each player has his own struct grid_flag.
 *
 *  flag[i][j] == 1, if there is a flag.
 *
 *  call[i][j] determine the power of attraction to this location.
 *    Must be updated when flags are added or removed.
 */

struct flag_grid {
  int width;
  int height;
  int flag [MAX_WIDTH][MAX_HEIGHT];
  int call [MAX_WIDTH][MAX_HEIGHT];
};

/* flag_grid_init (&fg, w, h)
 *  
 *  A simple initialization of the flag grid fg.
 */
void flag_grid_init(struct flag_grid *fg, int w, int h);

/* spread(&g, u, v, x, y, val, factor)
 *  and
 * even(&g, u, x, y, val)
 *
 *  Helper functions, primarily are used for maintaining call[i][j] for flag grids
 */
void spread (struct grid *g, int u[MAX_WIDTH][MAX_HEIGHT], int v[MAX_WIDTH][MAX_HEIGHT], int x, int y, int val, int factor);
void even (struct grid *g, int v[MAX_WIDTH][MAX_HEIGHT], int x, int y, int val);

/* add_flag (&g, &fg, x, y, v)
 *
 *  Adds a flag to the flag grid fg at the location (x,y) with power v.
 */
void add_flag (struct grid *g, struct flag_grid *fg, int x, int y, int val);

/* remove_flag (&g, &fg, x, y, v)
 *
 *  Removes a flag from the flag grid fg at the location (x,y) with power v.
 */
void remove_flag (struct grid *g, struct flag_grid *fg, int x, int y, int val);

/* remove_flags_with_prob (&g, &fg, prob)
 *
 *  Iterates over all tiles, and removes flags with probability prob.
 *  That is, it removes all flags if prob==1.
 */
void remove_flags_with_prob (struct grid *g, struct flag_grid *fg, float prob);

#endif
