#include <stdlib.h>

#include "list_iterator.h"

list_iterator *list_iterator_alloc() { return NULL; }

void list_iterator_dealloc(list_iterator *self) {}
void list_iterator_construct(list_iterator *self, list *list, list_node *node) {}

void list_iterator_destruct(list_iterator *self) {}