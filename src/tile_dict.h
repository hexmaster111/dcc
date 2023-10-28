#ifndef __TILE_HASHMAP_H__
#define __TILE_HASHMAP_H__

typedef struct
{
    char c; // Character to render and identify the tile
#define TILE_TYPE_WALL 1
#define TILE_TYPE_FLOOR 2
#define TILE_TYPE_ITEM 3
    int8_t type;
    int8_t chance;
} TILE_OBJECT_DATA;

typedef struct
{
    char *key;
    TILE_OBJECT_DATA *value;
} TILE_OBJECT;
typedef struct
{
    TILE_OBJECT *data;
    int size;
    int count;
} TILE_OBJECT_ARRAY;

typedef struct TILE_HASHMAP
{
    TILE_OBJECT_ARRAY *data;
    int size;
    int count;
} TILE_HASHMAP;

void tile_hashmap_new(TILE_HASHMAP *hm);
void tile_hashmap_free(TILE_HASHMAP *hm, char hash, TILE_OBJECT *value);
void tile_hashmap_get(TILE_HASHMAP *hm, char hash, TILE_OBJECT **value);

#endif // __TILE_HASHMAP_H__
