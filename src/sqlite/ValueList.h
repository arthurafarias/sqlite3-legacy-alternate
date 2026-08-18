
#pragma once

  struct BtCursor;
  struct sqlite3_value;

  struct ValueList;
  struct ValueList {
    BtCursor *pCsr;
    sqlite3_value *pOut;
  };


