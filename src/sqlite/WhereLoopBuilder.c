#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/WhereLoopBuilder.h"

#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/HiddenIndexInfo.h"
#include "sqlite/Index.h"
#include "sqlite/LogEst.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Table.h"
#include "sqlite/WhereAndInfo.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereLevel.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/WhereMaskSet.h"
#include "sqlite/WhereOrCost.h"
#include "sqlite/WhereOrInfo.h"
#include "sqlite/WhereOrSet.h"
#include "sqlite/WhereScan.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/i16.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_index_info.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int allConstraintsUsed(struct sqlite3_index_constraint_usage *aUsage, int nCons);
static int whereUsablePartialIndex(int iTab, u8 jointype, WhereClause *pWC, Expr *pWhere);

static int whereUsablePartialIndex(int iTab, u8 jointype, WhereClause *pWC, Expr *pWhere) {
  int i;
  WhereTerm *pTerm;
  Parse *pParse;

  if (jointype & 0x40)
    return 0;
  pParse = pWC->pWInfo->pParse;
  while (pWhere->op == 44) {
    if (!whereUsablePartialIndex(iTab, jointype, pWC, pWhere->pLeft))
      return 0;
    pWhere = pWhere->pRight;
  }
  for (i = 0, pTerm = pWC->a; i < pWC->nTerm; i++, pTerm++) {
    Expr *pExpr;
    pExpr = pTerm->pExpr;
    if ((!(((pExpr)->flags & (u32)(0x000001)) != 0) || pExpr->w.iJoin == iTab) && ((jointype & 0x20) == 0 || (((pExpr)->flags & (u32)(0x000001)) != 0)) && sqlite3ExprImpliesExpr(pParse, pExpr, pWhere, iTab) && !sqlite3ExprImpliesExpr(pParse, pExpr, pWhere, -1) && (pTerm->wtFlags & 0x0080) == 0) {
      return 1;
    }
  }
  return 0;
}

static int allConstraintsUsed(struct sqlite3_index_constraint_usage *aUsage, int nCons) {
  int ii;
  for (ii = 0; ii < nCons; ii++) {
    if (aUsage[ii].argvIndex <= 0)
      return 0;
  }
  return 1;
}


int whereLoopInsert(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate) {
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;
  int rc;

  if (pBuilder->iPlanLimit == 0) {
    ;
    if (pBuilder->pOrSet)
      pBuilder->pOrSet->n = 0;
    return 101;
  }
  pBuilder->iPlanLimit--;

  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);

  if (pBuilder->pOrSet != 0) {
    if (pTemplate->nLTerm) {

      whereOrInsert(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun, pTemplate->nOut);
    }
    return 0;
  }

  ppPrev = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  if (ppPrev == 0) {

    return 0;
  } else {
    p = *ppPrev;
  }

  if (p == 0) {

    *ppPrev = p = sqlite3DbMallocRawNN(db, sizeof(WhereLoop));
    if (p == 0)
      return 7;
    whereLoopInit(p);
    p->pNextLoop = 0;
  } else {

    WhereLoop **ppTail = &p->pNextLoop;
    WhereLoop *pToDel;
    while (*ppTail) {
      ppTail = whereLoopFindLesser(ppTail, pTemplate);
      if (ppTail == 0)
        break;
      pToDel = *ppTail;
      if (pToDel == 0)
        break;
      *ppTail = pToDel->pNextLoop;

      whereLoopDelete(db, pToDel);
    }
  }
  rc = whereLoopXfer(db, p, pTemplate);
  if ((p->wsFlags & 0x00000400) == 0) {
    Index *pIndex = p->u.btree.pIndex;
    if (pIndex && pIndex->idxType == 3) {
      p->u.btree.pIndex = 0;
    }
  }
  return rc;
}

