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
#include "output.h"
#include <curses.h>
#include <string.h>

/* Macros to map tiles location (i,j) to its position on the screen */
#define POSY(ui,i,j) ((j)+1)
#define POSX(ui,i,j) ((i)*4 + (j)*2 + 1) - (ui->xskip*(CELL_STR_LEN+1))

/* Returns a color_pair number for a player p */
int player_color(int p) {
  switch(p) {
    case 0: return 6; // Neutral 
    case 1: return 4; // Green (player)
    case 2: return 5; // Blue
    case 3: return 3; // Red
    case 4: return 6; // Yellow
    case 5: return 7; // Magenta
    case 6: return 8; // Cyan
    case 7: return 2; // Black
    default: return -1;
  }
}

/* Returns output attributes for a player p */
int player_style(int p) {
 if (p != NEUTRAL) 
   return A_BOLD | COLOR_PAIR(player_color(p));
 else
   return A_NORMAL | COLOR_PAIR(player_color(p));
}

/* Outputs population at the tile (i,j) */
void output_units(struct ui *ui, struct tile *t, int i, int j) {
  int p;
  int num = 0;
  for(p=0; p<MAX_PLAYER; ++p) {
    num += t->units[p][citizen];
  }
  move(POSY(ui,i,j), POSX(ui,i,j));
  if (num > 400) 
    addstr(":::"); 
  else if (num > 200) 
    addstr(".::"); 
  else if (num > 100) 
    addstr(" ::"); 
  else if (num > 50) 
    addstr(".:."); 
  else if (num > 25) 
    addstr(".: "); 
  else if (num > 12) 
    addstr(" : "); 
  else if (num > 6) 
    addstr("..."); 
  else if (num > 3) 
    addstr(".. "); 
  else if (num > 0) 
    addstr(" . "); 
}

/* A function to display information about the available keys that can be used by the player */
void output_key (int y, int x, char *key, int key_style, char *s, int s_style) {
  int keylen = strlen(key);
  attrset(key_style);
  mvaddstr(y, x+1, key);
  attrset(s_style);
  mvaddch(y, x, '['); 
  mvaddch(y, x+keylen+1, ']'); 
  mvaddstr(y, x+keylen+3, s); 
}

