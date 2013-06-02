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
#include "grid.h"

int is_a_city(enum tile_class t) {
  switch(t) {
    case village: return 1;
    case town: return 1;
    case castle: return 1;
    default: return 0;
  }
}

int is_inhabitable(enum tile_class t) {
  switch(t) {
    case mountain:
    case mine: return 0;
    default: return 1;
  }
}

const struct loc dirs[DIRECTIONS] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {1,-1}, {-1, 1}};

void grid_init(struct grid *g, int w, int h){
  g->width = MIN(w, MAX_WIDTH);
  g->height = MIN(h, MAX_HEIGHT);
  int i, j;
  for(i=0; i<g->width; ++i) {
    for(j=0; j<g->height; ++j) {
      g->tiles[i][j].cl = grassland;
      int x = rand() % 20;
      if(0 == x) { 
        int y = rand() % 6;
        switch (y){
          case 0: 
            g->tiles[i][j].cl = castle; break;
          case 1: case 2:
            g->tiles[i][j].cl = town; break;
          default:
            g->tiles[i][j].cl = village;
        }
      }
      if(x > 0 && x < 5) {
        // mountains and mineis
        if (rand() % 10 == 0)
          g->tiles[i][j].cl = mine;
        else
          g->tiles[i][j].cl = mountain;
        g->tiles[i][j].pl = NEUTRAL;
      }
      else { 
        x = 1 + rand() % (MAX_PLAYER - 1);
        if (x < MAX_PLAYER) g->tiles[i][j].pl = x;
        else
          g->tiles[i][j].pl = NEUTRAL; 
      }

      int p, c;
      for (p=0; p<MAX_PLAYER; ++p) {
        for (c=0; c<MAX_CLASS; ++c) {
          g->tiles[i][j].units[p][c] = 0;
        }
      }

      if (is_a_city(g->tiles[i][j].cl)) {
        int owner = g->tiles[i][j].pl;
        g->tiles[i][j].units[owner][citizen] = 10;
      }

    }
  }
}

/* simple shuffling of an array of integers */
void shuffle (int arr[], int len) {
  int t, i, j, s;
  for(t=0; t<len; ++t){
    i = rand() % len;
    j = rand() % len;
    s = arr[i];
    arr[i] = arr[j];
    arr[j] = s;
  }
}

void conflict (struct grid *g, int players[], int players_num, int locations_num, int human_player) {
  int i, j, p, c;
  /* first, remove all cities */
  for(i=0; i<g->width; ++i) {
    for(j=0; j<g->height; ++j) {
      for (p=0; p< MAX_PLAYER; ++p) {
        for (c = 0; c<MAX_CLASS; ++c) {
          g->tiles[i][j].units[p][c] = 0;
          g->tiles[i][j].pl = NEUTRAL;
          if (is_a_city(g->tiles[i][j].cl)) 
            g->tiles[i][j].cl = grassland;
            //g->tiles[i][j].pl = NEUTRAL;
        }
      }
    }
  }

  int d = 2;
  int m = 1;
  
  locations_num = IN_SEGMENT(locations_num, 2, 4);
  /* starting locations arrays */
  int xs[] = {d, g->width-1-d, d, g->width-1-d};
  int ys[] = {d, g->height-1-d, g->height-1-d, d};
  
  int num = MIN(locations_num, players_num);
 
  /* a shuffled copy of the players array */
  int *sh_players = malloc(sizeof(int)*players_num);
  for(i=0; i<players_num; ++i)
    sh_players[i] = players[i];
  shuffle(sh_players, players_num);

  /* shift in the positions arrays */
  int di = rand() % locations_num;
  int ihuman = rand() % num;
  i = 0;
  while (i < num) {
    int ii = (i + di) % locations_num;
    int x = xs[ii];
    int y = ys[ii];
    g->tiles[x][y].cl = castle;
    if (human_player && ihuman == i) 
      g->tiles[x][y].pl = human_player;
    else 
      g->tiles[x][y].pl = sh_players[i];
    
    g->tiles[x][y].units[ g->tiles[x][y].pl ][citizen] = 10;
    
    /* place mines nearby */
    int dir = rand() % DIRECTIONS;
    int ri = dirs[dir].i;
    int rj = dirs[dir].j;
    
    int mine_i = x + m*ri;
    int mine_j = y + m*rj;
    g->tiles[mine_i][mine_j].cl = mine;
    g->tiles[mine_i][mine_j].pl = NEUTRAL;
    mine_i = x - 2*m*ri;
    mine_j = y - 2*m*rj;
    g->tiles[mine_i][mine_j].cl = mine;
    g->tiles[mine_i][mine_j].pl = NEUTRAL;

    i++;
  }

  /* free allocated memory */
  free(sh_players);
}