int whereLoopAddBtreeIndex(WhereLoopBuilder *pBuilder, SrcItem *pSrc, Index *pProbe, LogEst nInMul) {
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  WhereLoop *pNew;
  WhereTerm *pTerm;
  int opMask;
  WhereScan scan;
  Bitmask saved_prereq;
  u16 saved_nLTerm;
  u16 saved_nEq;
  u16 saved_nBtm;
  u16 saved_nTop;
  u16 saved_nSkip;
  u32 saved_wsFlags;
  LogEst saved_nOut;
  int rc = 0;
  LogEst rSize;
  LogEst rLogSize;
  WhereTerm *pTop = 0, *pBtm = 0;

  pNew = pBuilder->pNew;

  if (pParse->nErr) {
    return pParse->rc;
  }

  ;

  if (pNew->wsFlags & 0x00000020) {
    opMask = (0x0002 << (57 - 54)) | (0x0002 << (56 - 54));
  } else {


    opMask = 0x0002 | 0x0001 | (0x0002 << (55 - 54)) | (0x0002 << (58 - 54)) | (0x0002 << (57 - 54)) | (0x0002 << (56 - 54)) | 0x0100 | 0x0080;
  }
  if (pProbe->bUnordered) {
    opMask &= ~((0x0002 << (55 - 54)) | (0x0002 << (58 - 54)) | (0x0002 << (57 - 54)) | (0x0002 << (56 - 54)));
  }

  saved_nEq = pNew->u.btree.nEq;
  saved_nBtm = pNew->u.btree.nBtm;
  saved_nTop = pNew->u.btree.nTop;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_nOut = pNew->nOut;
  pTerm = whereScanInit(&scan, pBuilder->pWC, pSrc->iCursor, saved_nEq, opMask, pProbe);
  pNew->rSetup = 0;
  rSize = pProbe->aiRowLogEst[0];
  rLogSize = estLog(rSize);
  for (; rc == 0 && pTerm != 0; pTerm = whereScanNext(&scan)) {
    u16 eOp = pTerm->eOperator;
    LogEst rCostIdx;
    LogEst nOutUnadjusted;
    int nIn = 0;

    if ((eOp == 0x0100 || (pTerm->wtFlags & 0x0080) != 0) && indexColumnNotNull(pProbe, saved_nEq)) {
      continue;
    }
    if (pTerm->prereqRight & pNew->maskSelf)
      continue;

    if (pTerm->wtFlags & 0x0100 && pTerm->eOperator == (0x0002 << (57 - 54)))
      continue;

    if ((pSrc->fg.jointype & (0x08 | 0x40 | 0x10)) != 0 && !constraintCompatibleWithOuterJoin(pTerm, pSrc)) {
      continue;
    }
    if (((pProbe)->onError != 0) && saved_nEq == pProbe->nKeyCol - 1) {
      pBuilder->bldFlags1 |= 0x0002;
    } else {
      pBuilder->bldFlags1 |= 0x0001;
    }
    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->u.btree.nBtm = saved_nBtm;
    pNew->u.btree.nTop = saved_nTop;
    pNew->nLTerm = saved_nLTerm;
    if (pNew->nLTerm >= pNew->nLSlot && whereLoopResize(db, pNew, pNew->nLTerm + 1)) {
      break;
    }
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereqRight) & ~pNew->maskSelf;



    if (eOp & 0x0001) {
      Expr *pExpr = pTerm->pExpr;
      if ((((pExpr)->flags & 0x001000) != 0)) {

        int i;
        int bRedundant = 0;
        nIn = 46;



        for (i = 0; i < pNew->nLTerm - 1; i++) {
          if (pNew->aLTerm[i] && pNew->aLTerm[i]->pExpr == pExpr) {
            nIn = 0;
            if (pNew->aLTerm[i]->u.x.iField == pTerm->u.x.iField) {

              bRedundant = 1;
            }
          }
        }
        if (bRedundant) {
          pNew->nLTerm--;
          continue;
        }
      } else if ((pExpr->x.pList && pExpr->x.pList->nExpr)) {

        nIn = sqlite3LogEst(pExpr->x.pList->nExpr);
      }
      if (pProbe->hasStat1 && rLogSize >= 10) {
        LogEst M, logK, x;

        M = pProbe->aiRowLogEst[saved_nEq];
        logK = estLog(nIn);

        x = M + logK + 10 - (nIn + rLogSize);
        if (x >= 0) {

          ;
        } else if (nInMul < 2 && (((db)->dbOptFlags & (0x00020000)) == 0)) {

          ;
          pNew->wsFlags |= 0x00100000;
        } else {

          ;
          continue;
        }
      }
      pNew->wsFlags |= 0x00000004;
    } else if (eOp & (0x0002 | 0x0080)) {
      int iCol = pProbe->aiColumn[saved_nEq];
      pNew->wsFlags |= 0x00000001;


      if (iCol == (-1) || (iCol >= 0 && nInMul == 0 && saved_nEq == pProbe->nKeyCol - 1)) {
        if (iCol == (-1) || pProbe->uniqNotNull || (pProbe->nKeyCol == 1 && pProbe->onError && (eOp & 0x0002))) {
          pNew->wsFlags |= 0x00001000;
        } else {
          pNew->wsFlags |= 0x00010000;
        }
      }
      if (scan.iEquiv > 1)
        pNew->wsFlags |= 0x00200000;
    } else if (eOp & 0x0100) {
      pNew->wsFlags |= 0x00000008;
    } else {
      int nVecLen = whereRangeVectorLen(pParse, pSrc->iCursor, pProbe, saved_nEq, pTerm);
      if (eOp & ((0x0002 << (55 - 54)) | (0x0002 << (58 - 54)))) {
        ;
        ;
        pNew->wsFlags |= 0x00000002 | 0x00000020;
        pNew->u.btree.nBtm = nVecLen;
        pBtm = pTerm;
        pTop = 0;
        if (pTerm->wtFlags & 0x0100) {

          pTop = &pTerm[1];






          if (whereLoopResize(db, pNew, pNew->nLTerm + 1))
            break;
          pNew->aLTerm[pNew->nLTerm++] = pTop;
          pNew->wsFlags |= 0x00000010;
          pNew->u.btree.nTop = 1;
        }
      } else {


        ;
        ;
        pNew->wsFlags |= 0x00000002 | 0x00000010;
        pNew->u.btree.nTop = nVecLen;
        pTop = pTerm;
        pBtm = (pNew->wsFlags & 0x00000020) != 0 ? pNew->aLTerm[pNew->nLTerm - 2] : 0;
      }
    }


    if (pNew->wsFlags & 0x00000002) {

      whereRangeScanEst(pParse, pBuilder, pBtm, pTop, pNew);
    } else {
      int nEq = ++pNew->u.btree.nEq;




      if (pTerm->truthProb <= 0 && pProbe->aiColumn[saved_nEq] >= 0) {


        ;
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      } else {

        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq - 1]);
          if (eOp & 0x0100) {

            pNew->nOut += 10;
          }
        }
      }
    }


    if (pProbe->idxType == 3) {

      rCostIdx = pNew->nOut + 16;
    } else {
      rCostIdx = pNew->nOut + 1 + (15 * pProbe->szIdxRow) / pSrc->pSTab->szTabRow;
    }
    rCostIdx = sqlite3LogEstAdd(rLogSize, rCostIdx);

    pNew->rRun = rCostIdx;
    if ((pNew->wsFlags & (0x00000040 | 0x00000100 | 0x04000000)) == 0) {
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    };

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, rSize);
    if (pSrc->fg.fromExists)
      pNew->nOut = 0;
    rc = whereLoopInsert(pBuilder, pNew);

    if (pNew->wsFlags & 0x00000002) {
      pNew->nOut = saved_nOut;
    } else {
      pNew->nOut = nOutUnadjusted;
    }

    if ((pNew->wsFlags & 0x00000010) == 0 && pNew->u.btree.nEq < pProbe->nColumn && (pNew->u.btree.nEq < pProbe->nKeyCol || pProbe->idxType != 2)) {
      if (pNew->u.btree.nEq > 3) {
        sqlite3ProgressCheck(pParse);
      }
      whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, nInMul + nIn);
    }
    pNew->nOut = saved_nOut;
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->u.btree.nBtm = saved_nBtm;
  pNew->u.btree.nTop = saved_nTop;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->nOut = saved_nOut;
  pNew->nLTerm = saved_nLTerm;

  if (saved_nEq == saved_nSkip && saved_nEq + 1 < pProbe->nKeyCol && saved_nEq == pNew->nLTerm && pProbe->noSkipScan == 0 && pProbe->hasStat1 != 0 && (((db)->dbOptFlags & (0x00004000)) == 0) && pProbe->aiRowLogEst[saved_nEq + 1] >= 42 && pSrc->fg.fromExists == 0 && (rc = whereLoopResize(db, pNew, pNew->nLTerm + 1)) == 0) {
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= 0x00008000;
    nIter = pProbe->aiRowLogEst[saved_nEq] - pProbe->aiRowLogEst[saved_nEq + 1];
    pNew->nOut -= nIter;

    nIter += 5;
    whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, nIter + nInMul);
    pNew->nOut = saved_nOut;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  ;
  return rc;
}

