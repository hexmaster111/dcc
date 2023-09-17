#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

typedef enum
{
    UNTOUCHED = 'U',
    LAND = 'L',
    COAST = 'C',
    SEA = 'S'
} Tile;

#define width 79
#define height 20

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
    static int r = 0;
    if (r == 0)
    {
        r = get_dev_urandom();
    }
    r = r * 1103515245 + 12345;
    return (unsigned int)(r / 65536) % 32768;
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

int main(int argc, char *argv[])
{
    char tiles[width][height] = {0};

    printf("gen test\n");

    // init tiles
    for (int y = 0; y < height; y++)
    {
        printf("\n");
        for (int x = 0; x < width; x++)
        {
            tiles[x][y] = UNTOUCHED;
            printf("%c", tiles[x][y]);
        }
    }

    tiles[0][0] = LAND; // Seed the first tile
    int no_tiles = 0;
    for (int i = 1; i < width * height; i++)
    {
        int x = i % width;
        int y = i / width;

#define ABOVE (y > 0 ? tiles[x][y - 1] : UNTOUCHED)
#define BELOW (y < height - 1 ? tiles[x][y + 1] : UNTOUCHED)
#define LEFT (x > 0 ? tiles[x - 1][y] : UNTOUCHED)
#define RIGHT (x < width - 1 ? tiles[x + 1][y] : UNTOUCHED)
#define land_chance 50
#define coast_chance 50
#define sea_chance 50
#define get_rand() _rand_in_range(0, 100)

        char around[4] = {0};
        int around_count = 0;
        get_around(tiles, x, y, around, &around_count);

        printf("x: %d, y: %d, above: %c, below: %c, left: %c, right: %c\n",
               x, y,
               ABOVE,
               BELOW,
               LEFT,
               RIGHT);

// land is allowed to be adjacent to land or coast
// coast is allowed to be adjacent to land, coast, or sea
// sea is allowed to be adjacent to coast or sea
#define is_around(what) (ABOVE == what || BELOW == what || LEFT == what || RIGHT == what)
        char new_tile = UNTOUCHED;
        bool can_be_land = is_around(LAND) || is_around(COAST);
        bool can_be_coast = is_around(LAND) || is_around(COAST) || is_around(SEA);
        bool can_be_sea = is_around(COAST) || is_around(SEA);
        printf("can_be_land: %d, can_be_coast: %d, can_be_sea: %d\n", can_be_land, can_be_coast, can_be_sea);

        // if it can only be one type then it should be that type
        if (can_be_coast && !can_be_sea && !can_be_land)
        {
            new_tile = COAST;
        }

        if (!can_be_coast && can_be_sea && !can_be_land)
        {
            new_tile = SEA;
        }

        if (!can_be_coast && !can_be_sea && can_be_land)
        {
            new_tile = LAND;
        }

        // if it can be two types then it should be the type with the highest chance
        if (can_be_coast && can_be_sea && !can_be_land)
        {
            if (get_rand() < coast_chance)
            {
                new_tile = COAST;
            }
            else
            {
                new_tile = SEA;
            }
        }

        if (can_be_coast && !can_be_sea && can_be_land)
        {
            if (get_rand() < land_chance)
            {
                new_tile = LAND;
            }
            else
            {
                new_tile = COAST;
            }
        }

        if (!can_be_coast && can_be_sea && can_be_land)
        {
            if (get_rand() < land_chance)
            {
                new_tile = LAND;
            }
            else
            {
                new_tile = SEA;
            }
        }

        // if it can be all three types then it should be the type with the highest chance

        if (can_be_coast && can_be_sea && can_be_land)
        {
            if (get_rand() < land_chance)
            {
                new_tile = LAND;
            }
            else if (get_rand() < coast_chance)
            {
                new_tile = COAST;
            }
            else
            {
                new_tile = SEA;
            }
        }

        tiles[x][y] = new_tile;
        printf("new_tile: %c\n", new_tile);

#undef ABOVE
#undef BELOW
#undef LEFT
#undef RIGHT
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
    printf("\nno_tiles: %d\n", no_tiles);

    return 0;
}