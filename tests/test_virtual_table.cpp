#include "test_common.h"

/* A minimal virtual table module: "int_series" produces integers between
 * two HIDDEN bound columns, exercising the xCreate/xConnect/xBestIndex/
 * xFilter/xNext/xEof/xColumn/xRowid/xDisconnect/xDestroy surface. */

typedef struct {
  sqlite3_vtab base;
} series_vtab;

typedef struct {
  sqlite3_vtab_cursor base;
  sqlite3_int64 value;
  sqlite3_int64 stop;
} series_cursor;

static int seriesInit(sqlite3 *db, void *aux, int argc, const char *const *argv,
                       sqlite3_vtab **ppVtab, char **pzErr) {
  (void)aux;
  (void)argc;
  (void)argv;
  (void)pzErr;
  int rc = sqlite3_declare_vtab(
      db, "CREATE TABLE x(value INTEGER, start HIDDEN, stop HIDDEN)");
  if (rc != SQLITE_OK)
    return rc;
  series_vtab *vtab = (series_vtab *) sqlite3_malloc(sizeof(series_vtab));
  if (!vtab)
    return SQLITE_NOMEM;
  memset(vtab, 0, sizeof(*vtab));
  *ppVtab = &vtab->base;
  return SQLITE_OK;
}

static int seriesDisconnect(sqlite3_vtab *pVtab) {
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

static int seriesBestIndex(sqlite3_vtab *tab, sqlite3_index_info *info) {
  int startIdx = -1, stopIdx = -1;
  for (int i = 0; i < info->nConstraint; i++) {
    if (!info->aConstraint[i].usable)
      continue;
    if (info->aConstraint[i].op != SQLITE_INDEX_CONSTRAINT_EQ)
      continue;
    if (info->aConstraint[i].iColumn == 1)
      startIdx = i;
    else if (info->aConstraint[i].iColumn == 2)
      stopIdx = i;
  }
  if (startIdx < 0 || stopIdx < 0) {
    tab->zErrMsg = sqlite3_mprintf(
        "int_series requires both start=? and stop=? constraints");
    return SQLITE_CONSTRAINT;
  }
  info->aConstraintUsage[startIdx].argvIndex = 1;
  info->aConstraintUsage[startIdx].omit = 1;
  info->aConstraintUsage[stopIdx].argvIndex = 2;
  info->aConstraintUsage[stopIdx].omit = 1;
  info->idxNum = 1;
  info->estimatedCost = 100.0;
  info->estimatedRows = 100;
  return SQLITE_OK;
}

static int seriesOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor) {
  (void)pVtab;
  series_cursor *c = (series_cursor *) sqlite3_malloc(sizeof(series_cursor));
  if (!c)
    return SQLITE_NOMEM;
  memset(c, 0, sizeof(*c));
  *ppCursor = &c->base;
  return SQLITE_OK;
}

static int seriesClose(sqlite3_vtab_cursor *cur) {
  sqlite3_free(cur);
  return SQLITE_OK;
}

static int seriesFilter(sqlite3_vtab_cursor *cur, int idxNum,
                         const char *idxStr, int argc, sqlite3_value **argv) {
  (void)idxStr;
  series_cursor *c = (series_cursor *)cur;
  if (idxNum != 1 || argc != 2)
    return SQLITE_ERROR;
  c->value = sqlite3_value_int64(argv[0]);
  c->stop = sqlite3_value_int64(argv[1]);
  return SQLITE_OK;
}

static int seriesNext(sqlite3_vtab_cursor *cur) {
  ((series_cursor *)cur)->value++;
  return SQLITE_OK;
}

static int seriesEof(sqlite3_vtab_cursor *cur) {
  series_cursor *c = (series_cursor *)cur;
  return c->value > c->stop;
}

static int seriesColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i) {
  series_cursor *c = (series_cursor *)cur;
  if (i == 0)
    sqlite3_result_int64(ctx, c->value);
  else
    sqlite3_result_null(ctx);
  return SQLITE_OK;
}

static int seriesRowid(sqlite3_vtab_cursor *cur, sqlite3_int64 *pRowid) {
  *pRowid = ((series_cursor *)cur)->value;
  return SQLITE_OK;
}

static sqlite3_module series_module = {
    .iVersion = 0,
    .xCreate = seriesInit,
    .xConnect = seriesInit,
    .xBestIndex = seriesBestIndex,
    .xDisconnect = seriesDisconnect,
    .xDestroy = seriesDisconnect,
    .xOpen = seriesOpen,
    .xClose = seriesClose,
    .xFilter = seriesFilter,
    .xNext = seriesNext,
    .xEof = seriesEof,
    .xColumn = seriesColumn,
    .xRowid = seriesRowid,
};

int main(void) {
  sqlite3 *db = test_open_memory_db();
  TEST_OK(db, sqlite3_create_module(db, "int_series", &series_module, NULL));
  test_exec_ok(db, "CREATE VIRTUAL TABLE series USING int_series()");

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT value FROM series WHERE start = 1 AND stop = 5",
                  -1, &stmt, NULL));
  sqlite3_int64 expected = 1;
  int rc;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    TEST_ASSERT_EQ_INT(sqlite3_column_int64(stmt, 0), expected);
    expected++;
  }
  TEST_ASSERT(rc == SQLITE_DONE);
  TEST_ASSERT_EQ_INT(expected, 6);
  sqlite3_finalize(stmt);

  /* SUM aggregate over the virtual table result set. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT SUM(value) FROM series WHERE start=1 AND stop=100",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int64(stmt, 0), 100LL * 101 / 2);
  sqlite3_finalize(stmt);

  /* Missing bound constraints should surface the xBestIndex error. */
  rc = sqlite3_exec(db, "SELECT value FROM series", NULL, NULL, NULL);
  TEST_ASSERT(rc != SQLITE_OK);

  test_close_ok(db);
  TEST_PASS("test_virtual_table");
  return 0;
}
