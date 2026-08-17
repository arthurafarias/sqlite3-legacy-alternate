#define _GNU_SOURCE 1

#include "sqlite/RenameCtx.h"

#include "sqlite/RenameToken.h"
#include "sqlite/Token.h"
RenameToken *renameColumnTokenNext(RenameCtx *pCtx) {
  RenameToken *pBest = pCtx->pList;
  RenameToken *pToken;
  RenameToken **pp;

  for (pToken = pBest->pNext; pToken; pToken = pToken->pNext) {
    if (pToken->t.z > pBest->t.z)
      pBest = pToken;
  }
  for (pp = &pCtx->pList; *pp != pBest; pp = &(*pp)->pNext)
    ;
  *pp = pBest->pNext;

  return pBest;
}
