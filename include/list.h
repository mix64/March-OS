#pragma once
#include <types.h>

typedef struct list {
    void *node;
    struct list *prev;
    struct list *next;
} list_t;

void list_push(list_t *list, void *node);
void list_remove(list_t *list, void *node);