int indexMightHelpWithOrderBy(WhereLoopBuilder *pBuilder, Index *pIndex, int iCursor) {
  ExprList *pOB;
  ExprList *aColExpr;
  int ii, jj;

  if (pIndex->bUnordered)
    return 0;
  if ((pOB = pBuilder->pWInfo->pOrderBy) == 0)
    return 0;
  for (ii = 0; ii < pOB->nExpr; ii++) {
    Expr *pExpr = sqlite3ExprSkipCollateAndLikely(pOB->a[ii].pExpr);
    if ((pExpr == 0))
      continue;
    if ((pExpr->op == 168 || pExpr->op == 170) && pExpr->iTable == iCursor) {
      if (pExpr->iColumn < 0)
        return 1;
      for (jj = 0; jj < pIndex->nKeyCol; jj++) {
        if (pExpr->iColumn == pIndex->aiColumn[jj])
          return 1;
      }
    } else if ((aColExpr = pIndex->aColExpr) != 0) {
      for (jj = 0; jj < pIndex->nKeyCol; jj++) {
        if (pIndex->aiColumn[jj] != (-2))
          continue;
        if (sqlite3ExprCompareSkip(pExpr, aColExpr->a[jj].pExpr, iCursor) == 0) {
          return 1;
        }
      }
    }
  }
  return 0;
}

