#ifndef __GAME_H
#define __GAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define newline '\n'

typedef struct point_t
{
    unsigned int x, y;
} XY;

typedef struct widthheight_t
{
    // Could be thought of as cols
    unsigned int w;
    // could be thought of as rows or lines
    unsigned int h;
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

typedef enum exit_e
{
    E_DOWN = 1,
    E_UP,
    E_DOOR
} E_EXIT;

typedef struct exit_t
{
    XY pos;
    E_EXIT type;
    char c;
} EXIT;

typedef struct exit_list_t
{
    int count;
    EXIT *exits;
} EXIT_LIST;

typedef char *STR;

typedef struct section_t
{
    RECT bounds;
    EXIT_LIST exits;
    STR render_data;
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
