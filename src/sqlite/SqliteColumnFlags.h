#pragma once

/* Column.colFlags Bit Values (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). Only the subset referenced by this
   library's sources is listed here; extend as more of them are needed. */
enum {
  COLFLAG_GENERATED = 0x0060, /* Combo: _STORED, _VIRTUAL */
};


