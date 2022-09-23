#include<iostream>
#include<thread>
#include<chrono>

#define MINE_CHAR '*'
#define EMPTY_CHAR ' '
#define COVER_CHAR '#'
#define UNCOVER_CHAR ' '
#define FLAG_CHAR 'f'

/*#define KEY_UP -38 // arrow keys
#define KEY_DOWN -40
#define KEY_RIGHT -39
#define KEY_LEFT -37
#define KEY_FLAG 32 // space
#define KEY_UNCOVER 10 // enter*/

#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_RIGHT 'd'
#define KEY_LEFT 'a'
#define KEY_FLAG 32
#define KEY_UNCOVER 10


#define COLOR_ENABLE

#ifdef COLOR_ENABLE
  #define COL_CYAN "\033[36m"
  #define COL_MAGENTA "\033[35m"
  #define COL_RED "\033[91m"
  #define COL_GREEN "\033[92m"
  #define COL_YELLOW "\033[33m"
  #define COL_WHITE "\033[0m"
  #define COL_GRAY "\033[90m"
#else // set all colors to white if in colorless mode
  #define COL_CYAN "\033[0m"
  #define COL_MAGENTA "\033[0m"
  #define COL_RED "\033[0m"
  #define COL_GREEN "\033[0m"
  #define COL_YELLOW "\033[0m"
  #define COL_WHITE "\033[0m"
  #define COL_GRAY "\033[0m"
#endif

#define COL_1 COL_CYAN
#define COL_2 COL_GREEN
#define COL_3 COL_RED
#define COL_4 COL_MAGENTA
#define COL_5 COL_RED
#define COL_6 COL_CYAN
#define COL_7 COL_GRAY
#define COL_8 COL_GRAY

#define COL_HIDDEN COL_GRAY
#define COL_EMPTY ""
#define COL_FLAG "\033[41m"

#define CURSOR_COLOR "\033[47m"

