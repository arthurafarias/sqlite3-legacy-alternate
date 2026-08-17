#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sqlite/DateTime.h"

#include "sqlite/Sqlite3Config.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int osLocaltime(time_t *t, struct tm *pTm);

int getDigits(const char *zDate, const char *zFormat, ...) {

  static const u16 aMx[] = {12, 14, 24, 31, 59, 14712};
  va_list ap;
  int cnt = 0;
  char nextC;

  va_start(

      ap, zFormat

  )

      ;
  do {
    char N = zFormat[0] - '0';
    char min = zFormat[1] - '0';
    int val = 0;
    u16 max;

    ((void)(0))

        ;
    max = aMx[zFormat[2] - 'a'];
    nextC = zFormat[3];
    val = 0;
    while (N--) {
      if (!(sqlite3CtypeMap[(unsigned char)(*zDate)] & 0x04)) {
        goto end_getDigits;
      }
      val = val * 10 + *zDate - '0';
      zDate++;
    }
    if (val < (int)min || val > (int)max || (nextC != 0 && nextC != *zDate)) {
      goto end_getDigits;
    }
    *

        va_arg(

            ap

            ,

            int *

            )

        = val;
    zDate++;
    cnt++;
    zFormat += 4;
  } while (nextC);
end_getDigits:

  va_end(

      ap

  )

      ;
  return cnt;
}

int validJulianDay(sqlite3_int64 iJD) { return iJD >= 0 && iJD <= ((((i64)0x1a640) << 32) | 0x1072fdff); }

static int osLocaltime(time_t *t, struct tm *pTm) {
  int rc;

  if (sqlite3Config.bLocaltimeFault) {
    if (sqlite3Config.xAltLocaltime != 0) {
      return sqlite3Config.xAltLocaltime((const void *)t, (void *)pTm);
    } else {
      return 1;
    }
  }

  rc = localtime_r(t, pTm) == 0;

  return rc;
}


void datetimeError(DateTime *p) {
  memset(p, 0, sizeof(*p));
  p->isError = 1;
}

void computeJD(DateTime *p) {
  int Y, M, D, A, B, X1, X2;

  if (p->validJD)
    return;
  if (p->validYMD) {
    Y = p->Y;
    M = p->M;
    D = p->D;
  } else {
    Y = 2000;
    M = 1;
    D = 1;
  }
  if (Y < -4713 || Y > 9999 || p->rawS) {
    datetimeError(p);
    return;
  }
  if (M <= 2) {
    Y--;
    M += 12;
  }
  A = (Y + 4800) / 100;
  B = 38 - A + (A / 4);
  X1 = 36525 * (Y + 4716) / 100;
  X2 = 306001 * (M + 1) / 10000;
  p->iJD = (sqlite3_int64)((X1 + X2 + D + B - 1524.5) * 86400000);
  p->validJD = 1;
  if (p->validHMS) {
    p->iJD += p->h * 3600000 + p->m * 60000 + (sqlite3_int64)(p->s * 1000 + 0.5);
    if (p->tz) {
      p->iJD -= p->tz * 60000;
      p->validYMD = 0;
      p->validHMS = 0;
      p->tz = 0;
      p->isUtc = 1;
      p->isLocal = 0;
    }
  }
}

void computeFloor(DateTime *p) {

  if (p->D <= 28) {
    p->nFloor = 0;
  } else if ((1 << p->M) & 0x15aa) {
    p->nFloor = 0;
  } else if (p->M != 2) {
    p->nFloor = (p->D == 31);
  } else if (p->Y % 4 != 0 || (p->Y % 100 == 0 && p->Y % 400 != 0)) {
    p->nFloor = p->D - 28;
  } else {
    p->nFloor = p->D - 29;
  }
}

void setRawDateNumber(DateTime *p, double r) {
  p->s = r;
  p->rawS = 1;
  if (r >= 0.0 && r < 5373484.5) {
    p->iJD = (sqlite3_int64)(r * 86400000.0 + 0.5);
    p->validJD = 1;
  }
}

