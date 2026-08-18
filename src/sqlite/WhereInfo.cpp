#define _GNU_SOURCE 1
#include <string.h>
#include <stddef.h>
#include "sqlite/WhereInfo.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/CoveringIndexCheck.h"
#include "sqlite/CteUse.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/HiddenIndexInfo.h"
#include "sqlite/Index.h"
#include "sqlite/IndexedExpr.h"
#include "sqlite/LogEst.h"
#include "sqlite/Parse.h"
#include "sqlite/Pgno.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Table.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/Walker.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereLevel.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/WhereMaskSet.h"
#include "sqlite/WhereMemBlock.h"
#include "sqlite/WherePath.h"
#include "sqlite/WhereRightJoin.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/i16.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_index_info.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
#include "sqlite/SqliteIndexConstraintOp.h"
#include "sqlite/SqliteResultCode.h"
LogEst estLog(LogEst N) {
  return N <= 10 ? 0 : sqlite3LogEst(N) - 33;
}

void codeDeferredSeek(WhereInfo *pWInfo, Index *pIdx, int iCur, int iIdxCur) {
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;

  pWInfo->bDeferredSeek = 1;
  sqlite3VdbeAddOp3(v, 143, iIdxCur, 0, iCur);
  if ((pWInfo->wctrlFlags & (0x0020 | 0x1000)) &&
      ((((pParse)->pToplevel ? (pParse)->pToplevel : (pParse))->writeMask) == 0)) {
    int i;
    Table *pTab = pIdx->pTable;
    u32 *ai = (u32 *)sqlite3DbMallocZero(pParse->db, sizeof(u32) * (pTab->nCol + 1));
    if (ai) {
      ai[0] = pTab->nCol;
      for (i = 0; i < pIdx->nColumn - 1; i++) {
        int x1, x2;

        x1 = pIdx->aiColumn[i];
        x2 = sqlite3TableColumnToStorage(pTab, x1);
        if (x1 >= 0)
          ai[x2 + 1] = i + 1;
      }
      sqlite3VdbeChangeP4(v, -1, (char *)ai, (-15));
    }
  }
}

