#define _GNU_SOURCE 1
#include <math.h>
#include "sqlite/SumCtx.h"
#include "sqlite/i64.h"
void kahanBabuskaNeumaierStep(volatile SumCtx *pSum, double r) {
  volatile double s = pSum->rSum;
  volatile double vr = r;
  volatile double t = s + vr;
  if (fabs(s) > fabs(vr)) {
    pSum->rErr += (s - t) + vr;
  } else {
    pSum->rErr += (vr - t) + s;
  }
  pSum->rSum = t;
}

void kahanBabuskaNeumaierStepInt64(volatile SumCtx *pSum, i64 iVal) {
  if (iVal <= -4503599627370496LL || iVal >= +4503599627370496LL) {
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(pSum, iBig);
    kahanBabuskaNeumaierStep(pSum, iSm);
  } else {
    kahanBabuskaNeumaierStep(pSum, (double)iVal);
  }
}

void kahanBabuskaNeumaierInit(volatile SumCtx *p, i64 iVal) {
  if (iVal <= -4503599627370496LL || iVal >= +4503599627370496LL) {
    i64 iSm = iVal % 16384;
    p->rSum = (double)(iVal - iSm);
    p->rErr = (double)iSm;
  } else {
    p->rSum = (double)iVal;
    p->rErr = 0.0;
  }
}
