#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/SrcList.h"

#include "sqlite/Column.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Index.h"
#include "sqlite/Parse.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/Subquery.h"
#include "sqlite/Table.h"
#include "sqlite/Token.h"
#include "sqlite/WhereAndInfo.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereMaskSet.h"
#include "sqlite/WhereOrInfo.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/Window.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int allowedOp(int op);
static u16 operatorMask(int op);

static int allowedOp(int op) {

  if (op > 58)
    return 0;
  if (op >= 54)
    return 1;
  return op == 50 || op == 51 || op == 45;
}

static u16 operatorMask(int op) {
  u16 c;

  if (op >= 54) {


    c = (u16)(0x0002 << (op - 54));
  } else if (op == 50) {
    c = 0x0001;
  } else if (op == 51) {
    c = 0x0100;
  } else {


    c = 0x0080;
  }

  return c;
}


int tableAndColumnIndex(SrcList *pSrc, int iStart, int iEnd, const char *zCol, int *piTab, int *piCol, int bIgnoreHidden) {
  int i;
  int iCol;

  for (i = iStart; i <= iEnd; i++) {
    iCol = sqlite3ColumnIndex(pSrc->a[i].pSTab, zCol);
    if (iCol >= 0 && (bIgnoreHidden == 0 || (((&pSrc->a[i].pSTab->aCol[iCol])->colFlags & 0x0002) != 0) == 0)) {
      if (piTab) {
        sqlite3SrcItemColumnUsed(&pSrc->a[i], iCol);
        *piTab = i;
        *piCol = iCol;
      }
      return 1;
    }
  }
  return 0;
}

SrcItem *isSelfJoinView(SrcList *pTabList, SrcItem *pThis, int iFirst, int iEnd) {
  SrcItem *pItem;
  Select *pSel;

  pSel = pThis->u4.pSubq->pSelect;

  if (pSel->selFlags & 0x1000000)
    return 0;
  while (iFirst < iEnd) {
    Select *pS1;
    pItem = &pTabList->a[iFirst++];
    if (!pItem->fg.isSubquery)
      continue;
    if (pItem->fg.viaCoroutine)
      continue;
    if (pItem->zName == 0)
      continue;




    if (pItem->pSTab->pSchema != pThis->pSTab->pSchema)
      continue;
    if (sqlite3_stricmp(pItem->zName, pThis->zName) != 0)
      continue;
    pS1 = pItem->u4.pSubq->pSelect;
    if (pItem->pSTab->pSchema == 0 && pSel->selId != pS1->selId) {

      continue;
    }
    if (pS1->selFlags & 0x1000000) {

      continue;
    }
    return pItem;
  }
  return 0;
}

