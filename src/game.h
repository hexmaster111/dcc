#ifndef __GAME_H
#define __GAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct point_t
{
    unsigned int x, y;
} XY;

typedef struct widthheight_t
{
    unsigned int w, h;
} WH;

typedef struct rect_t
{
    XY pos;
    WH size;
} RECT;

typedef struct entity_t
{
    XY pos;
} PLAYER;

typedef struct exit_t
{
    XY pos;
} EXIT;

typedef struct exit_list_t
{
    int count;
    EXIT *exits;
} EXIT_LIST;

typedef struct section_t
{
    RECT bounds;
    EXIT_LIST exits;

} SECTION;

/// @brief Game state
#define def_section_count 4
typedef struct gamestate_t
{
    PLAYER player;
    int section_count;
    SECTION sections[def_section_count]; // TODO: dynamic array
} GameState, *GameState_ptr;

void game_init(GameState_ptr);
int game_proc_keypress(GameState_ptr, int);
void game_free(GameState_ptr);

#endif // __GAME_H
