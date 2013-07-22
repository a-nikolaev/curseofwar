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
#include <unistd.h>
#include <stdio.h>
#include <curses.h>
#include <locale.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

#include "common.h"
#include "grid.h"
#include "state.h"
#include "output.h"
#include "messaging.h"
#include "client.h"
#include "server.h"

/*****************************************************************************/
/*                           Global Constants                                */
/*****************************************************************************/

volatile   sig_atomic_t   input_ready;
volatile   sig_atomic_t   time_to_redraw;
 
#define DEF_SERVER_ADDR "127.0.0.1"
#define DEF_SERVER_PORT "19140"
#define DEF_CLIENT_PORT "19150"

void on_timer(int signum);   /* handler for alarm     */
int  update_from_input( struct state *st, struct ui *ui );
int  update_from_input_client( struct state *st, struct ui *ui, int sfd, struct addrinfo *srv_addr);
int  update_from_input_server( struct state *st );

void print_help() {
  printf(
"                                 __                      \n"
"    ____                        /  ]                     \n"
"   / __ \\_ _ ___ ___ ___    __ _| |_  /\\      /\\___ ___  \n"
" _/ /  \\/ | |X _/ __/ __\\  /   \\   /  \\ \\ /\\ / /__ \\X _/ \n"
" \\ X    | | | | |__ | __X  | X || |    \\ V  V // _ | |   \n"
"  \\ \\__/\\ __X_| \\___/___/  \\___/| |     \\ /\\ / \\___X_|   \n"
"   \\____/                       |/       V  V            \n"
"\n"
"  Written by Alexey Nikolaev in 2013.\n"
"\n");
  
  printf("  Command line arguments:\n\n");
  printf(
    "-W width\n"
    "\tMap width (default is 21)\n\n"
    "-H height\n"
    "\tMap height (default is 21)\n\n"
    "-S [rhombus|rect|hex]\n"
    "\tMap shape (rectangle is default). Max number of countries N=4 for rhombus and rectangle, and N=6 for the hexagon.\n\n"
    "-l [2|3| ... N]\n"
    "\tSets L, the number of countries (default is N).\n\n"
    "-i [0|1|2|3|4]\n"
    "\tInequality between the countries (0 is the lowest, 4 in the highest).\n\n"
    "-q [1|2| ... L]\n"
    "\tChoose player's location by its quality (1 = the best available on the map, L = the worst). Only in the singleplayer mode.\n\n"
    "-r\n"
    "\tAbsolutely random initial conditions, overrides options -l, -i, and -q.\n\n"
    "-d [ee|e|n|h|hh]\n"
    "\tDifficulty level (AI) from the easiest to the hardest (default is normal).\n\n"
    "-s [p|sss|ss|s|n|f|ff|fff]\n"
    "\tGame speed from the slowest to the fastest (default is normal).\n\n"
    "-R seed\n"
    "\tSpecify a random seed (unsigned integer) for map generation.\n\n"
    "-T\n"
    "\tShow the timeline.\n\n"
    "-E [1|2| ... L]\n"
    "\tStart a server for not more than L clients.\n\n"
    "-e port\n"
    "\tServer's port (19140 is default).\n\n"
    "-C IP\n"
    "\tStart a client and connect to the provided server's IP-address.\n\n"
    "-c port\n"
    "\tClients's port (19150 is default).\n\n"
    "-h\n"
    "\tDisplay this help \n\n"
  );
}

void win_or_lose(struct state *st, int k) {
  int i, j, p;
  int pop[MAX_PLAYER];
  for (p=0; p<MAX_PLAYER; ++p) pop[p] = 0;

  for(i=0; i<st->grid.width; ++i){
    for(j=0; j<st->grid.height; ++j){
      if(is_inhabitable(st->grid.tiles[i][j].cl)) {
        for(p=0; p<MAX_PLAYER; ++p){
          pop[p] += st->grid.tiles[i][j].units[p][citizen];
        }
      }
    }
  }

  int win = 1;
  int lose = 0;
  int best = 0;
  for(p=0; p<MAX_PLAYER; ++p){
    if(pop[best] < pop[p]) best = p;
    if(p!=st->controlled && pop[p]>0) win = 0;
  }
  if(pop[st->controlled] == 0) lose = 1;

  if (win) {
    attrset(A_BOLD | COLOR_PAIR(4));
    mvaddstr(2 + st->grid.height, 31, "You are victorious!");
  }
  else if (lose) {
    attrset(A_BOLD | COLOR_PAIR(2));
    mvaddstr(2 + st->grid.height, 31, "You are defeated!");
  }
}