void whereCombineDisjuncts(SrcList *pSrc, WhereClause *pWC, WhereTerm *pOne, WhereTerm *pTwo) {
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;
  Expr *pNew;
  int op;
  int idxNew;
  Expr *pA, *pB;

  if ((pOne->wtFlags | pTwo->wtFlags) & 0x0080)
    return;
  if ((pOne->eOperator & (0x0002 | (0x0002 << (57 - 54)) | (0x0002 << (56 - 54)) | (0x0002 << (55 - 54)) | (0x0002 << (58 - 54)))) == 0)
    return;
  if ((pTwo->eOperator & (0x0002 | (0x0002 << (57 - 54)) | (0x0002 << (56 - 54)) | (0x0002 << (55 - 54)) | (0x0002 << (58 - 54)))) == 0)
    return;
  if ((eOp & (0x0002 | (0x0002 << (57 - 54)) | (0x0002 << (56 - 54)))) != eOp && (eOp & (0x0002 | (0x0002 << (55 - 54)) | (0x0002 << (58 - 54)))) != eOp)
    return;
  pA = pOne->pExpr;
  pB = pTwo->pExpr;

  if (sqlite3ExprCompare(0, pA->pLeft, pB->pLeft, -1))
    return;
  if (sqlite3ExprCompare(0, pA->pRight, pB->pRight, -1))
    return;
  if ((((pA)->flags & (u32)(0x000400)) != 0) != (((pB)->flags & (u32)(0x000400)) != 0)) {
    return;
  }

  if ((eOp & (eOp - 1)) != 0) {
    if (eOp & ((0x0002 << (57 - 54)) | (0x0002 << (56 - 54)))) {
      eOp = (0x0002 << (56 - 54));
    } else {


      eOp = (0x0002 << (58 - 54));
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3ExprDup(db, pA, 0);
  if (pNew == 0)
    return;
  for (op = 54; eOp != (0x0002 << (op - 54)); op++) {


  }
  pNew->op = op;
  idxNew = whereClauseInsert(pWC, pNew, 0x0002 | 0x0001);
  exprAnalyze(pSrc, pWC, idxNew);
}

void exprAnalyzeOrTerm(SrcList *pSrc, WhereClause *pWC, int idxTerm) {
  WhereInfo *pWInfo = pWC->pWInfo;
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  WhereTerm *pTerm = &pWC->a[idxTerm];
  Expr *pExpr = pTerm->pExpr;
  int i;
  WhereClause *pOrWc;
  WhereTerm *pOrTerm;
  WhereOrInfo *pOrInfo;
  Bitmask chngToIN;
  Bitmask indexable;

  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, sizeof(*pOrInfo));
  if (pOrInfo == 0)
    return;
  pTerm->wtFlags |= 0x0010;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, sizeof(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, pExpr, 43);
  sqlite3WhereExprAnalyze(pSrc, pOrWc);
  if (db->mallocFailed)
    return;

  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  for (i = pOrWc->nTerm - 1, pOrTerm = pOrWc->a; i >= 0 && indexable; i--, pOrTerm++) {
    if ((pOrTerm->eOperator & 0x01ff) == 0) {
      WhereAndInfo *pAndInfo;


      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, sizeof(*pAndInfo));
      if (pAndInfo) {
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        int j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= 0x0020;
        pOrTerm->eOperator = 0x0400;
        pOrTerm->leftCursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, sizeof(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->pExpr, 44);
        sqlite3WhereExprAnalyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        if (!db->mallocFailed) {
          for (j = 0, pAndTerm = pAndWC->a; j < pAndWC->nTerm; j++, pAndTerm++) {


            if (allowedOp(pAndTerm->pExpr->op) || pAndTerm->eOperator == 0x0040) {
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->leftCursor);
            }
          }
        }
        indexable &= b;
      }
    } else if (pOrTerm->wtFlags & 0x0008) {

    } else {
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->leftCursor);
      if (pOrTerm->wtFlags & 0x0002) {
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->leftCursor);
      }
      indexable &= b;
      if ((pOrTerm->eOperator & 0x0002) == 0) {
        chngToIN = 0;
      } else {
        chngToIN &= b;
      }
    }
  }

  pOrInfo->indexable = indexable;
  pTerm->eOperator = 0x0200;
  pTerm->leftCursor = -1;
  if (indexable) {
    pWC->hasOr = 1;
  }

  if (indexable && pOrWc->nTerm == 2) {
    int iOne = 0;
    WhereTerm *pOne;
    while ((pOne = whereNthSubterm(&pOrWc->a[0], iOne++)) != 0) {
      int iTwo = 0;
      WhereTerm *pTwo;
      while ((pTwo = whereNthSubterm(&pOrWc->a[1], iTwo++)) != 0) {
        whereCombineDisjuncts(pSrc, pWC, pOne, pTwo);
      }
    }
  }

  if (chngToIN) {
    int okToChngToIN = 0;
    int iColumn = -1;
    int iCursor = -1;
    int j = 0;

    for (j = 0; j < 2 && !okToChngToIN; j++) {
      Expr *pLeft = 0;
      pOrTerm = pOrWc->a;
      for (i = pOrWc->nTerm - 1; i >= 0; i--, pOrTerm++) {


        pOrTerm->wtFlags &= ~0x0040;
        if (pOrTerm->leftCursor == iCursor) {


          continue;
        }
        if ((chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->leftCursor)) == 0) {

          ;
          ;


          continue;
        }


        iColumn = pOrTerm->u.x.leftColumn;
        iCursor = pOrTerm->leftCursor;
        pLeft = pOrTerm->pExpr->pLeft;
        break;
      }
      if (i < 0) {






        break;
      };

      okToChngToIN = 1;
      for (; i >= 0 && okToChngToIN; i--, pOrTerm++) {




        if (pOrTerm->leftCursor != iCursor) {
          pOrTerm->wtFlags &= ~0x0040;
        } else if (pOrTerm->u.x.leftColumn != iColumn || (iColumn == (-2) && sqlite3ExprCompare(pParse, pOrTerm->pExpr->pLeft, pLeft, -1))) {
          okToChngToIN = 0;
        } else {
          int affLeft, affRight;

          affRight = sqlite3ExprAffinity(pOrTerm->pExpr->pRight);
          affLeft = sqlite3ExprAffinity(pOrTerm->pExpr->pLeft);
          if (affRight != 0 && affRight != affLeft) {
            okToChngToIN = 0;
          } else {
            pOrTerm->wtFlags |= 0x0040;
          }
        }
      }
    }

    if (okToChngToIN) {
      Expr *pDup;
      ExprList *pList = 0;
      Expr *pLeft = 0;
      Expr *pNew;

      for (i = pOrWc->nTerm - 1, pOrTerm = pOrWc->a; i >= 0; i--, pOrTerm++) {
        if ((pOrTerm->wtFlags & 0x0040) == 0)
          continue;








        pDup = sqlite3ExprDup(db, pOrTerm->pExpr->pRight, 0);
        pList = sqlite3ExprListAppend(pWInfo->pParse, pList, pDup);
        pLeft = pOrTerm->pExpr->pLeft;
      }


      pDup = sqlite3ExprDup(db, pLeft, 0);
      pNew = sqlite3PExpr(pParse, 50, pDup, 0);
      if (pNew) {
        int idxNew;
        transferJoinMarkings(pNew, pExpr);


        pNew->x.pList = pList;
        idxNew = whereClauseInsert(pWC, pNew, 0x0002 | 0x0001);
        ;
        exprAnalyze(pSrc, pWC, idxNew);

        markTermAsChild(pWC, idxNew, idxTerm);
      } else {
        sqlite3ExprListDelete(db, pList);
      }
    }
  }
}

