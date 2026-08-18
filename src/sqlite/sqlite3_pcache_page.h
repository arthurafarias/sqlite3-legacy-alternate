
#pragma once

  struct sqlite3_pcache_page;

  struct sqlite3_pcache_page {
    void *pBuf;
    void *pExtra;
  };


