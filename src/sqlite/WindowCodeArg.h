
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/WindowCsrAndReg.h"
  typedef struct Parse Parse;
  typedef struct Vdbe Vdbe;
  typedef struct Window Window;

  typedef struct WindowCodeArg WindowCodeArg;

  struct WindowCodeArg {
    Parse *pParse;
    Window *pMWin;
    Vdbe *pVdbe;
    int addrGosub;
    int regGosub;
    int regArg;
    int eDelete;
    int regRowid;

    WindowCsrAndReg start;
    WindowCsrAndReg current;
    WindowCsrAndReg end;
  };

  void windowReadPeerValues(WindowCodeArg * p, int csr, int reg);
  void windowAggStep(WindowCodeArg * p, Window * pMWin, int csr, int bInverse, int reg);
  void windowAggFinal(WindowCodeArg * p, int bFin);
  void windowFullScan(WindowCodeArg * p);
  void windowReturnOneRow(WindowCodeArg * p);
  void windowCodeRangeTest(WindowCodeArg * p, int op, int csr1, int regVal, int csr2, int lbl);
  int windowCodeOp(WindowCodeArg * p, int op, int regCountdown, int jumpOnEof);

#ifdef __cplusplus
}
#endif
