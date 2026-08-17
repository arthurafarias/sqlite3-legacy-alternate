#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/Expr.h"

#include "sqlite/Column.h"
#include "sqlite/ExprList.h"
#include "sqlite/IdxCover.h"
#include "sqlite/Index.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Table.h"
#include "sqlite/Vdbe.h"
#include "sqlite/Walker.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
void sqlite3DequoteExpr(Expr *p) {

  p->flags |= p->u.zToken[0] == '"' ? 0x4000000 | 0x000080 : 0x4000000;
  sqlite3Dequote(p->u.zToken);
}

void incrAggFunctionDepth(Expr *pExpr, int N) {
  if (N > 0) {
    Walker w;
    memset(&w, 0, sizeof(w));
    w.xExprCallback = incrAggDepth;
    w.u.n = N;
    sqlite3WalkExpr(&w, pExpr);
  }
}

Bitmask sqlite3ExprColUsed(Expr *pExpr) {
  int n;
  Table *pExTab;

  n = pExpr->iColumn;

  pExTab = pExpr->y.pTab;

  if ((pExTab->tabFlags & 0x00000060) != 0 && (pExTab->aCol[n].colFlags & 0x0060) != 0) {
    ;
    ;
    return pExTab->nCol >= ((int)(sizeof(Bitmask) * 8)) ? ((Bitmask)-1) : (((Bitmask)1) << (pExTab->nCol)) - 1;
  } else {
    ;
    ;
    if (n >= ((int)(sizeof(Bitmask) * 8)))
      n = ((int)(sizeof(Bitmask) * 8)) - 1;
    return ((Bitmask)1) << n;
  }
}

int exprProbability(Expr *p) {
  double r = -1.0;
  if (p->op != 154)
    return -1;

  sqlite3AtoF(p->u.zToken, &r);

  if (r > 1.0)
    return -1;
  return (int)(r * 134217728.0);
}

char sqlite3ExprAffinity(const Expr *pExpr) {
  int op;
  op = pExpr->op;
  while (1) {
    if (op == 168 || (op == 170 && pExpr->y.pTab != 0)) {

      ((void)(0))

          ;

      ((void)(0))

          ;
      return sqlite3TableColumnAffinity(pExpr->y.pTab, pExpr->iColumn);
    }
    if (op == 139) {

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;
      return sqlite3ExprAffinity(pExpr->x.pSelect->pEList->a[0].pExpr);
    }

    if (op == 36) {

      ((void)(0))

          ;
      return sqlite3AffinityType(pExpr->u.zToken, 0);
    }

    if (op == 178) {

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;
      return sqlite3ExprAffinity(pExpr->pLeft->x.pSelect->pEList->a[pExpr->iColumn].pExpr);
    }
    if (op == 177 || (op == 172 && pExpr->affExpr == 0x58)) {

      ((void)(0))

          ;
      return sqlite3ExprAffinity(pExpr->x.pList->a[0].pExpr);
    }
    if ((((pExpr)->flags & (u32)(0x002000 | 0x040000)) != 0)) {

      ((void)(0))

          ;
      pExpr = pExpr->pLeft;
      op = pExpr->op;
      continue;
    }
    if (op != 176)
      break;
    op = pExpr->op2;
    if ((op == 176))
      break;
  }
  return pExpr->affExpr;
}

int sqlite3ExprDataType(const Expr *pExpr) {
  while (pExpr) {
    switch (pExpr->op) {
    case 114:
    case 179:
    case 173: {
      pExpr = pExpr->pLeft;
      break;
    }
    case 122: {
      pExpr = 0;
      break;
    }
    case 118: {
      return 0x02;
    }
    case 155: {
      return 0x04;
    }
    case 112: {
      return 0x06;
    }
    case 157:
    case 169:
    case 172: {
      return 0x07;
    }
    case 168:
    case 170:
    case 139:
    case 36:
    case 178:
    case 177: {
      int aff = sqlite3ExprAffinity(pExpr);
      if (aff >= 0x43)
        return 0x05;
      if (aff == 0x42)
        return 0x06;
      return 0x07;
    }
    case 158: {
      int res = 0;
      int ii;
      ExprList *pList = pExpr->x.pList;

      ((void)(0))

          ;

      ((void)(0))

          ;
      for (ii = 1; ii < pList->nExpr; ii += 2) {
        res |= sqlite3ExprDataType(pList->a[ii].pExpr);
      }
      if (pList->nExpr % 2) {
        res |= sqlite3ExprDataType(pList->a[pList->nExpr - 1].pExpr);
      }
      return res;
    }
    default: {
      return 0x01;
    }
    }
  }
  return 0x00;
}

