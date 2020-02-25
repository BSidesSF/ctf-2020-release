#include <curses.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define OFFSET_TOP 5
#define OFFSET_LEFT 4

void print_row(int top, int index, int left, int cell, int between_cell, int right) {
  int spot = 0;

  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, index);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, ' ');
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, left);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, between_cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, between_cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, between_cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, between_cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, between_cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, between_cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, cell);
  mvaddch(OFFSET_TOP + top, OFFSET_LEFT + spot++, right);
}

void print_table() {
  int row = 0;

  // Print the headers
  mvprintw(OFFSET_TOP - 1, OFFSET_LEFT, "    A   B   C   D   E   F   G");

  // Print the table
  print_row(row++, ' ', ACS_ULCORNER, ACS_HLINE, ACS_TTEE,  ACS_URCORNER); // Top
  print_row(row++, '1', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LTEE,     ACS_HLINE, ACS_PLUS,  ACS_RTEE);     // Divider
  print_row(row++, '2', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LTEE,     ACS_HLINE, ACS_PLUS,  ACS_RTEE);     // Divider
  print_row(row++, '3', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LTEE,     ACS_HLINE, ACS_PLUS,  ACS_RTEE);     // Divider
  print_row(row++, '4', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LTEE,     ACS_HLINE, ACS_PLUS,  ACS_RTEE);     // Divider
  print_row(row++, '5', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LTEE,     ACS_HLINE, ACS_PLUS,  ACS_RTEE);     // Divider
  print_row(row++, '6', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LTEE,     ACS_HLINE, ACS_PLUS,  ACS_RTEE);     // Divider
  print_row(row++, '7', ACS_VLINE,    ' ',       ACS_VLINE, ACS_VLINE);    // Cell row
  print_row(row++, ' ', ACS_LLCORNER, ACS_HLINE, ACS_BTEE,  ACS_LRCORNER); // Bottom
}

void print_to_grid(int x, int y, int c) {
  mvaddch(OFFSET_TOP + (y*2) + 1, OFFSET_LEFT + (x*4) + 4, c);
}

// We can store 24 bits of data
void print_password(uint8_t a, uint8_t b, uint8_t c) {
  // Print the table
  mvprintw(17, OFFSET_LEFT, "   Press any key to continue");
  print_table();

  // Pack into a uint32
  uint32_t passcode = (a << 16 | b << 8 | c);

  // Xor it to make it more random looking
  passcode = passcode ^ 0x555555;

  // Set each bit appropriately
  int i;
  // Setting the checksum to 1 means that all-blank won't be valid
  int checksum = 1;
  int bit = 23;
  for(i = 0; i < 7; i++) {
    int j;
    for(j = 0; j < (i > 2 ? (6 - i) : (7 - i)); j++) {
      uint8_t thischar = (passcode >> bit) & 0x01 ? 1 : 0;
      bit--;

      // Count the number of bits
      checksum ^= thischar;
      print_to_grid(j, i, thischar ? ACS_DIAMOND : ' ');
      print_to_grid(6-j, 6-i, thischar ? ACS_DIAMOND : ' ');
    }
  }

  print_to_grid(3, 3, checksum ? ACS_DIAMOND : ' ');

  refresh();
  timeout(-1);
  getch();
}

uint8_t validate_password(uint8_t state[7][7], uint8_t *a, uint8_t *b, uint8_t *c) {
  uint32_t result = 0;

  int i;
  int checksum = 1;
  for(i = 0; i < 7; i++) {
    int j;
    for(j = 0; j < (i > 2 ? (6 - i) : (7 - i)); j++) {
      // Check for rotational symmetry
      if(state[i][j] != state[6 - i][6 - j]) {
        return 0;
      }

      // Update the checksum
      checksum ^= state[i][j];

      // Read the character
      result = (result << 1) | state[i][j];
    }
  }

  // Validate the checksum
  if(checksum != state[3][3]) {
    return 0;
  }

  result ^= 0x555555;

  *a = (result >> 16) & 0x0FF;
  *b = (result >> 8) & 0x0FF;
  *c = (result >> 0) & 0x0FF;

  return 1;
}

uint8_t read_password(uint8_t *a, uint8_t *b, uint8_t *c) {
  int x = 0;
  int y = 0;
  uint8_t state[7][7];
  memset(state, 0, sizeof(state));

  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);

  timeout(-1);
  for(;;) {
    clear();

    print_table();

    // Print the state
    int i;
    for(i = 0; i < 7; i++) {
      int j;
      for(j = 0; j < 7; j++) {
        if(state[i][j]) {
          print_to_grid(j, i, ACS_DIAMOND);
        }
      }
    }

    // Print the instructions
    mvprintw(OFFSET_TOP + 15, OFFSET_LEFT, "   Toggle with <space>");
    mvprintw(OFFSET_TOP + 16, OFFSET_LEFT, "   Submit with <enter>");

    // Print the cursor
    attron(COLOR_PAIR(1));
    if(state[y][x]) {
      print_to_grid(x, y, ACS_DIAMOND);
    } else {
      print_to_grid(x, y, ACS_BULLET);
    }
    attroff(COLOR_PAIR(1));

    // Go to 0,0 to keep the cursor out of the way
    move(0, 0);
    switch(getch()) {
      case 'w': case KEY_UP:    y = (y == 0) ? 6 : (y - 1); break;
      case 'a': case KEY_LEFT:  x = (x == 0) ? 6 : (x - 1); break;
      case 's': case KEY_DOWN:  y = (y + 1) % 7; break;
      case 'd': case KEY_RIGHT: x = (x + 1) % 7; break;
      case ' ': state[y][x] ^= 1; break;
      case '\n':
        if(validate_password(state, a, b, c)) {
          return 1;
        }

        clear();
        attron(COLOR_PAIR(2));
        mvprintw(0 + OFFSET_TOP, 0 + OFFSET_LEFT, "Something's wrong, please check");
        mvprintw(1 + OFFSET_TOP, 0 + OFFSET_LEFT, "your password and try again!");
        attroff(COLOR_PAIR(2));
        getch();
        break;

      default:
        return 0;
    }
  }
}
