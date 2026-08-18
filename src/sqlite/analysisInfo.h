
#pragma once

  struct sqlite3;
  struct analysisInfo;

  struct analysisInfo {
    sqlite3 *db;
    const char *zDatabase;
  };