void computeYMD(DateTime *p) {
  int Z, alpha, A, B, C, D, E, X1;
  if (p->validYMD)
    return;
  if (!p->validJD) {
    p->Y = 2000;
    p->M = 1;
    p->D = 1;
  } else if (!validJulianDay(p->iJD)) {
    datetimeError(p);
    return;
  } else {
    Z = (int)((p->iJD + 43200000) / 86400000);
    alpha = (int)((Z + 32044.75) / 36524.25) - 52;
    A = Z + 1 + alpha - ((alpha + 100) / 4) + 25;
    B = A + 1524;
    C = (int)((B - 122.1) / 365.25);
    D = (36525 * (C & 32767)) / 100;
    E = (int)((B - D) / 30.6001);
    X1 = (int)(30.6001 * E);
    p->D = B - D - X1;
    p->M = E < 14 ? E - 1 : E - 13;
    p->Y = p->M > 2 ? C - 4716 : C - 4715;
  }
  p->validYMD = 1;
}

void computeHMS(DateTime *p) {
  int day_ms, day_min;
  if (p->validHMS)
    return;
  computeJD(p);
  day_ms = (int)((p->iJD + 43200000) % 86400000);
  p->s = (day_ms % 60000) / 1000.0;
  day_min = day_ms / 60000;
  p->m = day_min % 60;
  p->h = day_min / 60;
  p->rawS = 0;
  p->validHMS = 1;
}

void computeYMD_HMS(DateTime *p) {
  computeYMD(p);
  computeHMS(p);
}

void clearYMD_HMS_TZ(DateTime *p) {
  p->validYMD = 0;
  p->validHMS = 0;
  p->tz = 0;
}

int toLocaltime(DateTime *p, sqlite3_context *pCtx) {
  time_t t;
  struct tm sLocal;
  int iYearDiff;

  memset(&sLocal, 0, sizeof(sLocal));

  computeJD(p);
  if (p->iJD < 2108667600 * (i64)100000 || p->iJD > 2130141456 * (i64)100000) {

    DateTime x = *p;
    computeYMD_HMS(&x);
    iYearDiff = (2000 + x.Y % 4) - x.Y;
    x.Y += iYearDiff;
    x.validJD = 0;
    computeJD(&x);
    t = (time_t)(x.iJD / 1000 - 21086676 * (i64)10000);
  } else {
    iYearDiff = 0;
    t = (time_t)(p->iJD / 1000 - 21086676 * (i64)10000);
  }
  if (osLocaltime(&t, &sLocal)) {
    sqlite3_result_error(pCtx, "local time unavailable", -1);
    return 1;
  }
  p->Y = sLocal.tm_year + 1900 - iYearDiff;
  p->M = sLocal.tm_mon + 1;
  p->D = sLocal.tm_mday;
  p->h = sLocal.tm_hour;
  p->m = sLocal.tm_min;
  p->s = sLocal.tm_sec + (p->iJD % 1000) * 0.001;
  p->validYMD = 1;
  p->validHMS = 1;
  p->validJD = 0;
  p->rawS = 0;
  p->tz = 0;
  p->isError = 0;
  return 0;
}

void autoAdjustDate(DateTime *p) {
  if (!p->rawS || p->validJD) {
    p->rawS = 0;
  } else if (p->s >= -21086676 * (i64)10000 && p->s <= (25340230 * (i64)10000) + 799) {
    double r = p->s * 1000.0 + 210866760000000.0;
    clearYMD_HMS_TZ(p);
    p->iJD = (sqlite3_int64)(r + 0.5);
    p->validJD = 1;
    p->rawS = 0;
  }
}

int daysAfterJan01(DateTime *pDate) {
  DateTime jan01 = *pDate;

  jan01.validJD = 0;
  jan01.M = 1;
  jan01.D = 1;
  computeJD(&jan01);
  return (int)((pDate->iJD - jan01.iJD + 43200000) / 86400000);
}

int daysAfterMonday(DateTime *pDate) { return (int)((pDate->iJD + 43200000) / 86400000) % 7; }

int daysAfterSunday(DateTime *pDate) { return (int)((pDate->iJD + 129600000) / 86400000) % 7; }