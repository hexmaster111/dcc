#ifndef PARSER_H
#define PARSER_H
#include "assume.h"
#include "global.h"
#include "game.h"
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <ncurses.h>

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
typedef const char *err;

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

    section->gen_key.top = p->arg[0];
    section->gen_key.bottem = p->arg[1];
    section->gen_key.left = p->arg[2];
    section->gen_key.right = p->arg[3];

    return NULL;
}

err parse_exit(struct parser_state *p, FILE *f, SECTION *section)
{
    // types "exit"
    ASSUME(p->header_item_type != NULL);
    ASSUME(strcmp(p->header_item_type, "exit") == 0);

    // alloc room for new exit
    section->exits.count++;
    section->exits.exits = (EXIT *)realloc(section->exits.exits, sizeof(EXIT) * section->exits.count);
    EXIT *e = &section->exits.exits[section->exits.count - 1];

    e->type = (E_EXIT)p->arg[0];
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

    section->render_data = (STR)malloc(expected_chars_count + 1);
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
            // glog_printf("<EOF>");
            return "got EOF reading building";
        }

        section->render_data[curr_ch] = ch;
        // glog_printf("%c\n", ch);
        curr_ch++;
    }

    glog_printf("Building had %d chars expected got %d\n",
                expected_chars_count,
                curr_ch);
    return NULL;
}

// NULL on OK
err __load_single_section(SECTION *section, char *file)
{
    glog_printf("opening file %s for loading\n", file);
    FILE *f = fopen(file, "r");
    struct parser_state p = {};
    const char *err = NULL;

    /*
        Header format:
        [item_type(arg1, arg2, arg3, arg4)]
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
    glog_printf("Building keys: Top:%x Botem:%x Left:%x Rigt:%x\n",
                section->gen_key.top,
                section->gen_key.bottem,
                section->gen_key.left,
                section->gen_key.right);
    glog_printf("render data:\n%s", section->render_data);
    fclose(f);
    return err;
}

void parse_load_section(GameState_ptr gs, char *folder)
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
#define DT_REG 8

        if (dir->d_type != DT_REG)
            continue;
#undef DT_REG
        char *ext = strrchr(dir->d_name, '.');
        if (ext != NULL && strcmp(ext, ".sec") != 0)
            continue;

        char *filepath = (char *)malloc(strlen(folder) + strlen(dir->d_name) + 2);
        strcpy(filepath, folder);
        strcat(filepath, "/");
        strcat(filepath, dir->d_name);
        parse_dgb("Loading section: %s\n", filepath);
        gs->sections.count++;
        gs->sections.s = (SECTION *)realloc(gs->sections.s, sizeof(SECTION) * gs->sections.count);
        SECTION *s = NULL;
        s = &gs->sections.s[gs->sections.count - 1]; // ptr to the new section
        // zero out the section
        *s = (SECTION){0};

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
#endif