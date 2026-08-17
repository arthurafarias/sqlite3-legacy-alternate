#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "db_limits.h"
#include "hash.h"
#include "stmt.h"
/* Only ever called from db_exec/db_prepare in this file: stays static and
 * out of db.h rather than becoming part of db's public contract. */
static int find_row(const db *self, const char *key) {
  if (self->row_count == 0) return -1;
  size_t start = fnv1a_hash(key) % (unsigned)db_limits_default.max_rows;
  for (int i = 0; i < db_limits_default.max_rows; i++) {
    size_t slot = (start + (size_t)i) % (unsigned)db_limits_default.max_rows;
    if (self->keys[slot][0] != '\0' && strcmp(self->keys[slot], key) == 0) {
      return (int)slot;
    }
  }
  return -1;
}

static int insert_row(db *self, const char *key, const char *value) {
  size_t start = fnv1a_hash(key) % (unsigned)db_limits_default.max_rows;
  for (int i = 0; i < db_limits_default.max_rows; i++) {
    size_t slot = (start + (size_t)i) % (unsigned)db_limits_default.max_rows;
    if (self->keys[slot][0] == '\0' || strcmp(self->keys[slot], key) == 0) {
      if (self->keys[slot][0] == '\0') self->row_count++;
      snprintf(self->keys[slot], sizeof self->keys[slot], "%s", key);
      snprintf(self->values[slot], sizeof self->values[slot], "%s", value);
      return 0;
    }
  }
  return -1;
}

db *db_open(const char *name) {
  db *self = calloc(1, sizeof *self);
  if (!self) return NULL;
  snprintf(self->name, sizeof self->name, "%s", name ? name : "");
  return self;
}

void db_close(db *self) { free(self); }

int db_exec(db *self, const char *sql) {
  char verb[8] = {0}, key[64] = {0}, value[256] = {0};
  if (sscanf(sql, "%7s %63s %255[^\n]", verb, key, value) < 3 ||
      strcmp(verb, "SET") != 0) {
    self->last_status = -1;
    return self->last_status;
  }
  self->last_status = insert_row(self, key, value);
  return self->last_status;
}

stmt *db_prepare(db *self, const char *sql) {
  char verb[8] = {0}, key[64] = {0};
  const char *value = "";
  if (sscanf(sql, "%7s %63s", verb, key) == 2 && strcmp(verb, "GET") == 0) {
    int slot = find_row(self, key);
    if (slot >= 0) value = self->values[slot];
  }
  self->last_status = 0;
  return stmt_create(self, value);
}
