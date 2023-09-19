#include "game.h"
#include "assume.h"
#include "global.h"
#include <curses.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

#ifndef DT_REG
#define DT_REG 8 // idk why, but this wasnt defined in my dirent.h but it compiled fine
#endif
typedef const char *err;

// START GAME LAYOUT ##################################################################

// 1: Pick a TILE at random
// 2: Assinge it a random section

// A must be next to B
// C must be below A
// A cant be next to D

// T0:A
// T1:B
// T2:A
// T4:
// T5:
// T6:
// T7:

TILE *game_get_tile_at_pos(MAP *map, int y_line, int x_col)
{
    if (y_line < 0 || y_line >= MAP_SIZE)
        return NULL;

    if (x_col < 0 || x_col >= MAP_SIZE)
        return NULL;

    return &map->map[y_line][x_col];
}

bool is_tile_unused(TILE *t)
{
    return t->section == NULL;
}

/// @brief Returns a unique random tile from the given map
TILE *get_random_unused_tile(MAP *map, int *used)
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

void game_layout_sections(GameState_ptr gs)
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

    int used = 0;
    TILE *first = get_random_unused_tile(&map, &used);
    ASSUME(first != NULL);

    //-1 for first
    for (int i = 0; i < (MAP_SIZE * MAP_SIZE); i++)
    {
        int row = i / MAP_SIZE;
        int col = i % MAP_SIZE;
        TILE *t = game_get_tile_at_pos(&map, row, col);
        t->section = &gs->sections.s[0];// debug set something
        ASSUME(t != NULL);
    }

    gs->map = map;
}

// END GAME LAYOUT ##################################################################

void game_init(GameState_ptr gs)
{
    gs->player.pos.x = 17;
    gs->player.pos.y = 17;
    gs->sections.count = 0;
    gs->sections.s = NULL;
};

