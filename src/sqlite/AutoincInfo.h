
#pragma once

  typedef struct Table Table;

  typedef struct AutoincInfo AutoincInfo;

  struct AutoincInfo {
    AutoincInfo *pNext;
    Table *pTab;
    int iDb;
    int regCtr;
  };


