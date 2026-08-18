#define _GNU_SOURCE 1
#include <string.h>
#include <stddef.h>
#include "sqlite/Walker.h"
#include "sqlite/AggInfo.h"
#include "sqlite/CheckOnCtx.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/CoveringIndexCheck.h"
#include "sqlite/Cte.h"
#include "sqlite/Db.h"
#include "sqlite/DbFixer.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FuncDef.h"
#include "sqlite/IdList.h"
#include "sqlite/IdxCover.h"
#include "sqlite/Index.h"
#include "sqlite/IndexedExpr.h"
#include "sqlite/NameContext.h"
#include "sqlite/OnOrUsing.h"
#include "sqlite/Parse.h"
#include "sqlite/RefSrcList.h"
#include "sqlite/RenameCtx.h"
#include "sqlite/RenameToken.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Table.h"
#include "sqlite/Token.h"
#include "sqlite/Trigger.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/Upsert.h"
#include "sqlite/VTable.h"
#include "sqlite/WhereConst.h"
#include "sqlite/Window.h"
#include "sqlite/WindowRewrite.h"
#include "sqlite/With.h"
#include "sqlite/bft.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
#include "sqlite/SqliteAuthorizerActionCode.h"
#include "sqlite/SqliteAuthorizerReturnCode.h"
#include "sqlite/SqliteFunctionFlags.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqlitePrepareFlags.h"
#include "sqlite/SqliteResultCode.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int inAnyUsingClause(const char *zName, SrcItem *pBase, int N);

static int inAnyUsingClause(const char *zName, SrcItem *pBase, int N) {
  while (N > 0) {
    N--;
    pBase++;
    if (pBase->fg.isUsing == 0)
      continue;
    if (pBase->u3.pUsing == 0)
      continue;
    if (sqlite3IdListIndex(pBase->u3.pUsing, zName) >= 0)
      return 1;
  }
  return 0;
}

int walkWindowList(Walker *pWalker, Window *pList, int bOneOnly) {
  Window *pWin;
  for (pWin = pList; pWin; pWin = pWin->pNextWin) {
    int rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    if (rc)
      return 2;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    if (rc)
      return 2;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    if (rc)
      return 2;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    if (rc)
      return 2;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    if (rc)
      return 2;
    if (bOneOnly)
      break;
  }
  return 0;
}

__attribute__((noinline)) int sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr) {
  int rc;
  while (1) {
    rc = pWalker->xExprCallback(pWalker, pExpr);
    if (rc)
      return rc & 2;
    if (!(((pExpr)->flags & (u32)((0x010000 | 0x800000))) != 0)) {
      if (pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft)) {
        return 2;
      }
      if (pExpr->pRight) {
        pExpr = pExpr->pRight;
        continue;
      } else if ((((pExpr)->flags & 0x001000) != 0)) {
        if (sqlite3WalkSelect(pWalker, pExpr->x.pSelect))
          return 2;
      } else {
        if (pExpr->x.pList) {
          if (sqlite3WalkExprList(pWalker, pExpr->x.pList))
            return 2;
        }

        if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
          if (walkWindowList(pWalker, pExpr->y.pWin, 1))
            return 2;
        }
      }
    }
    break;
  }
  return 0;
}

int sqlite3WalkExpr(Walker *pWalker, Expr *pExpr) {
  return pExpr ? sqlite3WalkExprNN(pWalker, pExpr) : 0;
}

int sqlite3WalkExprList(Walker *pWalker, ExprList *p) {
  int i;
  struct ExprList_item *pItem;
  if (p) {
    for (i = p->nExpr, pItem = p->a; i > 0; i--, pItem++) {
      if (sqlite3WalkExpr(pWalker, pItem->pExpr))
        return 2;
    }
  }
  return 0;
}

void sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p) {
  (void)(pWalker);
  (void)(p);
}

int sqlite3WalkSelectExpr(Walker *pWalker, Select *p) {
  if (sqlite3WalkExprList(pWalker, p->pEList))
    return 2;
  if (sqlite3WalkExpr(pWalker, p->pWhere))
    return 2;
  if (sqlite3WalkExprList(pWalker, p->pGroupBy))
    return 2;
  if (sqlite3WalkExpr(pWalker, p->pHaving))
    return 2;
  if (sqlite3WalkExprList(pWalker, p->pOrderBy))
    return 2;
  if (sqlite3WalkExpr(pWalker, p->pLimit))
    return 2;

  if (p->pWinDefn) {
    Parse *pParse;
    if (pWalker->xSelectCallback2 == sqlite3WalkWinDefnDummyCallback ||
        ((pParse = pWalker->pParse) != 0 && (pParse->eParseMode >= 2)) ||
        pWalker->xSelectCallback2 == sqlite3SelectPopWith) {
      int rc = walkWindowList(pWalker, p->pWinDefn, 0);
      return rc;
    }
  }

  return 0;
}

int sqlite3WalkSelectFrom(Walker *pWalker, Select *p) {
  SrcList *pSrc;
  int i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  if ((pSrc)) {
    for (i = pSrc->nSrc, pItem = pSrc->a; i > 0; i--, pItem++) {
      if (pItem->fg.isSubquery && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)) {
        return 2;
      }
      if (pItem->fg.isTabFunc && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)) {
        return 2;
      }
    }
  }
  return 0;
}

int sqlite3WalkSelect(Walker *pWalker, Select *p) {
  int rc;
  if (p == 0)
    return 0;
  if (pWalker->xSelectCallback == 0)
    return 0;
  do {
    rc = pWalker->xSelectCallback(pWalker, p);
    if (rc)
      return rc & 2;
    if (sqlite3WalkSelectExpr(pWalker, p) || sqlite3WalkSelectFrom(pWalker, p)) {
      return 2;
    }
    if (pWalker->xSelectCallback2) {
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  } while (p != 0);
  return 0;
}

int sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect) {
  (void)(pSelect);
  pWalker->walkerDepth++;
  return 0;
}

void sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect) {
  (void)(pSelect);
  pWalker->walkerDepth--;
}

int sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  return 0;
}

int sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  return 0;
}

int incrAggDepth(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 169)
    pExpr->op2 += pWalker->u.n;
  return 0;
}

