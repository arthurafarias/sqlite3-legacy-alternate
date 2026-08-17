#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
typedef struct db db;

typedef struct stmt stmt;
struct stmt {
  db *owner;
  char column0[256];
  unsigned checksum;
  int done;
};

/* No `stmt *self` to mutate yet, so it's placed here under the type it
 * returns rather than under db.h (its owner argument). */
stmt *stmt_create(db *owner, const char *value);

int stmt_step(stmt *self);
const char *stmt_column_text(stmt *self, int col);
void stmt_finalize(stmt *self);

#ifdef __cplusplus
}
#endif
