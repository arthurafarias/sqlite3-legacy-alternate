#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct list list;
typedef struct list_iterator list_iterator;

typedef bool (*list_find_predicate)(list *, list_iterator *);

#ifdef __cplusplus
}
#endif