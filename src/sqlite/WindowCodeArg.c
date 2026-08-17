#define _GNU_SOURCE 1

#include "sqlite/WindowCodeArg.h"

#include "sqlite/CollSeq.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FuncDef.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/Parse.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/Window.h"
#include "sqlite/WindowCsrAndReg.h"
#include "sqlite/WindowFunctionNames.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void windowReadPeerValues(WindowCodeArg *p, int csr, int reg) {
  Window *pMWin = p->pMWin;
  ExprList *pOrderBy = pMWin->pOrderBy;
  if (pOrderBy) {
    Vdbe *v = sqlite3GetVdbe(p->pParse);
    ExprList *pPart = pMWin->pPartition;
    int iColOff = pMWin->nBufferCol + (pPart ? pPart->nExpr : 0);
    int i;
    for (i = 0; i < pOrderBy->nExpr; i++) {
      sqlite3VdbeAddOp3(v, 96, csr, iColOff + i, reg + i);
    }
  }
}

void windowAggStep(WindowCodeArg *p, Window *pMWin, int csr, int bInverse, int reg) {
  Parse *pParse = p->pParse;
  Vdbe *v = sqlite3GetVdbe(pParse);
  Window *pWin;
  for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
    FuncDef *pFunc = pWin->pWFunc;
    int regArg;
    int nArg = pWin->bExprArgs ? 0 : windowArgCount(pWin);
    int i;
    int addrIf = 0;

    ((void)(0))

        ;

    ((void)(0))

        ;

    for (i = 0; i < nArg; i++) {
      if (i != 1 || pFunc->zName != nth_valueName) {
        sqlite3VdbeAddOp3(v, 96, csr, pWin->iArgCol + i, reg + i);
      } else {
        sqlite3VdbeAddOp3(v, 96, pMWin->iEphCsr, pWin->iArgCol + i, reg + i);
      }
    }
    regArg = reg;

    if (pWin->pFilter) {
      int regTmp;

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;
      regTmp = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, 96, csr, pWin->iArgCol + nArg, regTmp);
      addrIf = sqlite3VdbeAddOp3(v, 17, regTmp, 0, 1);
      ;
      sqlite3ReleaseTempReg(pParse, regTmp);
    }

    if (pMWin->regStartRowid == 0 && (pFunc->funcFlags & 0x1000) && (pWin->eStart != 91)) {
      int addrIsNull = sqlite3VdbeAddOp1(v, 51, regArg);
      ;
      if (bInverse == 0) {
        sqlite3VdbeAddOp2(v, 88, pWin->regApp + 1, 1);
        sqlite3VdbeAddOp2(v, 83, regArg, pWin->regApp);
        sqlite3VdbeAddOp3(v, 99, pWin->regApp, 2, pWin->regApp + 2);
        sqlite3VdbeAddOp2(v, 140, pWin->csrApp, pWin->regApp + 2);
      } else {
        sqlite3VdbeAddOp4Int(v, 23, pWin->csrApp, 0, regArg, 1);
        ;
        sqlite3VdbeAddOp1(v, 132, pWin->csrApp);
        sqlite3VdbeJumpHere(v, sqlite3VdbeCurrentAddr(v) - 2);
      }
      sqlite3VdbeJumpHere(v, addrIsNull);
    } else if (pWin->regApp) {

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;
      sqlite3VdbeAddOp2(v, 88, pWin->regApp + 1 - bInverse, 1);
    } else if (pFunc->xSFunc != noopStepFunc) {
      if (pWin->bExprArgs) {
        int iOp = sqlite3VdbeCurrentAddr(v);
        int iEnd;

        ((void)(0))

            ;
        nArg = pWin->pOwner->x.pList->nExpr;
        regArg = sqlite3GetTempRange(pParse, nArg);
        sqlite3ExprCodeExprList(pParse, pWin->pOwner->x.pList, regArg, 0, 0);

        for (iEnd = sqlite3VdbeCurrentAddr(v); iOp < iEnd; iOp++) {
          VdbeOp *pOp = sqlite3VdbeGetOp(v, iOp);
          if (pOp->opcode == 96 && pOp->p1 == pMWin->iEphCsr) {
            pOp->p1 = csr;
          }
        }
      }
      if (pFunc->funcFlags & 0x0020) {
        CollSeq *pColl;

        ((void)(0))

            ;

        ((void)(0))

            ;
        pColl = sqlite3ExprNNCollSeq(pParse, pWin->pOwner->x.pList->a[0].pExpr);
        sqlite3VdbeAddOp4(v, 87, 0, 0, 0, (const char *)pColl, (-2));
      }
      sqlite3VdbeAddOp3(v, bInverse ? 163 : 164, bInverse, regArg, pWin->regAccum);
      sqlite3VdbeAppendP4(v, pFunc, (-8));
      sqlite3VdbeChangeP5(v, (u16)nArg);
      if (pWin->bExprArgs) {
        sqlite3ReleaseTempRange(pParse, regArg, nArg);
      }
    }

    if (addrIf)
      sqlite3VdbeJumpHere(v, addrIf);
  }
}

