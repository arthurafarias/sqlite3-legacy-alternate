#define _GNU_SOURCE 1
#include "sqlite/sqlite3_threadsafe.h"
int sqlite3_threadsafe(void) {
  return 1;
}
