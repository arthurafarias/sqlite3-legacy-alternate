#pragma once

/* Virtual Table Scan Flags (from sqlite3.h) */
enum {
  SQLITE_INDEX_SCAN_UNIQUE = 0x00000001, /* Scan visits at most 1 row */
  SQLITE_INDEX_SCAN_HEX = 0x00000002,    /* Display idxNum as hex */
};


