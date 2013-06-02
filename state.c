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
#include "state.h"
#include <time.h>
#include <stdio.h>
#include <math.h>

enum config_speed faster(enum config_speed sp){
  switch (sp) {
    case sp_pause: return sp_slowest;
    case sp_slowest: return sp_slower;
    case sp_slower: return sp_slow;
    case sp_slow: return sp_normal;
    case sp_normal: return sp_fast;
    case sp_fast: return sp_faster;
    default: return sp_fastest;
  }
}

enum config_speed slower(enum config_speed sp){
  switch (sp) {
    case sp_fastest: return sp_faster;
    case sp_faster: return sp_fast;
    case sp_fast: return sp_normal;
    case sp_normal: return sp_slow;
    case sp_slow: return sp_slower;
    case sp_slower: return sp_slowest;
    default: return sp_pause;
  }
}

void state_init(struct state *s, int w, int h, int keep_random, int locations_num, enum config_speed speed, enum config_dif dif){
  srand(time(NULL));
  
  s->speed = speed;
  s->prev_speed = s->speed;
  s->dif = dif;

  grid_init(&s->grid, w, h);
  
  /* player controlled from the keyboard */
  s->controlled = 1;

  if (!keep_random) {
    //int players[] = {7, 2, 3, 5};
    //int players[] = {3, 4, 4, 6};
    int players[] = {2, 3, 4, 5, 6, 7};
    conflict(&s->grid, players, 6, locations_num, s->controlled);
  }

  int p;
  for(p = 0; p<MAX_PLAYER; ++p) {
    flag_grid_init(&s->fg[p], w, h);
    s->country[p].gold = 0;
  }
  
  
  /* cursor location */
  s->cursor.i = 0;
  s->cursor.j = 0;
  int i, j;
  for (i=0; i<s->grid.width; ++i) {
    for (j=0; j<s->grid.height; ++j) {
      if (s->grid.tiles[i][j].units[s->controlled][citizen] > 
          s->grid.tiles[s->cursor.i][s->cursor.j].units[s->controlled][citizen]) {
        s->cursor.i = i;
        s->cursor.j = j;
      }
    }
  }

  /* Kings */
  s->kings_num = 6;
  king_init (&s->king[0], 2, aggr_greedy, &s->grid, s->dif);
  king_init (&s->king[1], 3, one_greedy, &s->grid, s->dif);
  king_init (&s->king[2], 7, persistent_greedy, &s->grid, s->dif);
  king_init (&s->king[3], 5, opportunist, &s->grid, s->dif);
  king_init (&s->king[4], 4, none, &s->grid, s->dif);
  king_init (&s->king[5], 6, noble, &s->grid, s->dif);

}

#define GROWTH_1 1.10
#define GROWTH_2 1.20
#define GROWTH_3 1.30

#define ATTACK 0.1
#define MOVE 0.05
#define CALL_MOVE 0.10

float growth(enum tile_class t) {
  switch(t) {
    case village: return GROWTH_1;
    case town: return GROWTH_2;
    case castle: return GROWTH_3;
    default: return 0;
  }
}

int rnd_round(float x) {
  int i = (int) x;
  if ( (float)rand() / (float)RAND_MAX < (x - i) ) i++;
  return i;
}

void kings_move(struct state *s) {
  int i;
  int ev = 0;
  for(i=0; i<s->kings_num; ++i) {
    int pl = s->king[i].pl;
    place_flags(&s->king[i], &s->grid, &s->fg[pl]);
    int code = builder_default(&s->king[i], &s->country[pl], &s->grid, &s->fg[pl]);
    ev = ev || (code == 0);
  }
  if (ev) {
    for(i=0; i<s->kings_num; ++i) {
      king_evaluate_map(&s->king[i], &s->grid, s->dif);
    }
  }
}

