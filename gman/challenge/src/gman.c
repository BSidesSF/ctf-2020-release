// For usleep
#define _DEFAULT_SOURCE

#include <curses.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "password.h"

typedef enum {
  LEFT  = 0,
  RIGHT = 1,
  UP    = 2,
  DOWN  = 3,
} direction_t;

#define HOSTNAME "gman-ed6b851c.challenges.bsidessf.net"

#define MAX_LEVELS 100
#define PAD_LEFT 2
#define PAD_TOP 3
#define BOARD_COUNT 4
#define ROWS 21
#define COLS 27
#define PELLET ACS_BULLET

#define START_X 11
#define START_Y 15
#define START_DIRECTION RIGHT

#define GMAN '0'
#define DRAW_GMAN 1
#define GMAN_COLOR COLOR_YELLOW

#define WALL ACS_BLOCK
#define FLOOR ACS_BLOCK
#define DRAW_WALL 2
#define DRAW_FLOOR 3
#define WALL_COLOR COLOR_BLUE
#define FLOOR_COLOR COLOR_BLACK

static int GHOST_COLORS[] = { COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN, COLOR_RED };
#define GHOST_COUNT 4
#define GHOST_START_X 12
#define GHOST_START_Y 9
#define GHOST(n) ('A'+n)
#define DRAW_GHOST(n) (n+4)
#define GHOST_COLOR(n) GHOST_COLORS[n % 4]

// The odds ( /100) that the ghost will turn when it doesn't need to
#define GHOST_RANDOM_TURN_ODDS 80

// The odds ( / 100) that the ghost will turn towards gman instead of away
#define GHOST_TURN_TOWARDS_ODDS 75

#define PELLET_ODDS 1
#define PELLET_VALUE 100
#define DELAY 750

// Turn off a few things for debugging
#if 0
#undef PAD_LEFT
#undef PELLET
#undef FLOOR
#undef FLOOR_COLOR

#define HIDE_WALLS
#define DISABLE_GHOSTS
#define DISABLE_GMAN

#define PAD_LEFT 0
#define PELLET '1'
#define FLOOR '0'
#define FLOOR_COLOR COLOR_BLUE
#endif

static char BOARDS[BOARD_COUNT][ROWS][COLS] = {
  {
    "1111111111111 1111111111111",
    "11          1 1          11",
    "11 1111 111 1 1 111 1111 11",
    "11                       11",
    "11 1111 11111 11111 1111 11",
    "11         11 11         11",
    "1111111 11       11 1111111",
    "1111111 11111 11111 1111111",
    "1111111 11111 11111 1111111",
    "                           ",
    "1111111 1111   1111 1111111",
    "1111111 11111 11111 1111111",
    "1111111 111     111 1111111",
    "11          111          11",
    "11 11 111 1111111 111 11 11",
    "11    11           11    11",
    "11111 11 111111111 11 11111",
    "                           ",
    "11 111 11111 1 11111 111 11",
    "11                       11",
    "1111111111111 1111111111111",
  },
  {
    "111111111111111111111111111",
    "11                       11",
    "11 11111111 111 11111111 11",
    "                           ",
    "11 1111 11111111111 1111 11",
    "11         11111         11",
    "1111111 11       11 1111111",
    "1111111 11111 11111 1111111",
    "1111111 11111 11111 1111111",
    "11                       11",
    "11 1111 1111   1111 1111 11",
    "11 1111 11111 11111 1111 11",
    "11 1111 111     111 1111 11",
    "11          111          11",
    "11 11 111 1111111 111 11 11",
    "      11           11      ",
    "11111 11 111111111 11 11111",
    "11                       11",
    "11 111 11111 1 11111 111 11",
    "11                       11",
    "111111111111111111111111111",
  },
  {
    "111111111111111111111111111",
    "                           ",
    "111111111 1 111 1 111111111",
    "11                       11",
    "11 1111 11111111111 1111 11",
    "11         11111         11",
    "11 1111 11       11 1111 11",
    "11 1111 11111 11111 1111 11",
    "11 1111 11111 11111 1111 11",
    "11                       11",
    "11 1111 1111   1111 1111 11",
    "11 1111 11111 11111 1111 11",
    "11 1111 111     111 1111 11",
    "11          111          11",
    "11 11 111 1111111 111 11 11",
    "11    11           11    11",
    "11111 11 111111111 11 11111",
    "                           ",
    "111111111111 1 111111111111",
    "                           ",
    "111111111111111111111111111",
  },
  {
    "111111111111111111111111111",
    "1                         1",
    "1 1 1 111 1 1 1 1 111 1 1 1",
    "1 1 1 111 1 1 1 1 111 1 1 1",
    "1 1 1 111 1 1 1 1 111 1 1 1",
    "1 1 1 111  11 11  111 1 1 1",
    "1   1   11       11   1   1",
    "1 11111 11111 11111 11111 1",
    "1 11111 11111 11111 11111 1",
    "1                         1",
    "1 1 111 1111   1111 111 1 1",
    "1 1 111 11111 11111 111 1 1",
    "1 1 111 111     111 111 1 1",
    "1 1         111         1 1",
    "1 111 111 1111111 111 111 1",
    "1 1                     1 1",
    "1 11111111111111111111111 1",
    "1                         1",
    "1 1111 11111 1 11111 1111 1",
    "1                         1",
    "111111111111111111111111111",
  },
};

