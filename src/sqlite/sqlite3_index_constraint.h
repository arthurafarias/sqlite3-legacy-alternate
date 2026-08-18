#pragma once

typedef struct sqlite3_index_constraint sqlite3_index_constraint;
struct sqlite3_index_constraint {
  int iColumn;
  unsigned char op;
  unsigned char usable;
  int iTermOffset;
};


