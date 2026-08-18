
#pragma once

  struct DbClientData;

  struct DbClientData {
    DbClientData *pNext;
    void *pData;
    void (*xDestructor)(void *);
    char zName[1];
  };