Expr *sqlite3ExprSkipCollate(Expr *pExpr) {
  while (pExpr && (((pExpr)->flags & (u32)(0x002000)) != 0)) {

    ((void)(0))

        ;
    pExpr = pExpr->pLeft;
  }
  return pExpr;
}

Expr *sqlite3ExprSkipCollateAndLikely(Expr *pExpr) {
  while (pExpr && (((pExpr)->flags & (u32)(0x002000 | 0x080000)) != 0)) {
    if ((((pExpr)->flags & (u32)(0x080000)) != 0)) {

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;
      pExpr = pExpr->x.pList->a[0].pExpr;
    } else if (pExpr->op == 114) {
      pExpr = pExpr->pLeft;
    } else {
      break;
    }
  }
  return pExpr;
}

char sqlite3CompareAffinity(const Expr *pExpr, char aff2) {
  char aff1 = sqlite3ExprAffinity(pExpr);
  if (aff1 > 0x40 && aff2 > 0x40) {

    if (((aff1) >= 0x43) || ((aff2) >= 0x43)) {
      return 0x43;
    } else {
      return 0x41;
    }
  } else {

    ((void)(0))

        ;
    return (aff1 <= 0x40 ? aff2 : aff1) | 0x40;
  }
}

char comparisonAffinity(const Expr *pExpr) {
  char aff;

  aff = sqlite3ExprAffinity(pExpr->pLeft);
  if (pExpr->pRight) {
    aff = sqlite3CompareAffinity(pExpr->pRight, aff);
  } else if ((((pExpr)->flags & 0x001000) != 0)) {
    aff = sqlite3CompareAffinity(pExpr->x.pSelect->pEList->a[0].pExpr, aff);
  } else if (aff == 0) {
    aff = 0x41;
  }
  return aff;
}

int sqlite3IndexAffinityOk(const Expr *pExpr, char idx_affinity) {
  char aff = comparisonAffinity(pExpr);
  if (aff < 0x42) {
    return 1;
  }
  if (aff == 0x42) {
    return idx_affinity == 0x42;
  }
  return ((idx_affinity) >= 0x43);
}

u8 binaryCompareP5(const Expr *pExpr1, const Expr *pExpr2, int jumpIfNull) {
  u8 aff = (char)sqlite3ExprAffinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  return aff;
}

int sqlite3ExprIsVector(const Expr *pExpr) { return sqlite3ExprVectorSize(pExpr) > 1; }

int sqlite3ExprVectorSize(const Expr *pExpr) {
  u8 op = pExpr->op;
  if (op == 176)
    op = pExpr->op2;
  if (op == 177) {

    ((void)(0))

        ;
    return pExpr->x.pList->nExpr;
  } else if (op == 139) {

    ((void)(0))

        ;
    return pExpr->x.pSelect->pEList->nExpr;
  } else {
    return 1;
  }
}

Expr *sqlite3VectorFieldSubexpr(Expr *pVector, int i) {

  if (sqlite3ExprIsVector(pVector)) {

    ((void)(0))

        ;
    if (pVector->op == 139 || pVector->op2 == 139) {

      ((void)(0))

          ;
      return pVector->x.pSelect->pEList->a[i].pExpr;
    } else {

      ((void)(0))

          ;
      return pVector->x.pList->a[i].pExpr;
    }
  }
  return pVector;
}

void heightOfExpr(const Expr *p, int *pnHeight) {
  if (p) {
    if (p->nHeight > *pnHeight) {
      *pnHeight = p->nHeight;
    }
  }
}

void exprSetHeight(Expr *p) {
  int nHeight = p->pLeft ? p->pLeft->nHeight : 0;
  if ((p->pRight) && p->pRight->nHeight > nHeight) {
    nHeight = p->pRight->nHeight;
  }
  if ((((p)->flags & 0x001000) != 0)) {
    heightOfSelect(p->x.pSelect, &nHeight);
  } else if (p->x.pList) {
    heightOfExprList(p->x.pList, &nHeight);
    p->flags |= (0x000200 | 0x400000 | 0x000008) & sqlite3ExprListFlags(p->x.pList);
  }
  p->nHeight = nHeight + 1;
}

