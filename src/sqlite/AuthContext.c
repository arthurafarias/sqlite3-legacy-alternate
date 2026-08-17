#define _GNU_SOURCE 1

#include "sqlite/AuthContext.h"

#include "sqlite/Parse.h"
void sqlite3AuthContextPop(AuthContext *pContext) {
  if (pContext->pParse) {
    pContext->pParse->zAuthContext = pContext->zAuthContext;
    pContext->pParse = 0;
  }
}
