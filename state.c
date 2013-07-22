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

void state_init(struct state *s, int w, int h, enum stencil shape,
    unsigned int map_seed, int keep_random, int locations_num, int clients_num, 
    int conditions, int inequality, enum config_speed speed, enum config_dif dif){
 
  s->speed = speed;
  s->prev_speed = s->speed;
  s->dif = dif;
  s->map_seed = map_seed;
  s->conditions = conditions;
  s->inequality = inequality;
  s->time = (1850 + rand()%100) * 360 + rand()%360;

  /* player controlled from the keyboard */
  s->controlled = 1;
  /* int players[] = {7, 2, 3, 5}; */
  /* int players[] = {2, 3, 4, 5, 6, 7}; */

  int all_players[] = {1, 2, 3, 4, 5, 6, 7};
  int comp_players[MAX_PLAYER]; 
  int comp_players_num = 7 - clients_num;
 
  s->kings_num = comp_players_num;
  int ui_players[MAX_PLAYER];
  int ui_players_num = clients_num;
  int i;
  for (i=0; i<7; ++i) {
    if (i<clients_num)
      ui_players[i] = all_players[i];
    else {
      int j = i - clients_num; /* computer player index / king's index */
      comp_players[j] = all_players[i];
      switch (i){
        case 1:
          king_init (&s->king[j], i+1, opportunist, &s->grid, s->dif); 
          break;
        case 2:
          king_init (&s->king[j], i+1, one_greedy, &s->grid, s->dif);
          break;
        case 3:
          king_init (&s->king[j], i+1, none, &s->grid, s->dif);
          break;
        case 4:
          king_init (&s->king[j], i+1, aggr_greedy, &s->grid, s->dif);
          break;
        case 5:
          king_init (&s->king[j], i+1, noble, &s->grid, s->dif);
          break;
        case 6:
          king_init (&s->king[j], i+1, persistent_greedy, &s->grid, s->dif);
          break;
      }
    }
  }

  /* Initialize map generation with the map_seed */
  srand(s->map_seed);

  /* Map generation starts */
  {
    int conflict_code = 0;
    do {
      grid_init(&s->grid, w, h);
  
      /* starting locations arrays */
      struct loc loc_arr[MAX_AVLBL_LOC];
      int available_loc_num = 0;
      int d = 2;
      apply_stencil(shape, &s->grid, d, loc_arr, &available_loc_num);
     
      /* conflict mode */
      conflict_code = 0;
      if (!keep_random) 
        conflict_code = conflict(&s->grid, loc_arr, available_loc_num, 
            comp_players, comp_players_num, locations_num, ui_players, ui_players_num, s->conditions, s->inequality);
    } while(conflict_code != 0 || !is_connected(&s->grid));
  }
  /* Map is ready */

  int p;
  for(p = 0; p<MAX_PLAYER; ++p) {
    flag_grid_init(&s->fg[p], w, h);
    s->country[p].gold = 0;
  }

  /* kings evaluate the map */
  for(p = 0; p < s->kings_num; ++p) {
    king_evaluate_map(&s->king[p], &s->grid, dif);
  }

  /* Zero timeline */
  s->timeline.mark = -1;
  for(i=0; i<MAX_TIMELINE_MARK; ++i) {
    s->timeline.time[i] = s->time;
    for(p=0; p<MAX_PLAYER; ++p) {
      s->timeline.data[p][i] = 0.0;
    }
  }
}


void ui_init(struct state *s, struct ui *ui) {
  /* cursor location */
  ui->cursor.i = s->grid.width/2;
  ui->cursor.j = s->grid.height/2;
  int i, j;
  for (i=0; i<s->grid.width; ++i) {
    for (j=0; j<s->grid.height; ++j) {
      if (s->grid.tiles[i][j].units[s->controlled][citizen] > 
          s->grid.tiles[ui->cursor.i][ui->cursor.j].units[s->controlled][citizen]) {
        ui->cursor.i = i;
        ui->cursor.j = j;
      }
    }
  }
  /* find the leftmost visible tile */
  ui->xskip = MAX_WIDTH * 2 + 1; 
  for(i=0; i<s->grid.width; ++i)
    for(j=0; j<s->grid.height; ++j)
      if(is_visible(s->grid.tiles[i][j].cl)) {
        int x = i*2 + j;
        if (ui->xskip > x)
          ui->xskip = x;
      }
  ui->xskip = ui->xskip/2;
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

  /* increment time */
  s->time++;

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


void update_timeline(struct state *s) {
  /* shortcut notation */
  struct timeline *t = &s->timeline;
  int p, i, j;

  /* prepare to update */
  if (t->mark+1 < MAX_TIMELINE_MARK)
    t->mark += 1; 
  else {
    /* shift all recorded data to empty space */
    for(i=0; i<MAX_TIMELINE_MARK-1; ++i) {
      t->time[i] = t->time[i+1];
      for(p=0; p<MAX_PLAYER; ++p) {
        t->data[p][i] = t->data[p][i+1];
      }
    }
  }
  
  /* insert a new datapoint at position t->mark */
  t->time[t->mark] = s->time;
  /* we compute total population here */
  for (p=0; p<MAX_PLAYER; ++p) {
    int count = 0;
    for (i=0; i<MAX_WIDTH; ++i) {
      for (j=0; j<MAX_HEIGHT; ++j) {
        count += s->grid.tiles[i][j].units[p][citizen];
      }
    }
    t->data[p][t->mark] = (float)count; 
  }
  /* call output_timeline to print it out */
}
