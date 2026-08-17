#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct stmt stmt;

typedef struct db db;
struct db {
  char name[32];
  char keys[16][64];
  char values[16][256];
  int row_count;
  int last_status;
};

db *db_open(const char *name);
void db_close(db *self);
int db_exec(db *self, const char *sql);

/* Returns stmt*, but is declared here rather than in stmt.h: it mutates
 * `self` (its first argument, a db*) and only db, so the first-argument
 * rule takes precedence over the return-type rule. */
stmt *db_prepare(db *self, const char *sql);

#ifdef __cplusplus
}
#endif
