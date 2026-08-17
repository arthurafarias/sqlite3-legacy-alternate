
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct Parse Parse;

  typedef struct AuthContext AuthContext;

  struct AuthContext {
    const char *zAuthContext;
    Parse *pParse;
  };

  void sqlite3AuthContextPop(AuthContext *);

#ifdef __cplusplus
}
#endif
