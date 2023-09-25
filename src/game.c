#include "game.h"
#include "assume.h"
#include "global.h"
#include "tile_queue.h"
#include "parser.h"
#include <curses.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

// START GAME LAYOUT ##################################################################

#define SECT_LAYOUT_FLAG_HAS_BEEN_PLACED 1
int tile_is_placed(TILE *t)
{
    return t->layout_flags & SECT_LAYOUT_FLAG_HAS_BEEN_PLACED;
}

void tile_set_placed(TILE *t)
{
    t->layout_flags |= SECT_LAYOUT_FLAG_HAS_BEEN_PLACED;
}

void tile_set_section(TILE *t, SECTION *s)
{
    ASSUME(t != NULL);
    ASSUME(s != NULL);
    t->section = s;
}

TILE *game_get_tile_at_pos(MAP *map, int y_line, int x_col)
{
    if (y_line < 0 || y_line >= MAP_SIZE)
        return NULL;

    if (x_col < 0 || x_col >= MAP_SIZE)
        return NULL;

    map->tiles[y_line][x_col].map_pos.x = x_col;
    map->tiles[y_line][x_col].map_pos.y = y_line;

    return &map->tiles[y_line][x_col];
}

void section_list_add(SECTION_LIST *list, SECTION *s)
{
    ASSUME(list != NULL);
    ASSUME(s != NULL);
    list->count++;
    list->s = realloc(list->s, sizeof(SECTION) * list->count);
    ASSUME(list->s != NULL);
    list->s[list->count - 1] = *s;
}

typedef int USED_TILE;
/// @brief Returns a unique random tile from the given map
TILE *get_random_unused_tile(MAP *map, USED_TILE *used)
{
    // Pick a random number from 0 -> MAP_SIZE 3^2
    int r0 = rand() % (MAP_SIZE * MAP_SIZE);
    // if the number was not used continue
    while (*used & (1 << r0))
    {
        // if the number was used, generate a new random number and check it agine
        r0 = rand() % (MAP_SIZE * MAP_SIZE);
    }
    // the number was not used, mark it as used in the used ptr
    *used |= (1 << r0);

    // convert the flat 2d array map over to a row and col
    int lin = r0 / MAP_SIZE;
    int col = r0 % MAP_SIZE;

    // return the actual tile at the position that we just found
    return game_get_tile_at_pos(map, lin, col);
}

void tile_queue_push_if_not_null_and_not_placed(XY_QUEUE *q, TILE *t)
{
    ASSUME(q != NULL);
    if (t != NULL && !tile_is_placed(t))
    {
        xy_queue_push(q, &t->map_pos);
    }
}

void tile_choose_section(TILE *t,
                         TILE *above, TILE *below,
                         TILE *left, TILE *right,
                         SECTION_LIST *sections)
{
    ASSUME(t != NULL);
    ASSUME(sections != NULL);

    // A sections genkey is a bitfield that is essentaly its socket layout,
    // if two size match, they can be placed next to each other

    // List of sections that can be placed here
    SECTION_LIST can_place = {0};

    // for each section
    for (int i = 0; i < sections->count; i++)
    {
        SECTION *s = &sections->s[i];
        ASSUME(s != NULL);

#define not_null(x) (x != NULL && x->section != NULL)
        if (not_null(left) && s->gen_key.left != left->section->gen_key.right)
            continue;
        if (not_null(right) && s->gen_key.right != right->section->gen_key.left)
            continue;
        if (not_null(above) && s->gen_key.top != above->section->gen_key.bottem)
            continue;
        if (not_null(below) && s->gen_key.bottem != below->section->gen_key.top)
            continue;
#undef not_null

        // if we got here, this section can be placed here
        section_list_add(&can_place, s);
    }

    // if we have no sections that can be placed here, we have a problem
    ASSUME(can_place.count > 0);

    // pick a random section from the list of sections that can be placed here
    int r = rand() % can_place.count;
    SECTION *s = &can_place.s[r];
    ASSUME(s != NULL);
    t->section = s;
}