void windowAggFinal(WindowCodeArg *p, int bFin) {
  Parse *pParse = p->pParse;
  Window *pMWin = p->pMWin;
  Vdbe *v = sqlite3GetVdbe(pParse);
  Window *pWin;

  for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
    if (pMWin->regStartRowid == 0 && (pWin->pWFunc->funcFlags & 0x1000) && (pWin->eStart != 91)) {
      sqlite3VdbeAddOp2(v, 77, 0, pWin->regResult);
      sqlite3VdbeAddOp1(v, 32, pWin->csrApp);
      ;
      sqlite3VdbeAddOp3(v, 96, pWin->csrApp, 0, pWin->regResult);
      sqlite3VdbeJumpHere(v, sqlite3VdbeCurrentAddr(v) - 2);
    } else if (pWin->regApp) {

      ((void)(0))

          ;
    } else {
      int nArg = windowArgCount(pWin);
      if (bFin) {
        sqlite3VdbeAddOp2(v, 167, pWin->regAccum, nArg);
        sqlite3VdbeAppendP4(v, pWin->pWFunc, (-8));
        sqlite3VdbeAddOp2(v, 82, pWin->regAccum, pWin->regResult);
        sqlite3VdbeAddOp2(v, 77, 0, pWin->regAccum);
      } else {
        sqlite3VdbeAddOp3(v, 166, pWin->regAccum, nArg, pWin->regResult);
        sqlite3VdbeAppendP4(v, pWin->pWFunc, (-8));
      }
    }
  }
}

void windowFullScan(WindowCodeArg *p) {
  Window *pWin;
  Parse *pParse = p->pParse;
  Window *pMWin = p->pMWin;
  Vdbe *v = p->pVdbe;

  int regCRowid = 0;
  int regCPeer = 0;
  int regRowid = 0;
  int regPeer = 0;

  int nPeer;
  int lblNext;
  int lblBrk;
  int addrNext;
  int csr;

  ;

  csr = pMWin->csrApp;
  nPeer = (pMWin->pOrderBy ? pMWin->pOrderBy->nExpr : 0);

  lblNext = sqlite3VdbeMakeLabel(pParse);
  lblBrk = sqlite3VdbeMakeLabel(pParse);

  regCRowid = sqlite3GetTempReg(pParse);
  regRowid = sqlite3GetTempReg(pParse);
  if (nPeer) {
    regCPeer = sqlite3GetTempRange(pParse, nPeer);
    regPeer = sqlite3GetTempRange(pParse, nPeer);
  }

  sqlite3VdbeAddOp2(v, 137, pMWin->iEphCsr, regCRowid);
  windowReadPeerValues(p, pMWin->iEphCsr, regCPeer);

  for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
    sqlite3VdbeAddOp2(v, 77, 0, pWin->regAccum);
  }

  sqlite3VdbeAddOp3(v, 23, csr, lblBrk, pMWin->regStartRowid);
  ;
  addrNext = sqlite3VdbeCurrentAddr(v);
  sqlite3VdbeAddOp2(v, 137, csr, regRowid);
  sqlite3VdbeAddOp3(v, 55, pMWin->regEndRowid, lblBrk, regRowid);
  ;

  if (pMWin->eExclude == 86) {
    sqlite3VdbeAddOp3(v, 54, regCRowid, lblNext, regRowid);
    ;
  } else if (pMWin->eExclude != 67) {
    int addr;
    int addrEq = 0;
    KeyInfo *pKeyInfo = 0;

    if (pMWin->pOrderBy) {
      pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pMWin->pOrderBy, 0, 0);
    }
    if (pMWin->eExclude == 95) {
      addrEq = sqlite3VdbeAddOp3(v, 54, regCRowid, 0, regRowid);
      ;
    }
    if (pKeyInfo) {
      windowReadPeerValues(p, csr, regPeer);
      sqlite3VdbeAddOp3(v, 92, regPeer, regCPeer, nPeer);
      sqlite3VdbeAppendP4(v, (void *)pKeyInfo, (-9));
      addr = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3VdbeAddOp3(v, 14, addr, lblNext, addr);
      ;
    } else {
      sqlite3VdbeAddOp2(v, 9, 0, lblNext);
    }
    if (addrEq)
      sqlite3VdbeJumpHere(v, addrEq);
  }

  windowAggStep(p, pMWin, csr, 0, p->regArg);

  sqlite3VdbeResolveLabel(v, lblNext);
  sqlite3VdbeAddOp2(v, 40, csr, addrNext);
  ;
  sqlite3VdbeJumpHere(v, addrNext - 1);
  sqlite3VdbeJumpHere(v, addrNext + 1);
  sqlite3ReleaseTempReg(pParse, regRowid);
  sqlite3ReleaseTempReg(pParse, regCRowid);
  if (nPeer) {
    sqlite3ReleaseTempRange(pParse, regPeer, nPeer);
    sqlite3ReleaseTempRange(pParse, regCPeer, nPeer);
  }

  windowAggFinal(p, 1);
  ;
}

