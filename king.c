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
#include "king.h"
#include <math.h>

int build (struct grid *g, struct country *c, int pl, int i, int j) {
  if (i>=0 && i < g->width && j>=0 && j<g->height && g->tiles[i][j].pl == pl) {
    int price = 0;
    enum tile_class cl = grassland;
    switch (g->tiles[i][j].cl) {
      case grassland: price = PRICE_VILLAGE; cl = village; break;
      case village: price = PRICE_TOWN; cl = town; break;
      case town: price = PRICE_CASTLE; cl = castle; break;
      default: return -1;
    }
    if (c->gold >= price) { g->tiles[i][j].cl = cl; c->gold -= price; return 0; }
  }
  return -1;
}

int degrade (struct grid *g, int i, int j) {
  if (i>=0 && i < g->width && j>=0 && j<g->height) {
    enum tile_class cl = grassland;
    switch (g->tiles[i][j].cl) {
      case village: cl = grassland; break;
      case town: cl = village; break;
      case castle: cl = town; break;
      default: return -1;
    }
    g->tiles[i][j].cl = cl; 
    return 0; 
  }
  return -1;
}

void king_evaluate_map (struct king *k, struct grid *g, enum config_dif dif) {
  int i, j;
  int u [MAX_WIDTH][MAX_HEIGHT];
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      u[i][j] = 0;
      k->value[i][j] = 0;
    }
  }
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      if (is_inhabitable(g->tiles[i][j].cl)) k->value[i][j] += 1;
      switch (k->strategy) {
        case persistent_greedy:
          if (is_inhabitable(g->tiles[i][j].cl)) k->value[i][j] += 1;
          break;
        default: ;
      }

      switch (g->tiles[i][j].cl) {
        case castle:
          if(k->strategy == noble)
            spread (g, u, k->value, i, j, 32, 1);
          else
            spread (g, u, k->value, i, j, 16, 1);
          even(g, u, i, j, 0);
          break;
        case town:
          spread (g, u, k->value, i, j, 8, 1);
          even(g, u, i, j, 0);
          break;
        case village:
          if(k->strategy == noble)
            spread (g, u, k->value, i, j, 2, 1);
          else
            spread (g, u, k->value, i, j, 4, 1);
          even(g, u, i, j, 0);
          break;
        case mine: {
            int d;
            for(d=0; d<DIRECTIONS; ++d){
              int ii = i + dirs[d].i;
              int jj = j + dirs[d].j;
              if(k->strategy == midas)
                spread (g, u, k->value, ii, jj, 8, 1);
              else
                spread (g, u, k->value, ii, jj, 4, 1);
              even (g, u, ii, jj, 0);
            }
          }; 
          break;
        default: ;
      } 
    }
  }
  /* dumb down kings */
  int x;
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      switch(dif) {
        case dif_easiest: 
          x = k->value[i][j] / 4;
          x = x + rand()%7 - 3;
          k->value[i][j] = MAX(0, x);
          break;
        case dif_easy: 
          x = k->value[i][j] / 2;
          x = x + rand()%3 - 1;
          k->value[i][j] = MAX(0, x);
          break;
        default: ;
      }
    }
  }
}

void king_init (struct king *k, int pl, enum strategy strat, struct grid *g, enum config_dif dif) {
  k->pl = pl;
  k->strategy = strat;
  king_evaluate_map(k, g, dif);
}

int builder_default (struct king *k, struct country *c, struct grid *g, struct flag_grid *fg) {
  int i, j; 
  int i_best = 0, j_best = 0;
  float v_best = 0.0;
  float v;

  int n;
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      
      int ok = 0;
      if ( g->tiles[i][j].pl == k->pl && is_inhabitable(g->tiles[i][j].cl) ) {
        ok = 1;
        int di, dj;
        for(n=0; n<DIRECTIONS; ++n) {
          di = dirs[n].i;
          dj = dirs[n].j;
          if( i+di >= 0 && i+di < g->width && 
              j+dj >= 0 && j+dj < g->height &&
              is_inhabitable (g->tiles[i+di][j+dj].cl) ) {
            ok = ok && (g->tiles[i+di][j+dj].pl == k->pl);
          }
        }
      }

      int army = g->tiles[i][j].units[k->pl][citizen];
      int enemy = 0;
      int p;
      for (p=0; p<MAX_PLAYER; ++p) { 
        if (p!= k->pl) 
          enemy = enemy + g->tiles[i][j].units[p][citizen];
      }
      
      int base = 1.0;
      switch (g->tiles[i][j].cl) {
        case grassland: base = 1.0; break;
        case village: base = 8.0; break;
        case town: base = 32.0; break;
        default: base = 0.0;
      }
      if (k->strategy == midas)
        base *= (k->value[i][j] + 10);

      v = ok * base * (MAX_POP - army);
      if (army < MAX_POP / 10) v = 0.0;
      
      if (v > 0.0 && v > v_best) {
        i_best = i;
        j_best = j;
        v_best = v;
      }
    }
  }
  if (v_best > 0.0)
    return build(g, c, k->pl, i_best, j_best);
  else
    return -1;
}


/* Auxiliary functions for different kings' strategies: */

void action_aggr_greedy (struct king *k, struct grid *g, struct flag_grid *fg) {
  int i, j; 
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      if (fg->flag[i][j]) 
        remove_flag(g, fg, i, j, FLAG_POWER);

      // estimate the value of the grid point (i,j)
      int army = g->tiles[i][j].units[k->pl][citizen];
      int enemy = 0;
      int p;
      for (p=0; p<MAX_PLAYER; ++p) { 
        if (p!= k->pl) 
          enemy = enemy + g->tiles[i][j].units[p][citizen];
      }
      float v = (float) k->value[i][j] * (2.0*enemy - army) * pow(army, 0.5); 

      if (v > 5000) 
        add_flag(g, fg, i, j, FLAG_POWER);
    }
  }
}

