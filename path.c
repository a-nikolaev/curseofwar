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

#include <stdlib.h>
#include <string.h>

#ifndef WIN32
# include <sys/stat.h>
#endif

/* allocates memory */
char *concat(char *a, char *b) {
  char *res = (char*) malloc (sizeof(char) * (strlen(a) + strlen(b) + 1));
  strcpy(res, a);
  strcat(res, b);
  return res;
}

/* allocates memory */
char *get_xdg_data_home() {
  char *s;
  if ((s = getenv("XDG_DATA_HOME")) != NULL) {
    return strdup(s);
  }
  else if ((s = getenv("HOME")) != NULL) {
    char *suffix = "/.local/share";
    return concat (s, suffix);
  }
  else {
    char *a = concat ("/home/", getenv("USER"));
    char *b = concat (a, "/.local/share");
    free(a);
    return b;
  }
}

/* Create an array of possible paths to the game data 

   Allocates memory!
  
   1) "."
   2) $XDG_DATA_HOME/curseofwar/
      Defaults to ~/.local/share/curwseofwar/
   3) ~/.curseofwar/
   4) /usr/local/share/curseofwar/
   5) /usr/share/curseofwar/
   6) /usr/share/curseofwar-sdl/
   7) /usr/share/curseofwar-common/
 */
char **get_search_paths() {
  char *suffix = "/curseofwar/";

#ifndef WIN32
  int dirs_num = 8;
  char **path = (char**) malloc(sizeof(char*) * (dirs_num + 1));
  
  path[0] = strdup("");
  char *s;
  s = get_xdg_data_home();
  path[1] = concat(s, suffix);
  free(s);
  path[2] = concat(getenv("HOME"), "/.curseofwar/");
  path[3] = strdup("/usr/local/share/curseofwar/");
  path[4] = strdup("/usr/share/curseofwar/");
  path[5] = strdup("/usr/share/curseofwar-sdl/");
  path[6] = strdup("/usr/share/curseofwar-common/");
  path[7] = strdup(INSTALL_DATA);
#else
  int dirs_num = 1;
  char **path = (char**) malloc(sizeof(char*) * (dirs_num + 1));
  path[0] = strdup("");
#endif
  
  path[dirs_num] = NULL;

  return path;
}

void destroy_search_paths(char** path) {
  int i=0;
  while (path[i] != NULL) {
    free(path[i]);
    ++i;
  }
  free(path);
}

/* allocates memory */
char *find_file(char** path, char* suffix) {
  char *res;

#ifndef WIN32
  struct stat sb;
  int i = 0;
  while (path[i] != NULL) {
    res = concat(path[i], suffix);
    if ( stat(res, &sb) == 0 && S_ISREG(sb.st_mode) ) 
      return res;
    else
      free(res);
    ++i;
  }
#else
  /* in Windows, simply return the first path + suffix */
  return concat(path[0], suffix);
#endif

  return NULL;
}