void simulate(struct state *s) {
  int i, j;
  struct tile (* t) [MAX_HEIGHT] = s->grid.tiles;
  int enemy_pop [MAX_PLAYER];
  int my_pop [MAX_PLAYER];

  int need_to_reeval = 0;

  /* per tile events */
  for(i=0; i<s->grid.width; ++i) {
    for(j=0; j<s->grid.height; ++j) {
      /* mines ownership */
      if (t[i][j].cl == mine){
        int k;
        int owner = NEUTRAL;
        for(k=0; k<DIRECTIONS; ++k) {
          int di = dirs[k].i;
          int dj = dirs[k].j;
          if( i+di >= 0 && i+di < s->grid.width && 
              j+dj >= 0 && j+dj < s->grid.height &&
              is_inhabitable (t[i+di][j+dj].cl) ) {
            int pl = t[i+di][j+dj].pl;
            if (owner == NEUTRAL) 
              owner = pl;
            else if (owner != pl && pl != NEUTRAL)
              owner = -1;
          }
        }
        if (owner != -1) 
          t[i][j].pl = owner;
        else
          t[i][j].pl = NEUTRAL;
       
        if (t[i][j].pl != NEUTRAL) 
          s->country[owner].gold += 1;
        
      }

      /* fight */
      int p;
      int total_pop = 0;
      for(p=0; p<MAX_PLAYER; ++p){
        my_pop[p] = t[i][j].units[p][citizen];
        total_pop += my_pop[p];
      }
      int defender_dmg = 0;
      for(p=0; p<MAX_PLAYER; ++p){
        enemy_pop[p] = total_pop - my_pop[p];
        int dmg = rnd_round( (float)enemy_pop[p] * my_pop[p] / total_pop );
        t[i][j].units[p][citizen] = MAX(my_pop[p] - dmg, 0);
        
        if (t[i][j].pl == p)
          defender_dmg = dmg;
      }

      /* burning cities */
      if (defender_dmg > 2.0 * MAX_POP * ATTACK && is_a_city(t[i][j].cl)) {
        if (rand() % 1 == 0){
          need_to_reeval = 1;
          degrade (&s->grid, i, j);
        }
      }

      /* determine ownership */
      if (is_inhabitable(t[i][j].cl)){
        t[i][j].pl = NEUTRAL;
        for(p=0; p<MAX_PLAYER; ++p){
          if (t[i][j].units[p][citizen] > t[i][j].units[t[i][j].pl][citizen]) {
            t[i][j].pl = p;
          }
        }
      }
      
      if (is_a_city(t[i][j].cl)) {
        /* population growth */
        int owner = t[i][j].pl;
        int pop = t[i][j].units[owner][citizen];
        float fnpop = (float) pop * growth(t[i][j].cl);
        int npop = rnd_round(fnpop);
        npop = MIN(npop, MAX_POP);
        /* death */
        //npop = npop - rnd_round(0.01*pow(pop,1.5));
        //npop = npop - rnd_round(1.0e-20 * pow(2.0,pop));
        //npop = MAX(npop, 0);
        t[i][j].units[owner][citizen] = npop;
      }
    }
  }
  /* migration */
  int k, di, dj, p;

  int i_start, i_end, i_inc;
  int j_start, j_end, j_inc;

  if(rand()%2 == 0) { i_start = 0; i_end = s->grid.width; i_inc = 1; }
  else { i_start = s->grid.width-1; i_end = -1; i_inc = -1; }
  
  if(rand()%2 == 0) { j_start = 0; j_end = s->grid.height; j_inc = 1; }
  else { j_start = s->grid.height-1; j_end = -1; j_inc = -1; }

  for(i=i_start; i!=i_end; i+=i_inc) {
    for(j=j_start; j!=j_end; j+=j_inc) {
      for(p=0; p<MAX_PLAYER; ++p){
        int initial_pop = t[i][j].units[p][citizen];

        int k_shift = rand() % DIRECTIONS;
        for(k=0; k<DIRECTIONS; ++k) {
          di = dirs[(k + k_shift) % DIRECTIONS].i;
          dj = dirs[(k + k_shift) % DIRECTIONS].j;
          if( i+di >= 0 && i+di < s->grid.width && 
              j+dj >= 0 && j+dj < s->grid.height &&
              is_inhabitable (t[i+di][j+dj].cl) ) {
            int pop = t[i][j].units[p][citizen];
            int dcall = MAX(0, s->fg[p].call[i+di][j+dj] - s->fg[p].call[i][j]);
            if (pop > 0) {
              int dpop = rnd_round (MOVE * initial_pop + CALL_MOVE * dcall * initial_pop);
              dpop = MIN(dpop, pop);
              dpop = MIN(dpop, MAX_POP - t[i+di][j+dj].units[p][citizen]);
              t[i+di][j+dj].units[p][citizen] += dpop;
              t[i][j].units[p][citizen] -= dpop;
            }
          }
        }
      }
    }
  }

  /* determine ownership again */
  for(i=0; i<s->grid.width; ++i) {
    for(j=0; j<s->grid.height; ++j) {
      if (is_inhabitable(t[i][j].cl)){
        t[i][j].pl = NEUTRAL;
        for(p=0; p<MAX_PLAYER; ++p){
          if (t[i][j].units[p][citizen] > t[i][j].units[t[i][j].pl][citizen]) {
            t[i][j].pl = p;
          }
        }
      }
    }
  }
  /* Kings reevaluate the map */
  if(need_to_reeval) {
    for(i=0; i<s->kings_num; ++i) {
      king_evaluate_map(&s->king[i], &s->grid, s->dif);
    }
  }
        
  /* give gold to AI on hard difficulties */
  int add_gold = 0;
  switch(s->dif) {
    case dif_hard: add_gold = 1; break;
    case dif_hardest: add_gold = 2; break;
    default: ;
  }
  for(i=0; i<MAX_PLAYER; ++i){
    if (i != NEUTRAL && i != s->controlled && s->country[i].gold>0)
      s->country[i].gold += add_gold; 
  }
}