void sqlite3ExprSetErrorOffset(Expr *pExpr, int iOfst) {
  if (pExpr == 0)
    return;
  if (((((pExpr)->flags & (0x000002 | 0x000001)) != 0)))
    return;
  pExpr->w.iOfst = iOfst;
}

int exprStructSize(const Expr *p) {
  if ((((p)->flags & (u32)(0x010000)) != 0))
    return

        __builtin_offsetof(

            Expr

            ,

            pLeft

        )

            ;
  if ((((p)->flags & (u32)(0x004000)) != 0))
    return

        __builtin_offsetof(

            Expr

            ,

            iTable

        )

            ;
  return sizeof(Expr);
}

int dupedExprStructSize(const Expr *p, int flags) {
  int nSize;

  if (0 == flags || (((p)->flags & (u32)(0x020000)) != 0)) {
    nSize = sizeof(Expr);
  } else {

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    if (p->pLeft || p->x.pList) {
      nSize =

          __builtin_offsetof(

              Expr

              ,

              iTable

              )

          | 0x004000;
    } else {

      ((void)(0))

          ;
      nSize =

          __builtin_offsetof(

              Expr

              ,

              pLeft

              )

          | 0x010000;
    }
  }
  return nSize;
}

int dupedExprNodeSize(const Expr *p, int flags) {
  int nByte = dupedExprStructSize(p, flags) & 0xfff;
  if (!(((p)->flags & (u32)(0x000800)) != 0) && p->u.zToken) {
    nByte += (strlen(p->u.zToken) & 0x3fffffff) + 1;
  }
  return (((nByte) + 7) & ~7);
}

int dupedExprSize(const Expr *p) {
  int nByte;

  nByte = dupedExprNodeSize(p, 0x0001);
  if (p->pLeft)
    nByte += dupedExprSize(p->pLeft);
  if (p->pRight)
    nByte += dupedExprSize(p->pRight);

  return nByte;
}

int sqlite3ExprIdToTrueFalse(Expr *pExpr) {
  u32 v;

  if (!(((pExpr)->flags & (u32)(0x4000000 | 0x000800)) != 0) && (v = sqlite3IsTrueOrFalse(pExpr->u.zToken)) != 0) {
    pExpr->op = 171;
    (pExpr)->flags |= (u32)(v);
    return 1;
  }
  return 0;
}

int sqlite3ExprTruthValue(const Expr *pExpr) {
  pExpr = sqlite3ExprSkipCollateAndLikely((Expr *)pExpr);

  return pExpr->u.zToken[4] == 0;
}

Expr *sqlite3ExprSimplifiedAndOr(Expr *pExpr) {

  if (pExpr->op == 44 || pExpr->op == 43) {
    Expr *pRight = sqlite3ExprSimplifiedAndOr(pExpr->pRight);
    Expr *pLeft = sqlite3ExprSimplifiedAndOr(pExpr->pLeft);
    if ((((pLeft)->flags & (0x000001 | 0x10000000)) == 0x10000000) || (((pRight)->flags & (0x000001 | 0x20000000)) == 0x20000000)) {
      pExpr = pExpr->op == 44 ? pRight : pLeft;
    } else if ((((pRight)->flags & (0x000001 | 0x10000000)) == 0x10000000) || (((pLeft)->flags & (0x000001 | 0x20000000)) == 0x20000000)) {
      pExpr = pExpr->op == 44 ? pLeft : pRight;
    }
  }
  return pExpr;
}

int exprEvalRhsFirst(Expr *pExpr) {
  if ((((pExpr->pLeft)->flags & (u32)(0x400000)) != 0) && !(((pExpr->pRight)->flags & (u32)(0x400000)) != 0)) {
    return 1;
  } else {
    return 0;
  }
}

int sqlite3ExprIsTableConstant(Expr *p, int iCur, int bAllowSubq) {
  Walker w;
  w.eCode = 3;
  w.pParse = 0;
  w.xExprCallback = exprNodeIsConstant;
  if (bAllowSubq) {
    w.xSelectCallback = exprSelectWalkTableConstant;
  } else {
    w.xSelectCallback = sqlite3SelectWalkFail;
  }
  w.u.iCur = iCur;
  sqlite3WalkExpr(&w, p);
  return w.eCode;
}