__attribute__((noinline)) void sqlite3WhereRightJoinLoop(WhereInfo *pWInfo, int iLevel, WhereLevel *pLevel) {
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  WhereRightJoin *pRJ = pLevel->pRJ;
  Expr *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  SrcItem *pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  union {
    SrcList sSrc;
    u8 fromSpace[(offsetof(SrcList, a) + sizeof(SrcItem))];
  } uSrc;
  Bitmask mAll = 0;
  int k;

  sqlite3VdbeExplain(pParse, 1, "RIGHT-JOIN %s", pTabItem->pSTab->zName);

  for (k = 0; k < iLevel; k++) {
    int iIdxCur;
    SrcItem *pRight;

    pRight = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->maskSelf;
    if (pRight->fg.viaCoroutine) {
      Subquery *pSubq;

      pSubq = pRight->u4.pSubq;

      sqlite3VdbeAddOp3(v, 77, 0, pSubq->regResult, pSubq->regResult + pSubq->pSelect->pEList->nExpr - 1);
    }
    sqlite3VdbeAddOp1(v, 138, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    if (iIdxCur) {
      sqlite3VdbeAddOp1(v, 138, iIdxCur);
    }
  }
  if ((pTabItem->fg.jointype & 0x40) == 0) {
    mAll |= pLoop->maskSelf;
    for (k = 0; k < pWC->nTerm; k++) {
      WhereTerm *pTerm = &pWC->a[k];
      if ((pTerm->wtFlags & (0x0002 | 0x8000)) != 0 && pTerm->eOperator != 0x2000) {
        break;
      }
      if (pTerm->prereqAll & ~mAll)
        continue;
      if ((((pTerm->pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0))
        continue;
      pSubWhere = sqlite3ExprAnd(pParse, pSubWhere, sqlite3ExprDup(pParse->db, pTerm->pExpr, 0));
    }
  }
  if (pLevel->iIdxCur) {
    sqlite3VdbeAddOp1(v, 138, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  memcpy(&pFrom->a[0], pTabItem, sizeof(SrcItem));
  pFrom->a[0].fg.jointype = 0;

  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0, 0x1000, 0);
  if (pSubWInfo) {
    int iCur = pLevel->iTabCur;
    int r = ++pParse->nMem;
    int nPk;
    int jmp;
    int addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    Table *pTab = pTabItem->pSTab;
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, -1, r);
      nPk = 1;
    } else {
      int iPk;
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      nPk = pPk->nKeyCol;
      pParse->nMem += nPk - 1;
      for (iPk = 0; iPk < nPk; iPk++) {
        int iCol = pPk->aiColumn[iPk];
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, iCol, r + iPk);
      }
    }
    jmp = sqlite3VdbeAddOp4Int(v, 66, pRJ->regBloom, 0, r, nPk);
    sqlite3VdbeAddOp4Int(v, 29, pRJ->iMatch, addrCont, r, nPk);
    sqlite3VdbeJumpHere(v, jmp);
    sqlite3VdbeAddOp2(v, 10, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3WhereEnd(pSubWInfo);
  }
  sqlite3ExprDelete(pParse->db, pSubWhere);
  sqlite3VdbeExplainPop(pParse);

  pParse->withinRJSubrtn--;
}

LogEst sqlite3WhereOutputRowCount(WhereInfo *pWInfo) {
  return pWInfo->nRowOut;
}

int sqlite3WhereIsDistinct(WhereInfo *pWInfo) {
  return pWInfo->eDistinct;
}

int sqlite3WhereIsOrdered(WhereInfo *pWInfo) {
  return pWInfo->nOBSat < 0 ? 0 : pWInfo->nOBSat;
}

int sqlite3WhereOrderByLimitOptLabel(WhereInfo *pWInfo) {
  WhereLevel *pInner;
  if (!pWInfo->bOrderedInnerLoop) {
    return pWInfo->iContinue;
  }
  pInner = &pWInfo->a[pWInfo->nLevel - 1];

  return pInner->pRJ ? pWInfo->iContinue : pInner->addrNxt;
}

int sqlite3WhereContinueLabel(WhereInfo *pWInfo) {
  return pWInfo->iContinue;
}

int sqlite3WhereBreakLabel(WhereInfo *pWInfo) {
  return pWInfo->iBreak;
}

int sqlite3WhereOkOnePass(WhereInfo *pWInfo, int *aiCur) {
  memcpy(aiCur, pWInfo->aiCurOnePass, sizeof(int) * 2);

  return pWInfo->eOnePass;
}

int sqlite3WhereUsesDeferredSeek(WhereInfo *pWInfo) {
  return pWInfo->bDeferredSeek;
}

void *sqlite3WhereMalloc(WhereInfo *pWInfo, u64 nByte) {
  WhereMemBlock *pBlock;
  pBlock = sqlite3DbMallocRawNN(pWInfo->pParse->db, nByte + sizeof(*pBlock));
  if (pBlock) {
    pBlock->pNext = pWInfo->pMemToFree;
    pBlock->sz = nByte;
    pWInfo->pMemToFree = pBlock;
    pBlock++;
  }
  return (void *)pBlock;
}

void *sqlite3WhereRealloc(WhereInfo *pWInfo, void *pOld, u64 nByte) {
  void *pNew = sqlite3WhereMalloc(pWInfo, nByte);
  if (pNew && pOld) {
    WhereMemBlock *pOldBlk = (WhereMemBlock *)pOld;
    pOldBlk--;

    memcpy(pNew, pOld, pOldBlk->sz);
  }
  return pNew;
}

__attribute__((noinline)) void sqlite3ConstructBloomFilter(WhereInfo *pWInfo, int iLevel, WhereLevel *pLevel,
                                                           Bitmask notReady) {
  int addrOnce;
  int addrTop;
  int addrCont;
  const WhereTerm *pTerm;
  const WhereTerm *pWCEnd;
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  WhereLoop *pLoop = pLevel->pWLoop;
  int iCur;
  IndexedExpr *saved_pIdxEpr;
  IndexedExpr *saved_pIdxPartExpr;

  saved_pIdxEpr = pParse->pIdxEpr;
  saved_pIdxPartExpr = pParse->pIdxPartExpr;
  pParse->pIdxEpr = 0;
  pParse->pIdxPartExpr = 0;

  addrOnce = sqlite3VdbeAddOp0(v, 15);
  do {
    const SrcList *pTabList;
    const SrcItem *pItem;
    const Table *pTab;
    u64 sz;
    int iSrc;
    sqlite3WhereExplainBloomFilter(pParse, pWInfo, pLevel);
    addrCont = sqlite3VdbeMakeLabel(pParse);
    iCur = pLevel->iTabCur;
    pLevel->regFilter = ++pParse->nMem;

    pTabList = pWInfo->pTabList;
    iSrc = pLevel->iFrom;
    pItem = &pTabList->a[iSrc];

    pTab = pItem->pSTab;

    sz = sqlite3LogEstToInt(pTab->nRowLogEst);
    if (sz < 10000) {
      sz = 10000;
    } else if (sz > 10000000) {
      sz = 10000000;
    }
    sqlite3VdbeAddOp2(v, 79, (int)sz, pLevel->regFilter);

    addrTop = sqlite3VdbeAddOp1(v, 36, iCur);
    pWCEnd = &pWInfo->sWC.a[pWInfo->sWC.nTerm];
    for (pTerm = pWInfo->sWC.a; pTerm < pWCEnd; pTerm++) {
      Expr *pExpr = pTerm->pExpr;
      if ((pTerm->wtFlags & 0x0002) == 0 && sqlite3ExprIsSingleTableConstraint(pExpr, pTabList, iSrc, 0)) {
        sqlite3ExprIfFalse(pParse, pTerm->pExpr, addrCont, 0x10);
      }
    }
    if (pLoop->wsFlags & 0x00000100) {
      int r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, 137, iCur, r1);
      sqlite3VdbeAddOp4Int(v, 185, pLevel->regFilter, 0, r1, 1);
      sqlite3ReleaseTempReg(pParse, r1);
    } else {
      Index *pIdx = pLoop->u.btree.pIndex;
      int n = pLoop->u.btree.nEq;
      int r1 = sqlite3GetTempRange(pParse, n);
      int jj;
      for (jj = 0; jj < n; jj++) {
        sqlite3ExprCodeLoadIndexColumn(pParse, pIdx, iCur, jj, r1 + jj);
      }
      sqlite3VdbeAddOp4Int(v, 185, pLevel->regFilter, 0, r1, n);
      sqlite3ReleaseTempRange(pParse, r1, n);
    }
    sqlite3VdbeResolveLabel(v, addrCont);
    sqlite3VdbeAddOp2(v, 40, pLevel->iTabCur, addrTop + 1);
    sqlite3VdbeJumpHere(v, addrTop);
    pLoop->wsFlags &= ~0x00400000;
    if ((((pParse->db)->dbOptFlags & (0x00100000)) != 0))
      break;
    while (++iLevel < pWInfo->nLevel) {
      const SrcItem *pTabItem;
      pLevel = &pWInfo->a[iLevel];
      pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
      if (pTabItem->fg.jointype & (0x08 | 0x40))
        continue;
      pLoop = pLevel->pWLoop;
      if (pLoop == 0)
        continue;
      if (pLoop->prereq & notReady)
        continue;
      if ((pLoop->wsFlags & (0x00400000 | 0x00000004)) == 0x00400000) {
        break;
      }
    }
  } while (iLevel < pWInfo->nLevel);
  sqlite3VdbeJumpHere(v, addrOnce);
  pParse->pIdxEpr = saved_pIdxEpr;
  pParse->pIdxPartExpr = saved_pIdxPartExpr;
}

sqlite3_index_info *allocateIndexInfo(WhereInfo *pWInfo, WhereClause *pWC, Bitmask mUnusable, SrcItem *pSrc,
                                      u16 *pmNoOmit) {
  int i, j;
  int nTerm;
  Parse *pParse = pWInfo->pParse;
  struct sqlite3_index_constraint *pIdxCons;
  struct sqlite3_index_orderby *pIdxOrderBy;
  struct sqlite3_index_constraint_usage *pUsage;
  struct HiddenIndexInfo *pHidden;
  WhereTerm *pTerm;
  int nOrderBy;
  sqlite3_index_info *pIdxInfo;
  u16 mNoOmit = 0;
  const Table *pTab;
  int eDistinct = 0;
  ExprList *pOrderBy = pWInfo->pOrderBy;
  WhereClause *p;

  pTab = pSrc->pSTab;

  for (p = pWC, nTerm = 0; p; p = p->pOuter) {
    for (i = 0, pTerm = p->a; i < p->nTerm; i++, pTerm++) {
      pTerm->wtFlags &= ~0x0040;
      if (pTerm->leftCursor != pSrc->iCursor)
        continue;
      if (pTerm->prereqRight & mUnusable)
        continue;

      if ((pTerm->eOperator & ~(0x0800)) == 0)
        continue;
      if (pTerm->wtFlags & 0x0080)
        continue;

      if ((pSrc->fg.jointype & (0x08 | 0x40 | 0x10)) != 0 && !constraintCompatibleWithOuterJoin(pTerm, pSrc)) {
        continue;
      }
      nTerm++;
      pTerm->wtFlags |= 0x0040;
    }
  }

  nOrderBy = 0;
  if (pOrderBy) {
    int n = pOrderBy->nExpr;
    for (i = 0; i < n; i++) {
      Expr *pExpr = pOrderBy->a[i].pExpr;
      Expr *pE2;

      if (sqlite3ExprIsConstant(0, pExpr)) {
        continue;
      }

      if (pOrderBy->a[i].fg.sortFlags & 0x02)
        break;

      if (pExpr->op == 168 && pExpr->iTable == pSrc->iCursor) {
        continue;
      }

      if (pExpr->op == 114 && (pE2 = pExpr->pLeft)->op == 168 && pE2->iTable == pSrc->iCursor) {
        const char *zColl;

        pExpr->iColumn = pE2->iColumn;
        if (pE2->iColumn < 0)
          continue;
        zColl = sqlite3ColumnColl(&pTab->aCol[pE2->iColumn]);
        if (zColl == 0)
          zColl = sqlite3StrBINARY;
        if (sqlite3_stricmp(pExpr->u.zToken, zColl) == 0)
          continue;
      }

      break;
    }
    if (i == n) {
      int bSortByGroup = (pWInfo->wctrlFlags & 0x0200) != 0;
      nOrderBy = n;
      if ((pWInfo->wctrlFlags & 0x0080) && !pSrc->fg.rowidUsed) {
        eDistinct = 2 + bSortByGroup;
      } else if (pWInfo->wctrlFlags & 0x0040) {
        eDistinct = 1 - bSortByGroup;
      } else if (pWInfo->wctrlFlags & 0x0100) {
        eDistinct = 3;
      }
    }
  }

  pIdxInfo = sqlite3DbMallocZero(pParse->db, sizeof(*pIdxInfo) + (sizeof(*pIdxCons) + sizeof(*pUsage)) * nTerm +
                                                 sizeof(*pIdxOrderBy) * nOrderBy +
                                                 (offsetof(HiddenIndexInfo, aRhs) + (nTerm) * sizeof(sqlite3_value *)));
  if (pIdxInfo == 0) {
    sqlite3ErrorMsg(pParse, "out of memory");
    return 0;
  }
  pHidden = (struct HiddenIndexInfo *)&pIdxInfo[1];
  pIdxCons = (struct sqlite3_index_constraint *)&pHidden->aRhs[nTerm];
  pIdxOrderBy = (struct sqlite3_index_orderby *)&pIdxCons[nTerm];
  pUsage = (struct sqlite3_index_constraint_usage *)&pIdxOrderBy[nOrderBy];
  pIdxInfo->aConstraint = pIdxCons;
  pIdxInfo->aOrderBy = pIdxOrderBy;
  pIdxInfo->aConstraintUsage = pUsage;
  pIdxInfo->colUsed = (sqlite3_int64)pSrc->colUsed;
  if ((((pTab)->tabFlags & 0x00000080) == 0) == 0) {
    Index *pPk = sqlite3PrimaryKeyIndex((Table *)pTab);

    for (i = 0; i < pPk->nKeyCol; i++) {
      int iCol = pPk->aiColumn[i];

      if (iCol >= ((int)(sizeof(Bitmask) * 8)) - 1)
        iCol = ((int)(sizeof(Bitmask) * 8)) - 1;
      pIdxInfo->colUsed |= (((Bitmask)1) << (iCol));
    }
  }
  pHidden->pWC = pWC;
  pHidden->pParse = pParse;
  pHidden->eDistinct = eDistinct;
  pHidden->mIn = 0;
  for (p = pWC, i = j = 0; p; p = p->pOuter) {
    int nLast = i + p->nTerm;
    for (pTerm = p->a; i < nLast; i++, pTerm++) {
      u16 op;
      if ((pTerm->wtFlags & 0x0040) == 0)
        continue;
      pIdxCons[j].iColumn = pTerm->u.x.leftColumn;
      pIdxCons[j].iTermOffset = i;
      op = pTerm->eOperator & 0x3fff;
      if (op == 0x0001) {
        if ((pTerm->wtFlags & 0x8000) == 0) {
          pHidden->mIn |= ((j) <= 31 ? ((unsigned int)1) << (j) : 0);
        }
        op = 0x0002;
      }
      if (op == 0x0040) {
        pIdxCons[j].op = pTerm->eMatchOp;
      } else if (op & (0x0100 | 0x0080)) {
        if (op == 0x0100) {
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_ISNULL;
        } else {
          pIdxCons[j].op = SQLITE_INDEX_CONSTRAINT_IS;
        }
      } else {
        pIdxCons[j].op = (u8)op;

        if (op & ((0x0002 << (57 - 54)) | (0x0002 << (56 - 54)) | (0x0002 << (55 - 54)) | (0x0002 << (58 - 54))) &&
            sqlite3ExprIsVector(pTerm->pExpr->pRight)) {
          if (j < 16)
            mNoOmit |= (1 << j);
          if (op == (0x0002 << (57 - 54)))
            pIdxCons[j].op = (0x0002 << (56 - 54));
          if (op == (0x0002 << (55 - 54)))
            pIdxCons[j].op = (0x0002 << (58 - 54));
        }
      }

      j++;
    }
  }

  pIdxInfo->nConstraint = j;
  for (i = j = 0; i < nOrderBy; i++) {
    Expr *pExpr = pOrderBy->a[i].pExpr;
    if (sqlite3ExprIsConstant(0, pExpr))
      continue;

    pIdxOrderBy[j].iColumn = pExpr->iColumn;
    pIdxOrderBy[j].desc = pOrderBy->a[i].fg.sortFlags & 0x01;
    j++;
  }
  pIdxInfo->nOrderBy = j;

  *pmNoOmit = mNoOmit;
  return pIdxInfo;
}

__attribute__((noinline)) u32 whereIsCoveringIndex(WhereInfo *pWInfo, Index *pIdx, int iTabCur) {
  int i, rc;
  struct CoveringIndexCheck ck;
  Walker w;
  if (pWInfo->pSelect == 0) {
    return 0;
  }
  if (pIdx->bHasExpr == 0) {
    for (i = 0; i < pIdx->nColumn; i++) {
      if (pIdx->aiColumn[i] >= ((int)(sizeof(Bitmask) * 8)) - 1)
        break;
    }
    if (i >= pIdx->nColumn) {
      return 0;
    }
  }
  ck.pIdx = pIdx;
  ck.iTabCur = iTabCur;
  ck.bExpr = 0;
  ck.bUnidx = 0;
  memset(&w, 0, sizeof(w));
  w.xExprCallback = whereIsCoveringIndexWalkCallback;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pCovIdxCk = &ck;
  sqlite3WalkSelect(&w, pWInfo->pSelect);
  if (ck.bUnidx) {
    rc = 0;
  } else if (ck.bExpr) {
    rc = 0x04000000;
  } else {
    rc = 0x00000040;
  }
  return rc;
}

__attribute__((noinline)) int wherePathMatchSubqueryOB(WhereInfo *pWInfo, WhereLoop *pLoop, int iLoop, int iCur,
                                                       ExprList *pOrderBy, Bitmask *pRevMask, Bitmask *pOBSat) {
  int iOB;
  int jSub;
  u8 rev = 0;
  u8 revIdx = 0;
  Expr *pOBExpr;
  ExprList *pSubOB;

  pSubOB = pLoop->u.btree.pOrderBy;

  for (iOB = 0; ((((Bitmask)1) << (iOB)) & *pOBSat) != 0; iOB++) {
  }
  for (jSub = 0; jSub < pSubOB->nExpr && iOB < pOrderBy->nExpr; jSub++, iOB++) {
    if (pSubOB->a[jSub].u.x.iOrderByCol == 0)
      break;
    pOBExpr = pOrderBy->a[iOB].pExpr;
    if (pOBExpr->op != 168 && pOBExpr->op != 170)
      break;
    if (pOBExpr->iTable != iCur)
      break;
    if (pOBExpr->iColumn != pSubOB->a[jSub].u.x.iOrderByCol - 1)
      break;
    if ((pWInfo->wctrlFlags & 0x0040) == 0) {
      u8 sfOB = pOrderBy->a[iOB].fg.sortFlags;
      u8 sfSub = pSubOB->a[jSub].fg.sortFlags;
      if ((sfSub & 0x02) != (sfOB & 0x02)) {
        break;
      }
      revIdx = sfSub & 0x01;
      if (jSub > 0) {
        if ((rev ^ revIdx) != (sfOB & 0x01)) {
          break;
        }
      } else {
        rev = revIdx ^ (sfOB & 0x01);
        if (rev) {
          if ((pLoop->wsFlags & 0x02000000) != 0) {
            break;
          }
          *pRevMask |= (((Bitmask)1) << (iLoop));
        }
      }
    }
    *pOBSat |= (((Bitmask)1) << (iOB));
  }
  return jSub > 0;
}

i8 wherePathSatisfiesOrderBy(WhereInfo *pWInfo, ExprList *pOrderBy, WherePath *pPath, u16 wctrlFlags, u16 nLoop,
                             WhereLoop *pLast, Bitmask *pRevMask) {
  u8 revSet;
  u8 rev;
  u8 revIdx;
  u8 isOrderDistinct;
  u8 distinctColumns;
  u8 isMatch;
  u16 eqOpMask;
  u16 nKeyCol;
  u16 nColumn;
  u16 nOrderBy;
  int iLoop;
  int i, j;
  int iCur;
  int iColumn;
  WhereLoop *pLoop = 0;
  WhereTerm *pTerm;
  Expr *pOBExpr;
  CollSeq *pColl;
  Index *pIndex;
  sqlite3 *db = pWInfo->pParse->db;
  Bitmask obSat = 0;
  Bitmask obDone;
  Bitmask orderDistinctMask;
  Bitmask ready;

  if (nLoop && (((db)->dbOptFlags & (0x00000040)) != 0))
    return 0;

  nOrderBy = pOrderBy->nExpr;
  if (nOrderBy > ((int)(sizeof(Bitmask) * 8)) - 1)
    return 0;
  isOrderDistinct = 1;
  obDone = (((Bitmask)1) << (nOrderBy)) - 1;
  orderDistinctMask = 0;
  ready = 0;
  eqOpMask = 0x0002 | 0x0080 | 0x0100;
  if (wctrlFlags & (0x0800 | 0x0002 | 0x0001)) {
    eqOpMask |= 0x0001;
  }
  for (iLoop = 0; isOrderDistinct && obSat < obDone && iLoop <= nLoop; iLoop++) {
    if (iLoop > 0)
      ready |= pLoop->maskSelf;
    if (iLoop < nLoop) {
      pLoop = pPath->aLoop[iLoop];
      if (wctrlFlags & 0x0800)
        continue;
    } else {
      pLoop = pLast;
    }
    if (pLoop->wsFlags & 0x00000400) {
      if (pLoop->u.vtab.isOrdered && pWInfo->pOrderBy == pOrderBy) {
        obSat = obDone;
      } else {
        isOrderDistinct = 0;
      }
      break;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    for (i = 0; i < nOrderBy; i++) {
      if ((((Bitmask)1) << (i)) & obSat)
        continue;
      pOBExpr = sqlite3ExprSkipCollateAndLikely(pOrderBy->a[i].pExpr);
      if (pOBExpr == 0)
        continue;
      if (pOBExpr->op != 168 && pOBExpr->op != 170)
        continue;
      if (pOBExpr->iTable != iCur)
        continue;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn, ~ready, eqOpMask, 0);
      if (pTerm == 0)
        continue;
      if (pTerm->eOperator == 0x0001) {
        for (j = 0; j < pLoop->nLTerm && pTerm != pLoop->aLTerm[j]; j++) {
        }
        if (j >= pLoop->nLTerm)
          continue;
      }
      if ((pTerm->eOperator & (0x0002 | 0x0080)) != 0 && pOBExpr->iColumn >= 0) {
        Parse *pParse = pWInfo->pParse;
        CollSeq *pColl1 = sqlite3ExprNNCollSeq(pParse, pOrderBy->a[i].pExpr);
        CollSeq *pColl2 = sqlite3ExprCompareCollSeq(pParse, pTerm->pExpr);

        if (pColl2 == 0 || sqlite3StrICmp(pColl1->zName, pColl2->zName)) {
          continue;
        };
      }
      obSat |= (((Bitmask)1) << (i));
    }

    if ((pLoop->wsFlags & 0x00001000) == 0) {
      if (pLoop->wsFlags & 0x00000100) {
        if (pLoop->u.btree.pOrderBy && (((db)->dbOptFlags & (0x10000000)) == 0) &&
            wherePathMatchSubqueryOB(pWInfo, pLoop, iLoop, iCur, pOrderBy, pRevMask, &obSat)) {
          nColumn = 0;
          isOrderDistinct = 0;
        } else {
          nColumn = 1;
        }
        pIndex = 0;
        nKeyCol = 0;
      } else if ((pIndex = pLoop->u.btree.pIndex) == 0 || pIndex->bUnordered) {
        return 0;
      } else {
        nKeyCol = pIndex->nKeyCol;
        nColumn = pIndex->nColumn;

        isOrderDistinct = ((pIndex)->onError != 0) && (pLoop->wsFlags & 0x00008000) == 0;
      }

      rev = revSet = 0;
      distinctColumns = 0;
      for (j = 0; j < nColumn; j++) {
        u8 bOnce = 1;

        if (j < pLoop->u.btree.nEq && j >= pLoop->nSkip) {
          u16 eOp = pLoop->aLTerm[j]->eOperator;

          if ((eOp & eqOpMask) != 0) {
            if (eOp & (0x0100 | 0x0080)) {
              isOrderDistinct = 0;
            }
            continue;
          } else if ((eOp & 0x0001)) {
            Expr *pX = pLoop->aLTerm[j]->pExpr;
            for (i = j + 1; i < pLoop->u.btree.nEq; i++) {
              if (pLoop->aLTerm[i]->pExpr == pX) {
                bOnce = 0;
                break;
              }
            }
          }
        }

        if (pIndex) {
          iColumn = pIndex->aiColumn[j];
          revIdx = pIndex->aSortOrder[j] & 0x01;
          if (iColumn == pIndex->pTable->iPKey)
            iColumn = (-1);
        } else {
          iColumn = (-1);
          revIdx = 0;
        }

        if (isOrderDistinct) {
          if (iColumn >= 0 && j >= pLoop->u.btree.nEq && pIndex->pTable->aCol[iColumn].notNull == 0) {
            isOrderDistinct = 0;
          }
          if (iColumn == (-2)) {
            isOrderDistinct = 0;
          }
        }

        isMatch = 0;
        for (i = 0; bOnce && i < nOrderBy; i++) {
          if ((((Bitmask)1) << (i)) & obSat)
            continue;
          pOBExpr = sqlite3ExprSkipCollateAndLikely(pOrderBy->a[i].pExpr);
          if (pOBExpr == 0)
            continue;
          if ((wctrlFlags & (0x0040 | 0x0080)) == 0)
            bOnce = 0;
          if (iColumn >= (-1)) {
            if (pOBExpr->op != 168 && pOBExpr->op != 170)
              continue;
            if (pOBExpr->iTable != iCur)
              continue;
            if (pOBExpr->iColumn != iColumn)
              continue;
          } else {
            Expr *pIxExpr = pIndex->aColExpr->a[j].pExpr;
            if (sqlite3ExprCompareSkip(pOBExpr, pIxExpr, iCur)) {
              continue;
            }
          }
          if (iColumn != (-1)) {
            pColl = sqlite3ExprNNCollSeq(pWInfo->pParse, pOrderBy->a[i].pExpr);
            if (sqlite3StrICmp(pColl->zName, pIndex->azColl[j]) != 0)
              continue;
          }
          if (wctrlFlags & 0x0080) {
            pLoop->u.btree.nDistinctCol = j + 1;
          }
          isMatch = 1;
          break;
        }
        if (isMatch && (wctrlFlags & 0x0040) == 0) {
          if (revSet) {
            if ((rev ^ revIdx) != (pOrderBy->a[i].fg.sortFlags & 0x01)) {
              isMatch = 0;
            }
          } else {
            rev = revIdx ^ (pOrderBy->a[i].fg.sortFlags & 0x01);
            if (rev)
              *pRevMask |= (((Bitmask)1) << (iLoop));
            revSet = 1;
          }
        }
        if (isMatch && (pOrderBy->a[i].fg.sortFlags & 0x02)) {
          if (j == pLoop->u.btree.nEq) {
            pLoop->wsFlags |= 0x00080000;
          } else {
            isMatch = 0;
          }
        }
        if (isMatch) {
          if (iColumn == (-1)) {
            distinctColumns = 1;
          }
          obSat |= (((Bitmask)1) << (i));
        } else {
          if (j == 0 || j < nKeyCol) {
            isOrderDistinct = 0;
          }
          break;
        }
      }
      if (distinctColumns) {
        isOrderDistinct = 1;
      }
    }

    if (isOrderDistinct) {
      orderDistinctMask |= pLoop->maskSelf;
      for (i = 0; i < nOrderBy; i++) {
        Expr *p;
        Bitmask mTerm;
        if ((((Bitmask)1) << (i)) & obSat)
          continue;
        p = pOrderBy->a[i].pExpr;
        mTerm = sqlite3WhereExprUsage(&pWInfo->sMaskSet, p);
        if (mTerm == 0 && !sqlite3ExprIsConstant(0, p))
          continue;
        if ((mTerm & ~orderDistinctMask) == 0) {
          obSat |= (((Bitmask)1) << (i));
        }
      }
    }
  }
  if (obSat == obDone)
    return (i8)nOrderBy;
  if (!isOrderDistinct) {
    for (i = nOrderBy - 1; i > 0; i--) {
      Bitmask m = (i < ((int)(sizeof(Bitmask) * 8))) ? (((Bitmask)1) << (i)) - 1 : 0;
      if ((obSat & m) == m)
        return i;
    }
    return 0;
  }
  return -1;
}

int sqlite3WhereIsSorted(WhereInfo *pWInfo) {
  return pWInfo->sorted;
}

LogEst whereSortingCost(WhereInfo *pWInfo, LogEst nRow, int nOrderBy, int nSorted) {
  LogEst rSortCost, nCol;

  nCol = sqlite3LogEst((pWInfo->pSelect->pEList->nExpr + 59) / 30);
  rSortCost = nRow + nCol;
  if (nSorted > 0) {
    rSortCost += sqlite3LogEst((nOrderBy - nSorted) * 100 / nOrderBy) - 66;
  }

  if ((pWInfo->wctrlFlags & 0x4000) != 0) {
    rSortCost += 10;
    if (nSorted != 0) {
      rSortCost += 6;
    }
    if (pWInfo->iLimit < nRow) {
      nRow = pWInfo->iLimit;
    }
  } else if ((pWInfo->wctrlFlags & 0x0100)) {
    if (nRow > 10) {
      nRow -= 10;
    }
  }
  rSortCost += estLog(nRow);
  return rSortCost;
}

int computeMxChoice(WhereInfo *pWInfo) {
  int nLoop = pWInfo->nLevel;
  WhereLoop *pWLoop;

  if (nLoop >= 4 && !pWInfo->bStarDone && (((pWInfo->pParse->db)->dbOptFlags & (0x20000000)) == 0)) {
    SrcItem *aFromTabs;
    int iFromIdx;
    Bitmask m;
    Bitmask mSelfJoin = 0;
    WhereLoop *pStart;

    pWInfo->bStarDone = 1;

    aFromTabs = pWInfo->pTabList->a;
    pStart = pWInfo->pLoops;
    for (iFromIdx = 0, m = 1; iFromIdx < nLoop; iFromIdx++, m <<= 1) {
      int nDep = 0;
      LogEst mxRun;
      Bitmask mSeen = 0;
      SrcItem *pFactTab;

      pFactTab = aFromTabs + iFromIdx;
      if ((pFactTab->fg.jointype & (0x20 | 0x02)) != 0) {
        if (iFromIdx + 3 > nLoop) {
          break;
        }
        while (pStart && pStart->iTab <= iFromIdx) {
          pStart = pStart->pNextLoop;
        }
      }
      for (pWLoop = pStart; pWLoop; pWLoop = pWLoop->pNextLoop) {
        if ((aFromTabs[pWLoop->iTab].fg.jointype & (0x20 | 0x02)) != 0) {
          break;
        }
        if ((pWLoop->prereq & m) != 0 && (pWLoop->maskSelf & mSeen) == 0 && (pWLoop->maskSelf & mSelfJoin) == 0) {
          if (aFromTabs[pWLoop->iTab].pSTab == pFactTab->pSTab) {
            mSelfJoin |= m;
          } else {
            nDep++;
            mSeen |= pWLoop->maskSelf;
          }
        }
      }
      if (nDep <= 2) {
        continue;
      }

      pWInfo->bStarUsed = 1;

      mxRun = (-32768);
      for (pWLoop = pStart; pWLoop; pWLoop = pWLoop->pNextLoop) {
        if (pWLoop->iTab < iFromIdx)
          continue;
        if (pWLoop->iTab > iFromIdx)
          break;
        if (pWLoop->rRun > mxRun)
          mxRun = pWLoop->rRun;
      }
      if ((mxRun < (32767)))
        mxRun++;

      for (pWLoop = pStart; pWLoop; pWLoop = pWLoop->pNextLoop) {
        if ((pWLoop->maskSelf & mSeen) == 0)
          continue;
        if (pWLoop->nLTerm)
          continue;
        if (pWLoop->rRun < mxRun) {
          pWLoop->rRun = mxRun;
        }
      }
    }
  }
  return pWInfo->bStarUsed ? 18 : 12;
}

int wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst) {
  int mxChoice;
  int nLoop;
  Parse *pParse;
  int iLoop;
  int ii, jj;
  int mxI = 0;
  int nOrderBy;
  LogEst mxCost = 0;
  LogEst mxUnsort = 0;
  int nTo, nFrom;
  WherePath *aFrom;
  WherePath *aTo;
  WherePath *pFrom;
  WherePath *pTo;
  WhereLoop *pWLoop;
  WhereLoop **pX;
  LogEst *aSortCost = 0;
  char *pSpace;
  int nSpace;

  pParse = pWInfo->pParse;
  nLoop = pWInfo->nLevel;

  if (nLoop <= 1) {
    mxChoice = 1;
  } else if (nLoop == 2) {
    mxChoice = 5;
  } else if (pParse->nErr) {
    mxChoice = 1;
  } else {
    mxChoice = computeMxChoice(pWInfo);
  }

  if (pWInfo->pOrderBy == 0 || nRowEst == 0) {
    nOrderBy = 0;
  } else {
    nOrderBy = pWInfo->pOrderBy->nExpr;
  }

  nSpace = (sizeof(WherePath) + sizeof(WhereLoop *) * nLoop) * mxChoice * 2;
  nSpace += sizeof(LogEst) * nOrderBy;
  pSpace = sqlite3DbMallocRawNN(pParse->db, nSpace);
  if (pSpace == 0)
    return 7;
  aTo = (WherePath *)pSpace;
  aFrom = aTo + mxChoice;
  memset(aFrom, 0, sizeof(aFrom[0]));
  pX = (WhereLoop **)(aFrom + mxChoice);
  for (ii = mxChoice * 2, pFrom = aTo; ii > 0; ii--, pFrom++, pX += nLoop) {
    pFrom->aLoop = pX;
  }
  if (nOrderBy) {
    aSortCost = (LogEst *)pX;
    memset(aSortCost, 0, sizeof(LogEst) * nOrderBy);
  }

  aFrom[0].nRow = ((pParse->nQueryLoop) < (48) ? (pParse->nQueryLoop) : (48));

  nFrom = 1;

  if (nOrderBy) {
    aFrom[0].isOrdered = nLoop > 0 ? -1 : nOrderBy;
  }

  for (iLoop = 0; iLoop < nLoop; iLoop++) {
    nTo = 0;
    for (ii = 0, pFrom = aFrom; ii < nFrom; ii++, pFrom++) {
      for (pWLoop = pWInfo->pLoops; pWLoop; pWLoop = pWLoop->pNextLoop) {
        LogEst nOut;
        LogEst rCost;
        LogEst rUnsort;
        i8 isOrdered;
        Bitmask maskNew;
        Bitmask revMask;

        if ((pWLoop->prereq & ~pFrom->maskLoop) != 0)
          continue;
        if ((pWLoop->maskSelf & pFrom->maskLoop) != 0)
          continue;
        if ((pWLoop->wsFlags & 0x00004000) != 0 && pFrom->nRow < 3) {
          continue;
        }

        rUnsort = pWLoop->rRun + pFrom->nRow;
        if (pWLoop->rSetup) {
          rUnsort = sqlite3LogEstAdd(pWLoop->rSetup, rUnsort);
        }
        rUnsort = sqlite3LogEstAdd(rUnsort, pFrom->rUnsort);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        isOrdered = pFrom->isOrdered;
        if (isOrdered < 0) {
          revMask = 0;
          isOrdered =
              wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags, iLoop, pWLoop, &revMask);
        } else {
          revMask = pFrom->revLoop;
        }
        if (isOrdered >= 0 && isOrdered < nOrderBy) {
          if (aSortCost[(int)(isOrdered)] == 0) {
            aSortCost[(int)(isOrdered)] = whereSortingCost(pWInfo, nRowEst, nOrderBy, isOrdered);
          }

          rCost = sqlite3LogEstAdd(rUnsort, aSortCost[(int)(isOrdered)]) + 3;

        } else {
          rCost = rUnsort;
          rUnsort -= 2;
        }

        for (jj = 0, pTo = aTo; jj < nTo; jj++, pTo++) {
          if (pTo->maskLoop == maskNew && (((pTo->isOrdered ^ isOrdered) & 0x80) == 0 || iLoop == nLoop - 1)) {
            break;
          }
        }
        if (jj >= nTo) {
          if (nTo >= mxChoice && (rCost > mxCost || (rCost == mxCost && rUnsort >= mxUnsort))) {
            continue;
          }

          if (nTo < mxChoice) {
            jj = nTo++;
          } else {
            jj = mxI;
          }
          pTo = &aTo[jj];

        } else {
          if ((pTo->rCost < rCost) || (pTo->rCost == rCost && pTo->nRow < nOut) ||
              (pTo->rCost == rCost && pTo->nRow == nOut && pTo->rUnsort < rUnsort) ||
              (pTo->rCost == rCost && pTo->nRow == nOut && pTo->rUnsort == rUnsort &&
               whereLoopIsNoBetter(pWLoop, pTo->aLoop[iLoop]))) {
            continue;
          };
        }

        pTo->maskLoop = pFrom->maskLoop | pWLoop->maskSelf;
        pTo->revLoop = revMask;
        pTo->nRow = nOut;
        pTo->rCost = rCost;
        pTo->rUnsort = rUnsort;
        pTo->isOrdered = isOrdered;
        memcpy(pTo->aLoop, pFrom->aLoop, sizeof(WhereLoop *) * iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        if (nTo >= mxChoice) {
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsort = aTo[0].nRow;
          for (jj = 1, pTo = &aTo[1]; jj < mxChoice; jj++, pTo++) {
            if (pTo->rCost > mxCost || (pTo->rCost == mxCost && pTo->rUnsort > mxUnsort)) {
              mxCost = pTo->rCost;
              mxUnsort = pTo->rUnsort;
              mxI = jj;
            }
          }
        }
      }
    }

    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  if (nFrom == 0) {
    sqlite3ErrorMsg(pParse, "no query solution");
    sqlite3DbFreeNN(pParse->db, pSpace);
    return SQLITE_ERROR;
  }

  pFrom = aFrom;

  for (iLoop = 0; iLoop < nLoop; iLoop++) {
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  if ((pWInfo->wctrlFlags & 0x0100) != 0 && (pWInfo->wctrlFlags & 0x0080) == 0 && pWInfo->eDistinct == 0 && nRowEst) {
    Bitmask notUsed;
    int rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pResultSet, pFrom, 0x0080, nLoop - 1, pFrom->aLoop[nLoop - 1],
                                       &notUsed);
    if (rc == pWInfo->pResultSet->nExpr) {
      pWInfo->eDistinct = 2;
    }
  }
  pWInfo->bOrderedInnerLoop = 0;
  if (pWInfo->pOrderBy) {
    pWInfo->nOBSat = pFrom->isOrdered;
    if (pWInfo->wctrlFlags & 0x0080) {
      if (pFrom->isOrdered == pWInfo->pOrderBy->nExpr) {
        pWInfo->eDistinct = 2;
      }

    } else {
      pWInfo->revMask = pFrom->revLoop;
      if (pWInfo->nOBSat <= 0) {
        pWInfo->nOBSat = 0;
        if (nLoop > 0) {
          u32 wsFlags = pFrom->aLoop[nLoop - 1]->wsFlags;
          if ((wsFlags & 0x00001000) == 0 && (wsFlags & (0x00000100 | 0x00000004)) != (0x00000100 | 0x00000004)) {
            Bitmask m = 0;
            int rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom, 0x0800, nLoop - 1,
                                               pFrom->aLoop[nLoop - 1], &m);
            if (rc == pWInfo->pOrderBy->nExpr) {
              pWInfo->bOrderedInnerLoop = 1;
              pWInfo->revMask = m;
            }
          }
        }
      } else if (nLoop && pWInfo->nOBSat == 1 && (pWInfo->wctrlFlags & (0x0001 | 0x0002)) != 0) {
        pWInfo->bOrderedInnerLoop = 1;
      }
    }
    if ((pWInfo->wctrlFlags & 0x0200) && pWInfo->nOBSat == pWInfo->pOrderBy->nExpr && nLoop > 0) {
      Bitmask revMask = 0;
      int nOrder =
          wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, pFrom, 0, nLoop - 1, pFrom->aLoop[nLoop - 1], &revMask);

      if (nOrder == pWInfo->pOrderBy->nExpr) {
        pWInfo->sorted = 1;
        pWInfo->revMask = revMask;
      }
    }
  }

  pWInfo->nRowOut = pFrom->nRow;

  sqlite3DbFreeNN(pParse->db, pSpace);
  return SQLITE_OK;
}