typedef enum {
  RESULT_NONE,
  RESULT_DIE,
  RESULT_WIN,
} round_result_t;

typedef struct {
  uint8_t x, y;
  direction_t direction;
} ghost_t;

typedef struct {
  // Enough room to store all the walls + pellets as bits
  // I want pellets first, because that's where an exploitable address lives
  uint8_t pellets[(ROWS*COLS+1) / 8];
  uint8_t walls[(ROWS*COLS+1) / 8];
} board_t;

typedef struct {
  uint32_t score;
  int8_t lives;
  uint8_t level;

  uint8_t x, y;
  direction_t direction;
  direction_t next_direction;
  ghost_t ghosts[GHOST_COUNT];

  board_t boards[MAX_LEVELS];
} state_t;

void set_bit(uint8_t *array, int x, int y, uint8_t value) {
  int byte = ((y * COLS) + x) / 8;
  int bit  = ((y * COLS) + x) % 8;

  // Invert the bit so it reads left to right
  bit = 7 - bit;

  if(value) {
    array[byte] = array[byte] | (1 << bit);
  } else {
    array[byte] = array[byte] & ~(1 << bit);
  }
}

uint8_t get_bit(uint8_t *array, int x, int y) {
  int byte = ((y * COLS) + x) / 8;
  int bit = ((y * COLS) + x) % 8;

  // Invert the bit so it reads left to right
  bit = 7 - bit;

  return (array[byte] & (1 << bit)) ? 1 : 0;
}

void initialize_boards(state_t *state) {
  int b, x, y;

  for(b = 0; b < MAX_LEVELS; b++) {
    for(x = 0; x < COLS; x++) {
      for(y = 0; y < ROWS; y++) {
        // Set the wall
        set_bit(state->boards[b].walls, x, y, BOARDS[b % BOARD_COUNT][y][x] == '1');

        // Set the pellet
        set_bit(state->boards[b].pellets, x, y, BOARDS[b % BOARD_COUNT][y][x] == ' ');
      }
    }
  }
}

void reset_ghosts(state_t *state) {
  int i;

  for(i = 0; i < GHOST_COUNT; i++) {
    state->ghosts[i].x = GHOST_START_X + (i % 2);
    state->ghosts[i].y = GHOST_START_Y + (i / 2);
    state->ghosts[i].direction = -1;
  }
}

void reset_gman(state_t *state) {
  state->x = START_X;
  state->y = START_Y;
  state->direction = START_DIRECTION;
  state->next_direction = -1;
}

void reset(state_t *state) {
  reset_ghosts(state);
  reset_gman(state);
}