int resolveExprStep(Walker *pWalker, Expr *pExpr) {
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;

  pParse = pNC->pParse;

  switch (pExpr->op) {
    case 76: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;

      pItem = pSrcList->a;
      pExpr->op = 168;

      pExpr->y.pTab = pItem->pSTab;
      pExpr->iTable = pItem->iCursor;
      pExpr->iColumn--;
      pExpr->affExpr = 0x44;
      break;
    }

    case 52:
    case 51: {
      int anRef[8];
      NameContext *p;
      int i;
      for (i = 0, p = pNC; p && i < ((int)(sizeof(anRef) / sizeof(anRef[0]))); p = p->pNext, i++) {
        anRef[i] = p->nRef;
      }
      sqlite3WalkExpr(pWalker, pExpr->pLeft);
      if ((pParse->eParseMode >= 2))
        return 1;
      if (sqlite3ExprCanBeNull(pExpr->pLeft)) {
        return 1;
      }

      for (i = 0, p = pNC; p; p = p->pNext, i++) {
        if ((p->ncFlags & 0x100000) == 0) {
          return 1;
        }
      };

      pExpr->u.iValue = (pExpr->op == 52);
      pExpr->flags |= 0x000800;
      pExpr->op = 156;
      for (i = 0, p = pNC; p && i < ((int)(sizeof(anRef) / sizeof(anRef[0]))); p = p->pNext, i++) {
        p->nRef = anRef[i];
      }
      sqlite3ExprDelete(pParse->db, pExpr->pLeft);
      pExpr->pLeft = 0;
      return 1;
    }

    case 60:
    case 142: {
      const char *zTable;
      const char *zDb;
      Expr *pRight;

      if (pExpr->op == 60) {
        zDb = 0;
        zTable = 0;

        pRight = pExpr;
      } else {
        Expr *pLeft = pExpr->pLeft;

        if (((pNC)->ncFlags & (0x000020 | 0x000008)) != 0)
          notValidImpl(pParse, pNC, "the \".\" operator", 0, pExpr);
        pRight = pExpr->pRight;
        if (pRight->op == 60) {
          zDb = 0;
        } else {
          zDb = pLeft->u.zToken;
          pLeft = pRight->pLeft;
          pRight = pRight->pRight;
        }

        zTable = pLeft->u.zToken;

        if ((pParse->eParseMode >= 2)) {
          sqlite3RenameTokenRemap(pParse, (void *)pExpr, (void *)pRight);
          sqlite3RenameTokenRemap(pParse, (void *)&pExpr->y.pTab, (void *)pLeft);
        }
      }
      return lookupName(pParse, zDb, zTable, pRight, pNC, pExpr);
    }

    case 172: {
      ExprList *pList;
      int n;
      int no_such_func = 0;
      int wrong_num_args = 0;
      int is_agg = 0;
      const char *zId;
      FuncDef *pDef;
      u8 enc = ((pParse->db)->enc);
      int savedAllowFlags = (pNC->ncFlags & (0x000001 | 0x004000));

      Window *pWin =
          ((((((pExpr))->flags & (u32)(0x1000000)) != 0) && pExpr->y.pWin->eFrmType != 167) ? pExpr->y.pWin : 0);

      pList = pExpr->x.pList;
      n = pList ? pList->nExpr : 0;
      zId = pExpr->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      if (pDef == 0) {
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        if (pDef == 0) {
          no_such_func = 1;
        } else {
          wrong_num_args = 1;
        }
      } else {
        is_agg = pDef->xFinalize != 0;
        if (pDef->funcFlags & 0x0400) {
          (pExpr)->flags |= (u32)(0x080000);
          if (n == 2) {
            pExpr->iTable = exprProbability(pList->a[1].pExpr);
            if (pExpr->iTable < 0) {
              sqlite3ErrorMsg(pParse,
                              "second argument to %#T() must be a "
                              "constant between 0.0 and 1.0",
                              pExpr);
              pNC->nNcErr++;
            }
          } else {
            pExpr->iTable = pDef->zName[0] == 'u' ? 8388608 : 125829120;
          }
        }

        {
          int auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0, pDef->zName, 0);
          if (auth != SQLITE_OK) {
            if (auth == SQLITE_DENY) {
              sqlite3ErrorMsg(pParse, "not authorized to use function: %#T", pExpr);
              pNC->nNcErr++;
            }
            pExpr->op = 122;
            return 1;
          }
        }

        if ((pDef->funcFlags & SQLITE_SUBTYPE) || (((pExpr)->flags & (u32)(0x80000000)) != 0)) {
          resolveSetExprSubtypeArg(pList);
        }

        if (pDef->funcFlags & (0x0800 | 0x2000)) {
          (pExpr)->flags |= (u32)(0x100000);
        }
        if ((pDef->funcFlags & 0x0800) == 0) {
          if (((pNC)->ncFlags & (0x000020 | 0x000002 | 0x000008)) != 0)
            notValidImpl(pParse, pNC, "non-deterministic functions", 0, pExpr);
        } else {
          pExpr->op2 = pNC->ncFlags & 0x00002e;
        }
        if ((pDef->funcFlags & 0x00040000) != 0 && pParse->nested == 0 && (pParse->db->mDbFlags & 0x0020) == 0) {
          no_such_func = 1;
          pDef = 0;
        } else if ((pDef->funcFlags & (0x00080000 | 0x00200000)) != 0 && !(pParse->eParseMode >= 2)) {
          if (pNC->ncFlags & 0x040000)
            (pExpr)->flags |= (u32)(0x40000000);
          sqlite3ExprFunctionUsable(pParse, pExpr, pDef);
        }
      }

      if (0 == (pParse->eParseMode >= 2)) {
        if (pDef && pDef->xValue == 0 && pWin) {
          sqlite3ErrorMsg(pParse, "%#T() may not be used as a window function", pExpr);
          pNC->nNcErr++;
        } else if ((is_agg && (pNC->ncFlags & 0x000001) == 0) || (is_agg && (pDef->funcFlags & 0x00010000) && !pWin) ||
                   (is_agg && pWin && (pNC->ncFlags & 0x004000) == 0)) {
          const char *zType;
          if ((pDef->funcFlags & 0x00010000) || pWin) {
            zType = "window";
          } else {
            zType = "aggregate";
          }
          sqlite3ErrorMsg(pParse, "misuse of %s function %#T()", zType, pExpr);
          pNC->nNcErr++;
          is_agg = 0;
        }

        else if (no_such_func && pParse->db->init.busy == 0) {
          sqlite3ErrorMsg(pParse, "no such function: %#T", pExpr);
          pNC->nNcErr++;
        } else if (wrong_num_args) {
          sqlite3ErrorMsg(pParse, "wrong number of arguments to function %#T()", pExpr);
          pNC->nNcErr++;
        }

        else if (is_agg == 0 && (((pExpr)->flags & (u32)(0x1000000)) != 0)) {
          sqlite3ErrorMsg(pParse, "FILTER may not be used with non-aggregate %#T()", pExpr);
          pNC->nNcErr++;
        }

        else if (is_agg == 0 && pExpr->pLeft) {
          sqlite3ExprOrderByAggregateError(pParse, pExpr);
          pNC->nNcErr++;
        }
        if (is_agg) {
          pNC->ncFlags &= ~(0x004000 | (!pWin ? 0x000001 : 0));
        }
      } else if ((((pExpr)->flags & (u32)(0x1000000)) != 0) || pExpr->pLeft) {
        is_agg = 1;
      }
      sqlite3WalkExprList(pWalker, pList);
      if (is_agg) {
        if (pExpr->pLeft) {
          sqlite3WalkExprList(pWalker, pExpr->pLeft->x.pList);
        }

        if (pWin && pParse->nErr == 0) {
          Select *pSel = pNC->pWinSelect;

          if ((pParse->eParseMode >= 2) == 0) {
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            if (pParse->db->mallocFailed)
              break;
          }
          sqlite3WalkExprList(pWalker, pWin->pPartition);
          sqlite3WalkExprList(pWalker, pWin->pOrderBy);
          sqlite3WalkExpr(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= 0x008000;
        } else {
          NameContext *pNC2;
          pExpr->op = 169;
          pExpr->op2 = 0;

          if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
            sqlite3WalkExpr(pWalker, pExpr->y.pWin->pFilter);
          }

          pNC2 = pNC;
          while (pNC2 && sqlite3ReferencesSrcList(pParse, pExpr, pNC2->pSrcList) == 0) {
            pExpr->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }

          if (pNC2 && pDef) {
            pExpr->op2 += pNC2->nNestedSelect;

            pNC2->ncFlags |= 0x000010 | ((pDef->funcFlags ^ 0x08000000) & (0x1000 | 0x08000000));
          }
        }
        pNC->ncFlags |= savedAllowFlags;
      }

      return 1;
    }

    case 20:
    case 139:
    case 50: {
      if ((((pExpr)->flags & 0x001000) != 0)) {
        int nRef = pNC->nRef;

        if (pExpr->op == 20)
          pParse->bHasExists = 1;
        if (pNC->ncFlags & 0x00002e) {
          notValidImpl(pParse, pNC, "subqueries", pExpr, pExpr);
        } else {
          sqlite3WalkSelect(pWalker, pExpr->x.pSelect);
        }

        if (nRef != pNC->nRef) {
          (pExpr)->flags |= (u32)(0x000040);
          pExpr->x.pSelect->selFlags |= 0x20000000;
        }
        pNC->ncFlags |= 0x000040;
      }
      break;
    }
    case 157: {
      if (((pNC)->ncFlags & (0x000004 | 0x000002 | 0x000020 | 0x000008)) != 0)
        notValidImpl(pParse, pNC, "parameters", pExpr, pExpr);
      break;
    }
    case 45:
    case 46: {
      Expr *pRight = sqlite3ExprSkipCollateAndLikely(pExpr->pRight);

      if ((pRight) && (pRight->op == 60 || pRight->op == 171)) {
        int rc = resolveExprStep(pWalker, pRight);
        if (rc == 2)
          return 2;
        if (pRight->op == 171) {
          pExpr->op2 = pExpr->op;
          pExpr->op = 175;
          return 0;
        }
      }
      __attribute__((fallthrough));
    }
    case 49:
    case 54:
    case 53:
    case 57:
    case 56:
    case 55:
    case 58: {
      int nLeft, nRight;
      if (pParse->db->mallocFailed)
        break;

      nLeft = sqlite3ExprVectorSize(pExpr->pLeft);
      if (pExpr->op == 49) {
        nRight = sqlite3ExprVectorSize(pExpr->x.pList->a[0].pExpr);
        if (nRight == nLeft) {
          nRight = sqlite3ExprVectorSize(pExpr->x.pList->a[1].pExpr);
        }
      } else {
        nRight = sqlite3ExprVectorSize(pExpr->pRight);
      }
      if (nLeft != nRight) {
        sqlite3ErrorMsg(pParse, "row value misused");
        sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
      }
      break;
    }
  }

  return pParse->nErr ? 2 : 0;
}

int resolveRemoveWindowsCb(Walker *pWalker, Expr *pExpr) {
  (void)(pWalker);
  if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
    Window *pWin = pExpr->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  return 0;
}

