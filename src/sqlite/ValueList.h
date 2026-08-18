
#pragma once

  typedef struct BtCursor BtCursor;
  typedef struct sqlite3_value sqlite3_value;

  typedef struct ValueList ValueList;
  struct ValueList {
    BtCursor *pCsr;
    sqlite3_value *pOut;
  };