board_t *current_board(state_t *state) {
  return &state->boards[state->level];
}

uint8_t pellets_remaining(state_t *state) {
  int x, y;

  board_t *board = current_board(state);

  for(x = 0; x < COLS; x++) {
    for(y = 0; y < ROWS; y++) {
      if(get_bit(board->pellets, x, y)) {
        return 1;
      }
    }
  }

  return 0;
}

void draw_board(state_t *state) {
  int x, y;
  board_t *board = current_board(state);

  for(x = 0; x < COLS; x++) {
    for(y = 0; y < ROWS; y++) {
      // Draw gman
      if(x == state->x && y == state->y) {
        attron(COLOR_PAIR(DRAW_GMAN));
        mvaddch(y + PAD_TOP, x + PAD_LEFT, GMAN);
        attroff(COLOR_PAIR(DRAW_GMAN));

#ifndef HIDE_WALLS
      }
      // Draw walls
      else if(get_bit(board->walls, x, y)) {
        attron(COLOR_PAIR(DRAW_WALL));
        mvaddch(y + PAD_TOP, x + PAD_LEFT, WALL);
        attroff(COLOR_PAIR(DRAW_WALL));
#endif
      } else {
        // Draw a pellet or blank
        if(get_bit(board->pellets, x, y)) {
          mvaddch(y + PAD_TOP, x + PAD_LEFT, PELLET);
        } else {
          attron(COLOR_PAIR(DRAW_FLOOR));
          mvaddch(y + PAD_TOP, x + PAD_LEFT, FLOOR);
          attroff(COLOR_PAIR(DRAW_FLOOR));
        }

#ifndef DISABLE_GHOSTS
        // Check for ghosts, since they will go over top of pellet or blank
        int i;
        for(i = 0; i < GHOST_COUNT; i++) {
          if(state->ghosts[i].x == x && state->ghosts[i].y == y) {
            attron(COLOR_PAIR(DRAW_GHOST(i)));
            mvaddch(y + PAD_TOP, x + PAD_LEFT, GHOST(i));
            attroff(COLOR_PAIR(DRAW_GHOST(i)));
          }
        }
#endif
      }
    }
  }
  move(0, 0);

  refresh();
}

uint8_t can_move(state_t *state, direction_t direction, uint8_t x, uint8_t y) {
  board_t *board = current_board(state);

  switch(direction) {
    case UP:
      return y == 0 || !get_bit(board->walls, x, y - 1);

    case DOWN:
      return y == (ROWS - 1) || !get_bit(board->walls, x, y + 1);

    case LEFT:
      return x == 0 || !get_bit(board->walls, x - 1, y);

    case RIGHT:
      return x == (COLS - 1) || !get_bit(board->walls, x + 1, y);
  }

  return 0;
}

uint8_t can_turn(state_t *state, direction_t direction, uint8_t x, uint8_t y) {
  switch(direction) {
    case UP: case DOWN:
      return can_move(state, LEFT, x, y ) || can_move(state, RIGHT, x, y);

    case LEFT: case RIGHT:
      return can_move(state, UP, x, y ) || can_move(state, DOWN, x, y);
  }

  return 0;
}

void do_move(state_t *state, direction_t direction, uint8_t *x, uint8_t *y) {
  if(!can_move(state, direction, *x, *y)) {
    return;
  }

  // Handle teleport
  if(direction == LEFT && *x == 0) {
    *x = COLS - 1;
  } else if(direction == RIGHT && *x == COLS - 1) {
    *x = 0;
  } else if(direction == UP && *y == 0) {
    *y = ROWS - 1;
  } else if(direction == DOWN && *y == ROWS - 1) {
    *y = 0;
  } else {
    switch(direction) {
      case UP:    *y -= 1; break;
      case DOWN:  *y += 1; break;
      case LEFT:  *x -= 1; break;
      case RIGHT: *x += 1; break;
    }
  }
}

void change_direction_if_possible(state_t *state) {
  if(state->next_direction < 0) {
    return;
  }

  if(can_move(state, state->next_direction, state->x, state->y)) {
    state->direction = state->next_direction;
    state->next_direction = -1;
  }
}

