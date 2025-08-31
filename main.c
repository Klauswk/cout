#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define BAR_SIZE 4 
#define HEIGHT 10 
#define WIDTH 20 
#define BLOCK_ROWS 4
#define CHARACTER '-'
#define BALL 'o'
#define BLOCK 'x'

static char pixels[WIDTH * HEIGHT];

void print_pixels(int bar_ypos, int bar_xpos) {
  for (int i = 0; i < HEIGHT; i++) {
     for (int j = 0; j < WIDTH; j++) {
        printf("%c",pixels[i* WIDTH + j]);
     }
     printf("\n");
  }
}

void init_board(int bar_ypos, int bar_xpos, int ball_ypos, int ball_xpos) {
  for (int i = 0; i < HEIGHT; i++) {
     for (int j = 0; j < WIDTH; j++) {
        pixels[i* WIDTH + j] = ' ';
     }
  }
  for (int i = 1; i < BLOCK_ROWS; i++) {
     for (int j = 0; j < WIDTH; j++) {
        pixels[i* WIDTH + j] = BLOCK;
     }
  }

  pixels[WIDTH*ball_ypos + ball_xpos] = BALL;
}

static int points = 0; 

int calculate_ball_position(int *y, int *x, int *vy, int *vx) {
  if (pixels[WIDTH * (*y + *vy) + (*x + *vx)] == BLOCK) {
    pixels[WIDTH * (*y + *vy) + (*x + *vx)] = ' '; 
    points+=100;
    int multiplex = 1; 

    int random = rand();  

    if( random < RAND_MAX/2) multiplex = -1;
    
    *vx= (*vx)*(multiplex);
    *vy = (*vy)*(-1);
    return 0;
  }

  if (pixels[WIDTH * (*y + *vy) + (*x + *vx)] == CHARACTER) {
    int multiplex = 1; 

    int random = rand();  

    if( random < RAND_MAX/2) multiplex = -1;

    *vx= (*vx)*(multiplex);
    *vy = (*vy)*(-1);
    return 0;
  }

  if (*x + *vx > WIDTH - 1 || *x + *vx < 0) {
    *vx= (*vx)*(-1);
    return 0;
  } 

  if (*y + *vy >= HEIGHT)  {
    return 1;
  }

  if (*y + *vy < 0) *vy = (*vy)*(-1);
  
  return 0;
}

void update_pixels(int bar_ypos, int bar_xpos, int ball_ypos, int ball_xpos) {
  for (int i = 0; i < HEIGHT; i++) {
     for (int j = 0; j < WIDTH; j++) {
        if (pixels[i * WIDTH + j] != BLOCK) {
          pixels[i* WIDTH + j] = ' ';
        }
     }
  }

  for (int i = 0; i < BAR_SIZE; i++) pixels[WIDTH*bar_ypos + bar_xpos + i ] = CHARACTER; 

  pixels[ball_ypos * WIDTH + ball_xpos] = BALL;
}

void draw_screen(WINDOW* main_win) {
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      mvwaddch(main_win, i, j, pixels[i* WIDTH + j]);
    }
  }

}

int main() {
  
  srand(time(NULL));
  initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
  int row, col;
  getmaxyx(stdscr, col, row);

  WINDOW* main_win = newwin(col, row, 0, 0);
  wtimeout(main_win, 1000);

  int quit = 0;
  int ch = 'a';
  int bar_xpos = (WIDTH/2) - BAR_SIZE;
  int bar_ypos = HEIGHT - 1;
  int ball_xpos = WIDTH/2;
  int ball_ypos = HEIGHT/2;
  int ball_vx = 1;
  int ball_vy = 1;
  int dead = 0;
  init_board(bar_ypos, bar_xpos, ball_ypos, ball_xpos);
  update_pixels(bar_ypos, bar_xpos, ball_ypos, ball_xpos);
  draw_screen(main_win);

  while(!quit) {
    update_pixels(bar_ypos, bar_xpos, ball_ypos, ball_xpos);
    draw_screen(main_win);
    if (calculate_ball_position(&ball_ypos, &ball_xpos, &ball_vy, &ball_vx)) {
      dead = 1;
      mvwprintw(main_win, HEIGHT / 2, WIDTH / 2  - strlen("GAME OVER")/2, "%s", "GAME OVER");
      wrefresh(main_win);
      break;
    }
    mvwprintw(main_win, 0, WIDTH + 2, "Points: %d", points);
    mvwprintw(main_win, 1, WIDTH + 2, "Ball xpos: %d", ball_xpos);
    mvwprintw(main_win, 2, WIDTH + 2, "Ball ypos: %d", ball_ypos);
    mvwprintw(main_win, 3, WIDTH + 2, "Ball vypos: %d", ball_vy);
    mvwprintw(main_win, 4, WIDTH + 2, "Ball vxpos: %d", ball_vx);

    ch = wgetch(main_win);  
    if (ch == 'q') {
      endwin();
      return 1;
      break;
    } else if (ch == 'l') {
      if (!(bar_xpos + BAR_SIZE + 2 > WIDTH)) bar_xpos+=2;
    } else if (ch == 'h') {
      if (bar_xpos > 1) bar_xpos-=2;
    }

    ball_xpos+=(ball_vx);
    ball_ypos+=(ball_vy);
    wclear(main_win);
  }

  while (!quit) {
    ch = wgetch(main_win);  
    if (ch == 'q') {
      endwin();
      return 1;
      break;
    } 
  }

  endwin();
}
