
#pragma once

  struct Token;

  struct Token {
    const char *z;
    unsigned int n;
  };

  void sqlite3DequoteToken(Token *);
  void sqlite3TokenInit(Token *, char *);
  void sqlite3Parser(void *, int, Token);


