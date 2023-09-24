
#ifndef __XY_QUEUE_H__
#define __XY_QUEUE_H__
#include "game.h"
#include "assume.h"
#include <string.h>

// Tile queue
typedef struct xy_queue_t
{
    int count;
    int max_count;
    XY **tiles;
} XY_QUEUE;

void xy_queue_init(XY_QUEUE *q, int max_count)
{
    ASSUME(q != NULL);
    ASSUME(max_count > 0);
    q->count = 0;
    q->max_count = max_count;
    q->tiles = malloc(sizeof(XY *) * max_count);
    memset(q->tiles, 0, sizeof(XY *) * max_count);
    ASSUME(q->tiles != NULL);
}

void xy_queue_free(XY_QUEUE *q)
{
    ASSUME(q != NULL);
    free(q->tiles);
}

void xy_queue_push(XY_QUEUE *q, XY *t)
{
    ASSUME(q != NULL);
    ASSUME(t != NULL);
    ASSUME(q->count < q->max_count && "xy_queue_push: queue is full");
    q->tiles[q->count] = t;
    q->count++;
}

XY *xy_queue_pop(XY_QUEUE *q)
{
    ASSUME(q != NULL);
    ASSUME(q->count > 0);
    XY *t = q->tiles[0];
    q->count--;
    for (int i = 0; i < q->count; i++)
    {
        q->tiles[i] = q->tiles[i + 1];
    }
    q->tiles[q->count] = NULL;

    return t;
}

bool xy_queue_is_empty(XY_QUEUE *q)
{
    ASSUME(q != NULL);
    return q->count == 0;
}

#endif // __XY_QUEUE_H__