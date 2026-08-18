#pragma once

/* Select.selFlags Bit Values (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). Only the subset referenced by this
   library's sources is listed here; extend as more of them are needed. */
enum {
  SF_Distinct = 0x0000001,  /* Output should be DISTINCT */
  SF_Aggregate = 0x0000008, /* Contains agg functions or a GROUP BY */
};


