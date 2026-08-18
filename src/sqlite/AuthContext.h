
#pragma once

  struct Parse;

  struct AuthContext;

  struct AuthContext {
    const char *zAuthContext;
    Parse *pParse;
  };

  void sqlite3AuthContextPop(AuthContext *);