void flag_grid_init(struct flag_grid *fg, int w, int h) {
  fg->width = MIN(w, MAX_WIDTH);
  fg->height = MIN(h, MAX_HEIGHT);
  int i, j;
  for(i=0; i<fg->width; ++i) {
    for(j=0; j<fg->height; ++j) {
      fg->flag[i][j] = FLAG_OFF;
      fg->call[i][j] = 0;
    }
  }
}

void spread (struct grid *g, int u[MAX_WIDTH][MAX_HEIGHT], int v[MAX_WIDTH][MAX_HEIGHT], int x, int y, int val, int factor) {
  if (x < 0 || x >= g->width || y < 0 || y >= g->height || is_inhabitable(g->tiles[x][y].cl) == 0) {
    return;
  }
  int d = val - u[x][y];
  if (d > 0) {
    v[x][y] = MAX(0, v[x][y] + d * factor);
    u[x][y] += d;

    int k;
    for (k = 0; k<DIRECTIONS; ++k) 
      spread(g, u, v, x+dirs[k].i, y+dirs[k].j, val/2, factor);
  }
}

void even (struct grid *g, int v[MAX_WIDTH][MAX_HEIGHT], int x, int y, int val) {
  if (x < 0 || x >= g->width || y < 0 || y >= g->height || v[x][y] == val) {
    return;
  }
  v[x][y] = val;

  int k;
  for (k = 0; k<DIRECTIONS; ++k) 
    even(g, v, x+dirs[k].i, y+dirs[k].j, val);
}

void add_flag (struct grid *g, struct flag_grid *fg, int x, int y, int val) {
  // exit if
  if (x < 0 || x >= g->width || y < 0 || y >= g->height || 
      is_inhabitable(g->tiles[x][y].cl) == 0 || fg->flag[x][y] == FLAG_ON) {
    return;
  }
  
  int u[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for(i=0; i< MAX_WIDTH; ++i){
    for(j=0; j<MAX_HEIGHT; ++j){
      u[i][j] = 0;
    }
  }
  
  fg->flag[x][y] = FLAG_ON;
  spread(g, u, fg->call, x, y, val, 1);
}

void remove_flag (struct grid *g, struct flag_grid *fg, int x, int y, int val) {
  // exit if
  if (x < 0 || x >= g->width || y < 0 || y >= g->height || 
      is_inhabitable(g->tiles[x][y].cl) == 0 || fg->flag[x][y] == FLAG_OFF) {
    return;
  }
  
  int u[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for(i=0; i< MAX_WIDTH; ++i){
    for(j=0; j<MAX_HEIGHT; ++j){
      u[i][j] = 0;
    }
  }
  
  fg->flag[x][y] = FLAG_OFF;
  spread(g, u, fg->call, x, y, val, -1);
}

void remove_flags_with_prob (struct grid *g, struct flag_grid *fg, float prob) {
  int i, j;
  for (i=0; i<g->width; ++i) {
    for (j=0; j<g->height; ++j) {
      if (fg->flag[i][j] && (float)rand() / RAND_MAX <= prob) { 
        remove_flag(g, fg, i, j, FLAG_POWER);
      }
    }
  }
}
