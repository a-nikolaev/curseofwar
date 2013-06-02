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
#ifndef _KING_H
#define _KING_H

#include "grid.h"

/*
 struct country
    Data about each country
 */
struct country {
  long gold;
};

#define PRICE_VILLAGE 150
#define PRICE_TOWN 300
#define PRICE_CASTLE 600

/*
  build(&g, &c, pl, i, j)

    build a village, upgrade a village to a town, or upgrade a town to a castle.

    g and c are struct grid and struct country,
    pl is the player id (<MAX_PLAYER), 
    (i, j) are the location on the grid.

    pl must match the owner of the tile (i, j)

    Returns 0 on success, and -1 on failure
 */
int build (struct grid *g, struct country *c, int pl, int i, int j);

/*
  degrade(&g, i, j) 
    
    a castle at location (i,j) degrades to a town, 
    a town degrades to a vilalge,
    a vilalge is destroyed.
    
    Returns 0 on success, and -1 on failure
 */
int degrade (struct grid *g, int i, int j);

/* several greedy strategies for kings (AI) */
enum strategy {none, aggr_greedy, one_greedy, persistent_greedy, opportunist, noble, midas};

/*
  struct king
    
    King, an AI for computer opponents

    Member fields:
      value is a 2D array for values of each tile, 
        different kings may have different values for the same tile

      pl is the player controlled by this king.

      strategy is one of the above strategies used by the king
 */
struct king {
  int value[MAX_WIDTH][MAX_HEIGHT];
  int pl;
  enum strategy strategy;
};

/*
  king_evaluate_map (&k, &g, difficulty)

    King k evaluates the map grid g.
    difficulty determines the quality of evaluation
 */
void king_evaluate_map (struct king *k, struct grid *g, enum config_dif dif);


/*
  king_init (&k, pl, &g, dif)
    
    Initialize King k for controlling player pl.
    Map is evaluated too.
 */
void king_init (struct king *k, int pl, enum strategy strat,  struct grid *g, enum config_dif dif);

/*
  builder_default (&k, &c, &g, &fg)

    The default city building strategy. The same for all AIs.

    Returns code 0 if something was built,
      all kings have to reevaluate the map in this case,
    otherwise returns -1.
 */
int builder_default (struct king *k, struct country *c, struct grid *g, struct flag_grid *fg);

/*
  place_flags (&k, &g, &fg)

    Flag placing strategy.
    Depends on the strategy of the king k.

    Flag grid fg is modified.
 */
void place_flags (struct king *k, struct grid *g, struct flag_grid *fg);

#endif