void windowReturnOneRow(WindowCodeArg *p) {
  Window *pMWin = p->pMWin;
  Vdbe *v = p->pVdbe;

  if (pMWin->regStartRowid) {
    windowFullScan(p);
  } else {
    Parse *pParse = p->pParse;
    Window *pWin;

    for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
      FuncDef *pFunc = pWin->pWFunc;

      ((void)(0))

          ;
      if (pFunc->zName == nth_valueName || pFunc->zName == first_valueName) {
        int csr = pWin->csrApp;
        int lbl = sqlite3VdbeMakeLabel(pParse);
        int tmpReg = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, 77, 0, pWin->regResult);

        if (pFunc->zName == nth_valueName) {
          sqlite3VdbeAddOp3(v, 96, pMWin->iEphCsr, pWin->iArgCol + 1, tmpReg);
          windowCheckValue(pParse, tmpReg, 2);
        } else {
          sqlite3VdbeAddOp2(v, 73, 1, tmpReg);
        }
        sqlite3VdbeAddOp3(v, 107, tmpReg, pWin->regApp, tmpReg);
        sqlite3VdbeAddOp3(v, 55, pWin->regApp + 1, lbl, tmpReg);
        ;
        sqlite3VdbeAddOp3(v, 30, csr, 0, tmpReg);
        ;
        sqlite3VdbeAddOp3(v, 96, csr, pWin->iArgCol, pWin->regResult);
        sqlite3VdbeResolveLabel(v, lbl);
        sqlite3ReleaseTempReg(pParse, tmpReg);
      } else if (pFunc->zName == leadName || pFunc->zName == lagName) {
        int nArg = pWin->pOwner->x.pList->nExpr;
        int csr = pWin->csrApp;
        int lbl = sqlite3VdbeMakeLabel(pParse);
        int tmpReg = sqlite3GetTempReg(pParse);
        int iEph = pMWin->iEphCsr;

        if (nArg < 3) {
          sqlite3VdbeAddOp2(v, 77, 0, pWin->regResult);
        } else {
          sqlite3VdbeAddOp3(v, 96, iEph, pWin->iArgCol + 2, pWin->regResult);
        }
        sqlite3VdbeAddOp2(v, 137, iEph, tmpReg);
        if (nArg < 2) {
          int val = (pFunc->zName == leadName ? 1 : -1);
          sqlite3VdbeAddOp2(v, 88, tmpReg, val);
        } else {
          int op = (pFunc->zName == leadName ? 107 : 108);
          int tmpReg2 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp3(v, 96, iEph, pWin->iArgCol + 1, tmpReg2);
          sqlite3VdbeAddOp3(v, op, tmpReg2, tmpReg, tmpReg);
          sqlite3ReleaseTempReg(pParse, tmpReg2);
        }

        sqlite3VdbeAddOp3(v, 30, csr, lbl, tmpReg);
        ;
        sqlite3VdbeAddOp3(v, 96, csr, pWin->iArgCol, pWin->regResult);
        sqlite3VdbeResolveLabel(v, lbl);
        sqlite3ReleaseTempReg(pParse, tmpReg);
      }
    }
  }
  sqlite3VdbeAddOp2(v, 10, p->regGosub, p->addrGosub);
}

