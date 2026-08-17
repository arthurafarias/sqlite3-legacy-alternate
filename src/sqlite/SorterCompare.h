#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SortSubtask SortSubtask;

typedef int (*SorterCompare)(SortSubtask *, int *, const void *, int, const void *, int);

#ifdef __cplusplus
}
#endif