void action_one_greedy (struct king *k, struct grid *g, struct flag_grid *fg) {
  int i, j; 
  int i_best = 0, j_best = 0;
  float v_best = -1.0;
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      if (fg->flag[i][j]) 
        remove_flag(g, fg, i, j, FLAG_POWER);

      // estimate the value of the grid point (i,j)
      int army = g->tiles[i][j].units[k->pl][citizen];
      int enemy = 0;
      int p;
      for (p=0; p<MAX_PLAYER; ++p) { 
        if (p!= k->pl) 
          enemy = enemy + g->tiles[i][j].units[p][citizen];
      }
      float v = (float) k->value[i][j] * (5.0*enemy - army) * pow(army, 0.5); 

      if (v > v_best && v > 5000) {v_best = v; i_best = i; j_best = j;}
    }
  }
  if (v_best > 0)
    add_flag(g, fg, i_best, j_best, FLAG_POWER);
}

void action_persistent_greedy (struct king *k, struct grid *g, struct flag_grid *fg) {
  int i, j; 
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      // estimate the value of the grid point (i,j)
      int army = g->tiles[i][j].units[k->pl][citizen];
      int enemy = 0;
      int p;
      for (p=0; p<MAX_PLAYER; ++p) { 
        if (p!= k->pl) 
          enemy = enemy + g->tiles[i][j].units[p][citizen];
      }
      float v1 = (float) k->value[i][j] * (2.5*enemy - army) * pow(army, 0.7); 
      // weak opportunist
      float v2 = (float) k->value[i][j] * (MAX_POP - (enemy - army)) * pow(army, 0.7) * 0.5; 
      if (enemy <= army) v2 = -10000;
      float v = MAX(v1,v2);

      if (fg->flag[i][j] == FLAG_ON) {
        if (v < 1000) 
          remove_flag(g, fg, i, j, FLAG_POWER);
      }
      else {
        if (v > 9000) 
          add_flag(g, fg, i, j, FLAG_POWER);
      }
    }
  }
}

void action_opportunist (struct king *k, struct grid *g, struct flag_grid *fg) {
  int i, j; 
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      if (fg->flag[i][j]) 
        remove_flag(g, fg, i, j, FLAG_POWER);

      // estimate the value of the grid point (i,j)
      int army = g->tiles[i][j].units[k->pl][citizen];
      int enemy = 0;
      int p;
      for (p=0; p<MAX_PLAYER; ++p) { 
        if (p!= k->pl) 
          enemy = enemy + g->tiles[i][j].units[p][citizen];
      }
      float v = (float) k->value[i][j] * (MAX_POP - (enemy - army)) * pow(army, 0.5); 

      if (enemy > army && v > 7000) 
        add_flag(g, fg, i, j, FLAG_POWER);
    }
  }
}

/* Priority array */
#define MAX_PRIORITY 32
/* special no-location */
const struct loc no_loc = {-1, -1};

/* initialize array of locations */
void init_locval(struct loc loc[MAX_PRIORITY], int val[MAX_PRIORITY], int len) {
  int i;
  for(i=0; i<len; ++i) {
    loc[i] = no_loc;
    val[i] = -1;
  }
}

/* insert */
void insert_locval(struct loc loc[MAX_PRIORITY], int val[MAX_PRIORITY], int len, struct loc lx, int vx) {
  int i;
  for (i=0; i<len && i<MAX_PRIORITY && val[i]>=vx; ++i);
  if (i<len && i<MAX_PRIORITY){
    int j;
    for (j=len-1; j>i; --j){
      loc[j] = loc[j-1]; 
      val[j] = val[j-1];
    }
    loc[i] = lx;
    val[i] = vx;
  }
}

void action_noble (struct king *k, struct grid *g, struct flag_grid *fg) {
  int i, j; 
  /* number of flags */
  int locval_len = 5;
  struct loc loc[MAX_PRIORITY];
  int val[MAX_PRIORITY];
  init_locval(loc, val, locval_len);

  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      if (fg->flag[i][j]) 
        remove_flag(g, fg, i, j, FLAG_POWER);

      // estimate the value of the grid point (i,j)
      int army = g->tiles[i][j].units[k->pl][citizen];
      int enemy = 0;
      int p;
      for (p=0; p<MAX_PLAYER; ++p) { 
        if (p!= k->pl) 
          enemy = enemy + g->tiles[i][j].units[p][citizen];
      }
      float v = (float) k->value[i][j] * (MAX_POP - (enemy - army)) * pow(army, 0.5); 

      if (enemy > army && v > 7000) {
        //add_flag(g, fg, i, j, FLAG_POWER);
        struct loc lx = {i,j};
        insert_locval(loc, val, locval_len, lx, (int)v);
      }
    }
  }
  for (i=0; i<locval_len && val[i]>0; ++i){
    add_flag(g, fg, loc[i].i, loc[i].j, FLAG_POWER);
  }
}

void place_flags (struct king *k, struct grid *g, struct flag_grid *fg) {
  switch (k->strategy) {
    case aggr_greedy: action_aggr_greedy(k, g, fg); break;
    case one_greedy: action_one_greedy(k, g, fg); break;
    case persistent_greedy: action_persistent_greedy(k, g, fg); break;
    case opportunist: action_opportunist(k, g, fg); break;
    case noble: action_noble(k, g, fg); break;
    case midas: break;
    default: ;
  }
}


