
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Vdbe.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u8.h"
  typedef struct sqlite3_value Mem;

  typedef struct sqlite3 sqlite3;
  typedef struct sqlite3_stmt sqlite3_stmt;

  const char *sqlite3_sql(sqlite3_stmt * pStmt);
  char *sqlite3_expanded_sql(sqlite3_stmt * pStmt);
  int sqlite3_stmt_readonly(sqlite3_stmt * pStmt);
  int sqlite3_stmt_isexplain(sqlite3_stmt * pStmt);
  int sqlite3_stmt_explain(sqlite3_stmt * pStmt, int eMode);
  int sqlite3_stmt_busy(sqlite3_stmt *);

  int sqlite3_bind_blob(sqlite3_stmt *, int, const void *, int n, void (*)(void *));
  int sqlite3_bind_blob64(sqlite3_stmt *, int, const void *, sqlite3_uint64, void (*)(void *));
  int sqlite3_bind_double(sqlite3_stmt *, int, double);
  int sqlite3_bind_int(sqlite3_stmt *, int, int);
  int sqlite3_bind_int64(sqlite3_stmt *, int, sqlite3_int64);
  int sqlite3_bind_null(sqlite3_stmt *, int);
  int sqlite3_bind_text(sqlite3_stmt *, int, const char *, int, void (*)(void *));
  int sqlite3_bind_text16(sqlite3_stmt *, int, const void *, int, void (*)(void *));
  int sqlite3_bind_text64(sqlite3_stmt *, int, const char *, sqlite3_uint64, void (*)(void *), unsigned char encoding);
  int sqlite3_bind_value(sqlite3_stmt *, int, const sqlite3_value *);
  int sqlite3_bind_pointer(sqlite3_stmt *, int, void *, const char *, void (*)(void *));
  int sqlite3_bind_zeroblob(sqlite3_stmt *, int, int n);
  int sqlite3_bind_zeroblob64(sqlite3_stmt *, int, sqlite3_uint64);
  int sqlite3_bind_parameter_count(sqlite3_stmt *);
  const char *sqlite3_bind_parameter_name(sqlite3_stmt *, int);
  int sqlite3_bind_parameter_index(sqlite3_stmt *, const char *zName);
  int sqlite3_clear_bindings(sqlite3_stmt *);
  int sqlite3_column_count(sqlite3_stmt * pStmt);
  const char *sqlite3_column_name(sqlite3_stmt *, int N);
  const void *sqlite3_column_name16(sqlite3_stmt *, int N);
  const char *sqlite3_column_database_name(sqlite3_stmt *, int);
  const void *sqlite3_column_database_name16(sqlite3_stmt *, int);
  const char *sqlite3_column_table_name(sqlite3_stmt *, int);
  const void *sqlite3_column_table_name16(sqlite3_stmt *, int);
  const char *sqlite3_column_origin_name(sqlite3_stmt *, int);
  const void *sqlite3_column_origin_name16(sqlite3_stmt *, int);
  const char *sqlite3_column_decltype(sqlite3_stmt *, int);
  const void *sqlite3_column_decltype16(sqlite3_stmt *, int);
  int sqlite3_step(sqlite3_stmt *);
  int sqlite3_data_count(sqlite3_stmt * pStmt);
  const void *sqlite3_column_blob(sqlite3_stmt *, int iCol);
  double sqlite3_column_double(sqlite3_stmt *, int iCol);
  int sqlite3_column_int(sqlite3_stmt *, int iCol);
  sqlite3_int64 sqlite3_column_int64(sqlite3_stmt *, int iCol);
  const unsigned char *sqlite3_column_text(sqlite3_stmt *, int iCol);
  const void *sqlite3_column_text16(sqlite3_stmt *, int iCol);
  sqlite3_value *sqlite3_column_value(sqlite3_stmt *, int iCol);
  int sqlite3_column_bytes(sqlite3_stmt *, int iCol);
  int sqlite3_column_bytes16(sqlite3_stmt *, int iCol);
  int sqlite3_column_type(sqlite3_stmt *, int iCol);
  int sqlite3_finalize(sqlite3_stmt * pStmt);
  int sqlite3_reset(sqlite3_stmt * pStmt);
  int sqlite3_expired(sqlite3_stmt *);
  int sqlite3_transfer_bindings(sqlite3_stmt *, sqlite3_stmt *);

  sqlite3 *sqlite3_db_handle(sqlite3_stmt *);
  int sqlite3_stmt_status(sqlite3_stmt *, int op, int resetFlg);
  int sqlite3_stmt_scanstatus(sqlite3_stmt * pStmt, int idx, int iScanStatusOp, void *pOut);
  int sqlite3_stmt_scanstatus_v2(sqlite3_stmt * pStmt, int idx, int iScanStatusOp, int flags, void *pOut);
  void sqlite3_stmt_scanstatus_reset(sqlite3_stmt *);
  int sqlite3_carray_bind_v2(sqlite3_stmt * pStmt, int i, void *aData, int nData, int mFlags, void (*xDel)(void *),
                             void *pDel);
  int sqlite3_carray_bind(sqlite3_stmt * pStmt, int i, void *aData, int nData, int mFlags, void (*xDel)(void *));

  int sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
  Mem *columnMem(sqlite3_stmt * pStmt, int i);
  void columnMallocFailure(sqlite3_stmt * pStmt);
  const void *columnName(sqlite3_stmt * pStmt, int N, int useUtf16, int useType);
  int bindText(sqlite3_stmt * pStmt, int i, const void *zData, i64 nData, void (*xDel)(void *), u8 encoding);

#ifdef __cplusplus
}
#endif
