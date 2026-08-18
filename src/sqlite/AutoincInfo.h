
#pragma once

  struct Table;

  struct AutoincInfo;

  struct AutoincInfo {
    AutoincInfo *pNext;
    Table *pTab;
    int iDb;
    int regCtr;
  };


