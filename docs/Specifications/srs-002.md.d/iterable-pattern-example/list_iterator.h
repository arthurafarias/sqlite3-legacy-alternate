#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list list;
typedef struct list_node list_node;

typedef struct list_iterator list_iterator;
struct list_iterator {
  list *list;
  list_node *current;
};

list_iterator *list_iterator_alloc();
void list_iterator_dealloc(list_iterator *);
void list_iterator_construct(list_iterator *, list* list, list_node* node);
void list_iterator_destruct(list_iterator *);

typedef struct list list;
struct list {
  list_node *root;
};

#ifdef __cplusplus
}
#endif