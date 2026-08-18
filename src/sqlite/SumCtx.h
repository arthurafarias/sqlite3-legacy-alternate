
#pragma once

#include "sqlite/i64.h"
#include "sqlite/u8.h"
  struct SumCtx;
  struct SumCtx {
    double rSum;
    double rErr;
    i64 iSum;
    i64 cnt;
    u8 approx;
    u8 ovrfl;
  };

  void kahanBabuskaNeumaierStep(volatile SumCtx * pSum, double r);
  void kahanBabuskaNeumaierStepInt64(volatile SumCtx * pSum, i64 iVal);
  void kahanBabuskaNeumaierInit(volatile SumCtx * p, i64 iVal);