__attribute__((noinline)) void whereInterstageHeuristic(WhereInfo *pWInfo) {
  int i;

  for (i = 0; i < pWInfo->nLevel; i++) {
    WhereLoop *p = pWInfo->a[i].pWLoop;
    if (p == 0)
      break;
    if ((p->wsFlags & 0x00000400) != 0) {
      break;
    }
    if ((p->wsFlags & (0x00000001 | 0x00000008 | 0x00000004)) != 0) {
      u8 iTab = p->iTab;
      WhereLoop *pLoop;
      for (pLoop = pWInfo->pLoops; pLoop; pLoop = pLoop->pNextLoop) {
        if (pLoop->iTab != iTab)
          continue;
        if ((pLoop->wsFlags & (0x0000000f | 0x00004000)) != 0) {
          continue;
        }

        pLoop->prereq = ((Bitmask)-1);
      }
    } else {
      break;
    }
  }
}

__attribute__((noinline)) Bitmask whereOmitNoopJoin(WhereInfo *pWInfo, Bitmask notReady) {
  int i;
  Bitmask tabUsed;
  int hasRightJoin;

  tabUsed = sqlite3WhereExprListUsage(&pWInfo->sMaskSet, pWInfo->pResultSet);
  if (pWInfo->pOrderBy) {
    tabUsed |= sqlite3WhereExprListUsage(&pWInfo->sMaskSet, pWInfo->pOrderBy);
  }
  hasRightJoin = (pWInfo->pTabList->a[0].fg.jointype & 0x40) != 0;
  for (i = pWInfo->nLevel - 1; i >= 1; i--) {
    WhereTerm *pTerm, *pEnd;
    SrcItem *pItem;
    WhereLoop *pLoop;
    Bitmask m1;
    pLoop = pWInfo->a[i].pWLoop;
    pItem = &pWInfo->pTabList->a[pLoop->iTab];
    if ((pItem->fg.jointype & (0x08 | 0x10)) != 0x08)
      continue;
    if ((pWInfo->wctrlFlags & 0x0100) == 0 && (pLoop->wsFlags & 0x00001000) == 0) {
      continue;
    }
    if ((tabUsed & pLoop->maskSelf) != 0)
      continue;
    pEnd = pWInfo->sWC.a + pWInfo->sWC.nTerm;
    for (pTerm = pWInfo->sWC.a; pTerm < pEnd; pTerm++) {
      if ((pTerm->prereqAll & pLoop->maskSelf) != 0) {
        if (!(((pTerm->pExpr)->flags & (u32)(0x000001)) != 0) || pTerm->pExpr->w.iJoin != pItem->iCursor) {
          break;
        }
      }
      if (hasRightJoin && (((pTerm->pExpr)->flags & (u32)(0x000002)) != 0) &&
          (pTerm->pExpr->w.iJoin == pItem->iCursor)) {
        break;
      }
    }
    if (pTerm < pEnd)
      continue;
    m1 = (((Bitmask)1) << (i)) - 1;
    pWInfo->revMask = (m1 & pWInfo->revMask) | ((pWInfo->revMask >> 1) & ~m1);
    notReady &= ~pLoop->maskSelf;
    for (pTerm = pWInfo->sWC.a; pTerm < pEnd; pTerm++) {
      if ((pTerm->prereqAll & pLoop->maskSelf) != 0) {
        pTerm->wtFlags |= 0x0004;
        pTerm->prereqAll = 0;
      }
    }
    if (i != pWInfo->nLevel - 1) {
      int nByte = (pWInfo->nLevel - 1 - i) * sizeof(WhereLevel);
      memmove(&pWInfo->a[i], &pWInfo->a[i + 1], nByte);
    }
    pWInfo->nLevel--;
  }
  return notReady;
}