__attribute__((noinline)) int exprMightBeIndexed2(SrcList *pFrom, int *aiCurCol, Expr *pExpr, int j) {
  Index *pIdx;
  int i;
  int iCur;
  do {
    iCur = pFrom->a[j].iCursor;
    for (pIdx = pFrom->a[j].pSTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      if (pIdx->aColExpr == 0)
        continue;
      for (i = 0; i < pIdx->nKeyCol; i++) {
        if (pIdx->aiColumn[i] != (-2))
          continue;


        if (sqlite3ExprCompareSkip(pExpr, pIdx->aColExpr->a[i].pExpr, iCur) == 0 && !sqlite3ExprIsConstant(0, pIdx->aColExpr->a[i].pExpr)) {
          aiCurCol[0] = iCur;
          aiCurCol[1] = (-2);
          return 1;
        }
      }
    }
  } while (++j < pFrom->nSrc);
  return 0;
}

int exprMightBeIndexed(SrcList *pFrom, int *aiCurCol, Expr *pExpr, int op) {
  int i;

  if (pExpr->op == 177 && (op >= 55 && (op <= 58))) {


    pExpr = pExpr->x.pList->a[0].pExpr;
  }

  if (pExpr->op == 168) {
    aiCurCol[0] = pExpr->iTable;
    aiCurCol[1] = pExpr->iColumn;
    return 1;
  }

  for (i = 0; i < pFrom->nSrc; i++) {
    Index *pIdx;
    for (pIdx = pFrom->a[i].pSTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      if (pIdx->aColExpr) {
        return exprMightBeIndexed2(pFrom, aiCurCol, pExpr, i);
      }
    }
  }
  return 0;
}