int game_slowdown (int speed) {
  int slowdown = 20;
  switch (speed) {
    case sp_pause: slowdown = 1; break;
    case sp_slowest: slowdown = 160; break;
    case sp_slower: slowdown = 80; break;
    case sp_slow: slowdown = 40; break;
    case sp_normal: slowdown = 20; break;
    case sp_fast: slowdown = 10; break;
    case sp_faster: slowdown = 5; break;
    case sp_fastest: slowdown = 2; break;
  }
  return slowdown;
}

/* Run the game */
void run (struct state *st, struct ui *ui) {
  int k = 0;
  int finished = 0;
  while( !finished ) {
    if (time_to_redraw) {
      k++;
      if (k>=1600) k=0;
      
      int slowdown = game_slowdown(st->speed);
      if (k % slowdown == 0 && st->speed != sp_pause) { 
        kings_move(st);
        simulate(st);
        if (st->show_timeline) {
          if (st->time%10 == 0)
            update_timeline(st);
        }
      }
      output_grid(st, ui, k);
      if (st->show_timeline) {
        if (st->time%10 == 0)
          output_timeline(st, ui);
      }
      time_to_redraw = 0;

      if (k%100 == 0) win_or_lose(st, k);
    }
    finished = update_from_input(st, ui);
    pause(); // sleep until woken up by SIGALRM
  }
}

/* run client */
void run_client (struct state *st, struct ui *ui, char *s_server_addr, char *s_server_port, char *s_client_port) {
  int sfd; /* file descriptor of the socket */

  int ret_code; /* code returned by a function */
  
  struct addrinfo srv_addr;

  if ((ret_code = client_init_session 
        (&sfd, s_client_port, &srv_addr, s_server_addr, s_server_port)) != 0) {
    perror("Failed to initialize networking");
    return;
  }
  /* non-blocking mode */
  fcntl(sfd, F_SETFL, O_NONBLOCK);
 
  int k = 0;
  int finished = 0;
  int initialized = 0;
  st->time = 0;
  while( !finished ) {
    if (time_to_redraw) {
      k++;
      if (k>=1600) k=0;
      time_to_redraw = 0;
      
      if (k%50 == 0) 
        send_msg_c (sfd, &srv_addr, MSG_C_IS_ALIVE, 0, 0, 0);

      int msg = client_receive_msg_s (sfd, st);
      if (msg == MSG_S_STATE && initialized != 1) {
        initialized = 1;
        ui_init(st, ui);
      }
      if (initialized) {
        output_grid(st, ui, k);
      }
    }
    finished = update_from_input_client(st, ui, sfd, &srv_addr);
    pause();
  }

  close(sfd);
}

/* run server */
void run_server (struct state *st, int cl_num_need, char *s_server_port) {
  int sfd; /* file descriptor of the socket */
  struct sockaddr_storage peer_addr; /* address you receive a message from */
  socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
  ssize_t nread;

  uint8_t buf[MSG_BUF_SIZE]; /* message buffer */

  int ret_code; /* code returned by a function */
  
  if ((ret_code = server_init (&sfd, s_server_port)) != 0) {
    perror("Failed to initialize networking");
    return;
  }
  /* non-blocking mode */
  fcntl(sfd, F_SETFL, O_NONBLOCK);

  int cl_num = 0;
  struct client_record cl[MAX_CLIENT];
  enum server_mode mode = server_mode_lobby;
  int finished = 0;
  int k=0;
  while( !finished ) {
    if (time_to_redraw) {
      switch(mode){
        case server_mode_lobby:
          /* Lobby */
          nread = recvfrom(sfd, buf, MSG_BUF_SIZE-1, 0,
              (struct sockaddr *) &peer_addr, &peer_addr_len);
          /* try to add a new client */
          if ( server_get_msg(buf, nread) > 0 ) {
            int i;
            int found = 0;
            for(i=0; i<cl_num; ++i) {
              found = found || sa_match(&peer_addr, &cl[i].sa);
            }
            if (!found && cl_num < cl_num_need) { /* add the new client */
              cl[cl_num].name = "Jim";
              cl[cl_num].id = cl_num;
              cl[cl_num].pl = cl_num + 1; /* must be non-zero */
              cl[cl_num].sa = peer_addr;
                
              addstr("!");
              refresh();
              cl_num++;
            }

            if (cl_num >= cl_num_need) { 
              mode = server_mode_play; 
              cl_num = cl_num_need; 
            }
          }
          break;
        case server_mode_play:
          /* Game */
          k++;
          if (k>=1600) k=0;
          int slowdown = game_slowdown(st->speed);
          if (k % slowdown == 0 && st->speed != sp_pause) { 
            kings_move(st);
            simulate(st);
            server_send_msg_s_state(sfd, cl, cl_num, st);
          }
          
          nread = recvfrom(sfd, buf, MSG_BUF_SIZE-1, 0,
              (struct sockaddr *) &peer_addr, &peer_addr_len);
          if (nread != -1) {
            int found_i = -1;
            int i;
            for(i=0; i<cl_num; ++i) {
              if (sa_match(&peer_addr, &cl[i].sa)) found_i = i;
            }
            if (found_i>-1) {
              int msg = server_process_msg_c(buf, nread, st, cl[found_i].pl);
              if (msg == MSG_C_IS_ALIVE) addstr(".");
              else addstr("+");
              refresh();
            }
          }
          break;
      }

      time_to_redraw = 0;
    }
    finished = update_from_input_server(st);
    pause();
  }
  close(sfd);
}

