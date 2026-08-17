#define _GNU_SOURCE 1
#include "sqlite/sqlite3_free_table.h"
#include <stdint.h>
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_free.h"
void sqlite3_free_table(char **azResult) {
  if (azResult) {
    int i, n;
    azResult--;

    n = ((int)(intptr_t)(azResult[0]));
    for (i = 1; i < n; i++) {
      if (azResult[i])
        sqlite3_free(azResult[i]);
    }
    sqlite3_free(azResult);
  }
}
