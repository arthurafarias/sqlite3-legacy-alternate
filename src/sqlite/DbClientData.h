
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct DbClientData DbClientData;

  struct DbClientData {
    DbClientData *pNext;
    void *pData;
    void (*xDestructor)(void *);
    char zName[1];
  };

#ifdef __cplusplus
}
#endif
