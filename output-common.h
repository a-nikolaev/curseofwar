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

#ifndef _OUTPUT_COMMON_H
#define _OUTPUT_COMMON_H

/* 
  time_to_ymd(time, &y, &m, &d)
    updates arguments y, m, d = year, month, day respectively
 */
void time_to_ymd(unsigned long time, int *y, int *m, int *d);

/*
  pop_to_symbol(pop)
    returns 0-8 for non-zero valid population (1-499)
      0 is the smallest (1-3)
      ...
      8 is the largest population (401-499)
    returns -1 for 0 or less
 */

int pop_to_symbol(int pop);

#endif
