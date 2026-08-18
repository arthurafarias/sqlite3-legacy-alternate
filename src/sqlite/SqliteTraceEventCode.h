#pragma once

/* SQL Trace Event Codes (from sqlite3.h) */
enum {
  SQLITE_TRACE_STMT = 0x01,
  SQLITE_TRACE_PROFILE = 0x02,
  SQLITE_TRACE_ROW = 0x04,
  SQLITE_TRACE_CLOSE = 0x08,
};


