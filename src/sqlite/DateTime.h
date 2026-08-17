
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/sqlite3_int64.h"
  typedef struct sqlite3_context sqlite3_context;

  typedef struct DateTime DateTime;
  struct DateTime {
    sqlite3_int64 iJD;
    int Y, M, D;
    int h, m;
    int tz;
    double s;
    char validJD;
    char validYMD;
    char validHMS;
    char nFloor;
    unsigned rawS : 1;
    unsigned isError : 1;
    unsigned useSubsec : 1;
    unsigned isUtc : 1;
    unsigned isLocal : 1;
  };

  void datetimeError(DateTime * p);
  void computeJD(DateTime * p);
  void computeFloor(DateTime * p);
  void clearYMD_HMS_TZ(DateTime * p);
  void setRawDateNumber(DateTime * p, double r);
  void computeYMD(DateTime * p);
  void computeHMS(DateTime * p);
  void computeYMD_HMS(DateTime * p);
  int toLocaltime(DateTime * p, sqlite3_context * pCtx);
  void autoAdjustDate(DateTime * p);
  int daysAfterJan01(DateTime * pDate);
  int daysAfterMonday(DateTime * pDate);
  int daysAfterSunday(DateTime * pDate);

  int getDigits(const char *zDate, const char *zFormat, ...);
  int validJulianDay(sqlite3_int64 iJD);

#ifdef __cplusplus
}
#endif
