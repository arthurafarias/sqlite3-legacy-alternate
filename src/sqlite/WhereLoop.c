#define _GNU_SOURCE 1

#include "sqlite/WhereLoop.h"

#include "sqlite/Index.h"
#include "sqlite/LogEst.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
int whereLoopIsOneRow(WhereLoop *pLoop) {
  if (pLoop->u.btree.pIndex->onError && pLoop->nSkip == 0 && pLoop->u.btree.nEq == pLoop->u.btree.pIndex->nKeyCol) {
    int ii;
    for (ii = 0; ii < pLoop->u.btree.nEq; ii++) {
      if (pLoop->aLTerm[ii]->eOperator & (0x0080 | 0x0100)) {
        return 0;
      }
    }
    return 1;
  }
  return 0;
}

void whereLoopInit(WhereLoop *p) {
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = ((int)(sizeof(p->aLTermSpace) / sizeof(p->aLTermSpace[0])));
  p->wsFlags = 0;
}

int whereLoopCheaperProperSubset(const WhereLoop *pX, const WhereLoop *pY) {
  int i, j;
  if (pX->rRun > pY->rRun && pX->nOut > pY->nOut)
    return 0;

  if (pX->u.btree.nEq < pY->u.btree.nEq && pX->u.btree.pIndex == pY->u.btree.pIndex && pX->nSkip == 0 && pY->nSkip == 0) {
    return 1;
  }
  if (pX->nLTerm - pX->nSkip >= pY->nLTerm - pY->nSkip) {
    return 0;
  }
  if (pY->nSkip > pX->nSkip)
    return 0;
  for (i = pX->nLTerm - 1; i >= 0; i--) {
    if (pX->aLTerm[i] == 0)
      continue;
    for (j = pY->nLTerm - 1; j >= 0; j--) {
      if (pY->aLTerm[j] == pX->aLTerm[i])
        break;
    }
    if (j < 0)
      return 0;
  }
  if ((pX->wsFlags & 0x00000040) != 0 && (pY->wsFlags & 0x00000040) == 0) {
    return 0;
  }
  return 1;
}

void whereLoopAdjustCost(const WhereLoop *p, WhereLoop *pTemplate) {
  if ((pTemplate->wsFlags & 0x00000200) == 0)
    return;
  for (; p; p = p->pNextLoop) {
    if (p->iTab != pTemplate->iTab)
      continue;
    if ((p->wsFlags & 0x00000200) == 0)
      continue;
    if (whereLoopCheaperProperSubset(p, pTemplate)) {

      ;
      pTemplate->rRun = ((p->rRun) < (pTemplate->rRun) ? (p->rRun) : (pTemplate->rRun));
      pTemplate->nOut = ((p->nOut - 1) < (pTemplate->nOut) ? (p->nOut - 1) : (pTemplate->nOut));
    } else if (whereLoopCheaperProperSubset(pTemplate, p)) {

      ;
      pTemplate->rRun = ((p->rRun) > (pTemplate->rRun) ? (p->rRun) : (pTemplate->rRun));
      pTemplate->nOut = ((p->nOut + 1) > (pTemplate->nOut) ? (p->nOut + 1) : (pTemplate->nOut));
    }
  }
}

WhereLoop **whereLoopFindLesser(WhereLoop **ppPrev, const WhereLoop *pTemplate) {
  WhereLoop *p;
  for (p = (*ppPrev); p; ppPrev = &p->pNextLoop, p = *ppPrev) {
    if (p->iTab != pTemplate->iTab || p->iSortIdx != pTemplate->iSortIdx) {

      continue;
    }

    ((void)(0))

        ;

    ((void)(0))

        ;

    if ((p->wsFlags & 0x00004000) != 0 && (pTemplate->nSkip) == 0 && (pTemplate->wsFlags & 0x00000200) != 0 && (pTemplate->wsFlags & 0x00000001) != 0 && (p->prereq & pTemplate->prereq) == pTemplate->prereq) {
      break;
    }

    if ((p->prereq & pTemplate->prereq) == p->prereq && p->rSetup <= pTemplate->rSetup && p->rRun <= pTemplate->rRun && p->nOut <= pTemplate->nOut) {
      return 0;
    }

    if ((p->prereq & pTemplate->prereq) == pTemplate->prereq && p->rRun >= pTemplate->rRun && p->nOut >= pTemplate->nOut) {

      ((void)(0))

          ;
      break;
    }
  }
  return ppPrev;
}

__attribute__((noinline)) int whereLoopIsNoBetter(const WhereLoop *pCandidate, const WhereLoop *pBaseline) {
  if ((pCandidate->wsFlags & 0x00000200) == 0)
    return 1;
  if ((pBaseline->wsFlags & 0x00000200) == 0)
    return 1;
  if (pCandidate->u.btree.pIndex->szIdxRow < pBaseline->u.btree.pIndex->szIdxRow)
    return 0;
  return 1;
}