int resolveSelectStep(Walker *pWalker, Select *p) {
  NameContext *pOuterNC;
  NameContext sNC;
  int isCompound;
  int nCompound;
  Parse *pParse;
  int i;
  ExprList *pGroupBy;
  Select *pLeftmost;
  sqlite3 *db;

  if (p->selFlags & 0x0000004) {
    return 1;
  }
  pOuterNC = pWalker->u.pNC;
  pParse = pWalker->pParse;
  db = pParse->db;

  if ((p->selFlags & 0x0000040) == 0) {
    sqlite3SelectPrep(pParse, p, pOuterNC);
    return pParse->nErr ? 2 : 1;
  }

  isCompound = p->pPrior != 0;
  nCompound = 0;
  pLeftmost = p;
  while (p) {
    p->selFlags |= 0x0000004;

    memset(&sNC, 0, sizeof(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    if (sqlite3ResolveExprNames(&sNC, p->pLimit)) {
      return 2;
    }

    if (p->selFlags & 0x0010000) {
      Select *pSub;

      pSub = p->pSrc->a[0].u4.pSubq->pSelect;

      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }

    if (pOuterNC)
      pOuterNC->nNestedSelect++;
    for (i = 0; i < p->pSrc->nSrc; i++) {
      SrcItem *pItem = &p->pSrc->a[i];

      if (pItem->fg.isSubquery && (pItem->u4.pSubq->pSelect->selFlags & 0x0000004) == 0) {
        int nRef = pOuterNC ? pOuterNC->nRef : 0;
        const char *zSavedContext = pParse->zAuthContext;

        if (pItem->zName)
          pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        if (pParse->nErr)
          return 2;

        if (pOuterNC) {
          pItem->fg.isCorrelated = (pOuterNC->nRef > nRef);
        }
      }
    }
    if (pOuterNC && (pOuterNC->nNestedSelect > 0)) {
      pOuterNC->nNestedSelect--;
    }

    sNC.ncFlags = 0x000001 | 0x004000;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;

    if (sqlite3ResolveExprListNames(&sNC, p->pEList))
      return 2;
    sNC.ncFlags &= ~0x004000;

    pGroupBy = p->pGroupBy;
    if (pGroupBy || (sNC.ncFlags & 0x000010) != 0) {
      p->selFlags |= 0x0000008 | (sNC.ncFlags & (0x001000 | 0x8000000));
    } else {
      sNC.ncFlags &= ~0x000001;
    }

    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= 0x000080;
    if (p->pHaving) {
      if ((p->selFlags & 0x0000008) == 0) {
        sqlite3ErrorMsg(pParse, "HAVING clause on a non-aggregate query");
        return 2;
      }
      if (sqlite3ResolveExprNames(&sNC, p->pHaving))
        return 2;
    }
    sNC.ncFlags |= 0x100000;
    if (sqlite3ResolveExprNames(&sNC, p->pWhere))
      return 2;
    sNC.ncFlags &= ~0x100000;

    for (i = 0; i < p->pSrc->nSrc; i++) {
      SrcItem *pItem = &p->pSrc->a[i];
      if (pItem->fg.isTabFunc && sqlite3ResolveExprListNames(&sNC, pItem->u1.pFuncArg)) {
        return 2;
      }
    }

    if ((pParse->eParseMode >= 2)) {
      Window *pWin;
      for (pWin = p->pWinDefn; pWin; pWin = pWin->pNextWin) {
        if (sqlite3ResolveExprListNames(&sNC, pWin->pOrderBy) || sqlite3ResolveExprListNames(&sNC, pWin->pPartition)) {
          return 2;
        }
      }
    }

    sNC.ncFlags |= 0x000001 | 0x004000;

    if (p->selFlags & 0x0010000) {
      Select *pSub;

      pSub = p->pSrc->a[0].u4.pSubq->pSelect;

      p->pOrderBy = pSub->pOrderBy;
      pSub->pOrderBy = 0;
    }

    if (p->pOrderBy != 0 && isCompound <= nCompound && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")) {
      return 2;
    }
    if (db->mallocFailed) {
      return 2;
    }
    sNC.ncFlags &= ~0x004000;

    if (pGroupBy) {
      struct ExprList_item *pItem;

      if (resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed) {
        return 2;
      }
      for (i = 0, pItem = pGroupBy->a; i < pGroupBy->nExpr; i++, pItem++) {
        if ((((pItem->pExpr)->flags & (u32)(0x000010)) != 0)) {
          sqlite3ErrorMsg(pParse,
                          "aggregate functions are not allowed in "
                          "the GROUP BY clause");
          return 2;
        }
      }
    }

    if (p->pNext && p->pEList->nExpr != p->pNext->pEList->nExpr) {
      sqlite3SelectWrongNumTermsError(pParse, p->pNext);
      return 2;
    }

    if ((p->selFlags & 0x40000000)) {
      sqlite3SelectCheckOnClauses(pParse, p);
      if (pParse->nErr)
        return 2;
    }

    p = p->pPrior;
    nCompound++;
  }

  if (isCompound && resolveCompoundOrderBy(pParse, pLeftmost)) {
    return 2;
  }

  return 1;
}

int gatherSelectWindowsCallback(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 172 && (((pExpr)->flags & (u32)(0x1000000)) != 0)) {
    Select *pSelect = pWalker->u.pSelect;
    Window *pWin = pExpr->y.pWin;

    sqlite3WindowLink(pSelect, pWin);
  }
  return 0;
}

int gatherSelectWindowsSelectCallback(Walker *pWalker, Select *p) {
  return p == pWalker->u.pSelect ? 0 : 1;
}

int sqlite3SelectWalkFail(Walker *pWalker, Select *NotUsed) {
  (void)(NotUsed);
  pWalker->eCode = 0;
  return 2;
}

__attribute__((noinline)) int exprNodeIsConstantFunction(Walker *pWalker, Expr *pExpr) {
  int n;
  ExprList *pList;
  FuncDef *pDef;
  sqlite3 *db;

  if ((((pExpr)->flags & (u32)(0x010000)) != 0) || (pList = pExpr->x.pList) == 0) {
    n = 0;
  } else {
    n = pList->nExpr;
    sqlite3WalkExprList(pWalker, pList);
    if (pWalker->eCode == 0)
      return 2;
  }
  db = pWalker->pParse->db;
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, n, ((db)->enc), 0);
  if (pDef == 0 || pDef->xFinalize != 0 || (pDef->funcFlags & (0x0800 | 0x2000)) == 0 ||
      (((pExpr)->flags & (u32)(0x1000000)) != 0)) {
    pWalker->eCode = 0;
    return 2;
  }
  return 1;
}

int exprNodeIsConstant(Walker *pWalker, Expr *pExpr) {
  if (pWalker->eCode == 2 && (((pExpr)->flags & (u32)(0x000001)) != 0)) {
    pWalker->eCode = 0;
    return 2;
  }

  switch (pExpr->op) {
    case 172:
      if ((pWalker->eCode >= 4 || (((pExpr)->flags & (u32)(0x100000)) != 0)) &&
          !(((pExpr)->flags & (u32)(0x1000000)) != 0)) {
        if (pWalker->eCode == 5)
          (pExpr)->flags |= (u32)(0x40000000);
        return 0;
      } else if (pWalker->pParse) {
        return exprNodeIsConstantFunction(pWalker, pExpr);
      } else {
        pWalker->eCode = 0;
        return 2;
      }
    case 60:
      if (sqlite3ExprIdToTrueFalse(pExpr)) {
        return 1;
      }
      __attribute__((fallthrough));
    case 168:
    case 169:
    case 170:;
      if ((((pExpr)->flags & (u32)(0x000020)) != 0) && pWalker->eCode != 2) {
        return 0;
      }
      if (pWalker->eCode == 3 && pExpr->iTable == pWalker->u.iCur) {
        return 0;
      }
      __attribute__((fallthrough));
    case 179:
    case 176:
    case 142:
    case 72:;
      pWalker->eCode = 0;
      return 2;
    case 157:
      if (pWalker->eCode == 5) {
        pExpr->op = 122;
      } else if (pWalker->eCode == 4) {
        pWalker->eCode = 0;
        return 2;
      }
      __attribute__((fallthrough));
    default:;
      return 0;
  }
}

int exprSelectWalkTableConstant(Walker *pWalker, Select *pSelect) {
  if ((pSelect->selFlags & 0x20000000) != 0) {
    pWalker->eCode = 0;
    return 2;
  }
  return 1;
}

int exprNodeIsConstantOrGroupBy(Walker *pWalker, Expr *pExpr) {
  ExprList *pGroupBy = pWalker->u.pGroupBy;
  int i;

  for (i = 0; i < pGroupBy->nExpr; i++) {
    Expr *p = pGroupBy->a[i].pExpr;
    if (sqlite3ExprCompare(0, pExpr, p, -1) < 2) {
      CollSeq *pColl = sqlite3ExprNNCollSeq(pWalker->pParse, p);
      if (sqlite3IsBinary(pColl)) {
        return 1;
      }
    }
  }

  if ((((pExpr)->flags & 0x001000) != 0)) {
    pWalker->eCode = 0;
    return 2;
  }

  return exprNodeIsConstant(pWalker, pExpr);
}

