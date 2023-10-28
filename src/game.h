#ifndef __GAME_H
#define __GAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tile_dict.h"

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

/// @brief Like a warp point, steps up and down, places that cause a level change
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

/// @brief This structure describes what section this section is alowed next to
typedef struct section_gen_key_t
{
    int16_t top;
    int16_t bottem;
    int16_t left;
    int16_t right;

} SECTION_GEN_KEY;

typedef struct section_t
{
    WH bounds;
    EXIT_LIST exits;
    STR render_data;
    SECTION_GEN_KEY gen_key;
} SECTION;

typedef struct sectionlist_t
{
    int count;
    SECTION *s;
} SECTION_LIST;

#define TILE_SIZE 10
typedef struct tile_t
{
    SECTION *section;
    XY map_pos;
    int8_t layout_flags;
    TILE_HASHMAP *tile_data;
} TILE;

// #define MAP_SIZE 3
#define MAP_LINE_Y 5
#define MAP_COL_X 16

typedef struct map_t
{

    // map[LINE Y][COL X]
    TILE tiles[MAP_LINE_Y][MAP_COL_X];
} MAP;

/// @brief Game state
typedef struct gamestate_t
{
    PLAYER player;
    // Sections avalable to map gen
    SECTION_LIST sections;
    // the map being played
    MAP map;
} GameState, *GameState_ptr;

void game_init(GameState_ptr);
int game_proc_keypress(GameState_ptr, int);
void game_free(GameState_ptr);
void parse_load_section(GameState_ptr gs, char *filepath);
void game_gen_map(GameState_ptr gs);
TILE *game_get_tile_at_pos(MAP *map, int y_line, int x_col);

#endif // __GAME_H
