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
#include "main-common.h"

/*****************************************************************************/
/*                           Global Constants                                */
/*****************************************************************************/

volatile   sig_atomic_t   input_ready;
volatile   sig_atomic_t   time_to_redraw;
 
void on_timer(int signum);   /* handler for alarm     */
int  update_from_input( struct state *st, struct ui *ui );
int  update_from_input_client( struct state *st, struct ui *ui, int sfd, struct addrinfo *srv_addr);
int  update_from_input_server( struct state *st );

/* print the victory message */
void win_or_lose_message(struct state *st, int k) {
  if (k%100 == 0) {
    switch(win_or_lose(st)) {
      case 1:
        attrset(A_BOLD | COLOR_PAIR(4));
        mvaddstr(2 + st->grid.height, 31, "You are victorious!");
        break;
      case -1:
        attrset(A_BOLD | COLOR_PAIR(2));
        mvaddstr(2 + st->grid.height, 31, "You are defeated!");
        break;
      default:
        ;
    }
  }
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
      win_or_lose_message(st, k);
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
        /*
        if (st->show_timeline) {
          if (st->time%10 == 0)
            output_timeline(st, ui);
        }
        */
        win_or_lose_message(st, k);
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

  /* Read command line arguments */
  struct basic_options op;
  struct multi_options mop;
  if (get_options(argc, argv, &op, &mop) == 1) return 1;
 
  /* Setup signal handlers */
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

  state_init(&st, &op, &mop);
 
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

  if (!mop.multiplayer_flag) {
    /* Run the game */
    run(&st, &ui);
  }
  else {
    if (mop.server_flag) run_server(&st, mop.clients_num, mop.val_server_port);
    else run_client(&st, &ui, mop.val_server_addr, mop.val_server_port, mop.val_client_port);
  }

  /* Restore the teminal state */
  echo();
  curs_set(1);
  clear();
  endwin();

  if (!mop.multiplayer_flag || mop.server_flag)
    printf ("Random seed was %i\n", st.map_seed);

  free(mop.val_server_addr);
  free(mop.val_server_port);
  free(mop.val_client_port);
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
      switch(c){
        case 'q': case 'Q':
          finished = dialog_quit_confirm(st, ui);                     /* quit program */
          break;
        default:
          finished = singleplayer_process_input (st, ui, c);
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

