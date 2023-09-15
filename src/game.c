#include "game.h"
#include "assume.h"
#include "global.h"
#include <curses.h>
#include <string.h>
#include <dirent.h>

#ifndef DT_REG
#define DT_REG 8 // idk why, but this wasnt defined in my dirent.h but it compiled fine
#endif
typedef const char *err;

void game_init(GameState_ptr gs)
{
    gs->player.pos.x = 17;
    gs->player.pos.y = 17;
    gs->sections.count = 0;
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
//  #define parse_dgb(...) glog_printf(__VA_ARGS__)
#define parse_dgb(...)

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

    // TODO: handle placing buildings around map
    section->bounds.pos.x = 10;
    section->bounds.pos.y = 10;

    struct building_args a = {.wh.w = p->arg[1], .wh.h = p->arg[2], .type = p->arg[0]};
    section->bounds.size.w = a.wh.w - 1; // -1 because we are 0 based, humans are 1 based
    section->bounds.size.h = a.wh.h - 1;

    int expected_chars_count = (a.wh.w * a.wh.h) + (a.wh.h - 1);

    section->render_data = malloc(expected_chars_count + 1);
    memset(section->render_data, ' ', expected_chars_count);
    section->render_data[expected_chars_count] = '\0';
    // set expected \n chars
    for (int i = 0; i < a.wh.h - 1; i++)
    {
        section->render_data[(a.wh.w * (i + 1)) + i] = '\n';
    }

    char ch;
    int curr_ch = 0;
    bool done = false;
    while (!done)
    {
        ch = fgetc(f);

        if (curr_ch == 0 && ch == '\n')
            continue;

        if (curr_ch >= expected_chars_count)
            done = true; // We still get to finish this current char

        if (ch == EOF)
        {
            return "got EOF reading building";
        }

        section->render_data[curr_ch] = ch;
        curr_ch++;
    }

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
        if (match("exit"))
        {
            parse_dgb("Parsing exit\n");
            err = parse_exit(&p, f, section);
            if (err != NULL)
            {
                goto end;
            }

            p = (struct parser_state){};
            continue;
        }

        if (match("building"))
        {

            parse_dgb("Parsing building\n");
            err = parse_building(&p, f, section);
            if (err != NULL)
            {
                goto end;
                parse_dgb("Error parsing building: %s\n", err);
            }
            p = (struct parser_state){};
            continue;
        }
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
        SECTION *s = &gs->sections.s[gs->sections.count - 1];
        const char *err = __load_single_section(s, filepath);
        if (err != NULL)
        {
            endwin();
            parse_dgb("Error loading section: %s\nIn File: %s\n", err, filepath);
            exit(1);
        }
        free(filepath);
    }

    closedir(d);
}