int whereLoopAddBtree(WhereLoopBuilder *pBuilder, Bitmask mPrereq) {
  WhereInfo *pWInfo;
  Index *pProbe;
  Index sPk;
  LogEst aiRowEstPk[2];
  i16 aiColumnPk = -1;
  SrcList *pTabList;
  SrcItem *pSrc;
  WhereLoop *pNew;
  int rc = 0;
  int iSortIdx = 1;
  int b;
  LogEst rSize;
  WhereClause *pWC;
  Table *pTab;

  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pSTab;
  pWC = pBuilder->pWC;

  if (pSrc->fg.isIndexedBy) {



    pProbe = pSrc->u2.pIBIndex;
  } else if (!(((pTab)->tabFlags & 0x00000080) == 0)) {
    pProbe = pTab->pIndex;
  } else {

    Index *pFirst;
    memset(&sPk, 0, sizeof(Index));
    sPk.nKeyCol = 1;
    sPk.nColumn = 1;
    sPk.aiColumn = &aiColumnPk;
    sPk.aiRowLogEst = aiRowEstPk;
    sPk.onError = 5;
    sPk.pTable = pTab;
    sPk.szIdxRow = 3;
    sPk.idxType = 3;
    aiRowEstPk[0] = pTab->nRowLogEst;
    aiRowEstPk[1] = 0;
    pFirst = pSrc->pSTab->pIndex;
    if (pSrc->fg.notIndexed == 0) {

      sPk.pNext = pFirst;
    }
    pProbe = &sPk;
  }
  rSize = pTab->nRowLogEst;

  if (!pBuilder->pOrSet && (pWInfo->wctrlFlags & (0x1000 | 0x0020)) == 0 && (pWInfo->pParse->db->flags & 0x00008000) != 0 && !pSrc->fg.isIndexedBy && !pSrc->fg.notIndexed && !pSrc->fg.isCorrelated && !pSrc->fg.isRecursive && (pSrc->fg.jointype & 0x10) == 0) {

    LogEst rLogSize;
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    rLogSize = estLog(rSize);
    for (pTerm = pWC->a; rc == 0 && pTerm < pWCEnd; pTerm++) {
      if (pTerm->prereqRight & pNew->maskSelf)
        continue;
      if (termCanDriveIndex(pTerm, pSrc, 0)) {
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.pIndex = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = pTerm;

        pNew->rSetup = rLogSize + rSize;
        if (!((pTab)->eTabType == 2) && (pTab->tabFlags & 0x00004000) == 0) {
          pNew->rSetup += 28;
        } else {
          pNew->rSetup -= 25;
        };
        if (pNew->rSetup < 0)
          pNew->rSetup = 0;

        pNew->nOut = 43;


        pNew->rRun = sqlite3LogEstAdd(rLogSize, pNew->nOut);
        pNew->wsFlags = 0x00004000;
        pNew->prereq = mPrereq | pTerm->prereqRight;
        rc = whereLoopInsert(pBuilder, pNew);
      }
    }
  }

  for (; rc == 0 && pProbe; pProbe = (pSrc->fg.isIndexedBy ? 0 : pProbe->pNext), iSortIdx++) {
    if (pProbe->pPartIdxWhere != 0 && !whereUsablePartialIndex(pSrc->iCursor, pSrc->fg.jointype, pWC, pProbe->pPartIdxWhere)) {
      ;
      continue;
    }
    if (pProbe->bNoQuery)
      continue;
    rSize = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->u.btree.nBtm = 0;
    pNew->u.btree.nTop = 0;
    pNew->u.btree.nDistinctCol = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->iSortIdx = 0;
    pNew->rSetup = 0;
    pNew->prereq = mPrereq;
    pNew->nOut = rSize;
    pNew->u.btree.pIndex = pProbe;
    pNew->u.btree.pOrderBy = 0;
    b = indexMightHelpWithOrderBy(pBuilder, pProbe, pSrc->iCursor);


    if (pProbe->idxType == 3) {

      pNew->wsFlags = 0x00000100;

      pNew->iSortIdx = b ? iSortIdx : 0;

      pNew->rRun = rSize + 16;

      ;
      whereLoopOutputAdjust(pWC, pNew, rSize);
      if (pSrc->fg.isSubquery) {
        if (pSrc->fg.viaCoroutine)
          pNew->wsFlags |= 0x02000000;

        if ((pSrc->u4.pSubq->pSelect->selFlags & 0x0002000) == 0) {
          pNew->u.btree.pOrderBy = pSrc->u4.pSubq->pSelect->pOrderBy;
        }
      } else if (pSrc->fg.fromExists) {
        pNew->nOut = 0;
      }
      rc = whereLoopInsert(pBuilder, pNew);
      pNew->nOut = rSize;
      if (rc)
        break;
    } else {
      Bitmask m;
      if (pProbe->isCovering) {
        m = 0;
        pNew->wsFlags = 0x00000040 | 0x00000200;
      } else {
        m = pSrc->colUsed & pProbe->colNotIdxed;
        if (pProbe->pPartIdxWhere) {
          wherePartIdxExpr(pWInfo->pParse, pProbe, pProbe->pPartIdxWhere, &m, 0, 0);
        }
        pNew->wsFlags = 0x00000200;
        if (m == (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1)) || (pProbe->bHasExpr && !pProbe->bHasVCol && m != 0)) {
          u32 isCov = whereIsCoveringIndex(pWInfo, pProbe, pSrc->iCursor);
          if (isCov == 0) {

            ;


          } else {
            m = 0;
            pNew->wsFlags |= isCov;
            if (isCov & 0x00000040) {

              ;
            } else {



              ;
            }
          }
        } else if (m == 0 && ((((pTab)->tabFlags & 0x00000080) == 0) || pWInfo->pSelect != 0 || sqlite3FaultSim(700))) {

          ;
          pNew->wsFlags = 0x00000040 | 0x00000200;
        }
      }

      if (b || !(((pTab)->tabFlags & 0x00000080) == 0) || pProbe->pPartIdxWhere != 0 || pSrc->fg.isIndexedBy || (m == 0 && pProbe->bUnordered == 0 && (pProbe->szIdxRow < pTab->szTabRow) && (pWInfo->wctrlFlags & 0x0004) == 0 && sqlite3Config.bUseCis && (((pWInfo->pParse->db)->dbOptFlags & (0x00000020)) == 0))) {
        pNew->iSortIdx = b ? iSortIdx : 0;

        pNew->rRun = rSize + 1 + (15 * pProbe->szIdxRow) / pTab->szTabRow;
        if (m != 0) {

          LogEst nLookup = rSize + 16;
          int ii;
          int iCur = pSrc->iCursor;
          WhereClause *pWC2 = &pWInfo->sWC;
          for (ii = 0; ii < pWC2->nTerm; ii++) {
            WhereTerm *pTerm = &pWC2->a[ii];
            if (!sqlite3ExprCoveredByIndex(pTerm->pExpr, iCur, pProbe)) {
              break;
            }

            if (pTerm->truthProb <= 0) {
              nLookup += pTerm->truthProb;
            } else {
              nLookup--;
              if (pTerm->eOperator & (0x0002 | 0x0080))
                nLookup -= 19;
            }
          }

          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, nLookup);
        };
        whereLoopOutputAdjust(pWC, pNew, rSize);
        if ((pSrc->fg.jointype & 0x10) != 0 && pProbe->aColExpr) {

        } else {
          if (pSrc->fg.fromExists)
            pNew->nOut = 0;
          rc = whereLoopInsert(pBuilder, pNew);
        }
        pNew->nOut = rSize;
        if (rc)
          break;
      }
    }

    pBuilder->bldFlags1 = 0;
    rc = whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, 0);
    if (pBuilder->bldFlags1 == 0x0001) {

      pTab->tabFlags |= 0x00000100;
    }
  }
  return rc;
}