void game_gen_map(GameState_ptr gs)
{
    MAP map = {0};

    ASSUME(gs != NULL);
    for (int i = 0; i < gs->sections.count; i++)
    {
        // check and zero our sections positions
        SECTION *s = &gs->sections.s[i];
        ASSUME(s != NULL);
    }

    // null checking them all before i go random on them
    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++)
    {
        int lin = i / MAP_SIZE;
        int col = i % MAP_SIZE;
        TILE *tile = game_get_tile_at_pos(&map, lin, col);
        ASSUME(tile != NULL);
    }

    USED_TILE used = (USED_TILE)0;
    TILE *first = get_random_unused_tile(&map, &used);
    ASSUME(first != NULL);
    XY first_pos = first->map_pos;
    //  flood fill the map with sections
    XY_QUEUE q = {0};
    xy_queue_init(&q, MAP_SIZE * MAP_SIZE);
    xy_queue_push(&q, &first_pos);
    int tiles_set = 0;
    while (!xy_queue_is_empty(&q))
    {
        XY *pos = xy_queue_pop(&q);
        ASSUME(pos != NULL);
        int x = pos->x;
        int y = pos->y;

        // if this tile has been place, or is off grid, skip it
        if (x < 0 || y < 0 || x >= MAP_SIZE || y >= MAP_SIZE)
            continue;

        TILE *t = game_get_tile_at_pos(&map, y, x);
        ASSUME(t != NULL);

        if (tile_is_placed(t))
            continue;

        TILE *above = game_get_tile_at_pos(&map, y - 1, x);
        TILE *below = game_get_tile_at_pos(&map, y + 1, x);
        TILE *left = game_get_tile_at_pos(&map, y, x - 1);
        TILE *right = game_get_tile_at_pos(&map, y, x + 1);

        tile_choose_section(t, above, below, left, right, &gs->sections);
        // t->section = &gs->sections.s[2];

        tile_set_placed(t);
        tiles_set++;

        // add all the tiles around this one to the queue
        tile_queue_push_if_not_null_and_not_placed(&q, above);
        tile_queue_push_if_not_null_and_not_placed(&q, below);
        tile_queue_push_if_not_null_and_not_placed(&q, left);
        tile_queue_push_if_not_null_and_not_placed(&q, right);
    }

    gs->map = map;
}

// END GAME LAYOUT ########################################################

void game_init(GameState_ptr gs)
{
    gs->player.pos.x = 17;
    gs->player.pos.y = 17;
    gs->sections.count = 0;
    gs->sections.s = NULL;

    for (int rows = 0; rows < MAP_SIZE; rows++)
    {
        for (int cols = 0; cols < MAP_SIZE; cols++)
        {
            TILE *tile = game_get_tile_at_pos(&gs->map, rows, cols);
            tile->map_pos.x = cols;
            tile->map_pos.y = rows;
            ASSUME(tile != NULL);
            tile->section = NULL;
            tile->layout_flags = 0;
            gs->map.tiles[rows][cols] = *tile;
        }
    }

    ASSUME(gs->map.tiles[0][1].map_pos.x == 1);
    ASSUME(gs->map.tiles[0][1].map_pos.y == 0);
    ASSUME(gs->map.tiles[1][0].map_pos.x == 0);
    ASSUME(gs->map.tiles[1][0].map_pos.y == 1);
};

void game_free(GameState_ptr gs)
{
}

// #define BEEPY_MODE
#define PC_MODE

int game_proc_keypress(GameState_ptr gs, int ch)
{
    switch (ch)
    {
#ifdef BEEPY_MODE
    case '1':
        gs->player.pos.x--;
        gs->player.pos.y--;
        break;
    case '3':
        gs->player.pos.x++;
        gs->player.pos.y--;
        break;

    case '7':
        gs->player.pos.x--;
        gs->player.pos.y++;
        break;

    case '9':
        gs->player.pos.x++;
        gs->player.pos.y++;
        break;

    case '2':
    case KEY_UP:
        gs->player.pos.y--;
        break;
    case '8':
    case KEY_DOWN:
        gs->player.pos.y++;
        break;
    case '4':
    case KEY_LEFT:
        gs->player.pos.x--;
        break;
    case '6':
    case KEY_RIGHT:
        gs->player.pos.x++;
        break;

#endif

#ifdef PC_MODE
    case KEY_UP:
    case 8:
        gs->player.pos.y--;
        break;
    case KEY_DOWN:
    case 2:
        gs->player.pos.y++;
        break;

    case KEY_LEFT:
    case 4:
        gs->player.pos.x--;
        break;

    case KEY_RIGHT:
    case 6:
        gs->player.pos.x++;
        break;

    case 7:
        gs->player.pos.x--;
        gs->player.pos.y--;
        break;

    case 9:
        gs->player.pos.x++;
        gs->player.pos.y--;
        break;

    case 1:
        gs->player.pos.x--;
        gs->player.pos.y++;
        break;

    case 3:
        gs->player.pos.x++;
        gs->player.pos.y++;
        break;
#endif
    }

    return 0; // non 0 to quit
}

// TODO: Move this to a seperate file ---------- PARSER