using std::cout;
using std::string;

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
void cls() {
  system("cls");
}
int key_press() { // not working: F11 (-122, toggles fullscreen)
    KEY_EVENT_RECORD keyevent;
    INPUT_RECORD irec;
    DWORD events;
    while (true) {
        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &irec, 1, &events);
        if (irec.EventType == KEY_EVENT && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown) {
            keyevent = (KEY_EVENT_RECORD&)irec.Event;
            const int ca = (int)keyevent.uChar.AsciiChar;
            const int cv = (int)keyevent.wVirtualKeyCode;
            const int key = ca == 0 ? -cv : ca + (ca > 0 ? 0 : 256);
            switch (key) {
            case  -16: continue; // disable Shift
            case  -17: continue; // disable Ctrl / AltGr
            case  -18: continue; // disable Alt / AltGr
            case -220: continue; // disable first detection of "^" key (not "^" symbol)
            case -221: continue; // disable first detection of "`" key (not "`" symbol)
            case -191: continue; // disable AltGr + "#"
            case  -52: continue; // disable AltGr + "4"
            case  -53: continue; // disable AltGr + "5"
            case  -54: continue; // disable AltGr + "6"
            case  -12: continue; // disable num block 5 with num lock deactivated
            case   13: return  10; // enter
            case  -46: return 127; // delete
            case  -49: return 251; // ¹
            case    0: continue;
            case    1: continue; // disable Ctrl + a (selects all text)
            case    2: continue; // disable Ctrl + b
            case    3: continue; // disable Ctrl + c (terminates program)
            case    4: continue; // disable Ctrl + d
            case    5: continue; // disable Ctrl + e
            case    6: continue; // disable Ctrl + f (opens search)
            case    7: continue; // disable Ctrl + g
            //case    8: continue; // disable Ctrl + h (ascii for backspace)
            //case    9: continue; // disable Ctrl + i (ascii for tab)
            case   10: continue; // disable Ctrl + j
            case   11: continue; // disable Ctrl + k
            case   12: continue; // disable Ctrl + l
            //case   13: continue; // disable Ctrl + m (breaks console, ascii for new line)
            case   14: continue; // disable Ctrl + n
            case   15: continue; // disable Ctrl + o
            case   16: continue; // disable Ctrl + p
            case   17: continue; // disable Ctrl + q
            case   18: continue; // disable Ctrl + r
            case   19: continue; // disable Ctrl + s
            case   20: continue; // disable Ctrl + t
            case   21: continue; // disable Ctrl + u
            case   22: continue; // disable Ctrl + v (inserts clipboard)
            case   23: continue; // disable Ctrl + w
            case   24: continue; // disable Ctrl + x
            case   25: continue; // disable Ctrl + y
            case   26: continue; // disable Ctrl + z
            default: return key; // any other ASCII/virtual character
            }
        }
    }
}
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
void cls() {
  system("clear");
}
int key_press() { // not working: ¹ (251), num lock (-144), caps lock (-20), windows key (-91), kontext menu key (-93)
    struct termios term;
    tcgetattr(0, &term);
    while (true) {
        term.c_lflag &= ~(ICANON | ECHO); // turn off line buffering and echoing
        tcsetattr(0, TCSANOW, &term);
        int nbbytes;
        ioctl(0, FIONREAD, &nbbytes); // 0 is STDIN
        while (!nbbytes) {
            sleep(0.01);
            fflush(stdout);
            ioctl(0, FIONREAD, &nbbytes); // 0 is STDIN
        }
        int key = (int)getchar();
        if (key == 27 || key == 194 || key == 195) { // escape, 194/195 is escape for °ß´äöüÄÖÜ
            key = (int)getchar();
            if (key == 91) { // [ following escape
                key = (int)getchar(); // get code of next char after \e[
                if (key == 49) { // F5-F8
                    key = 62 + (int)getchar(); // 53, 55-57
                    if (key == 115) key++; // F5 code is too low by 1
                    getchar(); // take in following ~ (126), but discard code
                }
                else if (key == 50) { // insert or F9-F12
                    key = (int)getchar();
                    if (key == 126) { // insert
                        key = 45;
                    }
                    else { // F9-F12
                        key += 71; // 48, 49, 51, 52
                        if (key < 121) key++; // F11 and F12 are too low by 1
                        getchar(); // take in following ~ (126), but discard code
                    }
                }
                else if (key == 51 || key == 53 || key == 54) { // delete, page up/down
                    getchar(); // take in following ~ (126), but discard code
                }
            }
            else if (key == 79) { // F1-F4
                key = 32 + (int)getchar(); // 80-83
            }
            key = -key; // use negative numbers for escaped keys
        }
        term.c_lflag |= (ICANON | ECHO); // turn on line buffering and echoing
        tcsetattr(0, TCSANOW, &term);
        switch (key) {
        case  127: return   8; // backspace
        case  -27: return  27; // escape
        case  -51: return 127; // delete
        case -164: return 132; // ä
        case -182: return 148; // ö
        case -188: return 129; // ü
        case -132: return 142; // Ä
        case -150: return 153; // Ö
        case -156: return 154; // Ü
        case -159: return 225; // ß
        case -181: return 230; // µ
        case -167: return 245; // §
        case -176: return 248; // °
        case -178: return 253; // ²
        case -179: return 252; // ³
        case -180: return 239; // ´
        case  -65: return -38; // up arrow
        case  -66: return -40; // down arrow
        case  -68: return -37; // left arrow
        case  -67: return -39; // right arrow
        case  -53: return -33; // page up
        case  -54: return -34; // page down
        case  -72: return -36; // pos1
        case  -70: return -35; // end
        case    0: continue;
        case    1: continue; // disable Ctrl + a
        case    2: continue; // disable Ctrl + b
        case    3: continue; // disable Ctrl + c (terminates program)
        case    4: continue; // disable Ctrl + d
        case    5: continue; // disable Ctrl + e
        case    6: continue; // disable Ctrl + f
        case    7: continue; // disable Ctrl + g
        case    8: continue; // disable Ctrl + h
        //case    9: continue; // disable Ctrl + i (ascii for tab)
        //case   10: continue; // disable Ctrl + j (ascii for new line)
        case   11: continue; // disable Ctrl + k
        case   12: continue; // disable Ctrl + l
        case   13: continue; // disable Ctrl + m
        case   14: continue; // disable Ctrl + n
        case   15: continue; // disable Ctrl + o
        case   16: continue; // disable Ctrl + p
        case   17: continue; // disable Ctrl + q
        case   18: continue; // disable Ctrl + r
        case   19: continue; // disable Ctrl + s
        case   20: continue; // disable Ctrl + t
        case   21: continue; // disable Ctrl + u
        case   22: continue; // disable Ctrl + v
        case   23: continue; // disable Ctrl + w
        case   24: continue; // disable Ctrl + x
        case   25: continue; // disable Ctrl + y
        case   26: continue; // disable Ctrl + z (terminates program)
        default: return key; // any other ASCII character
        }
    }
}
#endif // Windows/Linux

