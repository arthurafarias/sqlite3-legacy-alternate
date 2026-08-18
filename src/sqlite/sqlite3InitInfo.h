#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/u8.h"
struct sqlite3InitInfo;
struct sqlite3InitInfo {
  Pgno newTnum;
  u8 iDb;
  u8 busy;
  unsigned orphanTrigger : 1;
  unsigned imposterTable : 2;
  unsigned reopenMemdb : 1;
  const char **azInit;
};


