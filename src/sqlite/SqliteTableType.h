#pragma once

/* Table.eTabType Values (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). */
enum {
  TABTYP_NORM = 0, /* Ordinary table */
  TABTYP_VTAB = 1, /* Virtual table */
  TABTYP_VIEW = 2, /* A view */
};


