#pragma once

typedef struct SortSubtask SortSubtask;

typedef int (*SorterCompare)(SortSubtask *, int *, const void *, int, const void *, int);