int whereLoopAddVirtualOne(WhereLoopBuilder *pBuilder, Bitmask mPrereq, Bitmask mUsable, u16 mExclude, sqlite3_index_info *pIdxInfo, u16 mNoOmit, int *pbIn, int *pbRetryLimit) {
  WhereClause *pWC = pBuilder->pWC;
  HiddenIndexInfo *pHidden = (HiddenIndexInfo *)&pIdxInfo[1];
  struct sqlite3_index_constraint *pIdxCons;
  struct sqlite3_index_constraint_usage *pUsage = pIdxInfo->aConstraintUsage;
  int i;
  int mxTerm;
  int rc = 0;
  WhereLoop *pNew = pBuilder->pNew;
  Parse *pParse = pBuilder->pWInfo->pParse;
  SrcItem *pSrc = &pBuilder->pWInfo->pTabList->a[pNew->iTab];
  int nConstraint = pIdxInfo->nConstraint;

  *pbIn = 0;
  pNew->prereq = mPrereq;

  pIdxCons = *(struct sqlite3_index_constraint **)&pIdxInfo->aConstraint;
  for (i = 0; i < nConstraint; i++, pIdxCons++) {
    WhereTerm *pTerm = termFromWhereClause(pWC, pIdxCons->iTermOffset);
    pIdxCons->usable = 0;
    if ((pTerm->prereqRight & mUsable) == pTerm->prereqRight && (pTerm->eOperator & mExclude) == 0 && (pbRetryLimit || !isLimitTerm(pTerm))) {
      pIdxCons->usable = 1;
    }
  }

  memset(pUsage, 0, sizeof(pUsage[0]) * nConstraint);

  pIdxInfo->idxStr = 0;
  pIdxInfo->idxNum = 0;
  pIdxInfo->orderByConsumed = 0;
  pIdxInfo->estimatedCost = (1e99) / (double)2;
  pIdxInfo->estimatedRows = 25;
  pIdxInfo->idxFlags = 0;
  pHidden->mHandleIn = 0;

  rc = vtabBestIndex(pParse, pSrc->pSTab, pIdxInfo);
  if (rc) {
    if (rc == 19) {

      ;
      freeIdxStr(pIdxInfo);
      return 0;
    }
    return rc;
  }

  mxTerm = -1;

  memset(pNew->aLTerm, 0, sizeof(pNew->aLTerm[0]) * nConstraint);
  memset(&pNew->u.vtab, 0, sizeof(pNew->u.vtab));
  pIdxCons = *(struct sqlite3_index_constraint **)&pIdxInfo->aConstraint;
  for (i = 0; i < nConstraint; i++, pIdxCons++) {
    int iTerm;
    if ((iTerm = pUsage[i].argvIndex - 1) >= 0) {
      WhereTerm *pTerm;
      int j = pIdxCons->iTermOffset;
      if (iTerm >= nConstraint || j < 0 || (pTerm = termFromWhereClause(pWC, j)) == 0 || pNew->aLTerm[iTerm] != 0 || pIdxCons->usable == 0) {
        sqlite3ErrorMsg(pParse, "%s.xBestIndex malfunction", pSrc->pSTab->zName);
        freeIdxStr(pIdxInfo);
        return 1;
      };
      ;
      ;
      pNew->prereq |= pTerm->prereqRight;


      pNew->aLTerm[iTerm] = pTerm;
      if (iTerm > mxTerm)
        mxTerm = iTerm;
      ;
      ;
      if (pUsage[i].omit) {
        if (i < 16 && ((1 << i) & mNoOmit) == 0) {
          ;
          pNew->u.vtab.omitMask |= 1 << iTerm;
        } else {
          ;
        }
        if (pTerm->eMatchOp == 74) {
          pNew->u.vtab.bOmitOffset = 1;
        }
      }
      if (((i) <= 31 ? ((unsigned int)1) << (i) : 0) & pHidden->mHandleIn) {
        pNew->u.vtab.mHandleIn |= (((unsigned int)1) << (iTerm));
      } else if ((pTerm->eOperator & 0x0001) != 0) {

        pIdxInfo->orderByConsumed = 0;
        pIdxInfo->idxFlags &= ~0x00000001;
        *pbIn = 1;


      }







      if (isLimitTerm(pTerm) && (*pbIn || !allConstraintsUsed(pUsage, i))) {

        freeIdxStr(pIdxInfo);
        *pbRetryLimit = 1;
        return 0;
      }
    }
  }

  pNew->nLTerm = mxTerm + 1;
  for (i = 0; i <= mxTerm; i++) {
    if (pNew->aLTerm[i] == 0) {

      sqlite3ErrorMsg(pParse, "%s.xBestIndex malfunction", pSrc->pSTab->zName);
      freeIdxStr(pIdxInfo);
      return 1;
    }
  }

  pNew->u.vtab.idxNum = pIdxInfo->idxNum;
  pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
  pIdxInfo->needToFreeIdxStr = 0;
  pNew->u.vtab.idxStr = pIdxInfo->idxStr;
  pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ? pIdxInfo->nOrderBy : 0);
  pNew->u.vtab.bIdxNumHex = (pIdxInfo->idxFlags & 0x00000002) != 0;
  pNew->rSetup = 0;
  pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
  pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);

  if (pIdxInfo->idxFlags & 0x00000001) {
    pNew->wsFlags |= 0x00001000;
  } else {
    pNew->wsFlags &= ~0x00001000;
  }
  rc = whereLoopInsert(pBuilder, pNew);
  if (pNew->u.vtab.needFree) {
    sqlite3_free(pNew->u.vtab.idxStr);
    pNew->u.vtab.needFree = 0;
  }

  ;

  return rc;
}

