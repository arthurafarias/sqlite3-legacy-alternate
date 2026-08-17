#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "list_find_predicate.h"
#include <stdlib.h>
typedef struct list_node list_node;

struct list {
  list_node *root;
};

list *list_alloc();
void list_dealloc(list *);
void list_construct(list *);
void list_destruct(list *);
void list_insert(list *);
void list_insert_before(list *, list_iterator *, list_node *);
void list_insert_after(list *, list_iterator *, list_node *);
list_iterator *list_find(list *, list_find_predicate *);
void *list_at(list *, size_t pos);

#ifdef __cplusplus
}
#endif