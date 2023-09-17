#include "render.h"
#include "global.h"
#include "game.h"
#include "assume.h"
#include <string.h>
#include <ncurses.h>

void init_win_params(WIN *, int w, int h);
void print_win_params(WIN *p_win);
void render_create_box(WIN *win, bool flag);
void render_gamestate(GameState_ptr, RENDER *);
void render_fill_rect(int y, int x, int w, int h, const char *c);

void init_color_pairs()
{
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
}

void render_init(RENDER *r)
{

    initscr();            /* Start curses mode 		*/
    start_color();        /* Start the color functionality */
    cbreak();             /* Line buffering disabled, Pass on
                           * everty thing to me 		*/
    keypad(stdscr, TRUE); /* I need that nifty F1 	*/
    noecho();
    init_color_pairs();

    /* Initialize the window parameters */
    init_win_params(&r->win, COLS - 1, LINES - 1);
    print_win_params(&r->win);

    attron(COLOR_PAIR(1));
    printw("Press F1 to exit");
    refresh();
    attroff(COLOR_PAIR(1));
    render_create_box(&r->win, TRUE);
}

void render_create_box(WIN *p_win, bool flag)
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
        render_fill_rect(y, x, w, h, " ");
    }

    refresh();
}

/// @brief NOTE! Uses y,x,w,h notation
void render_fill_rect(int y, int x, int w, int h, const char *c)
{
    for (int _y = 1; _y < h + 1; _y++)
    {
        for (int _x = 1; _x < w + 1; _x++)
        {
            mvaddstr(y + _y, x + _x, c);
        }
    }
}

void render_exits(TILE *tile, int c_y, int c_x)
{

    for (int i = 0; i < tile->section->exits.count; i++)
    {
        EXIT *e = &tile->section->exits.exits[i];
        mvaddch(c_y + e->pos.y, c_x + e->pos.x, e->c);
    }
}

void render_section(SECTION *s, int c_y, int c_x)
{
    // draw render data
    int y = 0, x = 0, curr_ch = 0;
    bool done = false;
    while (!done)
    {
        char ch = s->render_data[curr_ch];
        curr_ch++;
        if (curr_ch >= strlen(s->render_data))
            done = true; // We still get to finish this current char

        // inc line,
        if (ch == newline)
        {
            x = 0;
            y++;
            continue;
        }

        ASSUME(ch != newline);
        mvaddch(y + c_y, x + c_x, ch);
        x++;
    }
}

// #define DEV
void render_tile(TILE *tile, int c_y, int c_x)
{

    render_fill_rect(c_y, c_x, TILE_SIZE - 1, TILE_SIZE - 1, " ");

    render_section(tile->section, c_y, c_x);
#ifdef DEV
    mvaddch(c_y, c_x, '+');
    mvaddch(c_y + TILE_SIZE - 1, c_x, '+');
    mvaddch(c_y, c_x + TILE_SIZE - 1, '+');
    mvaddch(c_y + TILE_SIZE - 1, c_x + TILE_SIZE - 1, '+');
#endif
    render_exits(tile, c_y, c_x);
}

void render_map(MAP *map, WIN *win)
{
    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++)
    {
        int lin = i / MAP_SIZE;
        int col = i % MAP_SIZE;
        TILE *tile = game_get_tile_at_pos(map, lin, col);
        ASSUME(tile != NULL);
        //+1 on the px for the border
        //+ win pos for window offset
        render_tile(tile,
                    ((lin * TILE_SIZE) + 1) + win->starty,
                    ((col * TILE_SIZE) + 1) + win->startx);
    }
}

void render_gamestate(GameState_ptr gs, RENDER *r)
{
    render_map(&gs->map, &r->win);

    // Do last
    attron(COLOR_PAIR(1));
    mvprintw(gs->player.pos.y + r->win.starty,
             gs->player.pos.x + r->win.startx, "@");

    attroff(COLOR_PAIR(1));
    mvaddch(r->win.starty + r->win.height,
            r->win.startx + r->win.width, ' ');
    refresh();
}