void game_free(GameState_ptr gs)
{
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

// TODO: Move this to a seperate file ---------- PARSER
struct parser_state
{
    int curr;
    bool parsed_header;
    bool got_header_p0; //[ - start header
    bool got_header_p1; //( - start arg
    bool got_header_p2; //) - end arg
    bool got_header_p3; //] - end header

    int curr_arg;
    int arg[4];

    char *header_item_type;
    bool in_comment;
    int buff_pos;
    char buffer[30 * 30];
};

struct building_args
{
    int type;
    WH wh;
};

// Swap for parse debugging info
// #define parse_dgb(...) glog_printf(__VA_ARGS__)
#define parse_dgb(...)

err parse_gl_section_gen(struct parser_state *p, FILE *f, SECTION *section)
{

    ASSUME(p->header_item_type != NULL);
    ASSUME(strcmp(p->header_item_type, "section_gen") == 0);
    //  so we may see the number '10101' but it is actualy convaing '0b10101'

    for (int i = 0; i < 4; i++)
    {
        if (p->arg[i] == 0)
            continue;

        int tmp = p->arg[i];
        int curr = 0;
        int new_num = 0;
        while (tmp != 0)
        {
            int rem = tmp % 10;
            tmp /= 10;
            new_num += rem * pow(2, curr);
            curr++;
        }
        p->arg[i] = new_num;
    }

    return NULL;
}

err parse_exit(struct parser_state *p, FILE *f, SECTION *section)
{
    // types "exit"
    ASSUME(p->header_item_type != NULL);
    ASSUME(strcmp(p->header_item_type, "exit") == 0);

    // alloc room for new exit
    section->exits.count++;
    section->exits.exits = realloc(section->exits.exits, sizeof(EXIT) * section->exits.count);
    EXIT *e = &section->exits.exits[section->exits.count - 1];

    e->type = p->arg[0];
    e->pos.x = p->arg[1] - 1;
    e->pos.y = p->arg[2] - 1;
    e->c = (char)p->arg[3];

    parse_dgb("Exit: type: %d, x: %d, y: %d, c: '%c'\n",
              e->type,
              e->pos.x,
              e->pos.y,
              e->c);

    if (e->type == 0)
    {
        return "exit type 0 is not valid";
    }

    if (e->pos.x == 0 || e->pos.y == 0)
    {
        return "exit pos is 1 based";
    }

    return NULL;
}

err parse_building(struct parser_state *p, FILE *f, SECTION *section)
{

    ASSUME(p->header_item_type != NULL);
    ASSUME(strcmp(p->header_item_type, "building") == 0);

    // Buildings now MUST be 10x10
    section->bounds.w = 10; // -1 because we are 0 based, humans are 1 based
    section->bounds.h = 10;

    int expected_chars_count = (section->bounds.w * section->bounds.h) +
                               (section->bounds.h);

    section->render_data = malloc(expected_chars_count + 1);
    memset(section->render_data, ' ', expected_chars_count);
    section->render_data[expected_chars_count] = '\0';

    char ch;
    int curr_ch = 0;
    bool done = false;
    while (!done)
    {
        ch = fgetc(f);

        if (curr_ch == 0 && ch == '\n')
            continue;

        if (curr_ch >= expected_chars_count)
        {
            done = true; // We still get to finish this current char
        }

        if (ch == EOF)
        {
            glog_printf("<EOF>");
            return "got EOF reading building";
        }

        section->render_data[curr_ch] = ch;
        glog_printf("%c\n", ch);
        curr_ch++;
    }

    glog_printf("Loaded building:\n%s\n", section->render_data);
    glog_printf("Building had %d chars expected got %d",
                expected_chars_count,
                curr_ch);
    return NULL;
}

// NULL on OK
err __load_single_section(SECTION *section, char *file)
{
    FILE *f = fopen(file, "r");
    struct parser_state p = {};
    const char *err = NULL;

    /*
        Header format:
        [item_type(arg1, arg2, arg3, arg4)]
        where item_type is the type of item, and arg1-4 are the arguments for that item

        the content is the actual data for the item, for example, a building might look like:
        [building(1, 10, 10)]
        ##########
        #        #
        #        #
        #        #
        #        #
        #        #
        #        #
        #        #
        #        #
        ##########
    */

    if (f == NULL)
    {
        err = "Error opening file";
        goto end;
    }

    while ((p.curr = fgetc(f)) != -1)
    {
        if (p.curr == '\n' && p.in_comment)
        {
            p.in_comment = false;
        }

        parse_dgb(" %c ", p.curr);

        if (p.curr == '@' && !p.in_comment)
        {
            p.in_comment = true;
        }

        // skip comments
        if (p.in_comment)
        {
            parse_dgb("SKIP\n");
            continue;
        }
        if (!p.got_header_p3)
        {
            parse_dgb("HDR\n");

            if (p.curr == EOF)
            {
                err = "got EOF reading header";
                goto end;
            }
            if (p.curr == '\n')
                continue;

            if (!p.got_header_p0)
            {
                if (p.curr != '[')
                    continue;

                p.got_header_p0 = true;
                continue;
            }

            // we are reading what kind of item this is
            if (p.got_header_p0 && !p.got_header_p1)
            {

                if (p.curr == '(')
                {
                    // we found the end of the string, copy the header into the state
                    p.header_item_type = strdup(p.buffer);
                    p.got_header_p1 = true;
                    p.buff_pos = 0;
                    p.buffer[p.buff_pos] = '\0';
                    continue;
                }

                p.buffer[p.buff_pos] = p.curr;
                p.buff_pos++;
                continue;
            }

            if (p.got_header_p1 && !p.got_header_p2)
            {
                if (p.curr == ' ')
                    continue;

                // read to ) or ,
                if (p.curr == ',')
                {
                    parse_dgb("Parsing arg: %s\n", p.buffer);

                    if (p.buffer[0] == '\'')
                    {
                        // its a char
                        p.arg[p.curr_arg] = p.buffer[1];
                        parse_dgb("Got char: %c\n", p.arg[p.curr_arg]);
                        p.curr_arg++;
                        p.buff_pos = 0;
                        p.buffer[p.buff_pos] = '\0';
                        continue;
                    }

                    // save the arg !!COULD BE INDEX OUT OF RAGE HERE!!
                    p.arg[p.curr_arg] = atoi(p.buffer);
                    // inc curr
                    p.curr_arg++;

                    // reset buff
                    p.buff_pos = 0;
                    p.buffer[p.buff_pos] = '\0';
                    continue;
                }

                if (p.curr == ')')
                {
                    p.got_header_p2 = true;

                    if (p.buffer[0] == '\'')
                    {
                        // its a char
                        p.arg[p.curr_arg] = p.buffer[1];
                        parse_dgb("Got char: %c\n", p.arg[p.curr_arg]);
                        p.curr_arg++;
                        p.buff_pos = 0;
                        p.buffer[p.buff_pos] = '\0';
                        continue;
                    }

                    // save the arg !!COULD BE INDEX OUT OF RAGE HERE!!
                    p.arg[p.curr_arg] = atoi(p.buffer);
                    // inc curr
                    p.curr_arg++;

                    p.buff_pos = 0;
                    p.buffer[p.buff_pos] = '\0';
                    continue;
                }

                p.buffer[p.buff_pos] = p.curr;
                p.buff_pos++;
                continue;
            }

            if (p.got_header_p2 && !p.got_header_p3)
            {
                if (p.curr == ' ')
                    continue;

                // read the final ']'
                if (p.curr == ']')
                {
                    p.got_header_p3 = true;
                }
            }
        }

        parse_dgb("Parsing header item: %s\n", p.header_item_type);

#define match(s) strcmp(p.header_item_type, s) == 0

#define check_parse_error(s)                         \
    if (err != NULL)                                 \
    {                                                \
        parse_dgb("Error parsing %s: %s\n", s, err); \
        goto end;                                    \
    }                                                \
    p = (struct parser_state){};                     \
    continue;

        if (match("exit"))
        {
            err = parse_exit(&p, f, section);
            check_parse_error("exit");
        }

        if (match("building"))
        {
            err = parse_building(&p, f, section);
            check_parse_error("building");
        }

        if (match("section_gen"))
        {
            err = parse_gl_section_gen(&p, f, section);
            check_parse_error("section_gen");
        }

#undef check_parse_error
#undef match
    }

    parse_dgb("Done parsing file\n");

end:
    fclose(f);
    return err;
}

void game_load_section(GameState_ptr gs, char *folder)
{
    // find all the .sec files, load them
    DIR *d;
    struct dirent *dir;
    d = opendir(folder);
    if (!d)
    {
        parse_dgb("Error opening directory: %s\n", folder);
        return;
    }

    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type != DT_REG)
            continue;

        char *ext = strrchr(dir->d_name, '.');
        if (ext != NULL && strcmp(ext, ".sec") != 0)
            continue;

        char *filepath = malloc(strlen(folder) + strlen(dir->d_name) + 2);
        strcpy(filepath, folder);
        strcat(filepath, "/");
        strcat(filepath, dir->d_name);
        parse_dgb("Loading section: %s\n", filepath);
        gs->sections.count++;
        gs->sections.s = realloc(gs->sections.s, sizeof(SECTION) * gs->sections.count);
        SECTION *s = NULL;
        s = &gs->sections.s[gs->sections.count - 1]; // ptr to the new section
        // zero out the section
        s->exits.count = 0;
        s->exits.exits = NULL;

        const char *err = __load_single_section(s, filepath);
        if (err != NULL)
        {
            endwin();
            glog_printf("Error loading section: %s\nIn File: %s\n", err, filepath);
            printf("Error loading section: %s\nIn File: %s\n", err, filepath);
            exit(1);
        }
        free(filepath);
    }

    closedir(d);
}

// END PARSER ------------------------------------