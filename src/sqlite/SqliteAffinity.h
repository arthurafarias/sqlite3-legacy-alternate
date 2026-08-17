#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Column/Expression Affinity Codes (from the sqlite3 amalgamation's
   sqliteInt.h, internal/private - not part of the public sqlite3.h API
   and not guaranteed stable across versions). */
enum {
  SQLITE_AFF_NONE = 0x40,    /* '@' */
  SQLITE_AFF_BLOB = 0x41,    /* 'A' */
  SQLITE_AFF_TEXT = 0x42,    /* 'B' */
  SQLITE_AFF_NUMERIC = 0x43, /* 'C' */
  SQLITE_AFF_INTEGER = 0x44, /* 'D' */
  SQLITE_AFF_REAL = 0x45,    /* 'E' */
  SQLITE_AFF_FLEXNUM = 0x46, /* 'F' */
  SQLITE_AFF_DEFER = 0x58,   /* 'X' - defer computation until later */
  SQLITE_AFF_MASK = 0x47,
};

#ifdef __cplusplus
}
#endif
