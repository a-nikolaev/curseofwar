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

#ifndef _PATH_H
#define _PATH_H

/* Creates NULL terminater list of paths to search for game data files, allocates memory! */
char **get_search_paths();

/* Free alocated memory */
void destroy_search_paths(char** path);

/* Returns a string with found path, allocates memory */
char *find_file(char** path, char* suffix);

#endif
