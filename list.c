#include <list.h>
#include <mm.h>

void list_push(list_t *list, void *node) {
    list_t *last = list;
    for (; last->next != NULL; last = last->next);

    list_t *new = (list_t *)kmalloc(sizeof(list_t));
    new->node = node;
    new->prev = last;
    new->next = NULL;
    last->next = new;
}

void list_remove(list_t *list, void *node) {
    for (list_t *entry = list; entry != NULL; entry = entry->next) {
        if (entry->node == node) {
            if (entry->prev != NULL) {
                entry->prev->next = entry->next;
            }
            if (entry->next != NULL) {
                entry->next->prev = entry->prev;
            }
            kmfree(entry);
            return;
        }
    }
}