uint8_t handle_input(state_t *state) {
  // This loop discards all buffered characters except the last
  int c = getch();
  int next_c;
  do {
    next_c = getch();
    if(next_c != ERR && next_c != '\n' && next_c != '\r') {
      c = next_c;
    }
  } while(next_c != ERR);

  switch(c) {
    case 'w': case KEY_UP:    state->next_direction = UP;    break;
    case 'a': case KEY_LEFT:  state->next_direction = LEFT;  break;
    case 's': case KEY_DOWN:  state->next_direction = DOWN;  break;
    case 'd': case KEY_RIGHT: state->next_direction = RIGHT; break;
    case 'q': return 1;
  }

  // Immediately turn them, if it can
  change_direction_if_possible(state);

  return 0;
}

void show_password(state_t *state) {
  uint8_t seed = rand();

  print_password(state->level, state->lives, seed);
  srand(seed);
}

void do_win(state_t *state) {
  clear();
  mvprintw(3, 2, "Level %d completed!", state->level + 1);
  mvprintw(4, 2, "Your bonus: %d points", (state->level + 1) * 10000);
  state->score += ((state->level + 1) * 10000);
  state->level += 1;
  mvprintw(5, 2, "New Score: %d", state->score);
  mvprintw(7, 2, "Press any key to continue");
  refresh();

  timeout(-1);
  getch();

  clear();
  mvprintw(2, 2, "Password to return here:");
  show_password(state);
  reset(state);
}

round_result_t tick_gman(state_t *state) {
  board_t *board = current_board(state);

  // Move if we can
  do_move(state, state->direction, &state->x, &state->y);

  // Either change direction or stop trying
  change_direction_if_possible(state);

  // Grab a pellet if there's one
  if(get_bit(board->pellets, state->x, state->y)) {
    set_bit(board->pellets, state->x, state->y, 0);
    state->score += PELLET_VALUE;

    if(!pellets_remaining(state)) {
      return RESULT_WIN;
    }
  }

  return RESULT_NONE;
}

// Choose one of the three directions that they can move (they can't reverse course)
direction_t choose_ghost_direction(state_t *state, direction_t direction, uint8_t x, uint8_t y) {
  // If it can't move at all, then we've failed
  if(!can_move(state, LEFT, x, y) && !can_move(state, RIGHT, x, y) && !can_move(state, UP, x, y) && !can_move(state, DOWN, x, y)) {
    return direction;
  }

  // If it can't turn, just keep going
  if(direction == UP && !can_move(state, LEFT, x, y) && !can_move(state, RIGHT, x, y)) {
    return direction;
  }

  if(direction == DOWN && !can_move(state, LEFT, x, y) && !can_move(state, RIGHT, x, y)) {
    return direction;
  }

  if(direction == LEFT && !can_move(state, UP, x, y) && !can_move(state, DOWN, x, y)) {
    return direction;
  }

  if(direction == UP && !can_move(state, UP, x, y) && !can_move(state, DOWN, x, y)) {
    return direction;
  }

  // If we're here, it _can_ turn. But does it want to?
  if((rand() % 100) > GHOST_RANDOM_TURN_ODDS) {
    return direction;
  }

  // Choose a random direction; loop until it chooses one that isn't a reverse
  direction_t new_direction;

  // Give up eventually and just keep moving
  int i;
  for(i = 0; i < 100; i++) {
    // Figure out how much it wants to do left-right based on distances
    int left_right_odds = (int)(100.0 * ((double) abs(state->x - x)) / ((double) (abs(state->x - x) + abs(state->y - y))));

    // We don't ever want to be 100%
    if(left_right_odds <= 5) {
      left_right_odds = 5;
    } else if(left_right_odds >= 95) {
      left_right_odds = 95;
    }

    // Gman is up-left
    if(state->x <= x && state->y <= y) {
      //mvprintw(0, 0, "Want to go up-left");
      if((rand() % 100) < left_right_odds) {
        new_direction = LEFT;
      } else {
        new_direction = UP;
      }
    // Gman is down-left
    } else if(state->x <= x && state->y >= y) {
      //mvprintw(0, 0, "Want to go down-left");
      if((rand() % 100) < left_right_odds) {
        new_direction = LEFT;
      } else {
        new_direction = DOWN;
      }
    // Gman is up-right
    } else if(state->x >= x && state->y <= y) {
      //mvprintw(0, 0, "Want to go up-right");
      if((rand() % 100) < left_right_odds) {
        new_direction = RIGHT;
      } else {
        new_direction = UP;
      }
    // Gman is down-right
    } else {
      //mvprintw(0, 0, "Want to go down-right");
      if((rand() % 100) < left_right_odds) {
        new_direction = RIGHT;
      } else {
        new_direction = DOWN;
      }
    }

    // Sometimes go the wrong way
    if((rand() % 100) > GHOST_TURN_TOWARDS_ODDS) {
      if(new_direction == UP)         { new_direction = DOWN; }
      else if(new_direction == DOWN)  { new_direction = UP; }
      else if(new_direction == LEFT)  { new_direction = RIGHT; }
      else if(new_direction == RIGHT) { new_direction = LEFT; }
    }

    // Make sure it isn't reversing direction
    if(direction == UP    && new_direction == DOWN)  { continue; }
    if(direction == DOWN  && new_direction == UP)    { continue; }
    if(direction == LEFT  && new_direction == RIGHT) { continue; }
    if(direction == RIGHT && new_direction == LEFT)  { continue; }

    // Make sure it can actually move the new direction
    if(can_move(state, new_direction, x, y)) {
      return new_direction;
    }
  }

  // We don't really want to be here, but we also don't want an infinite loop
  return direction;
}