struct rel_pos {
  int x;
  int y;
};

struct rel_pos adjacent[8];

struct pos {
  unsigned int x;
  unsigned int y;
};

struct scr_buf {
  unsigned int width;
  unsigned int height;
  char** buf;
};

struct game_state {
  int uncovered;
  int flags;
  unsigned int mines;
  unsigned int maxmines;
  char difficulty; // 'e'asy, 'm'edium, 'E'xpert
  struct scr_buf screen;
  struct scr_buf grid;
  struct pos cursor;
};

void alloc_scr_buf(struct scr_buf& scr) {
  scr.buf = new char*[scr.height];
  for (unsigned int i = 0; i < scr.height; i++) {
    scr.buf[i] = new char[scr.width];
  }
}

void free_scr_buf(struct scr_buf& scr) {
  for (unsigned int i = 0; i < scr.height; i++) {
    delete[](scr.buf[i]);
  }
  delete[](scr.buf);
}

void quit_game(struct game_state& game) {
  cout << "\nGame over!\n";
  free_scr_buf(game.screen);
  free_scr_buf(game.grid);
  exit(0);
}

void print_buf(struct scr_buf& scr) {
  const unsigned int height = scr.height;
  const unsigned int width = scr.width;
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      cout << scr.buf[i][j];
    }
    cout << '\n';
  }
}

string get_color(char c) {
  switch(c) {
    case '1':
      return COL_1;
    case '2':
      return COL_2;
    case '3':
      return COL_3;
    case '4':
      return COL_4;
    case '5':
      return COL_5;
    case '6':
      return COL_6;
    case '7':
      return COL_7;
    case '8':
      return COL_8;
    case UNCOVER_CHAR:
      return COL_EMPTY;
    case COVER_CHAR:
      return COL_HIDDEN;
    case FLAG_CHAR:
      return COL_FLAG;
  }
  return "";
}

void print_game(struct game_state& game) {
  cls();
  struct scr_buf& scr = game.screen;
  struct scr_buf& grid = game.grid;
  char out;
  string extra_col;

  const unsigned int height = scr.height;
  const unsigned int width = scr.width;
  cout << "Mines Left: " << game.mines << "\n";
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      if (scr.buf[i][j] == UNCOVER_CHAR)
        out = grid.buf[i][j];
      else if (scr.buf[i][j] == FLAG_CHAR)
        out = FLAG_CHAR;
      else
        out = COVER_CHAR;
      extra_col = get_color(out);
      if (game.cursor.x == j && game.cursor.y == i)
        cout << extra_col << CURSOR_COLOR << out << COL_WHITE;
      else
        cout << extra_col << out << COL_WHITE;
    }
    cout << '\n';
  }
}

void shuffle_grid(struct scr_buf& grid) {
  const unsigned int height = grid.height;
  const unsigned int width = grid.width;
  char tmp;
  unsigned int n_x;
  unsigned int n_y;
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      tmp = grid.buf[i][j];
      n_x = rand() % width;
      n_y = rand() % height;
      grid.buf[i][j] = grid.buf[n_y][n_x];
      grid.buf[n_y][n_x] = tmp;
    }
  }
}

void set_adjacent(struct rel_pos* p) {
  p[0].x = 0; p[0].y =-1;
  p[1].x =-1; p[1].y = 0;
  p[2].x =-1; p[2].y =-1;
  p[3].x = 0; p[3].y = 1;
  p[4].x = 1; p[4].y = 0;
  p[5].x = 1; p[5].y = 1;
  p[6].x =-1; p[6].y = 1;
  p[7].x = 1; p[7].y =-1;
}

