#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Flags for sqlite3_deserialize() (from sqlite3.h) */
enum {
  SQLITE_DESERIALIZE_FREEONCLOSE = 1, /* Call sqlite3_free() on close */
  SQLITE_DESERIALIZE_RESIZEABLE = 2,  /* Resize using sqlite3_realloc64() */
  SQLITE_DESERIALIZE_READONLY = 4,    /* Database is read-only */
};

#ifdef __cplusplus
}
#endif
