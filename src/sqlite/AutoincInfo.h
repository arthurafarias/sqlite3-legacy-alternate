
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct Table Table;

  typedef struct AutoincInfo AutoincInfo;

  struct AutoincInfo {
    AutoincInfo *pNext;
    Table *pTab;
    int iDb;
    int regCtr;
  };

#ifdef __cplusplus
}
#endif
