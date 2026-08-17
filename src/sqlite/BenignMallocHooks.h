
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct BenignMallocHooks BenignMallocHooks;

  struct BenignMallocHooks {
    void (*xBenignBegin)(void);
    void (*xBenignEnd)(void);
  };

  extern BenignMallocHooks sqlite3Hooks;

  void sqlite3BenignMallocHooks(void (*)(void), void (*)(void));

#ifdef __cplusplus
}
#endif
