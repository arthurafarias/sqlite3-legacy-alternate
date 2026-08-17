#define _GNU_SOURCE 1

#include "sqlite/WhereLevel.h"

#include "sqlite/Expr.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void disableTerm(WhereLevel *pLevel, WhereTerm *pTerm) {
  int nLoop = 0;

  while ((pTerm->wtFlags & 0x0004) == 0 && (pLevel->iLeftJoin == 0 || (((pTerm->pExpr)->flags & (u32)(0x000001)) != 0)) && (pLevel->notReady & pTerm->prereqAll) == 0) {
    if (nLoop && (pTerm->wtFlags & 0x0400) != 0) {
      pTerm->wtFlags |= 0x0200;
    } else {
      pTerm->wtFlags |= 0x0004;
    }

    if (pTerm->iParent < 0)
      break;
    pTerm = &pTerm->pWC->a[pTerm->iParent];


    pTerm->nChild--;
    if (pTerm->nChild != 0)
      break;
    nLoop++;
  }
}