void tick_ghosts(state_t *state) {
  int i;
  for(i = 0; i < GHOST_COUNT; i++) {
    ghost_t *ghost = &state->ghosts[i];

    // Choose a direction
    ghost->direction = choose_ghost_direction(state, ghost->direction, ghost->x, ghost->y);

    // Move the ghost
    do_move(state, ghost->direction, &ghost->x, &ghost->y);
  }
}

round_result_t check_death(state_t *state) {
  int i;
  for(i = 0; i < GHOST_COUNT; i++) {
    if(state->x == state->ghosts[i].x && state->y == state->ghosts[i].y) {
      return RESULT_DIE;
    }
  }

  return RESULT_NONE;
}

round_result_t tick(state_t *state) {
  round_result_t result;

  // Move gman
#ifndef DISABLE_GMAN
  result = tick_gman(state);
  if(result != RESULT_NONE) {
    return result;
  }
#endif

  // Check death both before and after to make sure we can't "jump" a ghost
  result = check_death(state);
  if(result != RESULT_NONE) {
    return result;
  }

  // Move ghosts
#ifndef DISABLE_GHOSTS
  tick_ghosts(state);
#endif

  // Check death again
  result = check_death(state);
  if(result != RESULT_NONE) {
    return result;
  }

  // Keep going!
  return RESULT_NONE;
}

uint8_t get_password(state_t *state) {
  uint8_t level, lives, seed;

  // TODO: Can we have a signed issue (so level 128 - negative = allowed)?
  if(read_password(&level, &lives, &seed)) {
    state->level = level;
    state->lives = lives;
    if(state->level > 0) {
      state->score = 10000 * (state->level * (state->level + 1)) / 2;
    } else {
      state->score = 0;
    }

    srand(seed);
    reset(state);

    return 1;
  }

  return 0;
}

void main_menu(state_t *state) {
  for(;;) {
    clear();
    mvprintw(0, 0, "Welcome to G-man! Please select an option:");
    mvprintw(2, 0, "1. New game");
    mvprintw(3, 0, "2. Continue game (with password)");
    move(0, 0);
    timeout(-1);

    int option = getch();
    switch(option) {
      case '1':
        return;

      case '2':
        if(get_password(state)) {
          return;
        }
    }
  }
}