int sqlite3ExprIsSingleTableConstraint(Expr *pExpr, const SrcList *pSrcList, int iSrc, int bAllowSubq) {
  const SrcItem *pSrc = &pSrcList->a[iSrc];
  if (pSrc->fg.jointype & 0x40) {
    return 0;
  }
  if (pSrc->fg.jointype & 0x08) {
    if (!(((pExpr)->flags & (u32)(0x000001)) != 0))
      return 0;
    if (pExpr->w.iJoin != pSrc->iCursor)
      return 0;
  } else {
    if ((((pExpr)->flags & (u32)(0x000001)) != 0))
      return 0;
  }
  if ((((pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0) && (pSrcList->a[0].fg.jointype & 0x40) != 0) {
    int jj;
    for (jj = 0; jj < iSrc; jj++) {
      if (pExpr->w.iJoin == pSrcList->a[jj].iCursor) {
        if ((pSrcList->a[jj].fg.jointype & 0x40) != 0) {
          return 0;
        }
        break;
      }
    }
  }

  return sqlite3ExprIsTableConstant(pExpr, pSrc->iCursor, bAllowSubq);
}

int sqlite3ExprIsConstantOrFunction(Expr *p, u8 isInit) { return exprIsConst(0, p, 4 + isInit); }

int sqlite3ExprIsInteger(const Expr *p, int *pValue, Parse *pParse) {
  int rc = 0;
  if ((p == 0))
    return 0;

  if (p->flags & 0x000800) {
    *pValue = p->u.iValue;
    return 1;
  }
  switch (p->op) {
  case 173: {
    rc = sqlite3ExprIsInteger(p->pLeft, pValue, 0);
    break;
  }
  case 174: {
    int v = 0;
    if (sqlite3ExprIsInteger(p->pLeft, &v, 0)) {

      ((void)(0))

          ;
      *pValue = -v;
      rc = 1;
    }
    break;
  }
  case 157: {
    sqlite3_value *pVal;
    if (pParse == 0)
      break;
    if ((pParse->pVdbe == 0))
      break;
    if ((pParse->db->flags & 0x00800000) != 0)
      break;
    sqlite3VdbeSetVarmask(pParse->pVdbe, p->iColumn);
    pVal = sqlite3VdbeGetBoundValue(pParse->pReprepare, p->iColumn, 0x41);
    if (pVal) {
      if (sqlite3_value_type(pVal) == 1) {
        sqlite3_int64 vv = sqlite3_value_int64(pVal);
        if (vv == (vv & 0x7fffffff)) {
          *pValue = (int)vv;
          rc = 1;
        }
      }
      sqlite3ValueFree(pVal);
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

int sqlite3ExprCanBeNull(const Expr *p) {
  u8 op;

  while (p->op == 173 || p->op == 174) {
    p = p->pLeft;

    ((void)(0))

        ;
  }
  op = p->op;
  if (op == 176)
    op = p->op2;
  switch (op) {
  case 156:
  case 118:
  case 154:
  case 155:
    return 0;
  case 168:

    ((void)(0))

        ;
    return (((p)->flags & (u32)(0x200000)) != 0) || (p->y.pTab == 0)

           || (p->iColumn >= 0 && p->y.pTab->aCol != 0 && (p->iColumn < p->y.pTab->nCol) && p->y.pTab->aCol[p->iColumn].notNull == 0);
  default:
    return 1;
  }
}

int sqlite3ExprNeedsNoAffinityChange(const Expr *p, char aff) {
  u8 op;
  int unaryMinus = 0;
  if (aff == 0x41)
    return 1;
  while (p->op == 173 || p->op == 174) {
    if (p->op == 174)
      unaryMinus = 1;
    p = p->pLeft;
  }
  op = p->op;
  if (op == 176)
    op = p->op2;
  switch (op) {
  case 156: {
    return aff >= 0x43;
  }
  case 154: {
    return aff >= 0x43;
  }
  case 118: {
    return !unaryMinus && aff == 0x42;
  }
  case 155: {
    return !unaryMinus;
  }
  case 168: {

    ((void)(0))

        ;
    return aff >= 0x43 && p->iColumn < 0;
  }
  default: {
    return 0;
  }
  }
}

Select *isCandidateForInOpt(const Expr *pX) {
  Select *p;
  SrcList *pSrc;
  ExprList *pEList;
  Table *pTab;
  int i;
  if (!(((pX)->flags & 0x001000) != 0))
    return 0;
  if ((((pX)->flags & (u32)(0x000040)) != 0))
    return 0;
  p = pX->x.pSelect;
  if (p->pPrior)
    return 0;
  if (p->selFlags & (0x0000001 | 0x0000008)) {
    ;
    ;
    return 0;
  }

  if (p->pLimit)
    return 0;
  if (p->pWhere)
    return 0;
  pSrc = p->pSrc;

  if (pSrc->nSrc != 1)
    return 0;
  if (pSrc->a[0].fg.isSubquery)
    return 0;
  pTab = pSrc->a[0].pSTab;

  if (((pTab)->eTabType == 1))
    return 0;
  pEList = p->pEList;

  for (i = 0; i < pEList->nExpr; i++) {
    Expr *pRes = pEList->a[i].pExpr;
    if (pRes->op != 168)
      return 0;

    ((void)(0))

        ;
  }
  return p;
}

void sqlite3ExprToRegister(Expr *pExpr, int iReg) {
  Expr *p = sqlite3ExprSkipCollateAndLikely(pExpr);
  if ((p == 0))
    return;
  if (p->op == 176) {

    ((void)(0))

        ;
  } else {
    p->op2 = p->op;
    p->op = 176;
    p->iTable = iReg;
    (p)->flags &= ~(u32)(0x002000);
  }
}

int sqlite3ExprCompareSkip(Expr *pA, Expr *pB, int iTab) { return sqlite3ExprCompare(0, sqlite3ExprSkipCollate(pA), sqlite3ExprSkipCollate(pB), iTab); }

int sqlite3ExprIsNotTrue(Expr *pExpr) {
  int v;
  if (pExpr->op == 122)
    return 1;
  if (pExpr->op == 171 && sqlite3ExprTruthValue(pExpr) == 0)
    return 1;
  v = 1;
  if (sqlite3ExprIsInteger(pExpr, &v, 0) && v == 0)
    return 1;
  return 0;
}

int sqlite3ExprImpliesNonNullRow(Expr *p, int iTab, int isRJ) {
  Walker w;
  p = sqlite3ExprSkipCollateAndLikely(p);
  if (p == 0)
    return 0;
  if (p->op == 52) {
    p = p->pLeft;
  } else {
    while (p->op == 44) {
      if (sqlite3ExprImpliesNonNullRow(p->pLeft, iTab, isRJ))
        return 1;
      p = p->pRight;
    }
  }
  w.xExprCallback = impliesNotNullRow;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;
  w.eCode = 0;
  w.mWFlags = isRJ != 0;
  w.u.iCur = iTab;
  sqlite3WalkExpr(&w, p);
  return w.eCode;
}

int sqlite3ExprCoveredByIndex(Expr *pExpr, int iCur, Index *pIdx) {
  Walker w;
  struct IdxCover xcov;
  memset(&w, 0, sizeof(w));
  xcov.iCur = iCur;
  xcov.pIdx = pIdx;
  w.xExprCallback = exprIdxCover;
  w.u.pIdxCover = &xcov;
  sqlite3WalkExpr(&w, pExpr);
  return !w.eCode;
}

void sqlite3StringToId(Expr *p) {
  if (p->op == 118) {
    p->op = 60;
  } else if (p->op == 114 && p->pLeft->op == 118) {
    p->pLeft->op = 60;
  }
}

int sqlite3ExprReferencesUpdatedColumn(Expr *pExpr, int *aiChng, int chngRowid) {
  Walker w;
  memset(&w, 0, sizeof(w));
  w.eCode = 0;
  w.xExprCallback = checkConstraintExprNode;
  w.u.aiCol = aiChng;
  sqlite3WalkExpr(&w, pExpr);
  if (!chngRowid) {
    ;
    w.eCode &= ~0x02;
  };
  ;
  ;
  ;
  return w.eCode != 0;
}

void sqlite3SetJoinExpr(Expr *p, int iTable, u32 joinFlag) {

  while (p) {
    (p)->flags |= (u32)(joinFlag);

    ((void)(0))

        ;
    ;
    p->w.iJoin = iTable;
    if ((((p)->flags & 0x001000) == 0)) {
      if (p->x.pList) {
        int i;
        for (i = 0; i < p->x.pList->nExpr; i++) {
          sqlite3SetJoinExpr(p->x.pList->a[i].pExpr, iTable, joinFlag);
        }
      }
    }
    sqlite3SetJoinExpr(p->pLeft, iTable, joinFlag);
    p = p->pRight;
  }
}

void unsetJoinExpr(Expr *p, int iTable, int nullable) {
  while (p) {
    if (iTable < 0 || ((((p)->flags & (u32)(0x000001)) != 0) && p->w.iJoin == iTable)) {
      (p)->flags &= ~(u32)(0x000001 | 0x000002);
      if (iTable >= 0)
        (p)->flags |= (u32)(0x000002);
    }
    if (p->op == 168 && p->iTable == iTable && !nullable) {
      (p)->flags &= ~(u32)(0x200000);
    }
    if (p->op == 172) {

      ((void)(0))

          ;

      ((void)(0))

          ;
      if (p->x.pList) {
        int i;
        for (i = 0; i < p->x.pList->nExpr; i++) {
          unsetJoinExpr(p->x.pList->a[i].pExpr, iTable, nullable);
        }
      }
    }
    unsetJoinExpr(p->pLeft, iTable, nullable);
    p = p->pRight;
  }
}

void updateRangeAffinityStr(Expr *pRight, int n, char *zAff) {
  int i;
  for (i = 0; i < n; i++) {
    Expr *p = sqlite3VectorFieldSubexpr(pRight, i);
    if (sqlite3CompareAffinity(p, zAff[i]) == 0x41 || sqlite3ExprNeedsNoAffinityChange(p, zAff[i])) {
      zAff[i] = 0x41;
    }
  }
}

void whereApplyPartialIndexConstraints(Expr *pTruth, int iTabCur, WhereClause *pWC) {
  int i;
  WhereTerm *pTerm;
  while (pTruth->op == 44) {
    whereApplyPartialIndexConstraints(pTruth->pLeft, iTabCur, pWC);
    pTruth = pTruth->pRight;
  }
  for (i = 0, pTerm = pWC->a; i < pWC->nTerm; i++, pTerm++) {
    Expr *pExpr;
    if (pTerm->wtFlags & 0x0004)
      continue;
    pExpr = pTerm->pExpr;
    if (sqlite3ExprCompare(0, pExpr, pTruth, iTabCur) == 0) {
      pTerm->wtFlags |= 0x0004;
    }
  }
}

int sqlite3ExprIsLikeOperator(const Expr *pExpr) {
  static const struct {
    const char *zOp;
    unsigned char eOp;
  } aOp[] = {{"match", 64}, {"glob", 66}, {"like", 65}, {"regexp", 67}};
  int i;

  for (i = 0; i < ((int)(sizeof(aOp) / sizeof(aOp[0]))); i++) {
    if (sqlite3StrICmp(pExpr->u.zToken, aOp[i].zOp) == 0) {
      return aOp[i].eOp;
    }
  }
  return 0;
}

void transferJoinMarkings(Expr *pDerived, Expr *pBase) {
  if (pDerived && (((pBase)->flags & (u32)(0x000001 | 0x000002)) != 0)) {
    pDerived->flags |= pBase->flags & (0x000001 | 0x000002);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

Expr *whereRightSubexprIsColumn(Expr *p) {
  p = sqlite3ExprSkipCollateAndLikely(p->pRight);
  if ((p != 0) && p->op == 168 && !(((p)->flags & (u32)(0x000020)) != 0)) {
    return p;
  }
  return 0;
}

int estLikePatternLength(Expr *p, u16 eCode) {
  Walker w;
  w.u.sz = 0;
  w.eCode = eCode;
  w.xExprCallback = exprNodePatternLengthEst;
  w.xSelectCallback = sqlite3SelectWalkFail;

  sqlite3WalkExpr(&w, p);
  return w.u.sz;
}

int exprIsCoveredByIndex(const Expr *pExpr, const Index *pIdx, int iTabCur) {
  int i;
  for (i = 0; i < pIdx->nColumn; i++) {
    if (pIdx->aiColumn[i] == (-2) && sqlite3ExprCompare(0, pExpr, pIdx->aColExpr->a[i].pExpr, iTabCur) == 0) {
      return 1;
    }
  }
  return 0;
}

int exprIsDeterministic(Expr *p) {
  Walker w;
  memset(&w, 0, sizeof(w));
  w.eCode = 1;
  w.xExprCallback = exprNodeIsDeterministic;
  w.xSelectCallback = sqlite3SelectWalkFail;
  sqlite3WalkExpr(&w, p);
  return w.eCode;
}
