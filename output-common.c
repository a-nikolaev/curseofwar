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

void time_to_ymd(unsigned long time, int *y, int *m, int *d) {
  int year = time/360;
  int month = time - year*360;
  int day = month%30 + 1;
  month = month / 30 + 1;

  *y = year;
  *m = month;
  *d = day;
}

int pop_to_symbol(int num) {
  if (num > 400) 
    return 8; 
  else if (num > 200) 
    return 7; 
  else if (num > 100) 
    return 6; 
  else if (num > 50) 
    return 5; 
  else if (num > 25) 
    return 4; 
  else if (num > 12) 
    return 3; 
  else if (num > 6) 
    return 2; 
  else if (num > 3) 
    return 1; 
  else if (num > 0) 
    return 0;
  return -1;
}