uint32_t go() {
  state_t state;
  int i;

  initialize_boards(&state);
  state.score = 0;
  state.lives = 3;
  state.level = 0;
  reset(&state);

  srand(time(NULL));

  // Initialize curses
  initscr();

  // Don't buffer lines
  cbreak();

  // Disable the cursor
  curs_set(0);

  // Don't echo back characters
  noecho();

  // Allow colors
  start_color();

  // Enable arrow keys
  keypad(stdscr, TRUE);

  // Clear
  clear();

  main_menu(&state);

  // Note: This has to be after main_menu(), otherwise colours are wrong
  init_pair(DRAW_GMAN, GMAN_COLOR, COLOR_BLACK);
  init_pair(DRAW_WALL, WALL_COLOR, COLOR_BLACK);
  init_pair(DRAW_FLOOR, FLOOR_COLOR, COLOR_BLACK);

  for(i = 0; i < GHOST_COUNT; i++) {
    init_pair(DRAW_GHOST(i), GHOST_COLOR(i), COLOR_BLACK);
  }

  // Main loop
  for(;;) {
    round_result_t result;
    // Turn on non-blocking i/o
    timeout(0);

    if(handle_input(&state)) {
      break;
    }
    result = tick(&state);

    if(result == RESULT_WIN) {
      do_win(&state);
    } else if(result == RESULT_DIE) {
      state.lives -= 1;
      if(state.lives < 0) {
        break;
      }

      clear();
      mvprintw(0, 2, "       You died! %d lives left", state.lives);
      mvprintw(2, 2, "Password to return here:");
      show_password(&state);
      reset(&state);
    } else {
      clear();
      mvprintw(0, 0, "G-man", state.direction);
      mvprintw(1, 0, "Score: %d | Lives: %d | Level: %d", state.score, state.lives, state.level + 1);
      draw_board(&state);
      usleep(DELAY * 1000);
    }
  }

  clear();
  mvprintw(3, 2, "         Game Over!");
  mvprintw(4, 2, "     Final Score: %d", state.score);
  mvprintw(6, 2, "Press any key to continue");
  refresh();
  timeout(-1);
  getch();

  return state.score;
}

void save_highscore(char *file, char *name, uint32_t score) {
  FILE *f = fopen(file, "a");
  if(!f) {
    return;
  }

  fprintf(f, "%9d %s", score, name);
  fclose(f);
}

void show_highscores(char *file) {
  char command[1024];

  snprintf(command, 1023, "cat %s | sort -nr | head -n10\0\0\0\0", file);

  FILE *p = popen(command, "r");
  if(!p) {
    mvprintw(2, 0, "Error fetching highscores!");
    return;
  }

  mvprintw(0, 0, "Highscores");

  char buffer[192];
  int i = 0;
  while(fgets(buffer, 191, p) != NULL) {
    buffer[191] = '\0';
    i++;
    mvprintw(i+1, 4, "%d. %s", i, buffer);
  }
}

char name[256];
void instructions() {
  char *highscore_file = "/home/ctf/highscores.txt";

  printf("\n");
  printf("Welcome to G-man! For best experience, disable input buffering by running:\n");
  printf("\n");
  printf("  stty -icanon && nc -v "HOSTNAME" 1337\n");
  printf("\n");
  printf("If things don't look right after the game ends, reset your terminal with:\n");
  printf("\n");
  printf("  reset\n");
  printf("\n");
  printf("Please enter your name for the scoreboard: ");
  fflush(stdout);
  fgets(name, 127, stdin);
  name[127] = '\0';

  uint32_t score = go();

  save_highscore(highscore_file, name, score);

  clear();
  show_highscores(highscore_file);
  refresh();

  timeout(-1);
  getch();
  endwin();
}

int main(int argc, char *argv[])
{
  // These zeroes on the stack make the walls less random, and the killscreen
  // more solveable
  uint32_t dummy[] = {0,0,0,0,0,0,0,0,0,0};
  instructions();
  return 0;
}