void windowCodeRangeTest(WindowCodeArg *p, int op, int csr1, int regVal, int csr2, int lbl) {
  Parse *pParse = p->pParse;
  Vdbe *v = sqlite3GetVdbe(pParse);
  ExprList *pOrderBy = p->pMWin->pOrderBy;
  int reg1 = sqlite3GetTempReg(pParse);
  int reg2 = sqlite3GetTempReg(pParse);
  int regString = ++pParse->nMem;
  int arith = 107;
  int addrGe;
  int addrDone = sqlite3VdbeMakeLabel(pParse);
  CollSeq *pColl;

  windowReadPeerValues(p, csr1, reg1);
  windowReadPeerValues(p, csr2, reg2);

  if (pOrderBy->a[0].fg.sortFlags & 0x01) {
    switch (op) {
    case 58:
      op = 56;
      break;
    case 55:
      op = 57;
      break;
    default:

      ((void)(0))

          ;
      op = 58;
      break;
    }
    arith = 108;
  }

  ;

  if (pOrderBy->a[0].fg.sortFlags & 0x02) {

    int addr = sqlite3VdbeAddOp1(v, 52, reg1);
    ;
    switch (op) {
    case 58:
      sqlite3VdbeAddOp2(v, 9, 0, lbl);
      break;
    case 55:
      sqlite3VdbeAddOp2(v, 52, reg2, lbl);
      ;
      break;
    case 56:
      sqlite3VdbeAddOp2(v, 51, reg2, lbl);
      ;
      break;
    default:

      ((void)(0))

          ;
      break;
    }
    sqlite3VdbeAddOp2(v, 9, 0, addrDone);

    sqlite3VdbeJumpHere(v, addr);
    sqlite3VdbeAddOp2(v, 51, reg2, (op == 55 || op == 58) ? addrDone : lbl);
    ;
  }

  sqlite3VdbeAddOp4(v, 118, 0, regString, 0, "", (-1));
  addrGe = sqlite3VdbeAddOp3(v, 58, regString, 0, reg1);
  ;
  if ((op == 58 && arith == 107) || (op == 56 && arith == 108)) {
    sqlite3VdbeAddOp3(v, op, reg2, lbl, reg1);
    ;
  }
  sqlite3VdbeAddOp3(v, arith, regVal, reg1, reg1);
  sqlite3VdbeJumpHere(v, addrGe);

  sqlite3VdbeAddOp3(v, op, reg2, lbl, reg1);
  ;
  pColl = sqlite3ExprNNCollSeq(pParse, pOrderBy->a[0].pExpr);
  sqlite3VdbeAppendP4(v, (void *)pColl, (-2));
  sqlite3VdbeChangeP5(v, 0x80);
  sqlite3VdbeResolveLabel(v, addrDone);

  ;
  ;
  ;
  ;
  ;
  ;
  ;
  ;
  sqlite3ReleaseTempReg(pParse, reg1);
  sqlite3ReleaseTempReg(pParse, reg2);

  ;
}

