
#pragma once

  typedef struct sqlite3_pcache_page sqlite3_pcache_page;

  struct sqlite3_pcache_page {
    void *pBuf;
    void *pExtra;
  };


