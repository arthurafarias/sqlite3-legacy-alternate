#define _GNU_SOURCE 1
#include "sqlite/SelectDest.h"
#include "sqlite/u8.h"
void sqlite3SelectDestInit(SelectDest *pDest, int eDest, int iParm) {
  pDest->eDest = (u8)eDest;
  pDest->iSDParm = iParm;
  pDest->iSDParm2 = 0;
  pDest->zAffSdst = 0;
  pDest->iSdst = 0;
  pDest->nSdst = 0;
}
