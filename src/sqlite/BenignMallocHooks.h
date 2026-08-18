
#pragma once

  struct BenignMallocHooks;

  struct BenignMallocHooks {
    void (*xBenignBegin)(void);
    void (*xBenignEnd)(void);
  };

  extern BenignMallocHooks sqlite3Hooks;

  void sqlite3BenignMallocHooks(void (*)(void), void (*)(void));