void exprAnalyze(SrcList *pSrc, WhereClause *pWC, int idxTerm) {
  WhereInfo *pWInfo = pWC->pWInfo;
  WhereTerm *pTerm;
  WhereMaskSet *pMaskSet;
  Expr *pExpr;
  Bitmask prereqLeft;
  Bitmask prereqAll;
  Bitmask extraRight = 0;
  Expr *pStr1 = 0;
  int isComplete = 0;
  int noCase = 0;
  int op;
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  unsigned char eOp2 = 0;
  int nLeft;

  if (db->mallocFailed) {
    return;
  }

  pTerm = &pWC->a[idxTerm];

  pMaskSet = &pWInfo->sMaskSet;
  pExpr = pTerm->pExpr;

  pMaskSet->bVarSelect = 0;
  prereqLeft = sqlite3WhereExprUsage(pMaskSet, pExpr->pLeft);
  op = pExpr->op;
  if (op == 50) {


    if (sqlite3ExprCheckIN(pParse, pExpr))
      return;
    if ((((pExpr)->flags & 0x001000) != 0)) {
      pTerm->prereqRight = exprSelectUsage(pMaskSet, pExpr->x.pSelect);
    } else {
      pTerm->prereqRight = sqlite3WhereExprListUsage(pMaskSet, pExpr->x.pList);
    }
    prereqAll = prereqLeft | pTerm->prereqRight;
  } else {
    pTerm->prereqRight = sqlite3WhereExprUsage(pMaskSet, pExpr->pRight);
    if (pExpr->pLeft == 0 || (((pExpr)->flags & (u32)(0x001000 | 0x040000)) != 0) || pExpr->x.pList != 0) {
      prereqAll = sqlite3WhereExprUsageNN(pMaskSet, pExpr);
    } else {
      prereqAll = prereqLeft | pTerm->prereqRight;
    }
  }
  if (pMaskSet->bVarSelect)
    pTerm->wtFlags |= 0x1000;

  if ((((pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0)) {
    Bitmask x = sqlite3WhereGetMask(pMaskSet, pExpr->w.iJoin);
    if ((((pExpr)->flags & (u32)(0x000001)) != 0)) {
      prereqAll |= x;
      extraRight = x - 1;

    } else if ((prereqAll >> 1) >= x) {
      (pExpr)->flags &= ~(u32)(0x000002);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->leftCursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  if (allowedOp(op)) {
    int aiCurCol[2];
    Expr *pLeft = sqlite3ExprSkipCollate(pExpr->pLeft);
    Expr *pRight = sqlite3ExprSkipCollate(pExpr->pRight);
    u16 opMask = (pTerm->prereqRight & prereqLeft) == 0 ? 0x3fff : 0x0800;

    if (pTerm->u.x.iField > 0) {






      pLeft = pLeft->x.pList->a[pTerm->u.x.iField - 1].pExpr;
    }

    if (exprMightBeIndexed(pSrc, aiCurCol, pLeft, op)) {
      pTerm->leftCursor = aiCurCol[0];


      pTerm->u.x.leftColumn = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    if (op == 45)
      pTerm->wtFlags |= 0x0800;
    if (pRight && exprMightBeIndexed(pSrc, aiCurCol, pRight, op) && !(((pRight)->flags & (u32)(0x000020)) != 0)) {
      WhereTerm *pNew;
      Expr *pDup;
      u16 eExtraOp = 0;


      if (pTerm->leftCursor >= 0) {
        int idxNew;
        pDup = sqlite3ExprDup(db, pExpr, 0);
        if (db->mallocFailed) {
          sqlite3ExprDelete(db, pDup);
          return;
        }
        idxNew = whereClauseInsert(pWC, pDup, 0x0002 | 0x0001);
        if (idxNew == 0)
          return;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        if (op == 45)
          pNew->wtFlags |= 0x0800;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= 0x0008;


        if (termIsEquivalence(pParse, pDup, pWInfo->pTabList)) {
          pTerm->eOperator |= 0x0800;
          eExtraOp = 0x0800;
        }
      } else {
        pDup = pExpr;
        pNew = pTerm;
      }
      pNew->wtFlags |= exprCommute(pParse, pDup);
      pNew->leftCursor = aiCurCol[0];


      pNew->u.x.leftColumn = aiCurCol[1];
      ;
      pNew->prereqRight = prereqLeft | extraRight;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    } else if (op == 51 && !(((pExpr)->flags & (u32)(0x000001)) != 0) && 0 == sqlite3ExprCanBeNull(pLeft)) {


      pExpr->op = 171;
      pExpr->u.zToken = "false";
      (pExpr)->flags |= (u32)(0x20000000);
      pTerm->prereqAll = 0;
      pTerm->eOperator = 0;
    }
  }

  else if (pExpr->op == 49 && pWC->op == 44) {
    ExprList *pList;
    int i;
    static const u8 ops[] = {58, 56};


    pList = pExpr->x.pList;






    for (i = 0; i < 2; i++) {
      Expr *pNewExpr;
      int idxNew;
      pNewExpr = sqlite3PExpr(pParse, ops[i], sqlite3ExprDup(db, pExpr->pLeft, 0), sqlite3ExprDup(db, pList->a[i].pExpr, 0));
      transferJoinMarkings(pNewExpr, pExpr);
      idxNew = whereClauseInsert(pWC, pNewExpr, 0x0002 | 0x0001);
      ;
      exprAnalyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }

  else if (pExpr->op == 43 && !(((pExpr)->flags & (u32)(0x000200)) != 0)) {


    exprAnalyzeOrTerm(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }

  else if (pExpr->op == 52) {
    if (pExpr->pLeft->op == 168 && pExpr->pLeft->iColumn >= 0 && !(((pExpr)->flags & (u32)(0x000001)) != 0)) {
      Expr *pNewExpr;
      Expr *pLeft = pExpr->pLeft;
      int idxNew;
      WhereTerm *pNewTerm;

      pNewExpr = sqlite3PExpr(pParse, 55, sqlite3ExprDup(db, pLeft, 0), sqlite3ExprAlloc(db, 122, 0, 0));

      idxNew = whereClauseInsert(pWC, pNewExpr, 0x0002 | 0x0001 | 0x0080);
      if (idxNew) {
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereqRight = 0;
        pNewTerm->leftCursor = pLeft->iTable;
        pNewTerm->u.x.leftColumn = pLeft->iColumn;
        pNewTerm->eOperator = (0x0002 << (55 - 54));
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= 0x0008;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
    }
  }

  else if (pExpr->op == 172 && pWC->op == 44 && isLikeOrGlob(pParse, pExpr, &pStr1, &isComplete, &noCase)) {
    Expr *pLeft;
    Expr *pStr2;
    Expr *pNewExpr1;
    Expr *pNewExpr2;
    int idxNew1;
    int idxNew2;
    const char *zCollSeqName;
    const u16 wtFlags = 0x0100 | 0x0002 | 0x0001;


    pLeft = pExpr->x.pList->a[1].pExpr;
    pStr2 = sqlite3ExprDup(db, pStr1, 0);





    if (noCase && !pParse->db->mallocFailed) {
      int i;
      char c;
      pTerm->wtFlags |= 0x0400;
      for (i = 0; (c = pStr1->u.zToken[i]) != 0; i++) {
        pStr1->u.zToken[i] = ((c) & ~(sqlite3CtypeMap[(unsigned char)(c)] & 0x20));
        pStr2->u.zToken[i] = (sqlite3UpperToLower[(unsigned char)(c)]);
      }
    }

    if (!db->mallocFailed) {
      u8 *pC;
      pC = (u8 *)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken) - 1];
      if (noCase) {

        if (*pC == 'A' - 1)
          isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      while (*pC == 0xBF && pC > (u8 *)pStr2->u.zToken) {
        *pC = 0x80;
        pC--;
      }


      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3StrBINARY;
    pNewExpr1 = sqlite3ExprDup(db, pLeft, 0);
    pNewExpr1 = sqlite3PExpr(pParse, 58, sqlite3ExprAddCollateString(pParse, pNewExpr1, zCollSeqName), pStr1);
    transferJoinMarkings(pNewExpr1, pExpr);
    idxNew1 = whereClauseInsert(pWC, pNewExpr1, wtFlags);
    ;
    pNewExpr2 = sqlite3ExprDup(db, pLeft, 0);
    pNewExpr2 = sqlite3PExpr(pParse, 57, sqlite3ExprAddCollateString(pParse, pNewExpr2, zCollSeqName), pStr2);
    transferJoinMarkings(pNewExpr2, pExpr);
    idxNew2 = whereClauseInsert(pWC, pNewExpr2, wtFlags);
    ;
    exprAnalyze(pSrc, pWC, idxNew1);
    exprAnalyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    if (isComplete) {
      markTermAsChild(pWC, idxNew1, idxTerm);
      markTermAsChild(pWC, idxNew2, idxTerm);
    }
  }

  if ((pExpr->op == 54 || pExpr->op == 45) && (nLeft = sqlite3ExprVectorSize(pExpr->pLeft)) > 1 && sqlite3ExprVectorSize(pExpr->pRight) == nLeft && ((pExpr->pLeft->flags & 0x001000) == 0 || (pExpr->pRight->flags & 0x001000) == 0) && pWC->op == 44) {
    int i;
    for (i = 0; i < nLeft; i++) {
      int idxNew;
      Expr *pNew;
      Expr *pLeft = sqlite3ExprForVectorField(pParse, pExpr->pLeft, i, nLeft);
      Expr *pRight = sqlite3ExprForVectorField(pParse, pExpr->pRight, i, nLeft);

      pNew = sqlite3PExpr(pParse, pExpr->op, pLeft, pRight);
      transferJoinMarkings(pNew, pExpr);
      idxNew = whereClauseInsert(pWC, pNew, 0x0001 | 0x8000);
      exprAnalyze(pSrc, pWC, idxNew);
    }
    pTerm = &pWC->a[idxTerm];
    pTerm->wtFlags |= 0x0004 | 0x0002;
    pTerm->eOperator = 0x2000;
  }

  else if (pExpr->op == 50 && pTerm->u.x.iField == 0 && pExpr->pLeft->op == 177 && ((((pExpr)->flags & 0x001000) != 0)) && (pExpr->x.pSelect->pPrior == 0 || (pExpr->x.pSelect->selFlags & 0x0000200))

           && pExpr->x.pSelect->pWin == 0

           && pWC->op == 44 && pExpr->x.pSelect->pEList->nExpr <= ((((i64)1) << (sizeof(pTerm->nChild) * 8)) - 1)

  ) {
    int i;


    for (i = 0; i < sqlite3ExprVectorSize(pExpr->pLeft); i++) {
      int idxNew;
      idxNew = whereClauseInsert(pWC, pExpr, 0x0002 | 0x8000);
      pWC->a[idxNew].u.x.iField = i + 1;
      exprAnalyze(pSrc, pWC, idxNew);
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }

  else if (pWC->op == 44) {
    Expr *pRight = 0, *pLeft = 0;
    int res = isAuxiliaryVtabOperator(db, pExpr, &eOp2, &pLeft, &pRight);
    while (res-- > 0) {
      int idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereqExpr;

      prereqExpr = sqlite3WhereExprUsage(pMaskSet, pRight);
      prereqColumn = sqlite3WhereExprUsage(pMaskSet, pLeft);
      if ((prereqExpr & prereqColumn) == 0) {
        Expr *pNewExpr;
        pNewExpr = sqlite3PExpr(pParse, 47, 0, sqlite3ExprDup(db, pRight, 0));
        if ((((pExpr)->flags & (u32)(0x000001)) != 0) && pNewExpr) {
          (pNewExpr)->flags |= (u32)(0x000001);
          pNewExpr->w.iJoin = pExpr->w.iJoin;
        }
        idxNew = whereClauseInsert(pWC, pNewExpr, 0x0002 | 0x0001);
        ;
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereqRight = prereqExpr | extraRight;
        pNewTerm->leftCursor = pLeft->iTable;
        pNewTerm->u.x.leftColumn = pLeft->iColumn;
        pNewTerm->eOperator = 0x0040;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= 0x0008;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      {
        Expr *t = pLeft;
        pLeft = pRight;
        pRight = t;
      };
    }
  }

  ;
  pTerm = &pWC->a[idxTerm];
  pTerm->prereqRight |= extraRight;
}

void sqlite3WhereExprAnalyze(SrcList *pTabList, WhereClause *pWC) {
  int i;
  for (i = pWC->nTerm - 1; i >= 0; i--) {
    exprAnalyze(pTabList, pWC, i);
  }
}