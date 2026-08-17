#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "stmt.h"

stmt *stmt_create(db *owner, const char *value) {
  stmt *self = calloc(1, sizeof *self);
  if (!self) return NULL;
  self->owner = owner;
  snprintf(self->column0, sizeof self->column0, "%s", value ? value : "");
  self->checksum = fnv1a_hash(self->column0);
  self->done = 0;
  return self;
}

int stmt_step(stmt *self) {
  if (self->done) return 0;
  self->done = 1;
  return self->column0[0] != '\0' ? 1 : 0;
}

const char *stmt_column_text(stmt *self, int col) {
  (void)col;
  return self->column0;
}

void stmt_finalize(stmt *self) { free(self); }
