#pragma once

/* WhereTerm.wtFlags Bit Values (from the sqlite3 amalgamation's where.c,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). Only the subset referenced by this
   library's sources is listed here; extend as more of them are needed. */
enum {
  TERM_CODED = 0x0004, /* This term is already coded */
};


