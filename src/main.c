
#include "game.h"
#include "global.h"
#include "assume.h"
#include "render.h"
#include <curses.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
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
  parse_load_section(&gs, "section");
  ASSUME(gs.sections.count > 0);
  ASSUME(gs.sections.s != NULL);

regen:
  game_gen_map(&gs);

  render_gamestate(&gs, &rnd);
  while ((ch = getch()) != KEY_F(1))
  {

    if (ch == 'r')
    {
      goto regen;
    }
    else if (game_proc_keypress(&gs, ch))
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

void print_win_params(WIN *p_win)
{
#ifdef _DEBUG
  mvprintw(25, 0, "%d %d %d %d", p_win->startx, p_win->starty,
           p_win->width, p_win->height);
  refresh();
#endif
}