int windowCodeOp(WindowCodeArg *p, int op, int regCountdown, int jumpOnEof) {
  int csr, reg;
  Parse *pParse = p->pParse;
  Window *pMWin = p->pMWin;
  int ret = 0;
  Vdbe *v = p->pVdbe;
  int addrContinue = 0;
  int bPeer = (pMWin->eFrmType != 77);

  int lblDone = sqlite3VdbeMakeLabel(pParse);
  int addrNextRange = 0;

  if (op == 2 && pMWin->eStart == 91) {

    ((void)(0))

        ;
    return 0;
  }

  if (regCountdown > 0) {
    if (pMWin->eFrmType == 90) {
      addrNextRange = sqlite3VdbeCurrentAddr(v);

      ((void)(0))

          ;
      if (op == 2) {
        if (pMWin->eStart == 87) {
          windowCodeRangeTest(p, 56, p->current.csr, regCountdown, p->start.csr, lblDone);
        } else {
          windowCodeRangeTest(p, 58, p->start.csr, regCountdown, p->current.csr, lblDone);
        }
      } else {
        windowCodeRangeTest(p, 55, p->end.csr, regCountdown, p->current.csr, lblDone);
      }
    } else {
      sqlite3VdbeAddOp3(v, 61, regCountdown, lblDone, 1);
      ;
    }
  }

  if (op == 1 && pMWin->regStartRowid == 0) {
    windowAggFinal(p, 0);
  }
  addrContinue = sqlite3VdbeCurrentAddr(v);

  if (pMWin->eStart == pMWin->eEnd && regCountdown && pMWin->eFrmType == 90) {
    int regRowid1 = sqlite3GetTempReg(pParse);
    int regRowid2 = sqlite3GetTempReg(pParse);
    if (op == 2) {
      sqlite3VdbeAddOp2(v, 137, p->start.csr, regRowid1);
      sqlite3VdbeAddOp2(v, 137, p->end.csr, regRowid2);
      sqlite3VdbeAddOp3(v, 58, regRowid2, lblDone, regRowid1);
      ;
    } else if (p->regRowid) {
      sqlite3VdbeAddOp2(v, 137, p->end.csr, regRowid1);
      sqlite3VdbeAddOp3(v, 58, p->regRowid, lblDone, regRowid1);
      ;
    }
    sqlite3ReleaseTempReg(pParse, regRowid1);
    sqlite3ReleaseTempReg(pParse, regRowid2);

    ((void)(0))

        ;
  }

  switch (op) {
  case 1:
    csr = p->current.csr;
    reg = p->current.reg;
    windowReturnOneRow(p);
    break;

  case 2:
    csr = p->start.csr;
    reg = p->start.reg;
    if (pMWin->regStartRowid) {

      ((void)(0))

          ;
      sqlite3VdbeAddOp2(v, 88, pMWin->regStartRowid, 1);
    } else {
      windowAggStep(p, pMWin, csr, 1, p->regArg);
    }
    break;

  default:

    ((void)(0))

        ;
    csr = p->end.csr;
    reg = p->end.reg;
    if (pMWin->regStartRowid) {

      ((void)(0))

          ;
      sqlite3VdbeAddOp2(v, 88, pMWin->regEndRowid, 1);
    } else {
      windowAggStep(p, pMWin, csr, 0, p->regArg);
    }
    break;
  }

  if (op == p->eDelete) {
    sqlite3VdbeAddOp1(v, 132, csr);
    sqlite3VdbeChangeP5(v, 0x02);
  }

  if (jumpOnEof) {
    sqlite3VdbeAddOp2(v, 40, csr, sqlite3VdbeCurrentAddr(v) + 2);
    ;
    ret = sqlite3VdbeAddOp0(v, 9);
  } else {
    sqlite3VdbeAddOp2(v, 40, csr, sqlite3VdbeCurrentAddr(v) + 1 + bPeer);
    ;
    if (bPeer) {
      sqlite3VdbeAddOp2(v, 9, 0, lblDone);
    }
  }

  if (bPeer) {
    int nReg = (pMWin->pOrderBy ? pMWin->pOrderBy->nExpr : 0);
    int regTmp = (nReg ? sqlite3GetTempRange(pParse, nReg) : 0);
    windowReadPeerValues(p, csr, regTmp);
    windowIfNewPeer(pParse, pMWin->pOrderBy, regTmp, reg, addrContinue);
    sqlite3ReleaseTempRange(pParse, regTmp, nReg);
  }

  if (addrNextRange) {
    sqlite3VdbeAddOp2(v, 9, 0, addrNextRange);
  }
  sqlite3VdbeResolveLabel(v, lblDone);
  return ret;
}
