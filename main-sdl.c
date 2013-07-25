#include <stdlib.h>
#include <time.h>
#include "SDL.h"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 480 

#include "grid.h"
#include "common.h"

#define TILE_WIDTH 32
#define TILE_HEIGHT 16
void blit_subpic(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj) {
  static SDL_Rect src, dest;
   
  src.x = srci * TILE_WIDTH;
  src.y = srcj * TILE_HEIGHT;
  src.w = TILE_WIDTH;
  src.h = TILE_HEIGHT;
   
  dest.x = dsti * TILE_WIDTH + dstj*(TILE_WIDTH/2);
  dest.y = dstj * TILE_HEIGHT;
  dest.w = TILE_WIDTH;
  dest.h = TILE_HEIGHT;
   
  SDL_BlitSurface(src_surf, &src, dst_surf, &dest);
}

/* double height tile */
void blit_subpic_2h(SDL_Surface *src_surf, SDL_Surface *dst_surf, int srci, int srcj, int dsti, int dstj) {
  static SDL_Rect src, dest;
   
  src.x = srci * TILE_WIDTH;
  src.y = srcj * TILE_HEIGHT - TILE_HEIGHT;
  src.w = TILE_WIDTH;
  src.h = TILE_HEIGHT*2;
   
  dest.x = dsti * TILE_WIDTH + dstj*(TILE_WIDTH/2);
  dest.y = dstj * TILE_HEIGHT - TILE_HEIGHT;
  dest.w = TILE_WIDTH;
  dest.h = TILE_HEIGHT*2;
   
  SDL_BlitSurface(src_surf, &src, dst_surf, &dest);
}

void output_sdl (SDL_Surface *tileset, SDL_Surface *screen, struct grid *g, int variant[MAX_WIDTH][MAX_HEIGHT]) {
  int i,j;
  int srci=0, srcj=0;
  for (j=0; j<g->height; ++j) {
    for (i=0; i<g->width; ++i) {
      
      switch (g->tiles[i][j].cl) {
        /* case abyss: break; */
        default:
          /* draw grass */
          blit_subpic (tileset, screen, 0 + variant[i][j]%6, 0 + variant[i][j]/6%3, i, j+1);
          /* draw everything else */
          switch (g->tiles[i][j].cl) {
            case village: 
              srci=0; srcj=7; break;
            case town: 
              srci=1; srcj=7; break;
            case castle: 
              srci=2; srcj=7; break;
            case mountain: case mine:
              srci=0 + variant[i][j]%4; 
              srcj=5; break;
            case grassland:
              if (variant[i][j] % 2 == 0) continue;
              srci = 0 + (variant[i][j]/2)%9;
              srcj = 8;
              blit_subpic (tileset, screen, srci, srcj, i, j+1);
              continue;
            default:
              continue;
          }
          blit_subpic_2h (tileset, screen, srci, srcj, i, j+1);
          if (g->tiles[i][j].cl == mine) {
            blit_subpic_2h (tileset, screen, 5, 5, i, j+1);
          }
      }
    }
  }
}

Uint32 getpixel(SDL_Surface *surface, int x, int y) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      return *p;
      break;

    case 2:
      return *(Uint16 *)p;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;
      break;

    case 4:
      return *(Uint32 *)p;
      break;

    default:
      return 0;       /* shouldn't happen, but avoids warnings */
  }
}


int main(int argc, char *argv[]) {

  srand(time(NULL));

  if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  SDL_Surface *screen;
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_DOUBLEBUF);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
    exit(1);
  }

  /* Load Image */
  SDL_Surface *image;
  SDL_Surface *temp;
   
  temp = SDL_LoadBMP("../draw/tileset.bmp");
  if (temp == NULL) {
      printf("Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
  }

  SDL_LockSurface(temp);
  Uint32 colorkey = getpixel(temp, temp->w-1, temp->h-1);
  SDL_UnlockSurface(temp);
  colorkey = SDL_MapRGB(temp->format, 0, 255, 255);

  if (SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, colorkey) == -1) {
    fprintf(stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError());
  }
   
  image = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);

  /*
  SDL_Rect src, dest;
   
  src.x = 0;
  src.y = 0;
  src.w = image->w;
  src.h = image->h;
   
  dest.x = 100;
  dest.y = 100;
  dest.w = image->w;
  dest.h = image->h;
   
  SDL_BlitSurface(image, &src, screen, &dest);
  SDL_Flip(screen);
  */

  /* make a map */
  struct grid g;
  grid_init(&g, 20, 20);
  int tile_variant[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for (i=0; i<MAX_WIDTH; ++i)
    for (j=0; j<MAX_HEIGHT; ++j)
        tile_variant[i][j] = rand();

  int done = 0;
  SDL_Event event;
  while (!done){
    while (!done && SDL_PollEvent(&event)){
      switch (event.type){
        case SDL_KEYDOWN:
          done = 1;
          break;
      }
    }
    
    output_sdl(image, screen, &g, tile_variant);

    SDL_Flip(screen);
    SDL_Delay(10);
  }

  SDL_FreeSurface(image);
  return 0;
}
