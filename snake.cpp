#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

using std::cout;         using std::cin;
using std::string;       using std::vector;
using std::stringstream; using std::endl;

#define WIDTH 38
#define HEIGHT 22

#define HEAD_CHAR 'X'
#define SEG_CHAR 'x'
#define FOOD_CHAR 'o'
#define FLOOR_CHAR ' '
#define BORDER_CHAR '#'

#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_RIGHT 'd'
#define KEY_LEFT 'a'

#define COLOR_ENABLE
//#define SINGLE_DRAW // draw entire buffer at once - may improve performance in some cases

#ifdef SINGLE_DRAW
#define SCR_BUF buf_stream
#else
#define SCR_BUF cout
#endif

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

#define COL_BORDER COL_GRAY
#define COL_FOOD COL_RED
#define COL_HEAD COL_GREEN
#define COL_SEG COL_GREEN

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
void cls() {
  system("cls");
}
int key_press(char* mv) { // not working: F11 (-122, toggles fullscreen)
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
            default: *mv = (char)key; // any other ASCII/virtual character
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
int key_press(char* mv) { // not working: ¹ (251), num lock (-144), caps lock (-20), windows key (-91), kontext menu key (-93)
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
        default: *mv = (char)key; // any other ASCII character
        }
    }
}
#endif // Windows/Linux

struct scr_buf {
  const unsigned int width = WIDTH;
  const unsigned int height = HEIGHT;
  char buf[HEIGHT][WIDTH];
};

struct pos {
  unsigned int x;
  unsigned int y;
};

struct game_state {
  struct scr_buf screen;
  struct pos head;
  struct pos food;
  vector<struct pos> segments;
  char dir; // 'u', 'l', 'd', 'r'
  bool add_segment_next_loop = false;
};

void game_over(struct game_state& game) {
  cout << "\nGame over!\nFinal score: " << game.segments.size() - 1 << endl;
  exit(0);
}

void move_food(struct game_state& game) {
  game.food.x = rand() % game.screen.width;
  game.food.y = rand() % game.screen.height;
}

void print_screen(struct game_state& game) {
  struct scr_buf& screen = game.screen;
  stringstream buf_stream;

  #ifndef SINGLE_DRAW
    cls();
  #endif

  SCR_BUF << COL_BORDER;
  for (unsigned int j = 0; j < screen.width + 2; j++) {
    SCR_BUF << BORDER_CHAR;
  }
  SCR_BUF << COL_WHITE;
  SCR_BUF << " Score: " << game.segments.size() - 1;

  SCR_BUF << '\n';


  for (unsigned int i = 0; i < screen.height; i++) {
    SCR_BUF << COL_BORDER << BORDER_CHAR << COL_WHITE;
    for (unsigned int j = 0; j < screen.width; j++) {
      if (i == game.head.y && j == game.head.x)
        SCR_BUF << COL_HEAD << HEAD_CHAR << COL_WHITE;
      else if (i == game.food.y && j == game.food.x)
        SCR_BUF << COL_FOOD << FOOD_CHAR << COL_WHITE;
      else if (screen.buf[i][j] == SEG_CHAR)
        SCR_BUF << COL_SEG << SEG_CHAR << COL_WHITE;
      else
        SCR_BUF << screen.buf[i][j];
    }
    SCR_BUF << COL_BORDER << BORDER_CHAR << COL_WHITE;
    SCR_BUF << '\n';
  }

  SCR_BUF << COL_BORDER;
  for (unsigned int j = 0; j < screen.width + 2; j++) {
    SCR_BUF << BORDER_CHAR;
  }
  SCR_BUF << COL_WHITE;

  SCR_BUF << '\n';

  #ifdef SINGLE_DRAW
    cls();
    cout << SCR_BUF.str();
  #endif
}

void next_frame(struct game_state& game) {
  struct scr_buf& screen = game.screen;
  bool& add_segment_next_loop = game.add_segment_next_loop;
  static struct pos last_segment;
  static bool first_run = true;

  if (game.segments.size()) {
    struct pos new_segment;
    new_segment.x = game.segments.at(game.segments.size() - 1).x;
    new_segment.y = game.segments.at(game.segments.size() - 1).y;
    for (int i = game.segments.size() - 1; i > 0; i--) {
      game.segments.at(i) = game.segments.at(i - 1);
    }
    game.segments.at(0) = game.head;
    if (add_segment_next_loop) {
      game.segments.push_back(new_segment);
      add_segment_next_loop = false;
    }
  }

  switch (game.dir) {
    case 'u':
      if (game.head.y == 0)
        game_over(game);
      game.head.y--;
      break;
    case 'd':
      if (game.head.y == game.screen.height - 1)
        game_over(game);
      game.head.y++;
      break;
    case 'r':
      if (game.head.x == game.screen.width - 1)
        game_over(game);
      game.head.x++;
      break;
    case 'l':
      if (game.head.x == 0)
        game_over(game);
      game.head.x--;
      break;
  }

  if (game.segments.size()) {
    if (first_run) {
      first_run = false;
    }

    if (!game.add_segment_next_loop) {
      screen.buf[last_segment.y][last_segment.x] = FLOOR_CHAR;
      last_segment.x = game.segments.at(game.segments.size() - 1).x;
      last_segment.y = game.segments.at(game.segments.size() - 1).y;
    }

    screen.buf[game.segments.at(0).y][game.segments.at(0).x] = SEG_CHAR;
  }

  if (!(game.head.x == game.segments.at(0).x && game.head.y == game.segments.at(0).y)
      &&
      screen.buf[game.head.y][game.head.x] == SEG_CHAR && !first_run)
    game_over(game);

  if (game.head.x == game.food.x && game.head.y == game.food.y) {
    add_segment_next_loop = true;
    move_food(game);
    while (game.screen.buf[game.food.y][game.food.x] != FLOOR_CHAR)
      move_food(game);
  }
}

void eval_key(struct game_state& game, char key) {
  switch(key) {
    case KEY_UP:
      if (game.dir != 'd')
        game.dir = 'u';
      break;
    case KEY_DOWN:
      if (game.dir != 'u')
        game.dir = 'd';
      break;
    case KEY_RIGHT:
      if (game.dir != 'l')
        game.dir = 'r';
      break;
    case KEY_LEFT:
      if (game.dir != 'r')
        game.dir = 'l';
      break;
  }
}

void fill_screen(struct scr_buf& screen, char fill) {
  for (unsigned int i = 0; i < screen.height; i++) {
    for (unsigned int j = 0; j < screen.width; j++) {
      screen.buf[i][j] = fill;
    }
  }
}

void fill_segments(struct game_state& game) {
  for (int i = game.segments.size() - 1; i > 0; i--) {
    game.screen.buf[game.segments.at(i).y][game.segments.at(i).x] = SEG_CHAR;
  }
}

void init_game(struct game_state& game) {
  game.head.x = game.screen.width / 2;
  game.head.y = game.screen.height / 2;
  fill_screen(game.screen, FLOOR_CHAR);
  fill_segments(game);
  move_food(game);
  struct pos first_segment;
  first_segment.x = game.head.x;
  first_segment.y = game.head.y;
  game.segments.push_back(first_segment);
}

int main() {
  srand(time(0));

  struct game_state game;
  init_game(game);

  char mv;
  std::thread key_thread(key_press, &mv);

  while (true) {
    next_frame(game);
    print_screen(game);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    eval_key(game, mv);
  }

  return 0;
}
