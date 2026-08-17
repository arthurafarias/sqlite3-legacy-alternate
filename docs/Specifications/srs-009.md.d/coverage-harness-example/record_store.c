#include <stdlib.h>
#include <string.h>
#include "record_store.h"

struct record_store {
  long *keys;
  char **values;
  unsigned char *used;
  int capacity;
  int count;
};

static unsigned long hash_key(long key, int capacity) {
  unsigned long h = (unsigned long)key * 2654435761UL;
  return h % (unsigned long)capacity;
}

static char *dup_string(const char *s) {
  if (!s) return NULL;
  size_t len = strlen(s) + 1;
  char *copy = malloc(len);
  if (copy) memcpy(copy, s, len);
  return copy;
}

record_store *record_store_open(int capacity) {
  if (capacity <= 0) return NULL;
  record_store *self = calloc(1, sizeof *self);
  if (!self) return NULL;
  self->keys = calloc((size_t)capacity, sizeof *self->keys);
  self->values = calloc((size_t)capacity, sizeof *self->values);
  self->used = calloc((size_t)capacity, sizeof *self->used);
  if (!self->keys || !self->values || !self->used) {
    record_store_close(self);
    return NULL;
  }
  self->capacity = capacity;
  return self;
}

void record_store_close(record_store *self) {
  if (!self) return;
  if (self->values) {
    for (int i = 0; i < self->capacity; i++) free(self->values[i]);
  }
  free(self->keys);
  free(self->values);
  free(self->used);
  free(self);
}

int record_store_insert(record_store *self, long key, const char *value) {
  if (!self) return -1;
  unsigned long start = hash_key(key, self->capacity);
  for (int i = 0; i < self->capacity; i++) {
    unsigned long slot = (start + (unsigned long)i) % (unsigned long)self->capacity;
    if (!self->used[slot] || self->keys[slot] == key) {
      if (!self->used[slot]) {
        if (self->count >= self->capacity) return -1;
        self->used[slot] = 1;
        self->count++;
      }
      free(self->values[slot]);
      self->values[slot] = dup_string(value);
      self->keys[slot] = key;
      return 0;
    }
  }
  return -1;
}

const char *record_store_get(record_store *self, long key) {
  if (!self) return NULL;
  unsigned long start = hash_key(key, self->capacity);
  for (int i = 0; i < self->capacity; i++) {
    unsigned long slot = (start + (unsigned long)i) % (unsigned long)self->capacity;
    if (!self->used[slot]) return NULL;
    if (self->keys[slot] == key) return self->values[slot];
  }
  return NULL;
}
