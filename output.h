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
#ifndef _OUTPUT_H
#define _OUTPUT_H

#define CELL_STR_LEN 3

#include "grid.h"
#include "state.h"

/*
  output_grid(st, ktime)

    the main function to update the screen,
    st is the current game state,
    ktime is the timestep (can be used for animation primarily)
*/
void output_grid(struct state *st, struct ui *ui, int ktime);

void output_dialog_quit_on(struct state *st, struct ui *ui);
void output_dialog_quit_off(struct state *st, struct ui *ui);

#endif

