#ifndef __RENDER_H
#define __RENDER_H

#include "global.h"
#include "game.h"
#include <curses.h>

typedef struct _win_border_struct
{
    chtype ls, rs, ts, bs, tl, tr, bl, br;
} WIN_BORDER;

typedef struct _win_struct
{

    int startx, starty;
    int height, width;
    WIN_BORDER border;
} WIN;

typedef struct render_t
{
    WIN win;

} RENDER;

void render_gamestate(GameState_ptr gs, RENDER *render);
void render_init(RENDER *render);

#endif // __RENDER_H