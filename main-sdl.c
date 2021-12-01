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
#include <time.h>
#include "SDL.h"

#ifdef WIN32
#undef main
#endif

#include "grid.h"
#include "common.h"
#include "state.h"
#include "output-sdl.h"
#include "main-common.h"
#include "path.h"

/* no networking in Windows */
#ifndef WIN32
#include <fcntl.h>
#include "messaging.h"
#include "client.h"
#include "server.h"
#endif

/* delay in milliseconds */
#define TIME_DELAY 10

static int in_fullscreen;
static void toggle_fullscreen(SDL_Surface *surface) {
	in_fullscreen = !in_fullscreen;
	SDL_WM_ToggleFullScreen(surface);
}

void run(struct state *st, struct ui *ui, 
    SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *uisurf){
  /* tile variation */
  int tile_variant[MAX_WIDTH][MAX_HEIGHT];
  int pop_variant[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for (i=0; i<MAX_WIDTH; ++i)
    for (j=0; j<MAX_HEIGHT; ++j) {
      tile_variant[i][j] = rand();
      pop_variant[i][j] = rand();
    }

  int finished = 0;
  SDL_Event event;
  int previous_time = SDL_GetTicks();

  int k = 0;
  while (!finished){

    int time = SDL_GetTicks();

    if (time - previous_time >= TIME_DELAY) {

      previous_time = previous_time + TIME_DELAY;
      k++;
      if (k>=1600) k=0;

      char c = '\0';
      while (!finished && SDL_PollEvent(&event)){
        switch (event.type){
          case SDL_KEYDOWN:
            c = '\0';
            switch (event.key.keysym.sym) {
              case SDLK_RETURN:
                if((event.key.keysym.mod & KMOD_LALT) ||
                   (event.key.keysym.mod & KMOD_RALT))
                    toggle_fullscreen(screen);
                break;
              case SDLK_LEFT: c = 'h'; break;
              case SDLK_RIGHT: c = 'l'; break;
              case SDLK_UP: c = 'k'; break;
              case SDLK_DOWN: c = 'j'; break;
              case SDLK_q: c = 'q'; break;
              case SDLK_SPACE: c = ' '; break;
              default:
                if ( (event.key.keysym.unicode & 0xFF80) == 0 ) {
                  c = event.key.keysym.unicode & 0x7F;
                }
            }
            finished = singleplayer_process_input(st, ui, c);
            break;
        }
      }

      int slowdown = game_slowdown(st->speed);
      if (k % slowdown == 0 && st->speed != sp_pause) { 
        kings_move(st);
        simulate(st);
      }

      if (k % 5 == 0) {
        output_sdl(tileset, typeface, uisurf, screen, st, ui, tile_variant, pop_variant, k);
        SDL_Flip(screen);
      }
    }
    else{
      SDL_Delay(TIME_DELAY/2);
    }
  }
}

#ifndef WIN32
/* Client version */
void run_client(struct state *st, struct ui *ui, 
    SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *uisurf,
    char *s_server_addr, char *s_server_port, char *s_client_port){
  /* tile variation */
  int tile_variant[MAX_WIDTH][MAX_HEIGHT];
  int pop_variant[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for (i=0; i<MAX_WIDTH; ++i)
    for (j=0; j<MAX_HEIGHT; ++j) {
      tile_variant[i][j] = rand();
      pop_variant[i][j] = rand();
    }

  /* Init Networking */
  int sfd; /* file descriptor of the socket */
  int ret_code; /* code returned by a function */
  struct addrinfo srv_addr;
  if ((ret_code = client_init_session 
        (&sfd, s_client_port, &srv_addr, s_server_addr, s_server_port)) != 0) {
    perror("Failed to initialize networking");
    return;
  }
  fcntl(sfd, F_SETFL, O_NONBLOCK);
  /* */

  /* Starting the main loop */
  int finished = 0;
  SDL_Event event;
  int previous_time = SDL_GetTicks();

  int k = 0;
  int initialized = 0;
  st->time = 0;
  while (!finished){

    int time = SDL_GetTicks();

    if (time - previous_time >= TIME_DELAY) {

      previous_time = previous_time + TIME_DELAY;
      k++;
      if (k>=1600) k=0;

      char c = '\0';
      while (!finished && SDL_PollEvent(&event)){
        switch (event.type){
          case SDL_KEYDOWN:
            c = '\0';
            switch (event.key.keysym.sym) {
              case SDLK_RETURN:
                if((event.key.keysym.mod & KMOD_LALT) ||
                   (event.key.keysym.mod & KMOD_RALT))
                    toggle_fullscreen(screen);
                break;
              case SDLK_LEFT: c = 'h'; break;
              case SDLK_RIGHT: c = 'l'; break;
              case SDLK_UP: c = 'k'; break;
              case SDLK_DOWN: c = 'j'; break;
              case SDLK_q: c = 'q'; break;
              case SDLK_SPACE: c = ' '; break;
              default:
                if ( (event.key.keysym.unicode & 0xFF80) == 0 ) {
                  c = event.key.keysym.unicode & 0x7F;
                }
            }
            /* finished = singleplayer_process_input(st, ui, c); */
            finished = client_process_input (st, ui, c, sfd, &srv_addr);
            /* */
            break;
        }
      }
      
      /* Send Keep Alive & receive packets */
      if (k%50 == 0) 
        send_msg_c (sfd, &srv_addr, MSG_C_IS_ALIVE, 0, 0, 0);
      
      int msg = client_receive_msg_s (sfd, st);
      if (msg == MSG_S_STATE && initialized != 1) {
        initialized = 1;
        ui_init(st, ui);
      
        /* reset screen */
        int screen_width = (ui->xlength + 2) * TILE_WIDTH;
        int screen_height = (st->grid.height + 3) * TILE_HEIGHT + TYPE_HEIGHT*5;
        screen_width = MAX (screen_width, TILE_WIDTH + 75*TYPE_WIDTH + TILE_WIDTH);
        SDL_FreeSurface(screen);
        screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_DOUBLEBUF);
        if (screen == NULL) {
          fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
          exit(1);
        }
        /* */
      }
      /* */
      
      /* no simulation in the client */
      /*
      int slowdown = game_slowdown(st->speed);
      if (k % slowdown == 0 && st->speed != sp_pause) { 
        kings_move(st);
        simulate(st);
      }
      */

      if (initialized) {
        if (k % 5 == 0) {
          output_sdl(tileset, typeface, uisurf, screen, st, ui, tile_variant, pop_variant, k);
          SDL_Flip(screen);
        }
      }

    }
    else{
      SDL_Delay(TIME_DELAY/2);
    }
  }
}
#endif

int main(int argc, char *argv[]) {

  srand(time(NULL));

  /* Read command line arguments */
  struct basic_options op;
  struct multi_options mop;
  if (get_options(argc, argv, &op, &mop) == 1) return 1;

  /* Initialize the state */
  struct state st;
  struct ui ui;

  state_init(&st, &op, &mop);
  ui_init(&st, &ui);

  int screen_width = (ui.xlength + 2) * TILE_WIDTH;
  int screen_height = (st.grid.height + 3) * TILE_HEIGHT + TYPE_HEIGHT*5;
  screen_width = MAX (screen_width, TILE_WIDTH + 75*TYPE_WIDTH + TILE_WIDTH);

  /* Init SDL */
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  SDL_Surface *screen;
  screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_DOUBLEBUF);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(300, 30);

  /* Load Data */
  char **path;
  path = get_search_paths();
  
  /* Images */
  char *filename;
  
  SDL_Surface *tileset;
  filename = find_file (path, "images/tileset.bmp");
  if ( load_image(filename, &tileset) != 0 ) return 1;
  free(filename); 
  
  SDL_Surface *typeface;
  filename = find_file (path, "images/type.bmp");
  if ( load_image(filename, &typeface) != 0 ) return 1;
  free(filename);
  
  SDL_Surface *uisurf;
  filename = find_file (path, "images/ui.bmp");
  if ( load_image(filename, &uisurf) != 0 ) return 1;
  free(filename);

#ifdef WIN32 
  /* Run the game */
  run(&st, &ui, screen, tileset, typeface, uisurf);
#else
  if (!mop.multiplayer_flag) {
    /* Run the game */
    run(&st, &ui, screen, tileset, typeface, uisurf);
  }
  else {
    if (mop.server_flag)  {
      fprintf(stderr, "\nPlease, run the ncurses executable to start a server.\n\n");
      exit(1);
    }
    else run_client(&st, &ui, screen, tileset, typeface, uisurf, mop.val_server_addr, mop.val_server_port, mop.val_client_port);
  }
#endif

  if(in_fullscreen) toggle_fullscreen(screen);

  /* Finalize */
  SDL_FreeSurface(tileset);
  SDL_FreeSurface(typeface);
  SDL_FreeSurface(uisurf);
  
  if (!mop.multiplayer_flag || mop.server_flag)
    printf ("Random seed was %i\n", st.map_seed);

  destroy_search_paths(path);
  free(mop.val_client_port);
  free(mop.val_server_addr);
  free(mop.val_server_port);
  return 0;
}