int exprNodeCanReturnSubtype(Walker *pWalker, Expr *pExpr) {
  int n;
  FuncDef *pDef;
  sqlite3 *db;
  if (pExpr->op == 158 || pExpr->op == 173 || pExpr->op == 114 || pExpr->op == 36) {
    return 0;
  }
  if (pExpr->op != 172) {
    return 1;
  }

  db = pWalker->pParse->db;
  n = (pExpr->x.pList) ? pExpr->x.pList->nExpr : 0;
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, n, ((db)->enc), 0);
  if ((pDef == 0) || (pDef->funcFlags & SQLITE_RESULT_SUBTYPE) != 0) {
    pWalker->eCode = 1;
    return 2;
  }
  return 0;
}

void bothImplyNotNullRow(Walker *pWalker, Expr *pE1, Expr *pE2) {
  if (pWalker->eCode == 0) {
    sqlite3WalkExpr(pWalker, pE1);
    if (pWalker->eCode) {
      pWalker->eCode = 0;
      sqlite3WalkExpr(pWalker, pE2);
    }
  }
}

int impliesNotNullRow(Walker *pWalker, Expr *pExpr) {
  if ((((pExpr)->flags & (u32)(0x000001)) != 0))
    return 1;
  if ((((pExpr)->flags & (u32)(0x000002)) != 0) && pWalker->mWFlags) {
    return 1;
  }
  switch (pExpr->op) {
    case 46:
    case 51:
    case 52:
    case 45:
    case 177:
    case 172:
    case 175:
    case 158:;
      return 1;

    case 168:
      if (pWalker->u.iCur == pExpr->iTable) {
        pWalker->eCode = 1;
        return 2;
      }
      return 1;

    case 43:
    case 44:;
      bothImplyNotNullRow(pWalker, pExpr->pLeft, pExpr->pRight);
      return 1;

    case 50:
      if ((((pExpr)->flags & 0x001000) == 0) && (pExpr->x.pList->nExpr > 0)) {
        sqlite3WalkExpr(pWalker, pExpr->pLeft);
      }
      return 1;

    case 49:
      sqlite3WalkExpr(pWalker, pExpr->pLeft);
      bothImplyNotNullRow(pWalker, pExpr->x.pList->a[0].pExpr, pExpr->x.pList->a[1].pExpr);
      return 1;

    case 54:
    case 53:
    case 57:
    case 56:
    case 55:
    case 58: {
      Expr *pLeft = pExpr->pLeft;
      Expr *pRight = pExpr->pRight;

      if ((pLeft->op == 168 && (pLeft->y.pTab != 0) && ((pLeft->y.pTab)->eTabType == 1)) ||
          (pRight->op == 168 && (pRight->y.pTab != 0) && ((pRight->y.pTab)->eTabType == 1))) {
        return 1;
      }
      __attribute__((fallthrough));
    }
    default:
      return 0;
  }
}

int exprIdxCover(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 168 && pExpr->iTable == pWalker->u.pIdxCover->iCur &&
      sqlite3TableColumnToIndex(pWalker->u.pIdxCover->pIdx, pExpr->iColumn) < 0) {
    pWalker->eCode = 1;
    return 2;
  }
  return 0;
}

int selectRefEnter(Walker *pWalker, Select *pSelect) {
  struct RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  i64 i, j;
  int *piNew;
  if (pSrc->nSrc == 0)
    return 0;
  j = p->nExclude;
  p->nExclude += pSrc->nSrc;
  piNew = (int*)(sqlite3DbRealloc(p->db, p->aiExclude, p->nExclude * sizeof(int)));
  if (piNew == 0) {
    p->nExclude = 0;
    return 2;
  } else {
    p->aiExclude = piNew;
  }
  for (i = 0; i < pSrc->nSrc; i++, j++) {
    p->aiExclude[j] = pSrc->a[i].iCursor;
  }
  return 0;
}

void selectRefLeave(Walker *pWalker, Select *pSelect) {
  struct RefSrcList *p = pWalker->u.pRefSrcList;
  SrcList *pSrc = pSelect->pSrc;
  if (p->nExclude) {
    p->nExclude -= pSrc->nSrc;
  }
}

int exprRefToSrcList(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 168 || pExpr->op == 170) {
    int i;
    struct RefSrcList *p = pWalker->u.pRefSrcList;
    SrcList *pSrc = p->pRef;
    int nSrc = pSrc ? pSrc->nSrc : 0;
    for (i = 0; i < nSrc; i++) {
      if (pExpr->iTable == pSrc->a[i].iCursor) {
        pWalker->eCode |= 1;
        return 0;
      }
    }
    for (i = 0; i < p->nExclude && p->aiExclude[i] != pExpr->iTable; i++) {
    }
    if (i >= p->nExclude) {
      pWalker->eCode |= 2;
    }
  }
  return 0;
}

int agginfoPersistExprCb(Walker *pWalker, Expr *pExpr) {
  if ((!(((pExpr)->flags & (u32)(0x010000 | 0x004000)) != 0)) && pExpr->pAggInfo != 0) {
    AggInfo *pAggInfo = pExpr->pAggInfo;
    int iAgg = pExpr->iAgg;
    Parse *pParse = pWalker->pParse;
    sqlite3 *db = pParse->db;

    if (pExpr->op != 169) {
      if (iAgg < pAggInfo->nColumn && pAggInfo->aCol[iAgg].pCExpr == pExpr) {
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        if (pExpr && !sqlite3ExprDeferredDelete(pParse, pExpr)) {
          pAggInfo->aCol[iAgg].pCExpr = pExpr;
        }
      }
    } else {
      if ((iAgg < pAggInfo->nFunc) && pAggInfo->aFunc[iAgg].pFExpr == pExpr) {
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        if (pExpr && !sqlite3ExprDeferredDelete(pParse, pExpr)) {
          pAggInfo->aFunc[iAgg].pFExpr = pExpr;
        }
      }
    }
  }
  return 0;
}

void sqlite3AggInfoPersistWalkerInit(Walker *pWalker, Parse *pParse) {
  memset(pWalker, 0, sizeof(*pWalker));
  pWalker->pParse = pParse;
  pWalker->xExprCallback = agginfoPersistExprCb;
  pWalker->xSelectCallback = sqlite3SelectWalkNoop;
}

int analyzeAggregate(Walker *pWalker, Expr *pExpr) {
  int i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->uNC.pAggInfo;

  switch (pExpr->op) {
    default: {
      IndexedExpr *pIEpr;
      Expr tmp;

      if ((pNC->ncFlags & 0x020000) == 0)
        break;
      if (pParse->pIdxEpr == 0)
        break;
      for (pIEpr = pParse->pIdxEpr; pIEpr; pIEpr = pIEpr->pIENext) {
        int iDataCur = pIEpr->iDataCur;
        if (iDataCur < 0)
          continue;
        if (sqlite3ExprCompare(0, pExpr, pIEpr->pExpr, iDataCur) == 0)
          break;
      }
      if (pIEpr == 0)
        break;
      if ((!(((pExpr)->flags & (0x1000000 | 0x2000000)) == 0)))
        break;
      for (i = 0; i < pSrcList->nSrc; i++) {
        if (pSrcList->a[i].iCursor == pIEpr->iDataCur) {
          break;
        }
      }
      if (i >= pSrcList->nSrc)
        break;
      if ((pExpr->pAggInfo != 0))
        break;
      if (pParse->nErr) {
        return 2;
      }

      memset(&tmp, 0, sizeof(tmp));
      tmp.op = 170;
      tmp.iTable = pIEpr->iIdxCur;
      tmp.iColumn = pIEpr->iIdxCol;
      findOrCreateAggInfoColumn(pParse, pAggInfo, &tmp);
      if (pParse->nErr) {
        return 2;
      }

      pAggInfo->aCol[tmp.iAgg].pCExpr = pExpr;
      pExpr->pAggInfo = pAggInfo;
      pExpr->iAgg = tmp.iAgg;
      return 1;
    }
    case 179:
    case 170:
    case 168: {
      if ((pSrcList != 0)) {
        SrcItem *pItem = pSrcList->a;
        for (i = 0; i < pSrcList->nSrc; i++, pItem++) {
          if (pExpr->iTable == pItem->iCursor) {
            findOrCreateAggInfoColumn(pParse, pAggInfo, pExpr);
            break;
          }
        }
      }
      return 0;
    }
    case 169: {
      if ((pNC->ncFlags & 0x020000) == 0 && pWalker->walkerDepth == pExpr->op2 && pExpr->pAggInfo == 0) {
        struct AggInfo_func *pItem = pAggInfo->aFunc;
        int mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

        for (i = 0; i < pAggInfo->nFunc; i++, pItem++) {
          if (pItem->pFExpr == pExpr)
            break;
          if (sqlite3ExprCompare(0, pItem->pFExpr, pExpr, -1) == 0) {
            break;
          }
        }
        if (i > mxTerm) {
          sqlite3ErrorMsg(pParse, "more than %d aggregate terms", mxTerm);
          i = mxTerm;

        } else if (i >= pAggInfo->nFunc) {
          u8 enc = ((pParse->db)->enc);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          if (i >= 0) {
            int nArg;

            pItem = &pAggInfo->aFunc[i];
            pItem->pFExpr = pExpr;

            nArg = pExpr->x.pList ? pExpr->x.pList->nExpr : 0;
            pItem->pFunc = sqlite3FindFunction(pParse->db, pExpr->u.zToken, nArg, enc, 0);

            if (pExpr->pLeft && (pItem->pFunc->funcFlags & 0x0020) == 0) {
              ExprList *pOBList;

              pItem->iOBTab = pParse->nTab++;
              pOBList = pExpr->pLeft->x.pList;

              if (pOBList->nExpr == 1 && nArg == 1 &&
                  sqlite3ExprCompare(0, pOBList->a[0].pExpr, pExpr->x.pList->a[0].pExpr, 0) == 0) {
                pItem->bOBPayload = 0;
                pItem->bOBUnique = (((pExpr)->flags & (u32)(0x000004)) != 0);
              } else {
                pItem->bOBPayload = 1;
              }
              pItem->bUseSubtype = (pItem->pFunc->funcFlags & SQLITE_SUBTYPE) != 0;
            } else {
              pItem->iOBTab = -1;
            }
            if ((((pExpr)->flags & (u32)(0x000004)) != 0) && !pItem->bOBUnique) {
              pItem->iDistinct = pParse->nTab++;
            } else {
              pItem->iDistinct = -1;
            }
          }
        }

        pExpr->iAgg = (i16)i;
        pExpr->pAggInfo = pAggInfo;
        return 1;
      } else {
        return 0;
      }
    }
  }
  return 0;
}

