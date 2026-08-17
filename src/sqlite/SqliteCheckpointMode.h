#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Checkpoint Mode Values (from sqlite3.h) */
enum {
  SQLITE_CHECKPOINT_NOOP     = -1,  /* Do no work at all */
  SQLITE_CHECKPOINT_PASSIVE  = 0,  /* Do as much as possible w/o blocking */
  SQLITE_CHECKPOINT_FULL     = 1,  /* Wait for writers, then checkpoint */
  SQLITE_CHECKPOINT_RESTART  = 2,  /* Like FULL but wait for readers */
  SQLITE_CHECKPOINT_TRUNCATE = 3,  /* Like RESTART but also truncate WAL */
};

#ifdef __cplusplus
}
#endif
