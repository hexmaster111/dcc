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
    char *render_data;
} SECTION;

typedef struct sectionlist_t
{
    int count;
    SECTION *s;
} SECTION_LIST;

/// @brief Game state
typedef struct gamestate_t
{
    PLAYER player;
    SECTION_LIST sections;
} GameState, *GameState_ptr;

void game_init(GameState_ptr);
int game_proc_keypress(GameState_ptr, int);
void game_free(GameState_ptr);
void game_load_section(GameState_ptr gs, char *filepath);

#endif // __GAME_H