int renameUnmapExprCb(Walker *pWalker, Expr *pExpr) {
  Parse *pParse = pWalker->pParse;
  sqlite3RenameTokenRemap(pParse, 0, (const void *)pExpr);
  if ((((pExpr)->flags & (0x1000000 | 0x2000000)) == 0)) {
    sqlite3RenameTokenRemap(pParse, 0, (const void *)&pExpr->y.pTab);
  }
  return 0;
}

void renameWalkWith(Walker *pWalker, Select *pSelect) {
  With *pWith = pSelect->pWith;
  if (pWith) {
    Parse *pParse = pWalker->pParse;
    int i;
    With *pCopy = 0;

    if ((pWith->a[0].pSelect->selFlags & 0x0000040) == 0) {
      pCopy = sqlite3WithDup(pParse->db, pWith);
      pCopy = sqlite3WithPush(pParse, pCopy, 1);
    }
    for (i = 0; i < pWith->nCte; i++) {
      Select *p = pWith->a[i].pSelect;
      NameContext sNC;
      memset(&sNC, 0, sizeof(sNC));
      sNC.pParse = pParse;
      if (pCopy)
        sqlite3SelectPrep(sNC.pParse, p, &sNC);
      if (sNC.pParse->db->mallocFailed)
        return;
      sqlite3WalkSelect(pWalker, p);
      sqlite3RenameExprlistUnmap(pParse, pWith->a[i].pCols);
    }
    if (pCopy && pParse->pWith == pCopy) {
      pParse->pWith = pCopy->pOuter;
    }
  }
}

int renameUnmapSelectCb(Walker *pWalker, Select *p) {
  Parse *pParse = pWalker->pParse;
  int i;
  if (pParse->nErr)
    return 2;
  if (p->selFlags & (0x0200000 | 0x4000000)) {
    return 1;
  }
  if ((p->pEList)) {
    ExprList *pList = p->pEList;
    for (i = 0; i < pList->nExpr; i++) {
      if (pList->a[i].zEName && pList->a[i].fg.eEName == 0) {
        sqlite3RenameTokenRemap(pParse, 0, (void *)pList->a[i].zEName);
      }
    }
  }
  if ((p->pSrc)) {
    SrcList *pSrc = p->pSrc;
    for (i = 0; i < pSrc->nSrc; i++) {
      sqlite3RenameTokenRemap(pParse, 0, (void *)pSrc->a[i].zName);
      if (pSrc->a[i].fg.isUsing == 0) {
        sqlite3WalkExpr(pWalker, pSrc->a[i].u3.pOn);
      } else {
        unmapColumnIdlistNames(pParse, pSrc->a[i].u3.pUsing);
      }
    }
  }

  renameWalkWith(pWalker, p);
  return 0;
}

int renameColumnSelectCb(Walker *pWalker, Select *p) {
  if (p->selFlags & (0x0200000 | 0x4000000)) {
    return 1;
  }
  renameWalkWith(pWalker, p);
  return 0;
}

int renameColumnExprCb(Walker *pWalker, Expr *pExpr) {
  RenameCtx *p = pWalker->u.pRename;
  if (pExpr->op == 78 && pExpr->iColumn == p->iCol && pWalker->pParse->pTriggerTab == p->pTab) {
    renameTokenFind(pWalker->pParse, p, (void *)pExpr);
  } else if (pExpr->op == 168 && pExpr->iColumn == p->iCol && ((((pExpr)->flags & (0x1000000 | 0x2000000)) == 0)) &&
             p->pTab == pExpr->y.pTab) {
    renameTokenFind(pWalker->pParse, p, (void *)pExpr);
  }
  return 0;
}

void renameWalkTrigger(Walker *pWalker, Trigger *pTrigger) {
  TriggerStep *pStep;

  sqlite3WalkExpr(pWalker, pTrigger->pWhen);

  for (pStep = pTrigger->step_list; pStep; pStep = pStep->pNext) {
    sqlite3WalkSelect(pWalker, pStep->pSelect);
    sqlite3WalkExpr(pWalker, pStep->pWhere);
    sqlite3WalkExprList(pWalker, pStep->pExprList);
    if (pStep->pUpsert) {
      Upsert *pUpsert = pStep->pUpsert;
      sqlite3WalkExprList(pWalker, pUpsert->pUpsertTarget);
      sqlite3WalkExprList(pWalker, pUpsert->pUpsertSet);
      sqlite3WalkExpr(pWalker, pUpsert->pUpsertWhere);
      sqlite3WalkExpr(pWalker, pUpsert->pUpsertTargetWhere);
    }
    if (pStep->pSrc) {
      int i;
      SrcList *pSrc = pStep->pSrc;
      for (i = 0; i < pSrc->nSrc; i++) {
        if (pSrc->a[i].fg.isSubquery) {
          sqlite3WalkSelect(pWalker, pSrc->a[i].u4.pSubq->pSelect);
        }
      }
    }
  }
}

int renameTableExprCb(Walker *pWalker, Expr *pExpr) {
  RenameCtx *p = pWalker->u.pRename;
  if (pExpr->op == 168 && ((((pExpr)->flags & (0x1000000 | 0x2000000)) == 0)) && p->pTab == pExpr->y.pTab) {
    renameTokenFind(pWalker->pParse, p, (void *)&pExpr->y.pTab);
  }
  return 0;
}

int renameTableSelectCb(Walker *pWalker, Select *pSelect) {
  int i;
  RenameCtx *p = pWalker->u.pRename;
  SrcList *pSrc = pSelect->pSrc;
  if (pSelect->selFlags & (0x0200000 | 0x4000000)) {
    return 1;
  }
  if (pSrc == 0) {
    return 2;
  }
  for (i = 0; i < pSrc->nSrc; i++) {
    SrcItem *pItem = &pSrc->a[i];
    if (pItem->pSTab == p->pTab) {
      renameTokenFind(pWalker->pParse, p, pItem->zName);
    }
  }
  renameWalkWith(pWalker, pSelect);

  return 0;
}

int renameQuotefixExprCb(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 118 && (pExpr->flags & 0x000080)) {
    renameTokenFind(pWalker->pParse, pWalker->u.pRename, (const void *)pExpr);
  }
  return 0;
}

int fixExprCb(Walker *p, Expr *pExpr) {
  DbFixer *pFix = p->u.pFix;
  if (!pFix->bTemp)
    (pExpr)->flags |= (u32)(0x40000000);
  if (pExpr->op == 157) {
    if (pFix->pParse->db->init.busy) {
      pExpr->op = 122;
    } else {
      sqlite3ErrorMsg(pFix->pParse, "%s cannot use variables", pFix->zType);
      return 2;
    }
  }
  return 0;
}

