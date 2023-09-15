
#include <ncurses.h>
#include "game.h"
#include "global.h"
#include "assume.h"
#include <string.h>

typedef struct _win_border_struct
{
  chtype ls, rs, ts, bs, tl, tr, bl, br;
} WIN_BORDER;

typedef struct _WIN_struct
{

  int startx, starty;
  int height, width;
  WIN_BORDER border;
} WIN;

void init_win_params(WIN *, int w, int h);
void print_win_params(WIN *p_win);
void create_box(WIN *win, bool flag);
void render(GameState_ptr, WIN *);
void clear_rect(int y, int x, int w, int h, const char *c);

void init_color_pairs()
{
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
}

// WINSIZE is 50x30

int main(int argc, char *argv[])
{
  WIN win;
  int ch;
  GameState gs = {0};

  const char *log_file = "log.txt";
  FILE *f = fopen(log_file, "w");
  if (!f)
  {
    printf("Failed to open log file %s\n", log_file);
    return 1;
  }
  glog_init(f);

  game_init(&gs);
  game_load_section(&gs, "section");

  initscr();            /* Start curses mode 		*/
  start_color();        /* Start the color functionality */
  cbreak();             /* Line buffering disabled, Pass on
                         * everty thing to me 		*/
  keypad(stdscr, TRUE); /* I need that nifty F1 	*/
  noecho();
  init_color_pairs();

  /* Initialize the window parameters */
  init_win_params(&win, COLS - 1, LINES - 1);
  print_win_params(&win);

  attron(COLOR_PAIR(1));
  printw("Press F1 to exit");
  refresh();
  attroff(COLOR_PAIR(1));
  create_box(&win, TRUE);

  render(&gs, &win);
  while ((ch = getch()) != KEY_F(1))
  {
    if (game_proc_keypress(&gs, ch))
    {
      break;
    };

    render(&gs, &win);
  }
  endwin(); /* End curses mode		  */
  game_free(&gs);
  glog_destroy();
  return 0;
}

void init_win_params(WIN *p_win, int w, int h)
{
  p_win->height = h;
  p_win->width = w;
  p_win->starty = (LINES - p_win->height) / 2;
  p_win->startx = (COLS - p_win->width) / 2;

  p_win->border.ls = '|';
  p_win->border.rs = '|';
  p_win->border.ts = '-';
  p_win->border.bs = '-';
  p_win->border.tl = '+';
  p_win->border.tr = '+';
  p_win->border.bl = '+';
  p_win->border.br = '+';
}
void print_win_params(WIN *p_win)
{
#ifdef _DEBUG
  mvprintw(25, 0, "%d %d %d %d", p_win->startx, p_win->starty,
           p_win->width, p_win->height);
  refresh();
#endif
}
void create_box(WIN *p_win, bool flag)
{
  int x, y, w, h;

  x = p_win->startx;
  y = p_win->starty;
  w = p_win->width;
  h = p_win->height;

  if (flag == TRUE)
  {
    mvaddch(y, x, p_win->border.tl);
    mvaddch(y, x + w, p_win->border.tr);
    mvaddch(y + h, x, p_win->border.bl);
    mvaddch(y + h, x + w, p_win->border.br);
    mvhline(y, x + 1, p_win->border.ts, w - 1);
    mvhline(y + h, x + 1, p_win->border.bs, w - 1);
    mvvline(y + 1, x, p_win->border.ls, h - 1);
    mvvline(y + 1, x + w, p_win->border.rs, h - 1);
  }
  else
  {
    clear_rect(y, x, w, h, " ");
  }

  refresh();
}
#define DEV

/// @brief NOTE! Uses y,x,w,h notation
void clear_rect(int y, int x, int w, int h, const char *c)
{
  for (int _y = 1; _y < h + 1; _y++)
  {
    for (int _x = 1; _x < w + 1; _x++)
    {
      mvaddstr(y + _y, x + _x, c);
    }
  }
}

void render(GameState_ptr gs, WIN *win)
{
  // TODO: RENDER floor, walls, etc

  for (int i = 0; i < gs->sections.count; i++)
  {
    SECTION s = gs->sections.s[i];

    XY org = {.x = s.bounds.pos.x + win->startx,
              .y = s.bounds.pos.y + win->starty};

    clear_rect(org.y, org.x, s.bounds.size.w, s.bounds.size.h, " ");

    // draw render data
    int y = 0, x = 0, curr_ch = 0;
    bool done = false;
    while (!done)
    {
      char ch = s.render_data[curr_ch];
      curr_ch++;
      if (curr_ch >= strlen(s.render_data))
        done = true; // We still get to finish this current char

      // inc line,
      if (ch == newline)
      {
        x = 0;
        y++;
        continue;
      }

      ASSUME(ch != newline);
      mvaddch(y + org.y, x + org.x, ch);
      x++;
    }

    // mvprintw(org.y, org.x, s.render_data);
  }

#ifdef DEV

  int line_x, line_y;
  line_x = win->startx + 1;

  line_y = win->starty + 1;
  mvprintw(line_y, win->startx + 3,
           "Player: x:%d y:%d",
           gs->player.pos.x, gs->player.pos.y);

  // render the room outlines
  for (int i = 0; i < gs->sections.count; i++)
  {

    SECTION s = gs->sections.s[i];
    int x = s.bounds.pos.x;
    int y = s.bounds.pos.y;
    int w = s.bounds.size.w;
    int h = s.bounds.size.h;
    line_y++;
    mvprintw(line_y, line_x, "  section %d: x:%d y:%d w:%d h:%d", i, x, y, w, h);
    mvprintw(y + win->starty, x + win->startx, "+");
    mvprintw(y + win->starty, x + win->startx + w, "+");
    mvprintw(y + win->starty + h, x + win->startx, "+");
    mvprintw(y + win->starty + h, x + win->startx + w, "+");
    // render section exit with #
    if (0 < s.exits.count)
    {
      line_y++;
    }
    for (int j = 0; j < s.exits.count; j++)
    {
      EXIT *e = &s.exits.exits[j];
      mvprintw(line_y, line_x + 10, "x: %d, y: %d",
               e->pos.x,
               e->pos.y);

      mvaddch(
          s.bounds.pos.y + e->pos.y,
          s.bounds.pos.x + e->pos.x,
          '#');
    }
  }

#endif

  // Do last
  attron(COLOR_PAIR(1));
  mvprintw(gs->player.pos.y + win->starty, gs->player.pos.x + win->startx, "@");
  attroff(COLOR_PAIR(1));
  mvaddch(win->starty + win->height, win->startx + win->width, ' ');
  refresh();
}