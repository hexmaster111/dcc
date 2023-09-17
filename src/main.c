
#include "game.h"
#include "global.h"
#include "assume.h"
#include "render.h"
#include <curses.h>
#include <string.h>
#include <time.h>

// WINSIZE is 50x30

int main(int argc, char *argv[])
{
reset:
  int ch;
  GameState gs = {0};
  RENDER rnd = {0};
  const char *log_file = "log.txt";
  FILE *f = fopen(log_file, "w");
  srand(time(NULL));

  if (!f)
  {
    printf("Failed to open log file %s\n", log_file);
    return 1;
  }
  glog_init(f);
  render_init(&rnd);
  game_init(&gs);
  // TODO: this should be done by the level picker or something...
  game_load_section(&gs, "section");
  game_layout_sections(&gs);

  render_gamestate(&gs, &rnd);
  while ((ch = getch()) != KEY_F(1))
  {
    if (ch == KEY_F(2))
    {
      goto reset;
    }

    if (game_proc_keypress(&gs, ch))
    {
      break;
    };

    render_gamestate(&gs, &rnd);
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