int fixSelectCb(Walker *p, Select *pSelect) {
  DbFixer *pFix = p->u.pFix;
  int i;
  SrcItem *pItem;
  sqlite3 *db = pFix->pParse->db;
  int iDb = sqlite3FindDbName(db, pFix->zDb);
  SrcList *pList = pSelect->pSrc;

  if (pList == 0)
    return 0;
  for (i = 0, pItem = pList->a; i < pList->nSrc; i++, pItem++) {
    if (pFix->bTemp == 0 && pItem->fg.isSubquery == 0) {
      if (pItem->fg.fixedSchema == 0 && pItem->u4.zDatabase != 0) {
        if (iDb != sqlite3FindDbName(db, pItem->u4.zDatabase)) {
          sqlite3ErrorMsg(pFix->pParse, "%s %T cannot reference objects in database %s", pFix->zType, pFix->pName,
                          pItem->u4.zDatabase);
          return 2;
        }
        sqlite3DbFree(db, pItem->u4.zDatabase);
        pItem->fg.notCte = 1;
        pItem->fg.hadSchema = 1;
      }
      pItem->u4.pSchema = pFix->pSchema;
      pItem->fg.fromDDL = 1;
      pItem->fg.fixedSchema = 1;
    }

    if (pList->a[i].fg.isUsing == 0 && sqlite3WalkExpr(&pFix->w, pList->a[i].u3.pOn)) {
      return 2;
    }
  }
  if (pSelect->pWith) {
    for (i = 0; i < pSelect->pWith->nCte; i++) {
      if (sqlite3WalkSelect(p, pSelect->pWith->a[i].pSelect)) {
        return 2;
      }
    }
  }
  return 0;
}

int exprColumnFlagUnion(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 168 && pExpr->iColumn >= 0) {
    pWalker->eCode |= pWalker->u.pTab->aCol[pExpr->iColumn].colFlags;
  }
  return 0;
}

int checkConstraintExprNode(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 168) {
    if (pExpr->iColumn >= 0) {
      if (pWalker->u.aiCol[pExpr->iColumn] >= 0) {
        pWalker->eCode |= 0x01;
      }
    } else {
      pWalker->eCode |= 0x02;
    }
  }
  return 0;
}

int recomputeColumnsUsedExpr(Walker *pWalker, Expr *pExpr) {
  SrcItem *pItem;
  if (pExpr->op != 168)
    return 0;
  pItem = pWalker->u.pSrcItem;
  if (pItem->iCursor != pExpr->iTable)
    return 0;
  if (pExpr->iColumn < 0)
    return 0;
  pItem->colUsed |= sqlite3ExprColUsed(pExpr);
  return 0;
}

void renumberCursorDoMapping(Walker *pWalker, int *piCursor) {
  int *aCsrMap = pWalker->u.aiCol;
  int iCsr = *piCursor;
  if (iCsr < aCsrMap[0] && aCsrMap[iCsr + 1] > 0) {
    *piCursor = aCsrMap[iCsr + 1];
  }
}

int renumberCursorsCb(Walker *pWalker, Expr *pExpr) {
  int op = pExpr->op;
  if (op == 168 || op == 179) {
    renumberCursorDoMapping(pWalker, &pExpr->iTable);
  }
  if ((((pExpr)->flags & (u32)(0x000001)) != 0)) {
    renumberCursorDoMapping(pWalker, &pExpr->w.iJoin);
  }
  return 0;
}

int propagateConstantExprRewrite(Walker *pWalker, Expr *pExpr) {
  WhereConst *pConst = pWalker->u.pConst;

  if (pConst->bHasAffBlob) {
    if ((pExpr->op >= 54 && pExpr->op <= 58) || pExpr->op == 45) {
      propagateConstantExprRewriteOne(pConst, pExpr->pLeft, 0);
      if (pConst->pOomFault[0])
        return 1;
      if (sqlite3ExprAffinity(pExpr->pLeft) != 0x42) {
        propagateConstantExprRewriteOne(pConst, pExpr->pRight, 0);
      }
    }
  }
  return propagateConstantExprRewriteOne(pConst, pExpr, pConst->bHasAffBlob);
}

int convertCompoundSelectToSubquery(Walker *pWalker, Select *p) {
  int i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  struct ExprList_item *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  if (p->pPrior == 0)
    return 0;
  if (p->pOrderBy == 0)
    return 0;
  for (pX = p; pX && (pX->op == 136 || pX->op == 139); pX = pX->pPrior) {
  }
  if (pX == 0)
    return 0;
  a = p->pOrderBy->a;

  if (a[0].u.x.iOrderByCol)
    return 0;

  for (i = p->pOrderBy->nExpr - 1; i >= 0; i--) {
    if (a[i].pExpr->flags & 0x000200)
      break;
  }
  if (i < 0)
    return 0;

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = (Select*)(sqlite3DbMallocZero(db, sizeof(*pNew)));
  if (pNew == 0)
    return 2;
  memset(&dummy, 0, sizeof(dummy));
  pNewSrc = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &dummy, pNew, 0);

  if (pParse->nErr) {
    sqlite3SrcListDelete(db, pNewSrc);
    return 2;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3ExprListAppend(pParse, 0, sqlite3Expr(db, 180, 0));
  p->op = 139;
  p->pWhere = 0;
  pNew->pGroupBy = 0;
  pNew->pHaving = 0;
  pNew->pOrderBy = 0;
  p->pPrior = 0;
  p->pNext = 0;
  p->pWith = 0;

  p->pWinDefn = 0;

  p->selFlags &= ~(u32)0x0000100;

  p->selFlags |= 0x0010000;

  pNew->pPrior->pNext = pNew;
  pNew->pLimit = 0;
  return 0;
}

void sqlite3SelectPopWith(Walker *pWalker, Select *p) {
  Parse *pParse = pWalker->pParse;
  if ((pParse->pWith) && p->pPrior == 0) {
    With *pWith = findRightmost(p)->pWith;
    if (pWith != 0) {
      pParse->pWith = pWith->pOuter;
    }
  }
}