int whereLoopAddVirtual(WhereLoopBuilder *pBuilder, Bitmask mPrereq, Bitmask mUnusable) {
  int rc = 0;
  WhereInfo *pWInfo;
  Parse *pParse;
  WhereClause *pWC;
  SrcItem *pSrc;
  sqlite3_index_info *p;
  int nConstraint;
  int bIn;
  WhereLoop *pNew;
  Bitmask mBest;
  u16 mNoOmit;
  int bRetry = 0;

  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];

  p = allocateIndexInfo(pWInfo, pWC, mUnusable, pSrc, &mNoOmit);
  if (p == 0)
    return 7;
  pNew->rSetup = 0;
  pNew->wsFlags = 0x00000400;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  nConstraint = p->nConstraint;
  if (whereLoopResize(pParse->db, pNew, nConstraint)) {
    freeIndexInfo(pParse->db, p);
    return 7;
  }

  ;
  ;
  rc = whereLoopAddVirtualOne(pBuilder, mPrereq, ((Bitmask)-1), 0, p, mNoOmit, &bIn, &bRetry);
  if (bRetry) {


    rc = whereLoopAddVirtualOne(pBuilder, mPrereq, ((Bitmask)-1), 0, p, mNoOmit, &bIn, 0);
  }

  if (rc == 0 && ((mBest = (pNew->prereq & ~mPrereq)) != 0 || bIn)) {
    int seenZero = 0;
    int seenZeroNoIN = 0;
    Bitmask mPrev = 0;
    Bitmask mBestNoIn = 0;

    if (bIn) {
      ;
      rc = whereLoopAddVirtualOne(pBuilder, mPrereq, ((Bitmask)-1), 0x0001, p, mNoOmit, &bIn, 0);


      mBestNoIn = pNew->prereq & ~mPrereq;
      if (mBestNoIn == 0) {
        seenZero = 1;
        seenZeroNoIN = 1;
      }
    }

    while (rc == 0) {
      int i;
      Bitmask mNext = ((Bitmask)-1);


      for (i = 0; i < nConstraint; i++) {
        int iTerm = p->aConstraint[i].iTermOffset;
        Bitmask mThis = termFromWhereClause(pWC, iTerm)->prereqRight & ~mPrereq;
        if (mThis > mPrev && mThis < mNext)
          mNext = mThis;
      }
      mPrev = mNext;
      if (mNext == ((Bitmask)-1))
        break;
      if (mNext == mBest || mNext == mBestNoIn)
        continue;

      ;
      rc = whereLoopAddVirtualOne(pBuilder, mPrereq, mNext | mPrereq, 0, p, mNoOmit, &bIn, 0);
      if (pNew->prereq == mPrereq) {
        seenZero = 1;
        if (bIn == 0)
          seenZeroNoIN = 1;
      }
    }

    if (rc == 0 && seenZero == 0) {
      ;
      rc = whereLoopAddVirtualOne(pBuilder, mPrereq, mPrereq, 0, p, mNoOmit, &bIn, 0);
      if (bIn == 0)
        seenZeroNoIN = 1;
    }

    if (rc == 0 && seenZeroNoIN == 0) {
      ;
      rc = whereLoopAddVirtualOne(pBuilder, mPrereq, mPrereq, 0x0001, p, mNoOmit, &bIn, 0);
    }
  }

  freeIndexInfo(pParse->db, p);
  ;
  return rc;
}