unsigned int count_adjacent(struct scr_buf& grid, struct pos abs) {
  struct rel_pos c;
  unsigned int n = 0;
  for (int k = 0; k < 8; k++) {
    c = adjacent[k];
    if (abs.y+c.y >= 0 && abs.x+c.x >= 0 && abs.y+c.y < grid.height && abs.x+c.x < grid.width)
      if (grid.buf[abs.y+c.y][abs.x+c.x] == MINE_CHAR)
        n++;
  }
  return n;
}

unsigned int count_adjacent_flags(struct scr_buf& grid, struct pos abs) {
  struct rel_pos c;
  unsigned int n = 0;
  for (int k = 0; k < 8; k++) {
    c = adjacent[k];
    if (abs.y+c.y >= 0 && abs.x+c.x >= 0 && abs.y+c.y < grid.height && abs.x+c.x < grid.width)
      if (grid.buf[abs.y+c.y][abs.x+c.x] == FLAG_CHAR)
        n++;
  }
  return n;
}

void add_nums(struct scr_buf& grid) {
  unsigned int n;
  struct pos p;
  for (int i = 0; i < grid.height; i++) {
    for (int j = 0; j < grid.width; j++) {
      p.x = j;
      p.y = i;
      n = count_adjacent(grid, p);
      if (n && grid.buf[i][j] != MINE_CHAR)
        grid.buf[i][j] = '0' + n;
      else if (grid.buf[i][j] != MINE_CHAR)
        grid.buf[i][j] = EMPTY_CHAR;
    }
  }
}

void flush_mines(struct game_state& game) {
  for (unsigned int i = 0; i < game.grid.height; i++) {
    for (unsigned int j = 0; j < game.grid.width; j++) {
      game.grid.buf[i][j] = '0';
    }
  }
}

void fill_mines(struct game_state& game) {
  unsigned int set_mines = game.maxmines;
  for (unsigned int i = 0; i < game.grid.height; i++) {
    for (unsigned int j = 0; j < game.grid.width; j++) {
      game.grid.buf[i][j] = MINE_CHAR;
      set_mines--;
      if (!set_mines)
        break;
    }
    if (!set_mines)
      break;
  }
  shuffle_grid(game.grid);
  add_nums(game.grid);
}

void set_screen(struct game_state& game) {
  char** arr = game.screen.buf;
  for (unsigned int i = 0; i < game.screen.height; i++) {
    for (unsigned int j = 0; j < game.screen.width; j++) {
      arr[i][j] = COVER_CHAR;
    }
  }
}

void uncover_tile(struct game_state& game, struct pos tile) {
  struct rel_pos c;
  struct pos abs;
  if (game.screen.buf[tile.y][tile.x] == FLAG_CHAR)
    return;
  if (game.grid.buf[tile.y][tile.x] == MINE_CHAR)
    quit_game(game);
  if (game.screen.buf[tile.y][tile.x] == UNCOVER_CHAR &&
      (int)(game.grid.buf[tile.y][tile.x] - '0') == count_adjacent_flags(game.screen, tile)) {
    for (int k = 0; k < 8; k++) {
      c = adjacent[k];
      if (tile.y+c.y >= 0 && tile.x+c.x >= 0 && tile.y+c.y < game.grid.height && tile.x+c.x < game.grid.width) {
        abs.x = tile.x + c.x;
        abs.y = tile.y + c.y;
        if (game.screen.buf[abs.y][abs.x] == COVER_CHAR)
          uncover_tile(game, abs);
      }
    }
  }
  if (game.screen.buf[tile.y][tile.x] != UNCOVER_CHAR) {
    game.screen.buf[tile.y][tile.x] = UNCOVER_CHAR;
    game.uncovered++;
  }
  if (game.grid.buf[tile.y][tile.x] == EMPTY_CHAR) {
    for (int k = 0; k < 8; k++) {
      c = adjacent[k];
      if (tile.y+c.y >= 0 && tile.x+c.x >= 0 && tile.y+c.y < game.grid.height && tile.x+c.x < game.grid.width) {
        abs.x = tile.x + c.x;
        abs.y = tile.y + c.y;
        if (game.screen.buf[abs.y][abs.x] == COVER_CHAR)
          uncover_tile(game, abs);
      }
    }
  }
}