int selectExpander(Walker *pWalker, Select *p) {
  Parse *pParse = pWalker->pParse;
  int i, j, k, rc;
  SrcList *pTabList;
  ExprList *pEList;
  SrcItem *pFrom;
  sqlite3 *db = pParse->db;
  Expr *pE, *pRight, *pExpr;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= 0x0000040;
  if (db->mallocFailed) {
    return 2;
  }

  if ((selFlags & 0x0000040) != 0) {
    return 1;
  }
  if (pWalker->eCode) {
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  if (pParse->pWith && (p->selFlags & 0x0200000)) {
    if (p->pWith == 0) {
      p->pWith = (With *)sqlite3DbMallocZero(db, (offsetof(With, a) + (1) * sizeof(Cte)));
      if (p->pWith == 0) {
        return 2;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3WithPush(pParse, p->pWith, 0);

  sqlite3SrcListAssignCursors(pParse, pTabList);

  for (i = 0, pFrom = pTabList->a; i < pTabList->nSrc; i++, pFrom++) {
    Table *pTab;

    if (pFrom->pSTab)
      continue;

    if (pFrom->zName == 0) {
      Select *pSel;

      pSel = pFrom->u4.pSubq->pSelect;

      if (sqlite3WalkSelect(pWalker, pSel))
        return 2;
      if (sqlite3ExpandSubquery(pParse, pFrom))
        return 2;

    } else if ((rc = resolveFromTermToCte(pParse, pWalker, pFrom)) != 0) {
      if (rc > 1)
        return 2;
      pTab = pFrom->pSTab;

    } else {
      pFrom->pSTab = pTab = sqlite3LocateTableItem(pParse, 0, pFrom);
      if (pTab == 0)
        return 2;
      if (pTab->nTabRef >= 0xffff) {
        sqlite3ErrorMsg(pParse, "too many references to \"%s\": max 65535", pTab->zName);
        pFrom->pSTab = 0;
        return 2;
      }
      pTab->nTabRef++;
      if (!((pTab)->eTabType == 1) && cannotBeFunction(pParse, pFrom)) {
        return 2;
      }

      if (!((pTab)->eTabType == 0)) {
        i16 nCol;
        u8 eCodeOrig = pWalker->eCode;
        if (sqlite3ViewGetColumnNames(pParse, pTab))
          return 2;

        if ((pTab)->eTabType == 2) {
          if ((db->flags & 0x80000000) == 0 && pTab->pSchema != db->aDb[1].pSchema) {
            sqlite3ErrorMsg(pParse, "access to view \"%s\" prohibited", pTab->zName);
          }
          sqlite3SrcItemAttachSubquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }

        else if ((((pTab)->eTabType == 1)) && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL)) &&
                 (pTab->u.vtab.p != 0) && pTab->u.vtab.p->eVtabRisk > ((db->flags & 0x00000080) != 0)) {
          sqlite3ErrorMsg(pParse, "unsafe use of virtual table \"%s\"", pTab->zName);
        }

        nCol = pTab->nCol;
        pTab->nCol = -1;
        pWalker->eCode = 1;
        if (pFrom->fg.isSubquery) {
          sqlite3WalkSelect(pWalker, pFrom->u4.pSubq->pSelect);
        }
        pWalker->eCode = eCodeOrig;
        pTab->nCol = nCol;
      }
    }

    if (pFrom->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pFrom)) {
      return 2;
    }
  }

  if (pParse->nErr || sqlite3ProcessJoin(pParse, p)) {
    return 2;
  }

  for (k = 0; k < pEList->nExpr; k++) {
    pE = pEList->a[k].pExpr;
    if (pE->op == 180)
      break;

    if (pE->op == 142 && pE->pRight->op == 180)
      break;
    elistFlags |= pE->flags;
  }
  if (k < pEList->nExpr) {
    struct ExprList_item *a = pEList->a;
    ExprList *pNew = 0;
    int flags = pParse->db->flags;
    int longNames = (flags & 0x00000004) != 0 && (flags & 0x00000040) == 0;

    for (k = 0; k < pEList->nExpr; k++) {
      pE = a[k].pExpr;
      elistFlags |= pE->flags;
      pRight = pE->pRight;

      if (pE->op != 180 && (pE->op != 142 || pRight->op != 180)) {
        pNew = sqlite3ExprListAppend(pParse, pNew, a[k].pExpr);
        if (pNew) {
          pNew->a[pNew->nExpr - 1].zEName = a[k].zEName;
          pNew->a[pNew->nExpr - 1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].pExpr = 0;
      } else {
        int tableSeen = 0;
        char *zTName = 0;
        int iErrOfst;
        if (pE->op == 142) {
          zTName = pE->pLeft->u.zToken;

          iErrOfst = pE->pRight->w.iOfst;
        } else {
          iErrOfst = pE->w.iOfst;
        }
        for (i = 0, pFrom = pTabList->a; i < pTabList->nSrc; i++, pFrom++) {
          int nAdd;
          Table *pTab = pFrom->pSTab;
          ExprList *pNestedFrom;
          char *zTabName;
          const char *zSchemaName = 0;
          int iDb;
          IdList *pUsing;

          if ((zTabName = pFrom->zAlias) == 0) {
            zTabName = pTab->zName;
          }
          if (db->mallocFailed)
            break;

          if (pFrom->fg.isNestedFrom) {
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;

          } else {
            if (zTName && sqlite3StrICmp(zTName, zTabName) != 0) {
              continue;
            }
            pNestedFrom = 0;
            iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
            zSchemaName = iDb >= 0 ? db->aDb[iDb].zDbSName : "*";
          }
          if (i + 1 < pTabList->nSrc && pFrom[1].fg.isUsing && (selFlags & 0x0000800) != 0) {
            int ii;
            pUsing = pFrom[1].u3.pUsing;
            for (ii = 0; ii < pUsing->nId; ii++) {
              const char *zUName = pUsing->a[ii].zName;
              pRight = sqlite3Expr(db, 60, zUName);
              sqlite3ExprSetErrorOffset(pRight, iErrOfst);
              pNew = sqlite3ExprListAppend(pParse, pNew, pRight);
              if (pNew) {
                struct ExprList_item *pX = &pNew->a[pNew->nExpr - 1];

                pX->zEName = sqlite3MPrintf(db, "..%s", zUName);
                pX->fg.eEName = 2;
                pX->fg.bUsingTerm = 1;
              }
            }
          } else {
            pUsing = 0;
          }

          nAdd = pTab->nCol;
          if ((((pTab)->tabFlags & 0x00000200) == 0) && (selFlags & 0x0000800) != 0)
            nAdd++;
          for (j = 0; j < nAdd; j++) {
            const char *zName;
            struct ExprList_item *pX;

            if (j == pTab->nCol) {
              zName = sqlite3RowidAlias(pTab);
              if (zName == 0)
                continue;
            } else {
              zName = pTab->aCol[j].zCnName;

              if (pNestedFrom && pNestedFrom->a[j].fg.eEName == 3) {
                continue;
              }

              if (zTName && pNestedFrom && sqlite3MatchEName(&pNestedFrom->a[j], 0, zTName, 0, 0) == 0) {
                continue;
              }

              if ((p->selFlags & 0x0020000) == 0 && (((&pTab->aCol[j])->colFlags & 0x0002) != 0)) {
                continue;
              }
              if ((pTab->aCol[j].colFlags & 0x0400) != 0 && zTName == 0 && (selFlags & (0x0000800)) == 0) {
                continue;
              }
            }

            tableSeen = 1;

            if (i > 0 && zTName == 0 && (selFlags & 0x0000800) == 0) {
              if (pFrom->fg.isUsing && sqlite3IdListIndex(pFrom->u3.pUsing, zName) >= 0) {
                continue;
              }
            }
            pRight = sqlite3Expr(db, 60, zName);
            if ((pTabList->nSrc > 1 && ((pFrom->fg.jointype & 0x40) == 0 || (selFlags & 0x0000800) != 0 ||
                                        !inAnyUsingClause(zName, pFrom, pTabList->nSrc - i - 1))) ||
                (pParse->eParseMode >= 2)) {
              Expr *pLeft;
              pLeft = sqlite3Expr(db, 60, zTabName);
              pExpr = sqlite3PExpr(pParse, 142, pLeft, pRight);
              if ((pParse->eParseMode >= 2) && pE->pLeft) {
                sqlite3RenameTokenRemap(pParse, pLeft, pE->pLeft);
              }
              if (zSchemaName) {
                pLeft = sqlite3Expr(db, 60, zSchemaName);
                pExpr = sqlite3PExpr(pParse, 142, pLeft, pExpr);
              }
            } else {
              pExpr = pRight;
            }
            sqlite3ExprSetErrorOffset(pExpr, iErrOfst);
            pNew = sqlite3ExprListAppend(pParse, pNew, pExpr);
            if (pNew == 0) {
              break;
            }
            pX = &pNew->a[pNew->nExpr - 1];

            if ((selFlags & 0x0000800) != 0 && !(pParse->eParseMode >= 2)) {
              if (pNestedFrom && (!0 || j < pNestedFrom->nExpr)) {
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
              } else {
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s", zSchemaName, zTabName, zName);
              }
              pX->fg.eEName = (j == pTab->nCol ? 3 : 2);
              if ((pFrom->fg.isUsing && sqlite3IdListIndex(pFrom->u3.pUsing, zName) >= 0) ||
                  (pUsing && sqlite3IdListIndex(pUsing, zName) >= 0) ||
                  (j < pTab->nCol && (pTab->aCol[j].colFlags & 0x0400))) {
                pX->fg.bNoExpand = 1;
              }
            } else if (longNames) {
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = 0;
            } else {
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = 0;
            }
          }
        }
        if (!tableSeen) {
          if (zTName) {
            sqlite3ErrorMsg(pParse, "no such table: %s", zTName);
          } else {
            sqlite3ErrorMsg(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3ExprListDelete(db, pEList);
    p->pEList = pNew;
  }
  if (p->pEList) {
    if (p->pEList->nExpr > db->aLimit[SQLITE_LIMIT_COLUMN]) {
      sqlite3ErrorMsg(pParse, "too many columns in result set");
      return 2;
    }
    if ((elistFlags & (0x000008 | 0x400000)) != 0) {
      p->selFlags |= 0x0040000;
    }
  }

  return 0;
}

void selectAddSubqueryTypeInfo(Walker *pWalker, Select *p) {
  Parse *pParse;
  int i;
  SrcList *pTabList;
  SrcItem *pFrom;

  if (p->selFlags & 0x0000080)
    return;
  p->selFlags |= 0x0000080;
  pParse = pWalker->pParse;

  pTabList = p->pSrc;
  for (i = 0, pFrom = pTabList->a; i < pTabList->nSrc; i++, pFrom++) {
    Table *pTab = pFrom->pSTab;

    if ((pTab->tabFlags & 0x00004000) != 0 && pFrom->fg.isSubquery) {
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3SubqueryColumnTypes(pParse, pTab, pSel, 0x40);
    }
  }
}

int aggregateIdxEprRefToColCallback(Walker *pWalker, Expr *pExpr) {
  AggInfo *pAggInfo;
  struct AggInfo_col *pCol;
  (void)(pWalker);
  if (pExpr->pAggInfo == 0)
    return 0;
  if (pExpr->op == 170)
    return 0;
  if (pExpr->op == 169)
    return 0;
  if (pExpr->op == 179)
    return 0;
  pAggInfo = pExpr->pAggInfo;
  if ((pExpr->iAgg >= pAggInfo->nColumn))
    return 0;

  pCol = &pAggInfo->aCol[pExpr->iAgg];
  pExpr->op = 170;
  pExpr->iTable = pCol->iTable;
  pExpr->iColumn = pCol->iColumn;
  (pExpr)->flags &= ~(u32)(0x002000 | 0x000200 | 0x080000);
  return 1;
}

int havingToWhereExprCb(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op != 44) {
    Select *pS = pWalker->u.pSelect;

    if (sqlite3ExprIsConstantOrGroupBy(pWalker->pParse, pExpr, pS->pGroupBy) &&
        (((pExpr)->flags & (0x000001 | 0x20000000)) == 0x20000000) == 0 && pExpr->pAggInfo == 0) {
      sqlite3 *db = pWalker->pParse->db;
      Expr *pNew = sqlite3ExprInt32(db, 1);
      if (pNew) {
        Expr *pWhere = pS->pWhere;
        {
          Expr t = *pNew;
          *pNew = *pExpr;
          *pExpr = t;
        };
        pNew = sqlite3ExprAnd(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->eCode = 1;
      }
    }
    return 1;
  }
  return 0;
}

int selectCheckOnClausesExpr(Walker *pWalker, Expr *pExpr) {
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  if (((((pExpr)->flags & (u32)(0x000001)) != 0)) ||
      ((((pExpr)->flags & (u32)(0x000002)) != 0) && (((pCtx->pSrc)->a[0].fg.jointype & 0x40) != 0))) {
    if (pCtx->iJoin == 0) {
      pCtx->iJoin = pExpr->w.iJoin;
      sqlite3WalkExprNN(pWalker, pExpr);
      pCtx->iJoin = 0;
      return 1;
    }
  }

  if (pExpr->op == 168) {
    do {
      SrcList *pSrc = pCtx->pSrc;
      int nSrc = pSrc->nSrc;
      int iTab = pExpr->iTable;
      int ii;
      for (ii = 0; ii < nSrc && pSrc->a[ii].iCursor != iTab; ii++) {
      }
      if (ii < nSrc) {
        if (pCtx->iJoin && iTab > pCtx->iJoin) {
          sqlite3ErrorMsg(pWalker->pParse, "%s references tables to its right",
                          (pCtx->bFuncArg ? "table-function argument" : "ON clause"));
          return 2;
        }
        break;
      }
      pCtx = pCtx->pParent;
    } while (pCtx);
  }
  return 0;
}

int selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect) {
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  if (pSelect->pSrc == pCtx->pSrc || pSelect->pSrc->nSrc == 0) {
    return 0;
  } else {
    CheckOnCtx sCtx;
    memset(&sCtx, 0, sizeof(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.pParent = pCtx;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~0x40000000;
    return 1;
  }
}

int sqlite3ReturningSubqueryVarSelect(Walker *NotUsed, Expr *pExpr) {
  (void)(NotUsed);
  if ((((pExpr)->flags & 0x001000) != 0) && (pExpr->x.pSelect->selFlags & 0x20000000) != 0) {
    (pExpr)->flags |= (u32)(0x000040);
  }
  return 0;
}

int sqlite3ReturningSubqueryCorrelated(Walker *pWalker, Select *pSelect) {
  int i;
  SrcList *pSrc;

  pSrc = pSelect->pSrc;

  for (i = 0; i < pSrc->nSrc; i++) {
    if (pSrc->a[i].pSTab == pWalker->u.pTab) {
      pSelect->selFlags |= 0x20000000;
      pWalker->eCode = 1;
      break;
    }
  }
  return 0;
}

int exprNodePatternLengthEst(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 118) {
    int sz = 0;
    u8 *z = (u8 *)pExpr->u.zToken;
    u8 c;
    u8 c1, c2, c3;
    if (pWalker->eCode) {
      c1 = '%';
      c2 = '_';
      c3 = 0;
    } else {
      c1 = '*';
      c2 = '?';
      c3 = '[';
    }
    while ((c = *(z++)) != 0) {
      if (c == c3) {
        if (*z)
          z++;
        while (*z && *z != ']')
          z++;
      } else if (c != c1 && c != c2) {
        sz++;
      }
    }
    if (sz > pWalker->u.sz)
      pWalker->u.sz = sz;
  }
  return 0;
}

int whereIsCoveringIndexWalkCallback(Walker *pWalk, Expr *pExpr) {
  int i;
  const Index *pIdx;
  const i16 *aiColumn;
  u16 nColumn;
  CoveringIndexCheck *pCk;

  pCk = pWalk->u.pCovIdxCk;
  pIdx = pCk->pIdx;
  if ((pExpr->op == 168 || pExpr->op == 170)) {
    if (pExpr->iTable != pCk->iTabCur)
      return 0;
    pIdx = pWalk->u.pCovIdxCk->pIdx;
    aiColumn = pIdx->aiColumn;
    nColumn = pIdx->nColumn;
    for (i = 0; i < nColumn; i++) {
      if (aiColumn[i] == pExpr->iColumn)
        return 0;
    }
    pCk->bUnidx = 1;
    return 2;
  } else if (pIdx->bHasExpr && exprIsCoveredByIndex(pExpr, pIdx, pWalk->u.pCovIdxCk->iTabCur)) {
    pCk->bExpr = 1;
    return 1;
  }
  return 0;
}

int exprNodeIsDeterministic(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 172 && (((pExpr)->flags & (u32)(0x100000)) != 0) == 0) {
    pWalker->eCode = 0;
    return 2;
  }
  return 0;
}

int selectWindowRewriteExprCb(Walker *pWalker, Expr *pExpr) {
  struct WindowRewrite *p = pWalker->u.pRewrite;
  Parse *pParse = pWalker->pParse;

  if (p->pSubSelect) {
    if (pExpr->op != 168) {
      return 0;
    } else {
      int nSrc = p->pSrc->nSrc;
      int i;
      for (i = 0; i < nSrc; i++) {
        if (pExpr->iTable == p->pSrc->a[i].iCursor)
          break;
      }
      if (i == nSrc)
        return 0;
    }
  }

  switch (pExpr->op) {
    case 172:
      if (!(((pExpr)->flags & (u32)(0x1000000)) != 0)) {
        break;
      } else {
        Window *pWin;
        for (pWin = p->pWin; pWin; pWin = pWin->pNextWin) {
          if (pExpr->y.pWin == pWin) {
            return 1;
          }
        }
      }
      __attribute__((fallthrough));

    case 179:
    case 169:
    case 168: {
      int iCol = -1;
      if (pParse->db->mallocFailed)
        return 2;
      if (p->pSub) {
        int i;
        for (i = 0; i < p->pSub->nExpr; i++) {
          if (0 == sqlite3ExprCompare(0, p->pSub->a[i].pExpr, pExpr, -1)) {
            iCol = i;
            break;
          }
        }
      }
      if (iCol < 0) {
        Expr *pDup = sqlite3ExprDup(pParse->db, pExpr, 0);
        if (pDup && pDup->op == 169)
          pDup->op = 172;
        p->pSub = sqlite3ExprListAppend(pParse, p->pSub, pDup);
      }
      if (p->pSub) {
        int f = pExpr->flags & 0x000200;

        (pExpr)->flags |= (u32)(0x8000000);
        sqlite3ExprDelete(pParse->db, pExpr);
        (pExpr)->flags &= ~(u32)(0x8000000);
        memset(pExpr, 0, sizeof(Expr));

        pExpr->op = 168;
        pExpr->iColumn = (iCol < 0 ? p->pSub->nExpr - 1 : iCol);
        pExpr->iTable = p->pWin->iEphCsr;
        pExpr->y.pTab = p->pTab;
        pExpr->flags = f;
      }
      if (pParse->db->mallocFailed)
        return 2;
      break;
    }

    default:
      break;
  }

  return 0;
}

int selectWindowRewriteSelectCb(Walker *pWalker, Select *pSelect) {
  struct WindowRewrite *p = pWalker->u.pRewrite;
  Select *pSave = p->pSubSelect;
  if (pSave == pSelect) {
    return 0;
  } else {
    p->pSubSelect = pSelect;
    sqlite3WalkSelect(pWalker, pSelect);
    p->pSubSelect = pSave;
  }
  return 1;
}

int sqlite3WindowExtraAggFuncDepth(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 169 && pExpr->op2 >= pWalker->walkerDepth) {
    pExpr->op2++;
  }
  return 0;
}

int disallowAggregatesInOrderByCb(Walker *pWalker, Expr *pExpr) {
  if (pExpr->op == 169 && pExpr->pAggInfo == 0) {
    sqlite3ErrorMsg(pWalker->pParse, "misuse of aggregate: %s()", pExpr->u.zToken);
  }
  return 0;
}