int whereLoopAddOr(WhereLoopBuilder *pBuilder, Bitmask mPrereq, Bitmask mUnusable) {
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  WhereTerm *pTerm, *pWCEnd;
  int rc = 0;
  int iCur;
  WhereClause tempWC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet sSum, sCur;
  SrcItem *pItem;

  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  memset(&sSum, 0, sizeof(sSum));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  if (pItem->fg.jointype & 0x10)
    return 0;

  for (pTerm = pWC->a; pTerm < pWCEnd && rc == 0; pTerm++) {
    if ((pTerm->eOperator & 0x0200) != 0 && (pTerm->u.pOrInfo->indexable & pNew->maskSelf) != 0) {
      WhereClause *const pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm *const pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      int once = 1;
      int i, j;

      sSubBuild = *pBuilder;
      sSubBuild.pOrSet = &sCur;

      ;
      for (pOrTerm = pOrWC->a; pOrTerm < pOrWCEnd; pOrTerm++) {
        if ((pOrTerm->eOperator & 0x0400) != 0) {
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        } else if (pOrTerm->leftCursor == iCur) {
          tempWC.pWInfo = pWC->pWInfo;
          tempWC.pOuter = pWC;
          tempWC.op = 44;
          tempWC.nTerm = 1;
          tempWC.nBase = 1;
          tempWC.a = pOrTerm;
          sSubBuild.pWC = &tempWC;
        } else {
          continue;
        }
        sCur.n = 0;

        if (((pItem->pSTab)->eTabType == 1)) {
          rc = whereLoopAddVirtual(&sSubBuild, mPrereq, mUnusable);
        } else

        {
          rc = whereLoopAddBtree(&sSubBuild, mPrereq);
        }
        if (rc == 0) {
          rc = whereLoopAddOr(&sSubBuild, mPrereq, mUnusable);
        };
        ;
        if (sCur.n == 0) {
          sSum.n = 0;
          break;
        } else if (once) {
          whereOrMove(&sSum, &sCur);
          once = 0;
        } else {
          WhereOrSet sPrev;
          whereOrMove(&sPrev, &sSum);
          sSum.n = 0;
          for (i = 0; i < sPrev.n; i++) {
            for (j = 0; j < sCur.n; j++) {
              whereOrInsert(&sSum, sPrev.a[i].prereq | sCur.a[j].prereq, sqlite3LogEstAdd(sPrev.a[i].rRun, sCur.a[j].rRun), sqlite3LogEstAdd(sPrev.a[i].nOut, sCur.a[j].nOut));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = pTerm;
      pNew->wsFlags = 0x00002000;
      pNew->rSetup = 0;
      pNew->iSortIdx = 0;
      memset(&pNew->u, 0, sizeof(pNew->u));
      for (i = 0; rc == 0 && i < sSum.n; i++) {

        pNew->rRun = sSum.a[i].rRun + 1;
        pNew->nOut = sSum.a[i].nOut;
        pNew->prereq = sSum.a[i].prereq;
        rc = whereLoopInsert(pBuilder, pNew);
      };
    }
  }
  return rc;
}

int whereLoopAddAll(WhereLoopBuilder *pBuilder) {
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mPrereq = 0;
  Bitmask mPrior = 0;
  int iTab;
  SrcList *pTabList = pWInfo->pTabList;
  SrcItem *pItem;
  SrcItem *pEnd = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  int rc = 0;
  int bFirstPastRJ = 0;
  int hasRightCrossJoin = 0;
  WhereLoop *pNew;

  pNew = pBuilder->pNew;

  pBuilder->iPlanLimit = 20000;
  for (iTab = 0, pItem = pTabList->a; pItem < pEnd; iTab++, pItem++) {
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pBuilder->iPlanLimit += 1000;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    if (bFirstPastRJ || (pItem->fg.jointype & (0x20 | 0x02 | 0x40)) != 0) {

      if (pItem->fg.jointype & (0x40 | 0x02)) {
        ;
        ;
        hasRightCrossJoin = 1;
      }
      mPrereq |= mPrior;
      bFirstPastRJ = (pItem->fg.jointype & 0x10) != 0;
    } else if (pItem->fg.fromExists) {

      WhereClause *pWC = &pWInfo->sWC;
      WhereTerm *pTerm;
      int i;
      for (i = pWC->nBase, pTerm = pWC->a; i > 0; i--, pTerm++) {
        if ((pNew->maskSelf & pTerm->prereqAll) != 0) {
          mPrereq |= (pTerm->prereqAll & (pNew->maskSelf - 1));
        }
      }
    } else if (!hasRightCrossJoin) {
      mPrereq = 0;
    }

    if (((pItem->pSTab)->eTabType == 1)) {
      SrcItem *p;
      for (p = &pItem[1]; p < pEnd; p++) {
        if (mUnusable || (p->fg.jointype & (0x20 | 0x02))) {
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mPrereq, mUnusable);
    } else

    {
      rc = whereLoopAddBtree(pBuilder, mPrereq);
    }
    if (rc == 0 && pBuilder->pWC->hasOr) {
      rc = whereLoopAddOr(pBuilder, mPrereq, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    if (rc || db->mallocFailed) {
      if (rc == 101) {

        sqlite3_log(28, "abbreviated query algorithm search");
        rc = 0;
      } else {
        break;
      }
    }
  }

  whereLoopClear(db, pNew);
  return rc;
}

int whereShortCut(WhereLoopBuilder *pBuilder) {
  WhereInfo *pWInfo;
  SrcItem *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  int iCur;
  int j;
  Table *pTab;
  Index *pIdx;
  WhereScan scan;

  pWInfo = pBuilder->pWInfo;
  if (pWInfo->wctrlFlags & 0x0020)
    return 0;

  pItem = pWInfo->pTabList->a;
  pTab = pItem->pSTab;
  if (((pTab)->eTabType == 1))
    return 0;
  if (pItem->fg.isIndexedBy || pItem->fg.notIndexed) {
    ;
    ;
    return 0;
  }
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = whereScanInit(&scan, pWC, iCur, -1, 0x0002 | 0x0080, 0);
  while (pTerm && pTerm->prereqRight)
    pTerm = whereScanNext(&scan);
  if (pTerm) {
    ;
    pLoop->wsFlags = 0x00000001 | 0x00000100 | 0x00001000;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;

    pLoop->rRun = 33;
  } else {
    for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      int opMask;


      if (!((pIdx)->onError != 0) || pIdx->pPartIdxWhere != 0 || pIdx->nKeyCol > ((int)(sizeof(pLoop->aLTermSpace) / sizeof(pLoop->aLTermSpace[0]))))
        continue;
      opMask = pIdx->uniqNotNull ? (0x0002 | 0x0080) : 0x0002;
      for (j = 0; j < pIdx->nKeyCol; j++) {
        pTerm = whereScanInit(&scan, pWC, iCur, j, opMask, pIdx);
        while (pTerm && pTerm->prereqRight)
          pTerm = whereScanNext(&scan);
        if (pTerm == 0)
          break;
        ;
        pLoop->aLTerm[j] = pTerm;
      }
      if (j != pIdx->nKeyCol)
        continue;
      pLoop->wsFlags = 0x00000001 | 0x00001000 | 0x00000200;
      if (pIdx->isCovering || (pItem->colUsed & pIdx->colNotIdxed) == 0) {
        pLoop->wsFlags |= 0x00000040;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.pIndex = pIdx;

      pLoop->rRun = 39;
      break;
    }
  }
  if (pLoop->wsFlags) {
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;


    pLoop->maskSelf = 1;
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    if (pWInfo->pOrderBy)
      pWInfo->nOBSat = pWInfo->pOrderBy->nExpr;
    if (pWInfo->wctrlFlags & 0x0100) {
      pWInfo->eDistinct = 1;
    }
    if (scan.iEquiv > 1)
      pLoop->wsFlags |= 0x00200000;

    return 1;
  }
  return 0;
}