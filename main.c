#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "hotui.c"

#define BAR_SIZE 4 
#define HEIGHT 10 
#define WIDTH 20 
#define BLOCK_ROWS 4
#define CHARACTER '-'
#define BALL 'o'
#define BLOCK 'x'

static char pixels[WIDTH * HEIGHT];

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

void draw_screen(Window window) {
  hui_clear_window();
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      hui_put_character_at(pixels[i*WIDTH + j], i, j);           
    }
  }
}

int main() {
  
  srand(time(NULL));
  Window main_win = hui_init();

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
  char buffer[50];
  memset(&buffer, 0, 50);

  fd_set readfs;
  FD_ZERO(&readfs);
  FD_SET(1, &readfs);

  Window side_window = hui_create_window(100, 100, 0, WIDTH + 2);


  while(!quit) {
    int characters = 0;
    memset(&buffer, 0, 50);

    update_pixels(bar_ypos, bar_xpos, ball_ypos, ball_xpos);
    draw_screen(main_win);
    if (calculate_ball_position(&ball_ypos, &ball_xpos, &ball_vy, &ball_vx)) {
      dead = 1;
      char* game_over_text = "Game Over";
      size_t game_over_text_size = strlen(game_over_text);
      hui_put_text_at(game_over_text, game_over_text_size, HEIGHT / 2, WIDTH / 2 - game_over_text_size/2);
      break;
    }
    characters = sprintf(buffer, "Points: %d", points);
    hui_put_text_at_window(side_window, buffer, characters, 0, 0);

    characters = sprintf(buffer, "Ball xpos: %d", ball_xpos);
    hui_put_text_at_window(side_window, buffer, characters, 1, 0);

    characters = sprintf(buffer, "Ball ypos: %d", ball_ypos);
    hui_put_text_at_window(side_window, buffer, characters, 2, 0);

    characters = sprintf(buffer, "Ball vxpos: %d", ball_vx);
    hui_put_text_at_window(side_window, buffer, characters, 3, 0);

    characters = sprintf(buffer, "Ball vypos: %d", ball_vy);
    hui_put_text_at_window(side_window, buffer, characters, 4, 0);

    struct timeval tv;
    tv.tv_sec = 1; // Wait for 5 seconds
    tv.tv_usec = 0;

    int retval = select(1 + 1, &readfs, NULL, NULL, &tv);
    
    if (retval == -1) {
      characters = sprintf(buffer, "Erro reading select");
      hui_put_text_at(buffer, characters, 5, WIDTH + 2);
    } else if (retval) {
        read(1, &ch, 1);
        characters = sprintf(buffer, "character available");
        hui_put_text_at(buffer, characters, 5, WIDTH + 2);
        if (ch == 'q') {
          return 1;
          break;
        } else if (ch == 'l') {
          if (!(bar_xpos + BAR_SIZE + 2 > WIDTH)) bar_xpos+=2;
        } else if (ch == 'h') {
          if (bar_xpos > 1) bar_xpos-=2;
        }
    } else {
      characters = sprintf(buffer, "No character available");
      hui_put_text_at(buffer, characters, 5, WIDTH + 2);
    }

    ball_xpos+=(ball_vx);
    ball_ypos+=(ball_vy);
  }

  while (!quit) {
    read(1, &ch, 1);
    if (ch == 'q') {
      return 1;
    } 
  }
}