/* the main output function */
void output_grid(struct state *st, struct ui *ui, int ktime) {
  int i, j;
  for (i=0; i<st->grid.width; ++i) {
    for (j=0; j<st->grid.height; ++j) {

      move(POSY(ui,i,j), POSX(ui,i,j)-1);
      switch (st->grid.tiles[i][j].cl) {
        case mountain: 
          attrset(A_NORMAL | COLOR_PAIR(4));
          addstr(" /\\^ "); 
          break;
        case mine: 
          attrset(A_NORMAL | COLOR_PAIR(4));
          addstr(" /$\\ "); 
          move(POSY(ui,i,j), POSX(ui,i,j)+1);
          if (st->grid.tiles[i][j].pl != NEUTRAL) 
            attrset(A_BOLD | COLOR_PAIR(6));
          else
            attrset(A_NORMAL | COLOR_PAIR(6));
          addstr("$"); 
          break;
        /*
        case abyss: 
          attrset(A_NORMAL | COLOR_PAIR(6));
          addstr("  %  "); 
          break;
         */
        case grassland: 
          attrset(A_NORMAL | COLOR_PAIR(4));
          addstr("  -  "); 
          break;
        case village: 
          attrset(player_style(st->grid.tiles[i][j].pl));
          addstr("  n  "); 
          break;
        case town: 
          attrset(player_style(st->grid.tiles[i][j].pl));
          addstr(" i=i "); 
          break;
        case castle: 
          attrset(player_style(st->grid.tiles[i][j].pl));
          addstr(" W#W "); 
          break;
        default:;
      }
      attrset(A_NORMAL | COLOR_PAIR(1));
     
      if (st->grid.tiles[i][j].cl == grassland) {
        attrset(player_style(st->grid.tiles[i][j].pl));
        output_units(ui, &st->grid.tiles[i][j], i, j);
        attrset(A_NORMAL | COLOR_PAIR(1));
      }

      int p;
      for (p=0; p<MAX_PLAYER; ++p) {
        if (p != st->controlled) {
          if (st->fg[p].flag[i][j] != 0 && ((ktime + p) / 5)%10 < 10) {
            attrset(player_style(p));
            mvaddch(POSY(ui,i,j), POSX(ui,i,j), 'x');
            attrset(A_NORMAL | COLOR_PAIR(1));
          }
        }
      }

      /* for of war */
      int k;
      int b = 0;
      for(k=0; k<DIRECTIONS; ++k) {
        int di = dirs[k].i;
        int dj = dirs[k].j;
        if( i+di >= 0 && i+di < st->grid.width && 
            j+dj >= 0 && j+dj < st->grid.height &&
            (st->grid.tiles[i+di][j+dj].units[st->controlled][citizen] > 0) ) {
          b = 1;
          break;
        }
      }
      if (false && !b){
        move(POSY(ui,i,j), POSX(ui,i,j)-1);
        addstr("     "); 
      }
      // player 1 flags
      if (st->fg[st->controlled].flag[i][j] != 0 && (ktime / 5)%10 < 10) {
        attrset(A_BOLD | COLOR_PAIR(1));
        mvaddch(POSY(ui,i,j), POSX(ui,i,j)+2, 'P');
        attrset(A_NORMAL | COLOR_PAIR(1));
      }

    }
  }
  
  i = ui->cursor.i;
  j = ui->cursor.j;
  attrset(A_BOLD | COLOR_PAIR(1));
  mvaddch(POSY(ui,i,j), POSX(ui,i,j)-1, '(');
  mvaddch(POSY(ui,i+1,j), POSX(ui,i+1,j)-1, ')');
  attrset(A_NORMAL | COLOR_PAIR(1));

  /* print populations at the cursor */
  int p;
  char buf[32];
  int y = POSY(ui,0, st->grid.height) + 1;
  mvaddstr(y, 0, " Gold:");
  sprintf(buf, "%li    ", st->country[st->controlled].gold);
  attrset(player_style(st->controlled));
  mvaddstr(y, 8, buf);
  mvaddstr(y, 8, buf);
  attrset(A_NORMAL | COLOR_PAIR(1));
  mvaddstr(y+1, 0, " Prices: 150, 300, 600.");
  
  move(y+2, 1);
  addstr("Speed: ");
  attrset(player_style(st->controlled));
  switch(st->speed){
    case sp_fastest: addstr("Fastest"); break;
    case sp_faster:  addstr("Faster "); break;
    case sp_fast:    addstr("Fast   "); break;
    case sp_normal:  addstr("Normal "); break;
    case sp_slow:    addstr("Slow   "); break;
    case sp_slower:  addstr("Slower "); break;
    case sp_slowest: addstr("Slowest"); break;
    case sp_pause:   addstr("Pause  "); break;
  }
  
  attrset(A_BOLD | COLOR_PAIR(1));

  int text_style = A_NORMAL | COLOR_PAIR(1);
  int key_style = player_style(st->controlled);
  
  output_key (y+4, 1, "Space", key_style, "add/remove a flag", text_style);
  output_key (y+5, 1, "R or V", key_style, "build", text_style);
  
  output_key (y+4, 30, "X", key_style, "remove all flags", text_style);
  output_key (y+5, 30, "C", key_style, "remove 50\% of flags", text_style);
  
  output_key (y+5, 57, "S", key_style, "slow down", text_style);
  output_key (y+4, 57, "F", key_style, "speed up", text_style);
  output_key (y+6, 57, "P", key_style, "pause", text_style);
  
  mvaddstr(y+1, 30, " Population at the cursor:");
  for(p=0; p<MAX_PLAYER; ++p) {
    if (p == NEUTRAL) continue;
    attrset(player_style(p));
    sprintf(buf, "%3i", st->grid.tiles[i][j].units[p][citizen]);
    mvaddstr(y+2, 30 + p*5, buf);
    /*
    sprintf(buf, "%10li", st->country[p].gold);
    mvaddstr(y+2, 10 + p*12, buf);
    */
  }
  attrset(A_NORMAL | COLOR_PAIR(1));
  
  mvaddstr(y, 65, "Date:");
  attrset(key_style);
  int year = st->time/360;
  int month = st->time - year*360;
  int day = month%30 + 1;
  month = month / 30 + 1;
  sprintf(buf, "%i-%02i-%02i ", year, month, day);
  mvaddstr(y, 72, buf);

  refresh();
}

void output_dialog_quit_on(struct state *st, struct ui *ui) {
  int y = POSY(ui,st->grid.width/2, st->grid.height/2) ;
  int x = POSX(ui,st->grid.width/2, st->grid.height/2) - 8;
  
  attrset(A_NORMAL | COLOR_PAIR(1));
  mvaddstr(y-2, x, "                 ");
  mvaddstr(y-1, x, "   Quit? [Y/N]   ");
  mvaddstr(y+0, x, "        [Q/Esc]  ");
  mvaddstr(y+1, x, "                 ");
  
  int text_style = A_NORMAL | COLOR_PAIR(1);
  int key_style = player_style(st->controlled);

  output_key (y-1, x+9, "Y/N", key_style, "", text_style);
  /* output_key (y+0, x+8, "Q/Esc", key_style, "", text_style); */
  
  attrset(A_NORMAL | COLOR_PAIR(1));
  refresh();
}

void output_dialog_quit_off(struct state *st, struct ui *ui) {
  int y = POSY(ui,st->grid.width/2, st->grid.height/2);
  int x = POSX(ui,st->grid.width/2, st->grid.height/2) - 8;
  mvaddstr(y-1, x, "                 ");
  mvaddstr(y+0, x, "                 ");
  refresh();
}
