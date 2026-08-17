#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Allowed return values from sqlite3_txn_state() (from sqlite3.h) */
enum {
  SQLITE_TXN_NONE  = 0,
  SQLITE_TXN_READ  = 1,
  SQLITE_TXN_WRITE = 2,
};

#ifdef __cplusplus
}
#endif