/*****************************************************************************/
/*                                    Main                                   */
/*****************************************************************************/
int main(int argc, char* argv[]){
  /* Initialize pseudo random number generator */
  srand(time(NULL));

  /* Parse argv */
  int r_flag = 0; // random
  int dif_val = dif_normal; // diffiulty
  int sp_val = sp_normal; // speed
  int w_val = 21; // width
  int h_val = 21; // height
  int l_val = 0;  // the number of starting locations
  unsigned int seed_val = rand();
  int conditions_val = 0;
  int conditions_were_set = 0;
  int timeline_flag = 0;

  int ineq_val = RANDOM_INEQUALITY;
  enum stencil shape_val = st_rect;

  /* multiplayer option */
  int multiplayer_flag = 0;
  int server_flag = 1;
  
  char* val_client_port = strdup(DEF_CLIENT_PORT);
  char* val_server_addr = strdup(DEF_SERVER_ADDR);
  char* val_server_port = strdup(DEF_SERVER_PORT);

  int val_clients_num = 1;

  opterr = 0;
  int c;
  while ((c = getopt (argc, argv, "hrTW:H:i:l:q:d:s:R:S:E:e:C:c:")) != -1){
    switch(c){
      case 'r': r_flag = 1; break;
      case 'T': timeline_flag = 1; break;
      //case 'f': f_val = optarg; break;
      case 'W': { char* endptr = NULL;
                  w_val = MAX(14, strtol(optarg, &endptr, 10));
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'H': { char* endptr = NULL;
                  h_val = MAX(14, strtol(optarg, &endptr, 10));
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'i': { char* endptr = NULL;
                  ineq_val = strtol(optarg, &endptr, 10);
                  if (*endptr != '\0' || ineq_val < 0 || ineq_val > 4) {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'l': { char* endptr = NULL;
                  l_val = strtol(optarg, &endptr, 10);
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'q': { char* endptr = NULL;
                  conditions_val = strtol(optarg, &endptr, 10);
                  conditions_were_set = 1;
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'R': { char* endptr = NULL;
                  seed_val = abs(strtol(optarg, &endptr, 10));
                  if (*endptr != '\0' || *optarg == '\0') {
                    print_help();
                    return 1;
                  }
                };
                break;
      case 'd': if (strcmp(optarg, "n") == 0) dif_val = dif_normal;
                else if (strcmp(optarg, "e") == 0) dif_val = dif_easy;
                else if (strcmp(optarg, "e1") == 0) dif_val = dif_easy;
                else if (strcmp(optarg, "ee") == 0) dif_val = dif_easiest;
                else if (strcmp(optarg, "e2") == 0) dif_val = dif_easiest;
                else if (strcmp(optarg, "h") == 0) dif_val = dif_hard;
                else if (strcmp(optarg, "h1") == 0) dif_val = dif_hard;
                else if (strcmp(optarg, "hh") == 0) dif_val = dif_hardest;
                else if (strcmp(optarg, "h2") == 0) dif_val = dif_hardest;
                else {
                  print_help();
                  return 1;
                }
                break;
      case 's': if (strcmp(optarg, "n") == 0) sp_val = sp_normal;
                else if (strcmp(optarg, "s") == 0 || strcmp(optarg, "s1") == 0) sp_val = sp_slow;
                else if (strcmp(optarg, "ss") == 0 || strcmp(optarg, "s2") == 0) sp_val = sp_slower;
                else if (strcmp(optarg, "sss") == 0 || strcmp(optarg, "s3") == 0) sp_val = sp_slowest;
                else if (strcmp(optarg, "f") == 0 || strcmp(optarg, "f1") == 0) sp_val = sp_fast;
                else if (strcmp(optarg, "ff") == 0 || strcmp(optarg, "f2") == 0) sp_val = sp_faster;
                else if (strcmp(optarg, "fff") == 0 || strcmp(optarg, "f3") == 0) sp_val = sp_fastest;
                else if (strcmp(optarg, "p") == 0) sp_val = sp_pause;
                else {
                  print_help();
                  return 1;
                }
                break;
      case 'S': if (strcmp(optarg, "rhombus") == 0) shape_val = st_rhombus;
                else if (strcmp(optarg, "rect") == 0) shape_val = st_rect;
                else if (strcmp(optarg, "hex") == 0) shape_val = st_hex;
                else {
                  print_help();
                  return 1;
                }
                break;

                /* multiplayer-related options */
      case 'E': { char* endptr = NULL;
                  val_clients_num = strtol(optarg, &endptr, 10);
                  if (*endptr != '\0') {
                    print_help();
                    return 1;
                  }
                  multiplayer_flag = 1;
                  server_flag = 1; 
                }
                break;
      case 'e':
                free(val_server_port);
                val_server_port = strdup(optarg);
                break;
      case 'C': 
                multiplayer_flag = 1;
                server_flag = 0; 
                free(val_server_addr);
                val_server_addr = strdup(optarg);
                break;
      case 'c':
                free(val_client_port);
                val_client_port = strdup(optarg);
                break;
      case '?': case 'h':
          print_help();
          return 1;
      default: abort ();
    }
  }
  /* Adjust l_val and conditions_val */
  {
    int avlbl_loc_num = stencil_avlbl_loc_num (shape_val);
    if(l_val == 0) l_val = avlbl_loc_num;

    if (l_val < 2 || l_val > avlbl_loc_num) {
      print_help();
      return 1;
    }
    if (conditions_were_set && (conditions_val<1 || conditions_val>l_val)) {
      print_help();
      return 1;
    }

    if (val_clients_num < 1 || val_clients_num > l_val) {
      print_help();
      return 1;
    }

    if (shape_val == st_rect) {
      w_val = MIN(MAX_WIDTH-1, w_val+(h_val+1)/2);
    }
  }

  
  struct sigaction newhandler;            /* new settings         */
  sigset_t         blocked;               /* set of blocked sigs  */
  newhandler.sa_flags = SA_RESTART;       /* options     */
  sigemptyset(&blocked);                  /* clear all bits       */
  newhandler.sa_mask = blocked;           /* store blockmask      */
  newhandler.sa_handler = on_timer;      /* handler function     */
  if ( sigaction(SIGALRM, &newhandler, NULL) == -1 )
    perror("sigaction");


  /* prepare the terminal for the animation */
  setlocale(LC_ALL, "");
  initscr();     /* initialize the library and screen */
  cbreak();      /* put terminal into non-blocking input mode */
  noecho();      /* turn off echo */
  start_color();
  clear();       /* clear the screen */
  curs_set(0);   /* hide the cursor */

  use_default_colors();
  init_pair(0, COLOR_WHITE, COLOR_BLACK);
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_BLACK, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_GREEN, COLOR_BLACK);
  init_pair(5, COLOR_BLUE, COLOR_BLACK);
  init_pair(6, COLOR_YELLOW, COLOR_BLACK);
  init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(8, COLOR_CYAN, COLOR_BLACK);
 
  color_set(0, NULL);
  assume_default_colors(COLOR_WHITE, COLOR_BLACK);
  clear();
    
  struct state st;
  struct ui ui;

  /* Initialize the parameters of the program */
  attrset(A_BOLD | COLOR_PAIR(2));
  mvaddstr(0,0,"Map is generated. Please wait.");
  refresh();

  state_init(&st, w_val, h_val, shape_val, seed_val, r_flag, l_val, val_clients_num, conditions_val, ineq_val, sp_val, dif_val, timeline_flag);
 
  ui_init(&st, &ui);

  clear();
 
  /* non-blocking input */
  int fd_flags = fcntl(0, F_GETFL);
  fcntl(0, F_SETFL, (fd_flags|O_NONBLOCK));

  /* Start the real time interval timer with delay interval size */
  struct itimerval it;
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 10000;
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 10000;
  setitimer(ITIMER_REAL, &it, NULL);
  
  refresh();        
  input_ready = 0;
  time_to_redraw = 1;

  if (!multiplayer_flag) {
    /* Run the game */
    run(&st, &ui);
  }
  else {
    if (server_flag) run_server(&st, val_clients_num, val_server_port);
    else run_client(&st, &ui, val_server_addr, val_server_port, val_client_port);
  }

  /* Restore the teminal state */
  echo();
  curs_set(1);
  clear();
  endwin();

  if (!multiplayer_flag || server_flag)
    printf ("Random seed was %i\n", st.map_seed);

  free(val_server_addr);
  free(val_server_port);
  free(val_client_port);
  return 0;
}


/*****************************************************************************/
/*                             SIGIO Signal Handler                          */
/*****************************************************************************/

int dialog_quit_confirm(struct state *st, struct ui *ui) {
  output_dialog_quit_on(st, ui); 
  int done = 0;
  int finished = 0;
  char buf[1];
  while(!done) {
    if ( fread(buf, 1, 1, stdin) == 1 ) {
      char c = buf[0];
      switch(c){
        case 'y': case 'Y': case 'q': case 'Q':
          finished = 1;
          done = 1;
          break;
        case 'n': case 'N': case ESCAPE:
          finished = 0;
          done = 1;
          break;
        default:
          break;
      }
    }
    else
      pause();
  }
  output_dialog_quit_off(st, ui); 
  return finished;
}

int update_from_input( struct state *st, struct ui *ui )
{
    int c;
    char buf[1];
    int finished=0;

    while ( fread(buf, 1, 1, stdin) == 1 ) {
        c = buf[0];
        int cursi = ui->cursor.i;
        int cursj = ui->cursor.j;
        /*ndelay = 0; */
        switch (c) {
            case 'Q':
            case 'q': 
                finished = dialog_quit_confirm(st, ui);                     /* quit program */
                break;
            case 'f':
                st->prev_speed = st->speed;
                st->speed = faster(st->speed);
                break;
            case 's':
                st->prev_speed = st->speed;
                st->speed = slower(st->speed);
                break;
            case 'p':
                if (st->speed == sp_pause)
                  st->speed = st->prev_speed;
                else {
                  st->prev_speed = st->speed;
                  st->speed = sp_pause;
                }
                break;
            case 'h': case K_LEFT:
              cursi--;
              break;
            case 'l': case K_RIGHT:
              cursi++;
              break;
            case 'k': case K_UP:
              cursj--;
              if (cursj % 2 == 1)
                cursi++;
              break;
            case 'j': case K_DOWN:
              cursj++;
              if (cursj % 2 == 0)
                cursi--;
              break;
            case ' ':
              if (st->fg[st->controlled].flag[ui->cursor.i][ui->cursor.j] == 0)
                add_flag (&st->grid, &st->fg[st->controlled], ui->cursor.i, ui->cursor.j, FLAG_POWER);
              else
                remove_flag (&st->grid, &st->fg[st->controlled], ui->cursor.i, ui->cursor.j, FLAG_POWER);
              break;
            case 'x':
              remove_flags_with_prob (&st->grid, &st->fg[st->controlled], 1.0);
              break;
            case 'c':
              remove_flags_with_prob (&st->grid, &st->fg[st->controlled], 0.5);
              break;
            case 'r':
            case 'v':
              build (&st->grid, &st->country[st->controlled], st->controlled, ui->cursor.i, ui->cursor.j);
              break;
            
            case ESCAPE:
            case 91:
              break;
            }
        
        cursi = IN_SEGMENT(cursi, 0, st->grid.width-1);
        cursj = IN_SEGMENT(cursj, 0, st->grid.height-1);
        if ( is_visible(st->grid.tiles[cursi][cursj].cl) ) {
          ui->cursor.i = cursi;
          ui->cursor.j = cursj;
        }

    }                
    return finished;
}

/* client version */
int update_from_input_client ( struct state *st, struct ui *ui, int sfd, struct addrinfo *srv_addr)
{
    int c;
    char buf[1];
    int finished=0;

    while ( fread(buf, 1, 1, stdin) == 1 ) {
      c = buf[0];
      switch (c){
        case 'q': case 'Q':
          finished = dialog_quit_confirm(st, ui);                     /* quit program */
          break;
        default:
          finished = client_process_input (st, ui, c, sfd, srv_addr);
      }
    }                
    return finished;
}

/* server version */
int update_from_input_server ( struct state *st )
{
    int c;
    char buf[1];
    int finished=0;

    while ( fread(buf, 1, 1, stdin) == 1 ) {
      c = buf[0];
      switch (c) {
        case 'q': case 'Q': finished = 1; break;
      }
    }                
    return finished;
}
/*****************************************************************************/
/*                           SIGALRM Signal Handler                          */
/*****************************************************************************/

/* SIGALRM handler -- moves string on the screen when the signal is received */
void on_timer(int signum)
{
  time_to_redraw = 1;
//  refresh();
}

