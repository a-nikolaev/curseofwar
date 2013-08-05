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

#ifndef _OUTPUT_SDL_H
#define _OUTPUT_SDL_H

#include "common.h"
#include "state.h"
#include "SDL.h"

#define TILE_WIDTH 32
#define TILE_HEIGHT 16

#define TYPE_WIDTH 9
#define TYPE_HEIGHT 15
#define TYPE_LINE_LENGTH 32
#define TYPE_FIRST 33

Uint32 getpixel(SDL_Surface *surface, int x, int y);

/*
   load_image (filename, &surface_ptr)
    Load BMP image, returns 0 on success, or 1 if fails.
 */
int load_image(char *filename, SDL_Surface **image);

void blit_subpic(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj);

void blit_subpic_noise(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj, int rnd_variant);

void blit_subpic_2h(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj);

/* Text output functions */
void output_char(SDL_Surface *typeface, SDL_Surface *screen, int c, int dsti, int dstj);
void output_string(SDL_Surface *typeface, SDL_Surface *screen, char *str, int dsti, int dstj);

void output_string_alt(SDL_Surface *typeface, int player, SDL_Surface *screen, char *str, int dsti, int dstj);

/* Render everything */
void output_sdl (SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *screen, struct state *s, struct ui *ui,
    int variant[MAX_WIDTH][MAX_HEIGHT], int pop_variant[MAX_WIDTH][MAX_HEIGHT], int ktime);

#endif
