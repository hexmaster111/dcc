#include "game.h"
#include <curses.h>
#include "assert.h"
#include <string.h>
#include <dirent.h>

#ifndef DT_REG
#define DT_REG 8 // idk why, but this wasnt defined in my dirent.h but it compiled fine
#endif

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

    int buff_pos;
    char buffer[30 * 30];
};

struct building_args
{
    int type;
    WH wh;
};

// NULL on OK
const char *__load_single_section(SECTION *section, char *file)
{
    FILE *f = fopen(file, "r");
    struct parser_state p = {};
    const char *err = NULL;

    if (f == NULL)
    {
        err = "Error opening file";
        goto end;
    }

    while ((p.curr = fgetc(f)) != -1)
    {
        if (!p.got_header_p3)
        {

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
                    continue;
                }
            }
            continue;
        }

        // types "building"

        if (strcmp(p.header_item_type, "building") == 0)
        {
            // todo parse building
            section->bounds.pos.x = 10;
            section->bounds.pos.y = 10;

            struct building_args a = {.wh.w = p.arg[1], .wh.h = p.arg[2], .type = p.arg[0]};
            section->bounds.size = a.wh;
            int expected_chars_count = a.wh.w * a.wh.h;

            // TODO: stream in the data from the file into the sections render data
            // TODO: When this is not a const char, we should strcpy and not just assign the ptr
            section->render_data.c = "##########\n#        #\n#        #\n#        #\n#  LAB   #\n#        #\n#        #\n#        #\n#        #\n##########";
        }
    }

end:
    fclose(f);
    if (err != NULL)
        printf("error reading sec %s", err);
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
        printf("Error opening directory: %s\n", folder);
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
        printf("Loading section: %s\n", filepath);
        gs->sections.count++;
        gs->sections.s = realloc(gs->sections.s, sizeof(SECTION) * gs->sections.count);
        SECTION *s = &gs->sections.s[gs->sections.count - 1];
        __load_single_section(s, filepath);
        free(filepath);
    }

    closedir(d);
}
