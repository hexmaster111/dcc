#include "game.h"
#include <curses.h>

void game_init(GameState_ptr gs)
{
    gs->player.pos.x = 17;
    gs->player.pos.y = 17;

    gs->sections[0].bounds.pos.x = 15;
    gs->sections[0].bounds.pos.y = 10;
    gs->sections[0].bounds.size.w = 10;
    gs->sections[0].bounds.size.h = 10;

    gs->sections[0].exits.count = 1;
    gs->sections[0].exits.exits = malloc(sizeof(EXIT) * gs->sections[0].exits.count);
    gs->sections[0].exits.exits[0].pos.x = 2;
    gs->sections[0].exits.exits[0].pos.y = 2;

    gs->sections[1].bounds.pos.x = 30;
    gs->sections[1].bounds.pos.y = 25;
    gs->sections[1].bounds.size.w = 15;
    gs->sections[1].bounds.size.h = 15;
    gs->section_count = def_section_count;
};

void game_free(GameState_ptr gs)
{
    free(gs->sections[0].exits.exits);
}

int game_proc_keypress(GameState_ptr gs, int ch)
{
    switch (ch)
    {
    case KEY_UP:
        gs->player.pos.y--;
        break;
    case KEY_DOWN:
        gs->player.pos.y++;
        break;
    case KEY_LEFT:
        gs->player.pos.x--;
        break;
    case KEY_RIGHT:
        gs->player.pos.x++;
        break;
    }

    return 0; // non 0 to quit
}