void flag_tile(struct game_state& game, struct pos tile) {
  if (game.screen.buf[tile.y][tile.x] == COVER_CHAR) {
    game.screen.buf[tile.y][tile.x] = FLAG_CHAR;
    game.mines--;
  }
  else if (game.screen.buf[tile.y][tile.x] == FLAG_CHAR) {
    game.screen.buf[tile.y][tile.x] = COVER_CHAR;
    game.mines++;
  }
}

void init_game(struct game_state& game, char diff) {
  set_adjacent(adjacent);
  game.difficulty = diff;
  switch(diff) {
    case 'e':
      game.screen.width = 9;
      game.screen.height = 9;
      game.grid.width = 9;
      game.grid.height = 9;
      game.mines = 10;
      break;
    case 'm':
      game.screen.width = 15;
      game.screen.height = 15;
      game.grid.width = 15;
      game.grid.height = 15;
      game.mines = 40;
      break;
    case 'E':
      game.screen.width = 30;
      game.screen.height = 16;
      game.grid.width = 30;
      game.grid.height = 16;
      game.mines = 99;
      break;
  }
  game.flags = game.mines;
  game.uncovered = 0;
  game.maxmines = game.mines;
  alloc_scr_buf(game.screen);
  alloc_scr_buf(game.grid);
  fill_mines(game);
  game.cursor.x = game.screen.width / 2;
  game.cursor.y = game.screen.height / 2;
  set_screen(game);
}

void eval_key(struct game_state& game, char key) {
  switch (key) {
    case KEY_UP:
      if (game.cursor.y != 0)
        game.cursor.y--;
      break;
    case KEY_DOWN:
      if (game.cursor.y != game.screen.height - 1)
        game.cursor.y++;
      break;
    case KEY_LEFT:
      if (game.cursor.x != 0)
        game.cursor.x--;
      break;
    case KEY_RIGHT:
      if (game.cursor.x != game.screen.width - 1)
        game.cursor.x++;
      break;
    case KEY_UNCOVER:
      uncover_tile(game, game.cursor);
      break;
    case KEY_FLAG:
      flag_tile(game, game.cursor);
      break;
  }
}

void check_wincond(struct game_state& game) {
  int res = game.maxmines;
  if (!game.mines) {
    for (int i = 0; i < game.grid.height; i++) {
      for (int j = 0; j < game.grid.width; j++) {
        if (game.screen.buf[i][j] == FLAG_CHAR && game.grid.buf[i][j] == MINE_CHAR)
          res--;
      }
    }
  }
  if (game.uncovered == (game.grid.width * game.grid.height) - game.maxmines)
    res = 0;
  if (!res) {
        cout << "\nYou won!\n";
        free_scr_buf(game.screen);
        free_scr_buf(game.grid);
        exit(0);
  }
}

int main() {
  srand((unsigned)time(0));
  char dif = 0;
  cls();
  cout << "\nPlease choose a difficulty:\t(1) Easy\n\t\t\t\t(2) Intermediate\n\t\t\t\t(3) Expert:\n\n";
  while (dif != '1' && dif != '2' && dif != '3') {
    std::cin >> dif;
  }
  std::cin.ignore();
  if (dif == '1')
    dif = 'e';
  if (dif == '2')
    dif = 'm';
  if (dif == '3')
    dif = 'E';
  char mv;
  struct game_state game;
  init_game(game, dif);
  bool first_uncover = true;

  while (true) {
    print_game(game);
    mv = key_press();
    if (mv == KEY_UNCOVER && first_uncover && game.screen.buf[game.cursor.y][game.cursor.x] != FLAG_CHAR) { // Ensure first click always uncovers empty area
      while (game.grid.buf[game.cursor.y][game.cursor.x] != EMPTY_CHAR) {
        flush_mines(game);
        fill_mines(game);
      }
      first_uncover = false;
    }
    eval_key(game, mv);
    check_wincond(game);
  }
  return 0;
}
