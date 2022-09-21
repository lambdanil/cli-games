#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<string.h>

using std::cout;
using std::cin;
using std::string;
using std::vector;

#define WIDTH 38
#define HEIGHT 22

#define HEAD_CHAR 'X'
#define SEG_CHAR 'x'
#define FOOD_CHAR 'o'
#define FLOOR_CHAR ' '
#define BORDER_CHAR '#'

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
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
  unsigned int score = 0;
  unsigned int head_pos_x = 30;
  unsigned int head_pos_y = 15;
  unsigned int fruits = 0;
  vector<struct pos> segments;
  char dir = 'u'; // u d l r for direction
};


void cls() {
  system("clear");
}

void game_end(struct game_state game) {
  cout << "\nGame over!\n";
  cout << "Final score: " << game.segments.size() << std::endl;
  exit(0);
}

void print_scr(struct game_state game) {
  cls();

  char** buf = (char**)malloc(game.screen.height * sizeof(char*));
  for (unsigned int i = 0; i < game.screen.height; i++) {
    buf[i] = (char*)malloc(game.screen.width * sizeof(char));
  }
  for (unsigned int i = 0; i < game.screen.height; i++) {
    for (unsigned int j = 0; j < game.screen.width; j++) {
        buf[i][j] = game.screen.buf[i][j];
    }
  }

  for (unsigned int i = 0; i < game.segments.size(); i++) {
    buf[game.segments.at(i).y][game.segments.at(i).x] = SEG_CHAR;
  }

  for (unsigned int j = 0; j <= game.screen.width + 1; j++)
    cout << BORDER_CHAR;

  cout << " Score: " << game.segments.size();

  cout << "\n";

  for (unsigned int i = 0; i < game.screen.height; i++) {
    cout << BORDER_CHAR;
    for (unsigned int j = 0; j < game.screen.width; j++) {
      if (game.head_pos_x == j && game.head_pos_y == i) {
        if (buf[i][j] == SEG_CHAR)
          game_end(game);
        cout << SEG_CHAR;
      }
      else
        cout << buf[i][j];
    }
    cout << BORDER_CHAR;
    cout << "\n";
  }

  for (unsigned int i = 0; i < game.screen.height; i++) {
    free(buf[i]);
  }
  free(buf);

  for (unsigned int j = 0; j <= game.screen.width + 1; j++)
    cout << BORDER_CHAR;
}

void fill_screen(struct scr_buf& screen, const char f) {
  for (unsigned int i = 0; i < screen.height; i++) {
    for (unsigned int j = 0; j < screen.width; j++) {
      screen.buf[i][j] = f;
    }
  }
}

void add_fruit(struct game_state& game) {
  unsigned int x = rand() % game.screen.width;
  unsigned int y = rand() % game.screen.height;
  game.screen.buf[y][x] = FOOD_CHAR;
  game.fruits++;
}

void eval_move(struct game_state& game) {

  static bool add_segment_next_loop;

  unsigned int new_seg_x;
  unsigned int new_seg_y;

  if (game.segments.size()) {
    // used to append new segment
    new_seg_x = game.segments.at(game.segments.size() - 1).x;
    new_seg_y = game.segments.at(game.segments.size() - 1).y;
  }
  else {
    new_seg_x = game.head_pos_x;
    new_seg_y = game.head_pos_y;
  }

  if (game.segments.size()) {
    for (unsigned int i = game.segments.size() - 1; i > 0; i--) {
      game.segments.at(i).x = game.segments.at(i - 1).x;
      game.segments.at(i).y = game.segments.at(i - 1).y;
    }
  }

  if (game.segments.size()) {
    game.segments.at(0).x = game.head_pos_x;
    game.segments.at(0).y = game.head_pos_y;
  }

  if (add_segment_next_loop) {
    add_segment_next_loop = false;
    struct pos seg;
    seg.x = new_seg_x;
    seg.y = new_seg_y;
    game.segments.push_back(seg); // add new segment
  }

  switch(game.dir) {
    case 'u':
      if (game.head_pos_y == 0)
        game_end(game);
      game.head_pos_y--;
      break;
    case 'd':
      if (game.head_pos_y == game.screen.height - 1)
        game_end(game);
      game.head_pos_y++;
      break;
    case 'r':
      if (game.head_pos_x == game.screen.width - 1)
        game_end(game);
      game.head_pos_x++;
      break;
    case 'l':
      if (game.head_pos_x == 0)
        game_end(game);
      game.head_pos_x--;
      break;
  }

  // check for fruit
  if (game.screen.buf[game.head_pos_y][game.head_pos_x] == FOOD_CHAR) {
    add_segment_next_loop = true;
    game.screen.buf[game.head_pos_y][game.head_pos_x] = FLOOR_CHAR;
    add_fruit(game);
    game.fruits--;
  }
}


int main() {
  srand((unsigned)time(0));

  char* mv = new char;
  std::thread key_thread(key_press, std::ref(mv));
  struct game_state game;
  fill_screen(game.screen, FLOOR_CHAR);
  add_fruit(game);
  while (true) {
    *mv = 0;
    eval_move(game);
    print_scr(game);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    switch(*mv) {
      case 's':
        if (game.dir != 'u')
          game.dir = 'd';
        break;
      case 'a':
        if (game.dir != 'r')
          game.dir = 'l';
        break;
      case 'd':
        if (game.dir != 'l')
          game.dir = 'r';
        break;
      case 'w':
        if (game.dir != 'd')
          game.dir = 'u';
        break;
    }

  }

  return 0;
}
