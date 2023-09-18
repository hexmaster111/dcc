#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include "../src/assume.h"

typedef enum
{
    UNTOUCHED = 'U',
    LAND = 'L',
    COAST = 'C',
    SEA = 'S'
} Tile;

// #define width 79
// #define height 20
#define width 79
#define height 10

int get_dev_urandom()
{
    FILE *f = fopen("/dev/urandom", "r");
    int r = 0;
    fread(&r, sizeof(int), 1, f);
    fclose(f);
    return r;
}

int _rand()
{
    int r = 0;
    if (r == 0)
    {
        r = get_dev_urandom();
    }
    r = (r * 1103515245 + 12345) & 0x7fffffff;
    return r;
}

int _rand_in_range(int min, int max)
{
    return _rand() % (max - min + 1) + min;
}

void get_around(char tiles[width][height], int x, int y, char around[4], int *around_count)
{
    if (x > 0)
    {
        around[*around_count] = tiles[x - 1][y];
        *around_count += 1;
    }
    if (x < width - 1)
    {
        around[*around_count] = tiles[x + 1][y];
        *around_count += 1;
    }
    if (y > 0)
    {
        around[*around_count] = tiles[x][y - 1];
        *around_count += 1;
    }
    if (y < height - 1)
    {
        around[*around_count] = tiles[x][y + 1];
        *around_count += 1;
    }
}

void red()
{
    printf("\033[1;31m");
}

void green()
{
    printf("\033[1;32m");
}

void blue()
{
    printf("\033[1;34m");
}

void yellow()
{
    printf("\033[1;33m");
}

void reset()
{
    printf("\033[0m");
}

// #define dbg(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define dbg(fmt, ...)

char tiles[width][height] = {0};

int main(int argc, char *argv[])
{
    dbg("gen test\n");

    // init tiles
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            tiles[x][y] = UNTOUCHED;
        }
    }

    const bool straghten = true;

    bool scan_line_backwards = true;
    for (int i = 0; i < width * height; i++)
    {
        scan_line_backwards = !scan_line_backwards;
        int x = i % width;
        int y = i / width;

        if (straghten)
        {

            if (scan_line_backwards)
            {
                if (width % 2 == 0)
                    x = (width - x);
                else
                    x = (width - x) - 1;
            }
        }

#define ABOVE (y > 0 ? tiles[x][y - 1] : UNTOUCHED)
#define BELOW (y < height - 1 ? tiles[x][y + 1] : UNTOUCHED)
#define LEFT (x > 0 ? tiles[x - 1][y] : UNTOUCHED)
#define RIGHT (x < width - 1 ? tiles[x + 1][y] : UNTOUCHED)
#define ANY(what) (ABOVE == what || BELOW == what || LEFT == what || RIGHT == what)
#define get_rand() _rand_in_range(0, 100)

        char around[4] = {0};
        int around_count = 0;
        get_around(tiles, x, y, around, &around_count);

        dbg("x: %d, y: %d, above: %c, below: %c, left: %c, right: %c\n",
            x, y,
            ABOVE,
            BELOW,
            LEFT,
            RIGHT);

        // land is allowed to be adjacent to land or coast
        // coast must be adjacent to land, and sea
        // sea is allowed to be adjacent to coast or sea
        char new_tile = UNTOUCHED;
        bool any_sea = ANY(SEA);
        bool any_coast = ANY(COAST);
        bool any_land = ANY(LAND);

        bool could_be_land = true;
        bool could_be_coast = true;
        bool could_be_sea = true;

        (void)could_be_land;
        (void)could_be_coast;
        (void)could_be_sea;
        (void)any_sea;
        (void)any_coast;
        (void)any_land;

        if (any_sea)
            could_be_land = false;

        if (any_land)
            could_be_sea = false;

        ASSUME(could_be_land || could_be_coast || could_be_sea);

    reroll:
        if (could_be_land && get_rand() < 2)
        {
            new_tile = LAND;
        }
        else if (could_be_coast && get_rand() < 1)
        {
            new_tile = COAST;
        }
        else if (could_be_sea && get_rand() < 2)
        {
            new_tile = SEA;
        }

        if (new_tile != UNTOUCHED)
        {

            ASSUME(new_tile == LAND || new_tile == COAST || new_tile == SEA);

            tiles[x][y] = new_tile;
            dbg("new_tile: %c\n", new_tile);
            continue;
        }
        goto reroll;

#undef ABOVE
#undef BELOW
#undef LEFT
#undef RIGHT
#undef ANY
    }

    for (int i = 0; i < width * height; i++)
    {
        int x = i % width;
        int y = i / width;
        char ch = tiles[x][y];
        if (ch == UNTOUCHED)
        {
            red();
            printf(".");
            reset();
        }
        else if (ch == LAND)
        {
            green();
            printf("L");
            reset();
        }
        else if (ch == COAST)
        {
            yellow();
            printf("C");
            reset();
        }
        else if (ch == SEA)
        {
            blue();
            printf("S");
            reset();
        }
        else
        {
            printf("?");
        }

        if (x == width - 1)
        {
            printf("\n");
        }
    }

    printf("exiting\n");
    return 0;
}