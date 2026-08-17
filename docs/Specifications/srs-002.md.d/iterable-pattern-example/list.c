#include <stdlib.h>
#include "list.h"
#include "list_find_predicate.h"
#include "list_node.h"
list *list_alloc() { return NULL; }
void list_dealloc(list *self) {}
void list_construct(list *self) {}
void list_destruct(list *self) {}
void list_insert(list *self) {}
void list_insert_before(list *self, list_iterator *it, list_node *node) {}
void list_insert_after(list *self, list_iterator *it, list_node *node) {}
list_iterator *list_find(list *self, list_find_predicate *predicate) { return NULL; }
void *list_at(list *self, size_t pos) { return NULL; }
