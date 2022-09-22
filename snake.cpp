#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <string.h>
#include <cstdlib>
#include "input.h" // int key_press(char*); void cls();

using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::stringstream;

#define WIDTH 38
#define HEIGHT 22
#define COLOR_ENABLE // comment to false to disable colors

//#define SINGLE_DRAW // draw entire screen buffer at once - may improve performance on some consoles

#ifdef SINGLE_DRAW
#define SCR_BUF scr_buf
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

#define HEAD_CHAR 'X'
#define SEG_CHAR 'x'
#define FOOD_CHAR 'o'
#define FLOOR_CHAR ' '
#define BORDER_CHAR '#'

struct scr_buf {
  const unsigned int width = WIDTH;
  const unsigned int height = HEIGHT;
  char buf[HEIGHT][WIDTH];
  char sbuf[HEIGHT][WIDTH];
};

struct pos {
  unsigned int x;
  unsigned int y;
};

struct game_state {
  struct scr_buf screen;
  bool add_segment_next_loop = true;
  unsigned int score = 0;
  struct pos head;
  unsigned int fruits = 0;
  vector<struct pos> segments;
  char dir; // u d l r for direction
};


void game_end(struct game_state game) {
  cout << "\nGame over!\n";
  cout << "Final score: " << game.segments.size() - 2 << std::endl;
  exit(0);
}


void print_scr(struct game_state& game) {

  #ifndef SINGLE_DRAW
    cls();
  #endif

  stringstream scr_buf;

  if (game.segments.size())
    game.screen.sbuf[game.segments.at(game.segments.size() - 1).y][game.segments.at(game.segments.size() - 1).x] = game.screen.buf[game.segments.at(game.segments.size() - 1).y][game.segments.at(game.segments.size() - 1).x];

  if (game.screen.sbuf[game.head.y][game.head.x] == SEG_CHAR)
    game_end(game);

  game.screen.sbuf[game.head.y][game.head.x] = HEAD_CHAR;
  if (game.segments.size())
    game.screen.sbuf[game.segments.at(0).y][game.segments.at(0).x] = SEG_CHAR;

  // add fruit into empty space
  if (game.screen.buf[game.head.y][game.head.x] == FOOD_CHAR) {
    game.screen.buf[game.head.y][game.head.x] = FLOOR_CHAR;
    unsigned int fx = rand() % game.screen.width;
    unsigned int fy = rand() % game.screen.height;
    while (game.screen.sbuf[fy][fx] != FLOOR_CHAR) {
      fx = rand() % game.screen.width;
      fy = rand() % game.screen.height;
    }
    game.screen.buf[fy][fx] = FOOD_CHAR;
    if (game.screen.sbuf[fy][fx] == FLOOR_CHAR)
      game.screen.sbuf[fy][fx] = FOOD_CHAR;
    game.add_segment_next_loop = true;
  }

  for (unsigned int j = 0; j <= game.screen.width + 1; j++)
    SCR_BUF << COL_BORDER << BORDER_CHAR << COL_WHITE;

  SCR_BUF << " Score: " << game.segments.size() - 2;

  SCR_BUF << "\n";

  for (unsigned int i = 0; i < game.screen.height; i++) {
    SCR_BUF << COL_BORDER << BORDER_CHAR << COL_WHITE;
    for (unsigned int j = 0; j < game.screen.width; j++) {
        if (game.screen.sbuf[i][j] == SEG_CHAR)
          SCR_BUF << COL_SEG << game.screen.sbuf[i][j] << COL_WHITE;
        else if (game.screen.sbuf[i][j] == HEAD_CHAR)
          SCR_BUF << COL_HEAD << game.screen.sbuf[i][j] << COL_WHITE;
        else if (game.screen.sbuf[i][j] == FOOD_CHAR)
          SCR_BUF << COL_FOOD << game.screen.sbuf[i][j] << COL_WHITE;
        else
          SCR_BUF << game.screen.sbuf[i][j];
    }
    SCR_BUF << COL_BORDER << BORDER_CHAR << COL_WHITE;
    SCR_BUF << '\n';
    #ifdef SINGLE_DRAW
      cls();
      cout << SCR_BUF.str();
    #endif
  }

  for (unsigned int j = 0; j <= game.screen.width + 1; j++)
    cout << COL_BORDER << BORDER_CHAR << COL_WHITE;
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
}

void next_frame(struct game_state& game) {

  struct pos new_seg;

  if (game.segments.size()) {
    // used to append new segment
    new_seg.x = game.segments.at(game.segments.size() - 1).x;
    new_seg.y = game.segments.at(game.segments.size() - 1).y;
  }
  else {
    new_seg.x = game.head.x;
    new_seg.y = game.head.y;
  }

  if (game.segments.size()) {
    for (unsigned int i = game.segments.size() - 1; i > 0; i--) {
      game.segments.at(i).x = game.segments.at(i - 1).x;
      game.segments.at(i).y = game.segments.at(i - 1).y;
    }
  }

  if (game.segments.size()) {
    game.segments.at(0).x = game.head.x;
    game.segments.at(0).y = game.head.y;
  }

  if (game.add_segment_next_loop) {
    game.add_segment_next_loop = false;
    game.segments.push_back(new_seg); // add new segment
  }

  switch(game.dir) {
    case 'u':
      if (game.head.y == 0)
        game_end(game);
      game.head.y--;
      break;
    case 'd':
      if (game.head.y == game.screen.height - 1)
        game_end(game);
      game.head.y++;
      break;
    case 'r':
      if (game.head.x == game.screen.width - 1)
        game_end(game);
      game.head.x++;
      break;
    case 'l':
      if (game.head.x == 0)
        game_end(game);
      game.head.x--;
      break;
  }
}

void eval_move(struct game_state& game, char mv) {
  switch(mv) {
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

void copy_scr(char from[HEIGHT][WIDTH], char to[HEIGHT][WIDTH]) {
  for (unsigned int i = 0; i < HEIGHT; i++)
    for (unsigned int j = 0; j < WIDTH; j++)
      to[i][j] = from[i][j];
}

int main() {
  srand((unsigned)time(0));

  char* mv = new char;
  std::thread key_thread(key_press, std::ref(mv));
  struct game_state game;
  game.head.x = game.screen.width / 2;
  game.head.y = game.screen.height / 2;
  fill_screen(game.screen, FLOOR_CHAR);
  add_fruit(game);
  copy_scr(game.screen.buf, game.screen.sbuf);
  struct pos head; // head segment needed
  head.x = game.head.x;
  head.y = game.head.y;
  game.segments.push_back(head);
  while (true) {
    *mv = 0;
    next_frame(game);
    print_scr(game);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    eval_move(game, *mv);
  }
  return 0;
}
