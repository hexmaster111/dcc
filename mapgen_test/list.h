#ifndef __LIST_H
#define __LIST_H

#define LIST_TEMPLATE(type)                                                  \
    typedef struct type##List                                                \
    {                                                                        \
        type *data;                                                          \
        int size;                                                            \
        int capacity;                                                        \
    } type##List;                                                            \
                                                                             \
    type##List *type##_list_new()                                            \
    {                                                                        \
        type##List *list = malloc(sizeof(type##List));                       \
        list->size = 0;                                                      \
        list->capacity = 10;                                                 \
        list->data = malloc(sizeof(type) * list->capacity);                  \
        return list;                                                         \
    }                                                                        \
                                                                             \
    void type##_list_add(type##List *list, type item)                        \
    {                                                                        \
        if (list->size == list->capacity)                                    \
        {                                                                    \
            list->capacity *= 2;                                             \
            list->data = realloc(list->data, sizeof(type) * list->capacity); \
        }                                                                    \
        list->data[list->size++] = item;                                     \
    }                                                                        \
                                                                             \
    void type##_list_remove(type##List *list, int index)                     \
    {                                                                        \
                                                                             \
        if (index >= list->size || index < 0)                                \
        {                                                                    \
            return;                                                          \
        }                                                                    \
                                                                             \
        for (int i = index; i < list->size - 1; i++)                         \
        {                                                                    \
            list->data[i] = list->data[i + 1];                               \
        }                                                                    \
                                                                             \
        list->size--;                                                        \
    }                                                                        \
                                                                             \
    void type##_list_free(type##List *list)                                  \
    {                                                                        \
        free(list->data);                                                    \
        free(list);                                                          \
    }                                                                        \
    void type##_list_itterate(type##List *list, void (*func)(type))          \
    {                                                                        \
        for (int i = 0; i < list->size; i++)                                 \
        {                                                                    \
            func(list->data[i]);                                             \
        }                                                                    \
    }

#endif // __LIST_H