__attribute__((noinline)) void whereCheckIfBloomFilterIsUseful(const WhereInfo *pWInfo) {
  int i;
  LogEst nSearch = 0;

  for (i = 0; i < pWInfo->nLevel; i++) {
    WhereLoop *pLoop = pWInfo->a[i].pWLoop;
    const unsigned int reqFlags = (0x00800000 | 0x00000001);
    SrcItem *pItem = &pWInfo->pTabList->a[pLoop->iTab];
    Table *pTab = pItem->pSTab;
    if ((pTab->tabFlags & 0x00000010) == 0)
      break;
    pTab->tabFlags |= 0x00000100;
    if (i >= 1 && (pLoop->wsFlags & reqFlags) == reqFlags && ((pLoop->wsFlags & (0x00000100 | 0x00000200)) != 0)) {
      if (nSearch > pTab->nRowLogEst) {
        pLoop->wsFlags |= 0x00400000;
        pLoop->wsFlags &= ~0x00000040;
      }
    }
    nSearch += pLoop->nOut;
  }
}

__attribute__((noinline)) void whereReverseScanOrder(WhereInfo *pWInfo) {
  int ii;
  for (ii = 0; ii < pWInfo->pTabList->nSrc; ii++) {
    SrcItem *pItem = &pWInfo->pTabList->a[ii];
    if (!pItem->fg.isCte || pItem->u2.pCteUse->eM10d != 0 || (pItem->fg.isSubquery == 0) ||
        pItem->u4.pSubq->pSelect->pOrderBy == 0) {
      pWInfo->revMask |= (((Bitmask)1) << (ii));
    }
  }
}

void sqlite3WhereEnd(WhereInfo *pWInfo) {
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  int i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;
  int iEnd = sqlite3VdbeCurrentAddr(v);
  int nRJ = 0;

  int addrSeek = 0;

  for (i = pWInfo->nLevel - 1; i >= 0; i--) {
    int addr;
    pLevel = &pWInfo->a[i];
    if (pLevel->pRJ) {
      WhereRightJoin *pRJ = pLevel->pRJ;
      sqlite3VdbeResolveLabel(v, pLevel->addrCont);

      pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
      pRJ->endSubrtn = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, 69, pRJ->regReturn, pRJ->addrSubrtn, 1);
      nRJ++;
    }
    pLoop = pLevel->pWLoop;
    if (pLevel->op != 189) {
      Index *pIdx;
      int n;
      if (pWInfo->eDistinct == 2 && i == pWInfo->nLevel - 1 && (pLoop->wsFlags & 0x00000200) != 0 &&
          (pIdx = pLoop->u.btree.pIndex)->hasStat1 && (n = pLoop->u.btree.nDistinctCol) > 0 &&
          pIdx->aiRowLogEst[n] >= 36) {
        int r1 = pParse->nMem + 1;
        int j, op;
        int addrIfNull = 0;
        if (pLevel->iLeftJoin) {
          addrIfNull = sqlite3VdbeAddOp2(v, 20, pLevel->iIdxCur, r1);
        }
        for (j = 0; j < n; j++) {
          sqlite3VdbeAddOp3(v, 96, pLevel->iIdxCur, j, r1 + j);
        }
        pParse->nMem += n + 1;
        op = pLevel->op == 39 ? 21 : 24;
        addrSeek = sqlite3VdbeAddOp4Int(v, op, pLevel->iIdxCur, 0, r1, n);
        sqlite3VdbeAddOp2(v, 9, 1, pLevel->p2);
        if (pLevel->iLeftJoin) {
          sqlite3VdbeJumpHere(v, addrIfNull);
        }
      }
    }
    if (pTabList->a[pLevel->iFrom].fg.fromExists) {
      sqlite3VdbeAddOp2(v, 9, 0, pLevel->addrBrk);
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    if (pLevel->op != 189) {
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      if (pLevel->regBignull) {
        sqlite3VdbeResolveLabel(v, pLevel->addrBignull);
        sqlite3VdbeAddOp2(v, 63, pLevel->regBignull, pLevel->p2 - 1);
      }

      if (addrSeek) {
        sqlite3VdbeJumpHere(v, addrSeek);
        addrSeek = 0;
      }
    }
    if ((pLoop->wsFlags & 0x00000800) != 0 && pLevel->u.in.nIn > 0) {
      struct InLoop *pIn;
      int j;
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
      for (j = pLevel->u.in.nIn, pIn = &pLevel->u.in.aInLoop[j - 1]; j > 0; j--, pIn--) {
        sqlite3VdbeJumpHere(v, pIn->addrInTop + 1);
        if (pIn->eEndLoopOp != 189) {
          if (pIn->nPrefix) {
            int bEarlyOut = (pLoop->wsFlags & 0x00000400) == 0 && (pLoop->wsFlags & 0x00040000) != 0;
            if (pLevel->iLeftJoin) {
              sqlite3VdbeAddOp2(v, 25, pIn->iCur, sqlite3VdbeCurrentAddr(v) + 2 + bEarlyOut);
            }
            if (bEarlyOut) {
              sqlite3VdbeAddOp4Int(v, 26, pLevel->iIdxCur, sqlite3VdbeCurrentAddr(v) + 2, pIn->iBase, pIn->nPrefix);

              sqlite3VdbeJumpHere(v, pIn->addrInTop + 1);
            }
          }
          sqlite3VdbeAddOp2(v, pIn->eEndLoopOp, pIn->iCur, pIn->addrInTop);
        }
        sqlite3VdbeJumpHere(v, pIn->addrInTop - 1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    if (pLevel->pRJ) {
      sqlite3VdbeAddOp3(v, 69, pLevel->pRJ->regReturn, 0, 1);
    }
    if (pLevel->addrSkip) {
      sqlite3VdbeGoto(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip - 2);
    }

    if (pLevel->addrLikeRep) {
      sqlite3VdbeAddOp2(v, 63, (int)(pLevel->iLikeRepCntr >> 1), pLevel->addrLikeRep);
    }

    if (pLevel->iLeftJoin) {
      int ws = pLoop->wsFlags;
      addr = sqlite3VdbeAddOp1(v, 61, pLevel->iLeftJoin);

      if ((ws & 0x00000040) == 0) {
        SrcItem *pSrc = &pTabList->a[pLevel->iFrom];

        if (pSrc->fg.viaCoroutine) {
          int m, n;

          n = pSrc->u4.pSubq->regResult;

          m = pSrc->pSTab->nCol;
          sqlite3VdbeAddOp3(v, 77, 0, n, n + m - 1);
        }
        sqlite3VdbeAddOp1(v, 138, pLevel->iTabCur);
      }
      if ((ws & 0x00000200) || ((ws & 0x00002000) && pLevel->u.pCoveringIdx)) {
        if (ws & 0x00002000) {
          Index *pIx = pLevel->u.pCoveringIdx;
          int iDb = sqlite3SchemaToIndex(db, pIx->pSchema);
          sqlite3VdbeAddOp3(v, 113, pLevel->iIdxCur, pIx->tnum, iDb);
          sqlite3VdbeSetP4KeyInfo(pParse, pIx);
        }
        sqlite3VdbeAddOp1(v, 138, pLevel->iIdxCur);
      }
      if (pLevel->op == 69) {
        sqlite3VdbeAddOp2(v, 10, pLevel->p1, pLevel->addrFirst);
      } else {
        sqlite3VdbeGoto(v, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
  }

  for (i = 0, pLevel = pWInfo->a; i < pWInfo->nLevel; i++, pLevel++) {
    int k, last;
    VdbeOp *pOp, *pLastOp;
    Index *pIdx = 0;
    SrcItem *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pSTab;

    pLoop = pLevel->pWLoop;

    if (pLevel->pRJ) {
      sqlite3WhereRightJoinLoop(pWInfo, i, pLevel);
      continue;
    }

    if (pTabItem->fg.viaCoroutine) {
      translateColumnToCopy(pParse, pLevel->addrBody, pLevel->iTabCur, pTabItem->u4.pSubq->regResult, 0);
      continue;
    }

    if (pLoop->wsFlags & (0x00000200 | 0x00000040)) {
      pIdx = pLoop->u.btree.pIndex;
    } else if (pLoop->wsFlags & 0x00002000) {
      pIdx = pLevel->u.pCoveringIdx;
    }
    if (pIdx && !db->mallocFailed) {
      if (pWInfo->eOnePass == 0 || !(((pIdx->pTable)->tabFlags & 0x00000080) == 0)) {
        last = iEnd;
      } else {
        last = pWInfo->iEndWhere;
      }
      if (pIdx->bHasExpr) {
        IndexedExpr *p = pParse->pIdxEpr;
        while (p) {
          if (p->iIdxCur == pLevel->iIdxCur) {
            p->iDataCur = -1;
            p->iIdxCur = -1;
          }
          p = p->pIENext;
        }
      }
      k = pLevel->addrBody + 1;

      pOp = sqlite3VdbeGetOp(v, k);
      pLastOp = pOp + (last - k);

      do {
        if (pOp->p1 != pLevel->iTabCur) {
        } else if (pOp->opcode == 96) {
          int x = pOp->p2;

          if (!(((pTab)->tabFlags & 0x00000080) == 0)) {
            Index *pPk = sqlite3PrimaryKeyIndex(pTab);
            x = pPk->aiColumn[x];

          } else {
            x = sqlite3StorageColumnToTable(pTab, x);
          }
          x = sqlite3TableColumnToIndex(pIdx, x);
          if (x >= 0) {
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
          } else if (pLoop->wsFlags & (0x00000040 | 0x04000000)) {
            if (pLoop->wsFlags & 0x00000040) {
              sqlite3ErrorMsg(pParse, "internal query planner error");
              pParse->rc = SQLITE_INTERNAL;
            } else {
              pLoop->wsFlags &= ~0x04000000;
              sqlite3WhereAddExplainText(pParse, pLevel->addrBody - 1, pTabList, pLevel, pWInfo->wctrlFlags);
            }
          }
        } else if (pOp->opcode == 137) {
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = 144;
        } else if (pOp->opcode == 20) {
          pOp->p1 = pLevel->iIdxCur;
        }

      } while ((++pOp) < pLastOp);
    }
  }

  sqlite3VdbeResolveLabel(v, pWInfo->iBreak);

  pParse->nQueryLoop = pWInfo->savedNQueryLoop;
  whereInfoFree(db, pWInfo);
  pParse->withinRJSubrtn -= nRJ;
  return;
}