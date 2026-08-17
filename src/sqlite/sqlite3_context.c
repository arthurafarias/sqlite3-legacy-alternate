#define _GNU_SOURCE 1

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sqlite/sqlite3_context.h"

#include "sqlite/AuxData.h"
#include "sqlite/Btree.h"
#include "sqlite/CallCount.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/CountCtx.h"
#include "sqlite/DateTime.h"
#include "sqlite/Db.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FKey.h"
#include "sqlite/FuncDef.h"
#include "sqlite/GroupConcatCtx.h"
#include "sqlite/Hash.h"
#include "sqlite/HashElem.h"
#include "sqlite/IdList.h"
#include "sqlite/Index.h"
#include "sqlite/JsonCache.h"
#include "sqlite/JsonParse.h"
#include "sqlite/JsonPretty.h"
#include "sqlite/JsonString.h"
#include "sqlite/LastValueCtx.h"
#include "sqlite/Mem.h"
#include "sqlite/NameContext.h"
#include "sqlite/NthValueCtx.h"
#include "sqlite/NtileCtx.h"
#include "sqlite/Op.h"
#include "sqlite/Pager.h"
#include "sqlite/Parse.h"
#include "sqlite/Percentile.h"
#include "sqlite/PrintfArguments.h"
#include "sqlite/RCStr.h"
#include "sqlite/RenameCtx.h"
#include "sqlite/RenameToken.h"
#include "sqlite/RowSet.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/StatAccum.h"
#include "sqlite/StatSample.h"
#include "sqlite/StrAccum.h"
#include "sqlite/SumCtx.h"
#include "sqlite/Table.h"
#include "sqlite/Token.h"
#include "sqlite/Trigger.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/Upsert.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/Walker.h"
#include "sqlite/compareInfo.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_libversion.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite3_xauth.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/tRowcnt.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/aXformType_t.h"

/* Private helpers, formerly declared in _Uncategorized.h. */
static int getConstraint(const u8 *z);
static int getConstraintToken(const u8 *z, int *piToken);
static int getWhitespace(const u8 *z);
static int invokeValueDestructor(const void *p, void (*xDel)(void *), sqlite3_context *pCtx);
static int isNHex(const char *z, int N, u32 *pVal);
static int jsonAllAlphanum(const char *z, int n);
static int percentIsInfinity(double r);
static int percentSameValue(double a, double b);
static void percentSort(double *a, unsigned int n);
static int strContainsChar(const u8 *zStr, int nStr, u32 ch);

static const aXformType_t aXformType[] = {
    {6, "second", 4.6427e+14, 1.0}, {6, "minute", 7.7379e+12, 60.0}, {4, "hour", 1.2897e+11, 3600.0}, {3, "day", 5373485.0, 86400.0}, {5, "month", 176546.0, 2592000.0}, {4, "year", 14713.0, 31536000.0},
};

static int invokeValueDestructor(const void *p, void (*xDel)(void *), sqlite3_context *pCtx) {

  if (xDel == 0) {

  } else if (xDel == ((sqlite3_destructor_type)-1)) {

  } else {
    xDel((void *)p);
  }

  sqlite3_result_error_toobig(pCtx);

  return 18;
}

static int getConstraintToken(const u8 *z, int *piToken) {
  int iOff = 0;
  int t = 0;
  do {
    iOff += sqlite3GetToken(&z[iOff], &t);
  } while (t == 184 || t == 185);

  *piToken = t;

  if (t == 22) {
    int nNest = 1;
    while (nNest > 0) {
      iOff += sqlite3GetToken(&z[iOff], &t);
      if (t == 22) {
        nNest++;
      } else if (t == 23) {
        t = 22;
        nNest--;
      } else if (t == 186) {
        break;
      }
    }
  }

  *piToken = t;
  return iOff;
}

static int getWhitespace(const u8 *z) {
  int nRet = 0;
  while (1) {
    int t = 0;
    int n = sqlite3GetToken(&z[nRet], &t);
    if (t != 184 && t != 185)
      break;
    nRet += n;
  }
  return nRet;
}

static int getConstraint(const u8 *z) {
  int iOff = 0;
  int t = 0;

  while (1) {
    int n = getConstraintToken(&z[iOff], &t);
    if (t == 120 || t == 123 || t == 19 || t == 124 || t == 125 || t == 121 || t == 114 || t == 126 || t == 133 || t == 23 || t == 25 || t == 186 || t == 24 || t == 96) {
      break;
    }
    iOff += n;
  }

  return iOff;
}

int patternCompare(const u8 *zPattern, const u8 *zString, const struct compareInfo *pInfo, u32 matchOther) {
  u32 c, c2;
  u32 matchOne = pInfo->matchOne;
  u32 matchAll = pInfo->matchAll;
  u8 noCase = pInfo->noCase;
  const u8 *zEscaped = 0;

  while ((c = (zPattern[0] < 0x80 ? *(zPattern++) : sqlite3Utf8Read(&zPattern))) != 0) {
    if (c == matchAll) {

      while ((c = (zPattern[0] < 0x80 ? *(zPattern++) : sqlite3Utf8Read(&zPattern))) == matchAll || (c == matchOne && matchOne != 0)) {
        if (c == matchOne && sqlite3Utf8Read(&zString) == 0) {
          return 2;
        }
      }
      if (c == 0) {
        return 0;
      } else if (c == matchOther) {
        if (pInfo->matchSet == 0) {
          c = sqlite3Utf8Read(&zPattern);
          if (c == 0)
            return 2;
        } else {

          ((void)(0))

              ;
          while (*zString) {
            int bMatch = patternCompare(&zPattern[-1], zString, pInfo, matchOther);
            if (bMatch != 1)
              return bMatch;
            {
              if ((*(zString++)) >= 0xc0) {
                while ((*zString & 0xc0) == 0x80) {
                  zString++;
                }
              }
            };
          }
          return 2;
        }
      }

      if (c < 0x80) {
        char zStop[3];
        int bMatch;
        if (noCase) {
          zStop[0] = ((c) & ~(sqlite3CtypeMap[(unsigned char)(c)] & 0x20));
          zStop[1] = (sqlite3UpperToLower[(unsigned char)(c)]);
          zStop[2] = 0;
        } else {
          zStop[0] = c;
          zStop[1] = 0;
        }
        while (1) {
          zString += strcspn((const char *)zString, zStop);
          if (zString[0] == 0)
            break;
          zString++;
          bMatch = patternCompare(zPattern, zString, pInfo, matchOther);
          if (bMatch != 1)
            return bMatch;
        }
      } else {
        int bMatch;
        while ((c2 = (zString[0] < 0x80 ? *(zString++) : sqlite3Utf8Read(&zString))) != 0) {
          if (c2 != c)
            continue;
          bMatch = patternCompare(zPattern, zString, pInfo, matchOther);
          if (bMatch != 1)
            return bMatch;
        }
      }
      return 2;
    }
    if (c == matchOther) {
      if (pInfo->matchSet == 0) {
        c = sqlite3Utf8Read(&zPattern);
        if (c == 0)
          return 1;
        zEscaped = zPattern;
      } else {
        u32 prior_c = 0;
        int seen = 0;
        int invert = 0;
        c = sqlite3Utf8Read(&zString);
        if (c == 0)
          return 1;
        c2 = sqlite3Utf8Read(&zPattern);
        if (c2 == '^') {
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        if (c2 == ']') {
          if (c == ']')
            seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        while (c2 && c2 != ']') {
          if (c2 == '-' && zPattern[0] != ']' && zPattern[0] != 0 && prior_c > 0) {
            c2 = sqlite3Utf8Read(&zPattern);
            if (c >= prior_c && c <= c2)
              seen = 1;
            prior_c = 0;
          } else {
            if (c == c2) {
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        if (c2 == 0 || (seen ^ invert) == 0) {
          return 1;
        }
        continue;
      }
    }
    c2 = (zString[0] < 0x80 ? *(zString++) : sqlite3Utf8Read(&zString));
    if (c == c2)
      continue;
    if (noCase && (sqlite3UpperToLower[(unsigned char)(c)]) == (sqlite3UpperToLower[(unsigned char)(c2)]) && c < 0x80 && c2 < 0x80) {
      continue;
    }
    if (c == matchOne && zPattern != zEscaped && c2 != 0)
      continue;
    return 1;
  }
  return *zString == 0 ? 0 : 1;
}

static int isNHex(const char *z, int N, u32 *pVal) {
  int i;
  u32 v = 0;
  for (i = 0; i < N; i++) {
    if (!(sqlite3CtypeMap[(unsigned char)(z[i])] & 0x08))
      return 0;
    v = (v << 4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  return 1;
}

static int strContainsChar(const u8 *zStr, int nStr, u32 ch) {
  const u8 *zEnd = &zStr[nStr];
  const u8 *z = zStr;
  while (z < zEnd) {
    u32 tst = (z[0] < 0x80 ? *(z++) : sqlite3Utf8Read(&z));
    if (tst == ch)
      return 1;
  }
  return 0;
}

static int percentIsInfinity(double r) {
  sqlite3_uint64 u;

  memcpy(&u, &r, sizeof(u));
  return ((u >> 52) & 0x7ff) == 0x7ff;
}

static int percentSameValue(double a, double b) {
  a -= b;
  return a >= -0.001 && a <= 0.001;
}

static void percentSort(double *a, unsigned int n) {
  int iLt;
  int iGt;
  int i;
  double rPivot;

  while (n >= 2) {
    if (a[0] > a[n - 1]) {
      {
        double ttt = (a[0]);
        (a[0]) = (a[n - 1]);
        (a[n - 1]) = ttt;
      }
    }
    if (n == 2)
      return;
    iGt = n - 1;
    i = n / 2;
    if (a[0] > a[i]) {
      {
        double ttt = (a[0]);
        (a[0]) = (a[i]);
        (a[i]) = ttt;
      }
    } else if (a[i] > a[iGt]) {
      {
        double ttt = (a[i]);
        (a[i]) = (a[iGt]);
        (a[iGt]) = ttt;
      }
    }
    if (n == 3)
      return;
    rPivot = a[i];
    iLt = i = 1;
    do {
      if (a[i] < rPivot) {
        if (i > iLt) {
          double ttt = (a[i]);
          (a[i]) = (a[iLt]);
          (a[iLt]) = ttt;
        }
        iLt++;
        i++;
      } else if (a[i] > rPivot) {
        do {
          iGt--;
        } while (iGt > i && a[iGt] > rPivot);
        {
          double ttt = (a[i]);
          (a[i]) = (a[iGt]);
          (a[iGt]) = ttt;
        }
      } else {
        i++;
      }
    } while (i < iGt);
    if (iLt > (int)(n / 2)) {
      if (n - iGt >= 2)
        percentSort(a + iGt, n - iGt);
      n = iLt;
    } else {
      if (iLt >= 2)
        percentSort(a, iLt);
      a += iGt;
      n -= iGt;
    }
  }
}

static int jsonAllAlphanum(const char *z, int n) {
  int i;
  for (i = 0; i < n && ((sqlite3CtypeMap[(unsigned char)(z[i])] & 0x06) || z[i] == '_'); i++) {
  }
  return i == n;
}


int setDateTimeToCurrent(sqlite3_context *context, DateTime *p) {
  p->iJD = sqlite3StmtCurrentTime(context);
  if (p->iJD > 0) {
    p->validJD = 1;
    p->isUtc = 1;
    p->isLocal = 0;
    clearYMD_HMS_TZ(p);
    return 0;
  } else {
    return 1;
  }
}

int parseDateOrTime(sqlite3_context *context, const char *zDate, DateTime *p) {
  double r;
  if (parseYyyyMmDd(zDate, p) == 0) {
    return 0;
  } else if (parseHhMmSs(zDate, p) == 0) {
    return 0;
  } else if (sqlite3StrICmp(zDate, "now") == 0 && sqlite3NotPureFunc(context)) {
    return setDateTimeToCurrent(context, p);
  } else if (sqlite3AtoF(zDate, &r) > 0) {
    setRawDateNumber(p, r);
    return 0;
  } else if ((sqlite3StrICmp(zDate, "subsec") == 0 || sqlite3StrICmp(zDate, "subsecond") == 0) && sqlite3NotPureFunc(context)) {
    p->useSubsec = 1;
    return setDateTimeToCurrent(context, p);
  }
  return 1;
}

int parseModifier(sqlite3_context *pCtx, const char *z, int n, DateTime *p, int idx) {
  int rc = 1;
  double r;
  switch (sqlite3UpperToLower[(u8)z[0]]) {
  case 'a': {

    if (sqlite3_stricmp(z, "auto") == 0) {
      if (idx > 1)
        return 1;
      autoAdjustDate(p);
      rc = 0;
    }
    break;
  }
  case 'c': {

    if (sqlite3_stricmp(z, "ceiling") == 0) {
      computeJD(p);
      clearYMD_HMS_TZ(p);
      rc = 0;
      p->nFloor = 0;
    }
    break;
  }
  case 'f': {

    if (sqlite3_stricmp(z, "floor") == 0) {
      computeJD(p);
      p->iJD -= p->nFloor * 86400000;
      clearYMD_HMS_TZ(p);
      rc = 0;
    }
    break;
  }
  case 'j': {

    if (sqlite3_stricmp(z, "julianday") == 0) {
      if (idx > 1)
        return 1;
      if (p->validJD && p->rawS) {
        rc = 0;
        p->rawS = 0;
      }
    }
    break;
  }

  case 'l': {

    if (sqlite3_stricmp(z, "localtime") == 0 && sqlite3NotPureFunc(pCtx)) {
      rc = p->isLocal ? 0 : toLocaltime(p, pCtx);
      p->isUtc = 0;
      p->isLocal = 1;
    }
    break;
  }

  case 'u': {

    if (sqlite3_stricmp(z, "unixepoch") == 0 && p->rawS) {
      if (idx > 1)
        return 1;
      r = p->s * 1000.0 + 210866760000000.0;
      if (r >= 0.0 && r < 464269060800000.0) {
        clearYMD_HMS_TZ(p);
        p->iJD = (sqlite3_int64)(r + 0.5);
        p->validJD = 1;
        p->rawS = 0;
        rc = 0;
      }
    }

    else if (sqlite3_stricmp(z, "utc") == 0 && sqlite3NotPureFunc(pCtx)) {
      if (p->isUtc == 0) {
        i64 iOrigJD;
        i64 iGuess;
        int cnt = 0;
        i64 iErr;

        computeJD(p);
        iGuess = iOrigJD = p->iJD;
        iErr = 0;
        do {
          DateTime new;
          memset(&new, 0, sizeof(new));
          iGuess -= iErr;
          new.iJD = iGuess;
          new.validJD = 1;
          rc = toLocaltime(&new, pCtx);
          if (rc)
            return rc;
          computeJD(&new);
          iErr = new.iJD - iOrigJD;
        } while (iErr && cnt++ < 3);
        memset(p, 0, sizeof(*p));
        p->iJD = iGuess;
        p->validJD = 1;
        p->isUtc = 1;
        p->isLocal = 0;
      }
      rc = 0;
    }

    break;
  }
  case 'w': {

    if (sqlite3_strnicmp(z, "weekday ", 8) == 0 && sqlite3AtoF(&z[8], &r) > 0 && r >= 0.0 && r < 7.0 && (n = (int)r) == r) {
      sqlite3_int64 Z;
      computeYMD_HMS(p);
      p->tz = 0;
      p->validJD = 0;
      computeJD(p);
      Z = ((p->iJD + 129600000) / 86400000) % 7;
      if (Z > n)
        Z -= 7;
      p->iJD += (n - Z) * 86400000;
      clearYMD_HMS_TZ(p);
      rc = 0;
    }
    break;
  }
  case 's': {

    if (sqlite3_strnicmp(z, "start of ", 9) != 0) {
      if (sqlite3_stricmp(z, "subsec") == 0 || sqlite3_stricmp(z, "subsecond") == 0) {
        p->useSubsec = 1;
        rc = 0;
      }
      break;
    }
    if (!p->validJD && !p->validYMD && !p->validHMS)
      break;
    z += 9;
    computeYMD(p);
    p->validHMS = 1;
    p->h = p->m = 0;
    p->s = 0.0;
    p->rawS = 0;
    p->tz = 0;
    p->validJD = 0;
    if (sqlite3_stricmp(z, "month") == 0) {
      p->D = 1;
      rc = 0;
    } else if (sqlite3_stricmp(z, "year") == 0) {
      p->M = 1;
      p->D = 1;
      rc = 0;
    } else if (sqlite3_stricmp(z, "day") == 0) {
      rc = 0;
    }
    break;
  }
  case '+':
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9': {
    double rRounder;
    int i, rx;
    int Y, M, D, h, m, x;
    const char *z2 = z;
    char *zCopy;
    sqlite3 *db = sqlite3_context_db_handle(pCtx);
    char z0 = z[0];
    for (n = 1; z[n]; n++) {
      if (z[n] == ':')
        break;
      if ((sqlite3CtypeMap[(unsigned char)(z[n])] & 0x01))
        break;
      if (z[n] == '-') {
        if (n == 5 && getDigits(&z[1], "40f", &Y) == 1)
          break;
        if (n == 6 && getDigits(&z[1], "50f", &Y) == 1)
          break;
      }
    }
    zCopy = sqlite3DbStrNDup(db, z, n);
    if (zCopy == 0)
      break;
    rx = sqlite3AtoF(zCopy, &r) <= 0;
    sqlite3DbFree(db, zCopy);
    if (rx) {

      ((void)(0))

          ;
      break;
    }
    if (z[n] == '-') {

      if (z0 != '+' && z0 != '-')
        break;
      if (n == 5) {
        if (getDigits(&z[1], "40f-20a-20d", &Y, &M, &D) != 3)
          break;
      } else {

        ((void)(0))

            ;
        if (getDigits(&z[1], "50f-20a-20d", &Y, &M, &D) != 3)
          break;
        z++;
      }
      if (M >= 12)
        break;
      if (D >= 31)
        break;
      computeYMD_HMS(p);
      p->validJD = 0;
      if (z0 == '-') {
        p->Y -= Y;
        p->M -= M;
        D = -D;
      } else {
        p->Y += Y;
        p->M += M;
      }
      x = p->M > 0 ? (p->M - 1) / 12 : (p->M - 12) / 12;
      p->Y += x;
      p->M -= x * 12;
      computeFloor(p);
      computeJD(p);
      p->validHMS = 0;
      p->validYMD = 0;
      p->iJD += (i64)D * 86400000;
      if (z[11] == 0) {
        rc = 0;
        break;
      }
      if ((sqlite3CtypeMap[(unsigned char)(z[11])] & 0x01) && getDigits(&z[12], "20c:20e", &h, &m) == 2) {
        z2 = &z[12];
        n = 2;
      } else {
        break;
      }
    }
    if (z2[n] == ':') {

      DateTime tx;
      sqlite3_int64 day;
      if (!(sqlite3CtypeMap[(unsigned char)(*z2)] & 0x04))
        z2++;
      memset(&tx, 0, sizeof(tx));
      if (parseHhMmSs(z2, &tx))
        break;
      computeJD(&tx);
      tx.iJD -= 43200000;
      day = tx.iJD / 86400000;
      tx.iJD -= day * 86400000;
      if (z0 == '-')
        tx.iJD = -tx.iJD;
      computeJD(p);
      clearYMD_HMS_TZ(p);
      p->iJD += tx.iJD;
      rc = 0;
      break;
    }

    z += n;
    while ((sqlite3CtypeMap[(unsigned char)(*z)] & 0x01))
      z++;
    n = sqlite3Strlen30(z);
    if (n < 3 || n > 10)
      break;
    if (sqlite3UpperToLower[(u8)z[n - 1]] == 's')
      n--;
    computeJD(p);

    ((void)(0))

        ;
    rRounder = r < 0 ? -0.5 : +0.5;
    p->nFloor = 0;
    for (i = 0; i < ((int)(sizeof(aXformType) / sizeof(aXformType[0]))); i++) {
      if (aXformType[i].nName == n && sqlite3_strnicmp(aXformType[i].zName, z, n) == 0 && r > -aXformType[i].rLimit && r < aXformType[i].rLimit) {
        switch (i) {
        case 4: {

          ((void)(0))

              ;
          computeYMD_HMS(p);
          p->M += (int)r;
          x = p->M > 0 ? (p->M - 1) / 12 : (p->M - 12) / 12;
          p->Y += x;
          p->M -= x * 12;
          computeFloor(p);
          p->validJD = 0;
          r -= (int)r;
          break;
        }
        case 5: {
          int y = (int)r;

          ((void)(0))

              ;
          computeYMD_HMS(p);

          ((void)(0))

              ;
          p->Y += y;
          computeFloor(p);
          p->validJD = 0;
          r -= (int)r;
          break;
        }
        }
        computeJD(p);
        p->iJD += (sqlite3_int64)(r * 1000.0 * aXformType[i].rXform + rRounder);
        rc = 0;
        break;
      }
    }
    clearYMD_HMS_TZ(p);
    break;
  }
  default: {
    break;
  }
  }
  return rc;
}

int isDate(sqlite3_context *context, int argc, sqlite3_value **argv, DateTime *p) {
  int i, n;
  const unsigned char *z;
  int eType;
  memset(p, 0, sizeof(*p));
  if (argc == 0) {
    if (!sqlite3NotPureFunc(context))
      return 1;
    return setDateTimeToCurrent(context, p);
  }
  if ((eType = sqlite3_value_type(argv[0])) == 2 || eType == 1) {
    setRawDateNumber(p, sqlite3_value_double(argv[0]));
  } else {
    z = sqlite3_value_text(argv[0]);
    if (!z || parseDateOrTime(context, (char *)z, p)) {
      return 1;
    }
  }
  for (i = 1; i < argc; i++) {
    z = sqlite3_value_text(argv[i]);
    n = sqlite3_value_bytes(argv[i]);
    if (z == 0 || parseModifier(context, (char *)z, n, p, i))
      return 1;
  }
  computeJD(p);
  if (p->isError || !validJulianDay(p->iJD))
    return 1;
  if (argc == 1 && p->validYMD && p->D > 28) {

    ((void)(0))

        ;
    p->validYMD = 0;
  }
  return 0;
}

void juliandayFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  DateTime x;
  if (isDate(context, argc, argv, &x) == 0) {
    computeJD(&x);
    sqlite3_result_double(context, x.iJD / 86400000.0);
  }
}

void unixepochFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  DateTime x;
  if (isDate(context, argc, argv, &x) == 0) {
    computeJD(&x);
    if (x.useSubsec) {
      sqlite3_result_double(context, (x.iJD - 21086676 * (i64)10000000) / 1000.0);
    } else {
      sqlite3_result_int64(context, x.iJD / 1000 - 21086676 * (i64)10000);
    }
  }
}

void datetimeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  DateTime x;
  if (isDate(context, argc, argv, &x) == 0) {
    int Y, s, n;
    char zBuf[32];
    computeYMD_HMS(&x);
    Y = x.Y;
    if (Y < 0)
      Y = -Y;
    zBuf[1] = '0' + (Y / 1000) % 10;
    zBuf[2] = '0' + (Y / 100) % 10;
    zBuf[3] = '0' + (Y / 10) % 10;
    zBuf[4] = '0' + (Y) % 10;
    zBuf[5] = '-';
    zBuf[6] = '0' + (x.M / 10) % 10;
    zBuf[7] = '0' + (x.M) % 10;
    zBuf[8] = '-';
    zBuf[9] = '0' + (x.D / 10) % 10;
    zBuf[10] = '0' + (x.D) % 10;
    zBuf[11] = ' ';
    zBuf[12] = '0' + (x.h / 10) % 10;
    zBuf[13] = '0' + (x.h) % 10;
    zBuf[14] = ':';
    zBuf[15] = '0' + (x.m / 10) % 10;
    zBuf[16] = '0' + (x.m) % 10;
    zBuf[17] = ':';
    if (x.useSubsec) {
      s = (int)(1000.0 * x.s + 0.5);
      zBuf[18] = '0' + (s / 10000) % 10;
      zBuf[19] = '0' + (s / 1000) % 10;
      zBuf[20] = '.';
      zBuf[21] = '0' + (s / 100) % 10;
      zBuf[22] = '0' + (s / 10) % 10;
      zBuf[23] = '0' + (s) % 10;
      zBuf[24] = 0;
      n = 24;
    } else {
      s = (int)x.s;
      zBuf[18] = '0' + (s / 10) % 10;
      zBuf[19] = '0' + (s) % 10;
      zBuf[20] = 0;
      n = 20;
    }
    if (x.Y < 0) {
      zBuf[0] = '-';
      sqlite3_result_text(context, zBuf, n, ((sqlite3_destructor_type)-1));
    } else {
      sqlite3_result_text(context, &zBuf[1], n - 1, ((sqlite3_destructor_type)-1));
    }
  }
}

void timeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  DateTime x;
  if (isDate(context, argc, argv, &x) == 0) {
    int s, n;
    char zBuf[16];
    computeHMS(&x);
    zBuf[0] = '0' + (x.h / 10) % 10;
    zBuf[1] = '0' + (x.h) % 10;
    zBuf[2] = ':';
    zBuf[3] = '0' + (x.m / 10) % 10;
    zBuf[4] = '0' + (x.m) % 10;
    zBuf[5] = ':';
    if (x.useSubsec) {
      s = (int)(1000.0 * x.s + 0.5);
      zBuf[6] = '0' + (s / 10000) % 10;
      zBuf[7] = '0' + (s / 1000) % 10;
      zBuf[8] = '.';
      zBuf[9] = '0' + (s / 100) % 10;
      zBuf[10] = '0' + (s / 10) % 10;
      zBuf[11] = '0' + (s) % 10;
      zBuf[12] = 0;
      n = 12;
    } else {
      s = (int)x.s;
      zBuf[6] = '0' + (s / 10) % 10;
      zBuf[7] = '0' + (s) % 10;
      zBuf[8] = 0;
      n = 8;
    }
    sqlite3_result_text(context, zBuf, n, ((sqlite3_destructor_type)-1));
  }
}

void dateFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  DateTime x;
  if (isDate(context, argc, argv, &x) == 0) {
    int Y;
    char zBuf[16];
    computeYMD(&x);
    Y = x.Y;
    if (Y < 0)
      Y = -Y;
    zBuf[1] = '0' + (Y / 1000) % 10;
    zBuf[2] = '0' + (Y / 100) % 10;
    zBuf[3] = '0' + (Y / 10) % 10;
    zBuf[4] = '0' + (Y) % 10;
    zBuf[5] = '-';
    zBuf[6] = '0' + (x.M / 10) % 10;
    zBuf[7] = '0' + (x.M) % 10;
    zBuf[8] = '-';
    zBuf[9] = '0' + (x.D / 10) % 10;
    zBuf[10] = '0' + (x.D) % 10;
    zBuf[11] = 0;
    if (x.Y < 0) {
      zBuf[0] = '-';
      sqlite3_result_text(context, zBuf, 11, ((sqlite3_destructor_type)-1));
    } else {
      sqlite3_result_text(context, &zBuf[1], 10, ((sqlite3_destructor_type)-1));
    }
  }
}

void strftimeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  DateTime x;
  size_t i, j;
  sqlite3 *db;
  const char *zFmt;
  sqlite3_str sRes;

  if (argc == 0)
    return;
  zFmt = (const char *)sqlite3_value_text(argv[0]);
  if (zFmt == 0 || isDate(context, argc - 1, argv + 1, &x))
    return;
  db = sqlite3_context_db_handle(context);
  sqlite3StrAccumInit(&sRes, 0, 0, 0, db->aLimit[0]);

  computeJD(&x);
  computeYMD_HMS(&x);
  for (i = j = 0; zFmt[i]; i++) {
    char cf;
    if (zFmt[i] != '%')
      continue;
    if (j < i)
      sqlite3_str_append(&sRes, zFmt + j, (int)(i - j));
    i++;
    j = i + 1;
    cf = zFmt[i];
    switch (cf) {
    case 'd':
    case 'e': {
      sqlite3_str_appendf(&sRes, cf == 'd' ? "%02d" : "%2d", x.D);
      break;
    }
    case 'f': {
      double s = x.s;
      if ((s > 59.999))
        s = 59.999;
      sqlite3_str_appendf(&sRes, "%06.3f", s);
      break;
    }
    case 'F': {
      sqlite3_str_appendf(&sRes, "%04d-%02d-%02d", x.Y, x.M, x.D);
      break;
    }
    case 'G':
    case 'g': {
      DateTime y = x;

      ((void)(0))

          ;

      y.iJD += (3 - daysAfterMonday(&x)) * 86400000;
      y.validYMD = 0;
      computeYMD(&y);
      if (cf == 'g') {
        sqlite3_str_appendf(&sRes, "%02d", y.Y % 100);
      } else {
        sqlite3_str_appendf(&sRes, "%04d", y.Y);
      }
      break;
    }
    case 'H':
    case 'k': {
      sqlite3_str_appendf(&sRes, cf == 'H' ? "%02d" : "%2d", x.h);
      break;
    }
    case 'I':
    case 'l': {
      int h = x.h;
      if (h > 12)
        h -= 12;
      if (h == 0)
        h = 12;
      sqlite3_str_appendf(&sRes, cf == 'I' ? "%02d" : "%2d", h);
      break;
    }
    case 'j': {
      sqlite3_str_appendf(&sRes, "%03d", daysAfterJan01(&x) + 1);
      break;
    }
    case 'J': {
      sqlite3_str_appendf(&sRes, "%.16g", x.iJD / 86400000.0);
      break;
    }
    case 'm': {
      sqlite3_str_appendf(&sRes, "%02d", x.M);
      break;
    }
    case 'M': {
      sqlite3_str_appendf(&sRes, "%02d", x.m);
      break;
    }
    case 'p':
    case 'P': {
      if (x.h >= 12) {
        sqlite3_str_append(&sRes, cf == 'p' ? "PM" : "pm", 2);
      } else {
        sqlite3_str_append(&sRes, cf == 'p' ? "AM" : "am", 2);
      }
      break;
    }
    case 'R': {
      sqlite3_str_appendf(&sRes, "%02d:%02d", x.h, x.m);
      break;
    }
    case 's': {
      if (x.useSubsec) {
        sqlite3_str_appendf(&sRes, "%.3f", (x.iJD - 21086676 * (i64)10000000) / 1000.0);
      } else {
        i64 iS = (i64)(x.iJD / 1000 - 21086676 * (i64)10000);
        sqlite3_str_appendf(&sRes, "%lld", iS);
      }
      break;
    }
    case 'S': {
      sqlite3_str_appendf(&sRes, "%02d", (int)x.s);
      break;
    }
    case 'T': {
      sqlite3_str_appendf(&sRes, "%02d:%02d:%02d", x.h, x.m, (int)x.s);
      break;
    }
    case 'u':
    case 'w': {
      char c = (char)daysAfterSunday(&x) + '0';
      if (c == '0' && cf == 'u')
        c = '7';
      sqlite3_str_appendchar(&sRes, 1, c);
      break;
    }
    case 'U': {
      sqlite3_str_appendf(&sRes, "%02d", (daysAfterJan01(&x) - daysAfterSunday(&x) + 7) / 7);
      break;
    }
    case 'V': {
      DateTime y = x;

      ((void)(0))

          ;
      y.iJD += (3 - daysAfterMonday(&x)) * 86400000;
      y.validYMD = 0;
      computeYMD(&y);
      sqlite3_str_appendf(&sRes, "%02d", daysAfterJan01(&y) / 7 + 1);
      break;
    }
    case 'W': {
      sqlite3_str_appendf(&sRes, "%02d", (daysAfterJan01(&x) - daysAfterMonday(&x) + 7) / 7);
      break;
    }
    case 'Y': {
      sqlite3_str_appendf(&sRes, "%04d", x.Y);
      break;
    }
    case '%': {
      sqlite3_str_appendchar(&sRes, 1, '%');
      break;
    }
    default: {
      sqlite3_str_reset(&sRes);
      return;
    }
    }
  }
  if (j < i)
    sqlite3_str_append(&sRes, zFmt + j, (int)(i - j));
  sqlite3ResultStrAccum(context, &sRes);
}

void ctimeFunc(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  timeFunc(context, 0, 0);
}

void cdateFunc(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  dateFunc(context, 0, 0);
}

void timediffFunc(sqlite3_context *context, int NotUsed1, sqlite3_value **argv) {
  char sign;
  int Y, M;
  DateTime d1, d2;
  sqlite3_str sRes;
  (void)(NotUsed1);
  if (isDate(context, 1, &argv[0], &d1))
    return;
  if (isDate(context, 1, &argv[1], &d2))
    return;
  computeYMD_HMS(&d1);
  computeYMD_HMS(&d2);
  if (d1.iJD >= d2.iJD) {
    sign = '+';
    Y = d1.Y - d2.Y;
    if (Y) {
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d1.M - d2.M;
    if (M < 0) {
      Y--;
      M += 12;
    }
    if (M != 0) {
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    while (d1.iJD < d2.iJD) {
      M--;
      if (M < 0) {
        M = 11;
        Y--;
      }
      d2.M--;
      if (d2.M < 1) {
        d2.M = 12;
        d2.Y--;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD -= d2.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  } else {
    sign = '-';
    Y = d2.Y - d1.Y;
    if (Y) {
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d2.M - d1.M;
    if (M < 0) {
      Y--;
      M += 12;
    }
    if (M != 0) {
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    while (d1.iJD > d2.iJD) {
      M--;
      if (M < 0) {
        M = 11;
        Y--;
      }
      d2.M++;
      if (d2.M > 12) {
        d2.M = 1;
        d2.Y++;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD = d2.iJD - d1.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  }
  clearYMD_HMS_TZ(&d1);
  computeYMD_HMS(&d1);
  sqlite3StrAccumInit(&sRes, 0, 0, 0, 100);
  sqlite3_str_appendf(&sRes, "%c%04d-%02d-%02d %02d:%02d:%06.3f", sign, Y, M, d1.D - 1, d1.h, d1.m, d1.s);
  sqlite3ResultStrAccum(context, &sRes);
}

void ctimestampFunc(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  datetimeFunc(context, 0, 0);
}

void sqlite3ResultStrAccum(sqlite3_context *pCtx, StrAccum *p) {
  if (p->accError) {
    sqlite3_result_error_code(pCtx, p->accError);
    sqlite3_str_reset(p);
  } else if ((((p)->printfFlags & 0x04) != 0)) {
    sqlite3_result_text(pCtx, p->zText, p->nChar, ((sqlite3_destructor_type)sqlite3RowSetClear));
  } else {
    sqlite3_result_text(pCtx, "", 0, ((sqlite3_destructor_type)0));
    sqlite3_str_reset(p);
  }
}

int sqlite3NotPureFunc(sqlite3_context *pCtx) {
  const VdbeOp *pOp;

  pOp = pCtx->pVdbe->aOp + pCtx->iOp;
  if (pOp->opcode == 67) {
    const char *zContext;
    char *zMsg;
    if (pOp->p5 & 0x000004) {
      zContext = "a CHECK constraint";
    } else if (pOp->p5 & 0x000008) {
      zContext = "a generated column";
    } else {
      zContext = "an index";
    }
    zMsg = sqlite3_mprintf("non-deterministic use of %s() in %s", pCtx->pFunc->zName, zContext);
    sqlite3_result_error(pCtx, zMsg, -1);
    sqlite3_free(zMsg);
    return 0;
  }
  return 1;
}

const char *sqlite3VdbeFuncName(const sqlite3_context *pCtx) { return pCtx->pFunc->zName; }

void setResultStrOrError(sqlite3_context *pCtx, const char *z, int n, u8 enc, void (*xDel)(void *)) {
  Mem *pOut = pCtx->pOut;
  int rc;
  if (enc == 1) {
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
  } else if (enc == 16) {

    ((void)(0))

        ;
    rc = sqlite3VdbeMemSetText(pOut, z, n, xDel);
    pOut->flags |= 0x0200;
  } else {
    rc = sqlite3VdbeMemSetStr(pOut, z, n, enc, xDel);
  }
  if (rc) {
    if (rc == 18) {
      sqlite3_result_error_toobig(pCtx);
    } else {

      ((void)(0))

          ;
      sqlite3_result_error_nomem(pCtx);
    }
    return;
  }
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  if (sqlite3VdbeMemTooBig(pOut)) {
    sqlite3_result_error_toobig(pCtx);
  }
}

void sqlite3_result_blob(sqlite3_context *pCtx, const void *z, int n, void (*xDel)(void *)) { setResultStrOrError(pCtx, z, n, 0, xDel); }

void sqlite3_result_blob64(sqlite3_context *pCtx, const void *z, sqlite3_uint64 n, void (*xDel)(void *)) {

  if (n > 0x7fffffff) {
    (void)invokeValueDestructor(z, xDel, pCtx);
  } else {
    setResultStrOrError(pCtx, z, (int)n, 0, xDel);
  }
}

void sqlite3_result_double(sqlite3_context *pCtx, double rVal) { sqlite3VdbeMemSetDouble(pCtx->pOut, rVal); }

void sqlite3_result_error(sqlite3_context *pCtx, const char *z, int n) {

  pCtx->isError = 1;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, 1, ((sqlite3_destructor_type)-1));
}

void sqlite3_result_error16(sqlite3_context *pCtx, const void *z, int n) {

  pCtx->isError = 1;
  sqlite3VdbeMemSetStr(pCtx->pOut, z, n, 2, ((sqlite3_destructor_type)-1));
}

void sqlite3_result_int(sqlite3_context *pCtx, int iVal) { sqlite3VdbeMemSetInt64(pCtx->pOut, (i64)iVal); }

void sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal) { sqlite3VdbeMemSetInt64(pCtx->pOut, iVal); }

void sqlite3_result_null(sqlite3_context *pCtx) { sqlite3VdbeMemSetNull(pCtx->pOut); }

void sqlite3_result_pointer(sqlite3_context *pCtx, void *pPtr, const char *zPType, void (*xDestructor)(void *)) {
  Mem *pOut;

  pOut = pCtx->pOut;

  sqlite3VdbeMemRelease(pOut);
  pOut->flags = 0x0001;
  sqlite3VdbeMemSetPointer(pOut, pPtr, zPType, xDestructor);
}

void sqlite3_result_subtype(sqlite3_context *pCtx, unsigned int eSubtype) {
  Mem *pOut;

  pOut = pCtx->pOut;

  pOut->eSubtype = eSubtype & 0xff;
  pOut->flags |= 0x0800;
}

void sqlite3_result_text(sqlite3_context *pCtx, const char *z, int n, void (*xDel)(void *)) { setResultStrOrError(pCtx, z, n, 1, xDel); }

void sqlite3_result_text64(sqlite3_context *pCtx, const char *z, sqlite3_uint64 n, void (*xDel)(void *), unsigned char enc) {

  if (enc != 1 && enc != 16) {
    if (enc == 4)
      enc = 2;
    n &= ~(u64)1;
  }
  if (n > 0x7fffffff) {
    (void)invokeValueDestructor(z, xDel, pCtx);
  } else {
    setResultStrOrError(pCtx, z, (int)n, enc, xDel);
    sqlite3VdbeMemZeroTerminateIfAble(pCtx->pOut);
  }
}

void sqlite3_result_text16(sqlite3_context *pCtx, const void *z, int n, void (*xDel)(void *)) { setResultStrOrError(pCtx, z, n & ~(u64)1, 2, xDel); }

void sqlite3_result_text16be(sqlite3_context *pCtx, const void *z, int n, void (*xDel)(void *)) { setResultStrOrError(pCtx, z, n & ~(u64)1, 3, xDel); }

void sqlite3_result_text16le(sqlite3_context *pCtx, const void *z, int n, void (*xDel)(void *)) { setResultStrOrError(pCtx, z, n & ~(u64)1, 2, xDel); }

void sqlite3_result_value(sqlite3_context *pCtx, sqlite3_value *pValue) {
  Mem *pOut;

  pOut = pCtx->pOut;

  sqlite3VdbeMemCopy(pOut, pValue);
  sqlite3VdbeChangeEncoding(pOut, pCtx->enc);
  if (sqlite3VdbeMemTooBig(pOut)) {
    sqlite3_result_error_toobig(pCtx);
  }
}

void sqlite3_result_zeroblob(sqlite3_context *pCtx, int n) { sqlite3_result_zeroblob64(pCtx, n > 0 ? n : 0); }

int sqlite3_result_zeroblob64(sqlite3_context *pCtx, u64 n) {
  Mem *pOut;

  pOut = pCtx->pOut;

  if (n > (u64)pOut->db->aLimit[0]) {
    sqlite3_result_error_toobig(pCtx);
    return 18;
  }

  sqlite3VdbeMemSetZeroBlob(pCtx->pOut, (int)n);
  return 0;
}

void sqlite3_result_error_code(sqlite3_context *pCtx, int errCode) {

  pCtx->isError = errCode ? errCode : -1;

  if (pCtx->pOut->flags & 0x0001) {
    setResultStrOrError(pCtx, sqlite3ErrStr(errCode), -1, 1, ((sqlite3_destructor_type)0));
  }
}

void sqlite3_result_error_toobig(sqlite3_context *pCtx) {

  pCtx->isError = 18;
  sqlite3VdbeMemSetStr(pCtx->pOut, "string or blob too big", -1, 1, ((sqlite3_destructor_type)0));
}

void sqlite3_result_error_nomem(sqlite3_context *pCtx) {

  sqlite3VdbeMemSetNull(pCtx->pOut);
  pCtx->isError = 7;
  sqlite3OomFault(pCtx->pOut->db);
}

void sqlite3ResultIntReal(sqlite3_context *pCtx) {

  if (pCtx->pOut->flags & 0x0004) {
    pCtx->pOut->flags &= ~0x0004;
    pCtx->pOut->flags |= 0x0020;
  }
}

void *sqlite3_user_data(sqlite3_context *p) { return p->pFunc->pUserData; }

sqlite3 *sqlite3_context_db_handle(sqlite3_context *p) { return p->pOut->db; }

int sqlite3_vtab_nochange(sqlite3_context *p) { return sqlite3_value_nochange(p->pOut); }

sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *p) {
  int rc;

  sqlite3_int64 *piTime = &p->pVdbe->iCurrentTime;

  if (*piTime == 0) {
    rc = sqlite3OsCurrentTimeInt64(p->pOut->db->pVfs, piTime);
    if (rc)
      *piTime = 0;
  }
  return *piTime;
}

__attribute__((noinline)) void *createAggContext(sqlite3_context *p, int nByte) {
  Mem *pMem = p->pMem;

  if (nByte <= 0) {
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
  } else {
    sqlite3VdbeMemClearAndResize(pMem, nByte);
    pMem->flags = 0x8000;
    pMem->u.pDef = p->pFunc;
    if (pMem->z) {
      memset(pMem->z, 0, nByte);
    }
  }
  return (void *)pMem->z;
}

void *sqlite3_aggregate_context(sqlite3_context *p, int nByte) {

  ;
  if ((p->pMem->flags & 0x8000) == 0) {
    return createAggContext(p, nByte);
  } else {
    return (void *)p->pMem->z;
  }
}

void *sqlite3_get_auxdata(sqlite3_context *pCtx, int iArg) {
  AuxData *pAuxData;

  for (pAuxData = pCtx->pVdbe->pAuxData; pAuxData; pAuxData = pAuxData->pNextAux) {
    if (pAuxData->iAuxArg == iArg && (pAuxData->iAuxOp == pCtx->iOp || iArg < 0)) {
      return pAuxData->pAux;
    }
  }
  return 0;
}

void sqlite3_set_auxdata(sqlite3_context *pCtx, int iArg, void *pAux, void (*xDelete)(void *)) {
  AuxData *pAuxData;
  Vdbe *pVdbe;

  pVdbe = pCtx->pVdbe;

  for (pAuxData = pVdbe->pAuxData; pAuxData; pAuxData = pAuxData->pNextAux) {
    if (pAuxData->iAuxArg == iArg && (pAuxData->iAuxOp == pCtx->iOp || iArg < 0)) {
      break;
    }
  }
  if (pAuxData == 0) {
    pAuxData = sqlite3DbMallocZero(pVdbe->db, sizeof(AuxData));
    if (!pAuxData)
      goto failed;
    pAuxData->iAuxOp = pCtx->iOp;
    pAuxData->iAuxArg = iArg;
    pAuxData->pNextAux = pVdbe->pAuxData;
    pVdbe->pAuxData = pAuxData;
    if (pCtx->isError == 0)
      pCtx->isError = -1;
  } else if (pAuxData->xDeleteAux) {
    pAuxData->xDeleteAux(pAuxData->pAux);
  }

  pAuxData->pAux = pAux;
  pAuxData->xDeleteAux = xDelete;
  return;

failed:
  if (xDelete) {
    xDelete(pAux);
  }
}

int sqlite3_aggregate_count(sqlite3_context *p) { return p->pMem->n; }

void errorMPrintf(sqlite3_context *pCtx, const char *zFmt, ...) {
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  char *zErr = 0;
  va_list ap;

  va_start(

      ap, zFmt

  )

      ;
  zErr = sqlite3VMPrintf(db, zFmt, ap);

  va_end(

      ap

  )

      ;
  if (zErr) {
    sqlite3_result_error(pCtx, zErr, -1);
    sqlite3DbFree(db, zErr);
  } else {
    sqlite3_result_error_nomem(pCtx);
  }
}

void renameColumnParseError(sqlite3_context *pCtx, const char *zWhen, sqlite3_value *pType, sqlite3_value *pObject, Parse *pParse) {
  const char *zT = (const char *)sqlite3_value_text(pType);
  const char *zN = (const char *)sqlite3_value_text(pObject);
  char *zErr;

  zErr = sqlite3MPrintf(pParse->db, "error in %s %s%s%s: %s", zT, zN, (zWhen[0] ? " " : ""), zWhen, pParse->zErrMsg);
  sqlite3_result_error(pCtx, zErr, -1);
  sqlite3DbFree(pParse->db, zErr);
}

int renameEditSql(sqlite3_context *pCtx, RenameCtx *pRename, const char *zSql, const char *zNew, int bQuote) {
  i64 nNew = sqlite3Strlen30(zNew);
  i64 nSql = sqlite3Strlen30(zSql);
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  int rc = 0;
  char *zQuot = 0;
  char *zOut;
  i64 nQuot = 0;
  char *zBuf1 = 0;
  char *zBuf2 = 0;

  if (zNew) {

    zQuot = sqlite3MPrintf(db, "\"%w\" ", zNew);
    if (zQuot == 0) {
      return 7;
    } else {
      nQuot = sqlite3Strlen30(zQuot) - 1;
    }

    ((void)(0))

        ;
    zOut = sqlite3DbMallocZero(db, (u64)nSql + pRename->nList * (u64)nQuot + 1);
  } else {

    ((void)(0))

        ;
    zOut = (char *)sqlite3DbMallocZero(db, (2 * (u64)nSql + 1) * 3);
    if (zOut) {
      zBuf1 = &zOut[nSql * 2 + 1];
      zBuf2 = &zOut[nSql * 4 + 2];
    }
  }

  if (zOut) {
    i64 nOut = nSql;

    ((void)(0))

        ;
    memcpy(zOut, zSql, (size_t)nSql);
    while (pRename->pList) {
      int iOff;
      i64 nReplace;
      const char *zReplace;
      RenameToken *pBest = renameColumnTokenNext(pRename);

      if (zNew) {
        if (bQuote == 0 && sqlite3IsIdChar(*(u8 *)pBest->t.z)) {
          nReplace = nNew;
          zReplace = zNew;
        } else {
          nReplace = nQuot;
          zReplace = zQuot;
          if (pBest->t.z[pBest->t.n] == '"')
            nReplace++;
        }
      } else {

        memcpy(zBuf1, pBest->t.z, pBest->t.n);
        zBuf1[pBest->t.n] = 0;
        sqlite3Dequote(zBuf1);

        ((void)(0))

            ;
        sqlite3_snprintf((int)(nSql * 2), zBuf2, "%Q%s", zBuf1, pBest->t.z[pBest->t.n] == '\'' ? " " : "");
        zReplace = zBuf2;
        nReplace = sqlite3Strlen30(zReplace);
      }

      iOff = (int)(pBest->t.z - zSql);
      if (pBest->t.n != nReplace) {
        memmove(&zOut[iOff + nReplace], &zOut[iOff + pBest->t.n], nOut - (iOff + pBest->t.n));
        nOut += nReplace - pBest->t.n;
        zOut[nOut] = '\0';
      }
      memcpy(&zOut[iOff], zReplace, nReplace);
      sqlite3DbFree(db, pBest);
    }

    sqlite3_result_text(pCtx, zOut, -1, ((sqlite3_destructor_type)-1));
    sqlite3DbFree(db, zOut);
  } else {
    rc = 7;
  }

  sqlite3_free(zQuot);
  return rc;
}

void renameColumnFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  RenameCtx sCtx;
  const char *zSql = (const char *)sqlite3_value_text(argv[0]);
  const char *zDb = (const char *)sqlite3_value_text(argv[3]);
  const char *zTable = (const char *)sqlite3_value_text(argv[4]);
  int iCol = sqlite3_value_int(argv[5]);
  const char *zNew = (const char *)sqlite3_value_text(argv[6]);
  int bQuote = sqlite3_value_int(argv[7]);
  int bTemp = sqlite3_value_int(argv[8]);
  const char *zOld;
  int rc;
  Parse sParse;
  Walker sWalker;
  Index *pIdx;
  int i;
  Table *pTab;

  sqlite3_xauth xAuth = db->xAuth;

  (void)(NotUsed);
  if (zSql == 0)
    return;
  if (zTable == 0)
    return;
  if (zNew == 0)
    return;
  if (iCol < 0)
    return;
  sqlite3BtreeEnterAll(db);
  pTab = sqlite3FindTable(db, zTable, zDb);
  if (pTab == 0 || iCol >= pTab->nCol) {
    sqlite3BtreeLeaveAll(db);
    return;
  }
  zOld = pTab->aCol[iCol].zCnName;
  memset(&sCtx, 0, sizeof(sCtx));
  sCtx.iCol = ((iCol == pTab->iPKey) ? -1 : iCol);

  db->xAuth = 0;

  rc = renameParseSql(&sParse, zDb, db, zSql, bTemp);

  memset(&sWalker, 0, sizeof(Walker));
  sWalker.pParse = &sParse;
  sWalker.xExprCallback = renameColumnExprCb;
  sWalker.xSelectCallback = renameColumnSelectCb;
  sWalker.u.pRename = &sCtx;

  sCtx.pTab = pTab;
  if (rc != 0)
    goto renameColumnFunc_done;
  if (sParse.pNewTable) {
    if (((sParse.pNewTable)->eTabType == 2)) {
      Select *pSelect = sParse.pNewTable->u.view.pSelect;
      pSelect->selFlags &= ~(u32)0x0200000;
      sParse.rc = 0;
      sqlite3SelectPrep(&sParse, pSelect, 0);
      rc = (db->mallocFailed ? 7 : sParse.rc);
      if (rc == 0) {
        sqlite3WalkSelect(&sWalker, pSelect);
      }
      if (rc != 0)
        goto renameColumnFunc_done;
    } else if (((sParse.pNewTable)->eTabType == 0)) {

      int bFKOnly = sqlite3_stricmp(zTable, sParse.pNewTable->zName);
      FKey *pFKey;
      sCtx.pTab = sParse.pNewTable;
      if (bFKOnly == 0) {
        if (iCol < sParse.pNewTable->nCol) {
          renameTokenFind(&sParse, &sCtx, (void *)sParse.pNewTable->aCol[iCol].zCnName);
        }
        if (sCtx.iCol < 0) {
          renameTokenFind(&sParse, &sCtx, (void *)&sParse.pNewTable->iPKey);
        }
        sqlite3WalkExprList(&sWalker, sParse.pNewTable->pCheck);
        for (pIdx = sParse.pNewTable->pIndex; pIdx; pIdx = pIdx->pNext) {
          sqlite3WalkExprList(&sWalker, pIdx->aColExpr);
        }
        for (pIdx = sParse.pNewIndex; pIdx; pIdx = pIdx->pNext) {
          sqlite3WalkExprList(&sWalker, pIdx->aColExpr);
        }

        for (i = 0; i < sParse.pNewTable->nCol; i++) {
          Expr *pExpr = sqlite3ColumnExpr(sParse.pNewTable, &sParse.pNewTable->aCol[i]);
          sqlite3WalkExpr(&sWalker, pExpr);
        }
      }

      ((void)(0))

          ;
      for (pFKey = sParse.pNewTable->u.tab.pFKey; pFKey; pFKey = pFKey->pNextFrom) {
        for (i = 0; i < pFKey->nCol; i++) {
          if (bFKOnly == 0 && pFKey->aCol[i].iFrom == iCol) {
            renameTokenFind(&sParse, &sCtx, (void *)&pFKey->aCol[i]);
          }
          if (0 == sqlite3_stricmp(pFKey->zTo, zTable) && 0 == sqlite3_stricmp(pFKey->aCol[i].zCol, zOld)) {
            renameTokenFind(&sParse, &sCtx, (void *)pFKey->aCol[i].zCol);
          }
        }
      }
    }
  } else if (sParse.pNewIndex) {
    sqlite3WalkExprList(&sWalker, sParse.pNewIndex->aColExpr);
    sqlite3WalkExpr(&sWalker, sParse.pNewIndex->pPartIdxWhere);
  } else {

    TriggerStep *pStep;
    rc = renameResolveTrigger(&sParse);
    if (rc != 0)
      goto renameColumnFunc_done;

    for (pStep = sParse.pNewTrigger->step_list; pStep; pStep = pStep->pNext) {
      if (pStep->pSrc) {
        Table *pTarget = sqlite3LocateTableItem(&sParse, 0, &pStep->pSrc->a[0]);
        if (pTarget == pTab) {
          if (pStep->pUpsert) {
            ExprList *pUpsertSet = pStep->pUpsert->pUpsertSet;
            renameColumnElistNames(&sParse, &sCtx, pUpsertSet, zOld);
          }
          renameColumnIdlistNames(&sParse, &sCtx, pStep->pIdList, zOld);
          renameColumnElistNames(&sParse, &sCtx, pStep->pExprList, zOld);
        }
      }
    }

    if (sParse.pTriggerTab == pTab) {
      renameColumnIdlistNames(&sParse, &sCtx, sParse.pNewTrigger->pColumns, zOld);
    }

    renameWalkTrigger(&sWalker, sParse.pNewTrigger);
  }

  rc = renameEditSql(context, &sCtx, zSql, zNew, bQuote);

renameColumnFunc_done:
  if (rc != 0) {
    if (rc == 1 && sqlite3WritableSchema(db)) {
      sqlite3_result_value(context, argv[0]);
    } else if (sParse.zErrMsg) {
      renameColumnParseError(context, "", argv[1], argv[2], &sParse);
    } else {
      sqlite3_result_error_code(context, rc);
    }
  }

  renameParseCleanup(&sParse);
  renameTokenFree(db, sCtx.pList);

  db->xAuth = xAuth;

  sqlite3BtreeLeaveAll(db);
}

void renameTableFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  const char *zDb = (const char *)sqlite3_value_text(argv[0]);
  const char *zInput = (const char *)sqlite3_value_text(argv[3]);
  const char *zOld = (const char *)sqlite3_value_text(argv[4]);
  const char *zNew = (const char *)sqlite3_value_text(argv[5]);
  int bTemp = sqlite3_value_int(argv[6]);
  (void)(NotUsed);

  if (zInput && zOld && zNew) {
    Parse sParse;
    int rc;
    int bQuote = 1;
    RenameCtx sCtx;
    Walker sWalker;

    sqlite3_xauth xAuth = db->xAuth;
    db->xAuth = 0;

    sqlite3BtreeEnterAll(db);

    memset(&sCtx, 0, sizeof(RenameCtx));
    sCtx.pTab = sqlite3FindTable(db, zOld, zDb);
    memset(&sWalker, 0, sizeof(Walker));
    sWalker.pParse = &sParse;
    sWalker.xExprCallback = renameTableExprCb;
    sWalker.xSelectCallback = renameTableSelectCb;
    sWalker.u.pRename = &sCtx;

    rc = renameParseSql(&sParse, zDb, db, zInput, bTemp);

    if (rc == 0) {
      int isLegacy = (db->flags & 0x04000000);
      if (sParse.pNewTable) {
        Table *pTab = sParse.pNewTable;

        if (((pTab)->eTabType == 2)) {
          if (isLegacy == 0) {
            Select *pSelect = pTab->u.view.pSelect;
            NameContext sNC;
            memset(&sNC, 0, sizeof(sNC));
            sNC.pParse = &sParse;

            ((void)(0))

                ;
            pSelect->selFlags &= ~(u32)0x0200000;
            sqlite3SelectPrep(&sParse, pTab->u.view.pSelect, &sNC);
            if (sParse.nErr) {
              rc = sParse.rc;
            } else {
              sqlite3WalkSelect(&sWalker, pTab->u.view.pSelect);
            }
          }
        } else {

          if ((isLegacy == 0 || (db->flags & 0x00004000)) && !((pTab)->eTabType == 1)) {
            FKey *pFKey;

            ((void)(0))

                ;
            for (pFKey = pTab->u.tab.pFKey; pFKey; pFKey = pFKey->pNextFrom) {
              if (sqlite3_stricmp(pFKey->zTo, zOld) == 0) {
                renameTokenFind(&sParse, &sCtx, (void *)pFKey->zTo);
              }
            }
          }

          if (sqlite3_stricmp(zOld, pTab->zName) == 0) {
            sCtx.pTab = pTab;
            if (isLegacy == 0) {
              sqlite3WalkExprList(&sWalker, pTab->pCheck);
            }
            renameTokenFind(&sParse, &sCtx, pTab->zName);
          }
        }
      }

      else if (sParse.pNewIndex) {
        renameTokenFind(&sParse, &sCtx, sParse.pNewIndex->zName);
        if (isLegacy == 0) {
          sqlite3WalkExpr(&sWalker, sParse.pNewIndex->pPartIdxWhere);
        }
      }

      else {
        Trigger *pTrigger = sParse.pNewTrigger;
        TriggerStep *pStep;
        if (0 == sqlite3_stricmp(sParse.pNewTrigger->table, zOld) && sCtx.pTab->pSchema == pTrigger->pTabSchema) {
          renameTokenFind(&sParse, &sCtx, sParse.pNewTrigger->table);
        }

        if (isLegacy == 0) {
          rc = renameResolveTrigger(&sParse);
          if (rc == 0) {
            renameWalkTrigger(&sWalker, pTrigger);
            for (pStep = pTrigger->step_list; pStep; pStep = pStep->pNext) {
              if (pStep->pSrc) {
                int i;
                for (i = 0; i < pStep->pSrc->nSrc; i++) {
                  SrcItem *pItem = &pStep->pSrc->a[i];
                  if (0 == sqlite3_stricmp(pItem->zName, zOld)) {
                    renameTokenFind(&sParse, &sCtx, pItem->zName);
                  }
                }
              }
            }
          }
        }
      }
    }

    if (rc == 0) {
      rc = renameEditSql(context, &sCtx, zInput, zNew, bQuote);
    }
    if (rc != 0) {
      if (rc == 1 && sqlite3WritableSchema(db)) {
        sqlite3_result_value(context, argv[3]);
      } else if (sParse.zErrMsg) {
        renameColumnParseError(context, "", argv[1], argv[2], &sParse);
      } else {
        sqlite3_result_error_code(context, rc);
      }
    }

    renameParseCleanup(&sParse);
    renameTokenFree(db, sCtx.pList);
    sqlite3BtreeLeaveAll(db);

    db->xAuth = xAuth;
  }

  return;
}

void renameQuotefixFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  char const *zDb = (const char *)sqlite3_value_text(argv[0]);
  char const *zInput = (const char *)sqlite3_value_text(argv[1]);

  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;

  sqlite3BtreeEnterAll(db);

  (void)(NotUsed);
  if (zDb && zInput) {
    int rc;
    Parse sParse;
    rc = renameParseSql(&sParse, zDb, db, zInput, 0);

    if (rc == 0) {
      RenameCtx sCtx;
      Walker sWalker;

      memset(&sCtx, 0, sizeof(RenameCtx));
      memset(&sWalker, 0, sizeof(Walker));
      sWalker.pParse = &sParse;
      sWalker.xExprCallback = renameQuotefixExprCb;
      sWalker.xSelectCallback = renameColumnSelectCb;
      sWalker.u.pRename = &sCtx;

      if (sParse.pNewTable) {
        if (((sParse.pNewTable)->eTabType == 2)) {
          Select *pSelect = sParse.pNewTable->u.view.pSelect;
          pSelect->selFlags &= ~(u32)0x0200000;
          sParse.rc = 0;
          sqlite3SelectPrep(&sParse, pSelect, 0);
          rc = (db->mallocFailed ? 7 : sParse.rc);
          if (rc == 0) {
            sqlite3WalkSelect(&sWalker, pSelect);
          }
        } else {
          int i;
          sqlite3WalkExprList(&sWalker, sParse.pNewTable->pCheck);

          for (i = 0; i < sParse.pNewTable->nCol; i++) {
            sqlite3WalkExpr(&sWalker, sqlite3ColumnExpr(sParse.pNewTable, &sParse.pNewTable->aCol[i]));
          }
        }
      } else if (sParse.pNewIndex) {
        sqlite3WalkExprList(&sWalker, sParse.pNewIndex->aColExpr);
        sqlite3WalkExpr(&sWalker, sParse.pNewIndex->pPartIdxWhere);
      } else {

        rc = renameResolveTrigger(&sParse);
        if (rc == 0) {
          renameWalkTrigger(&sWalker, sParse.pNewTrigger);
        }
      }

      if (rc == 0) {
        rc = renameEditSql(context, &sCtx, zInput, 0, 0);
      }
      renameTokenFree(db, sCtx.pList);
    }
    if (rc != 0) {
      if (sqlite3WritableSchema(db) && rc == 1) {
        sqlite3_result_value(context, argv[1]);
      } else {
        sqlite3_result_error_code(context, rc);
      }
    }
    renameParseCleanup(&sParse);
  }

  db->xAuth = xAuth;

  sqlite3BtreeLeaveAll(db);
}

void renameTableTest(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  char const *zDb = (const char *)sqlite3_value_text(argv[0]);
  char const *zInput = (const char *)sqlite3_value_text(argv[1]);
  int bTemp = sqlite3_value_int(argv[4]);
  int isLegacy = (db->flags & 0x04000000);
  char const *zWhen = (const char *)sqlite3_value_text(argv[5]);
  int bNoDQS = sqlite3_value_int(argv[6]);

  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;

  (void)(NotUsed);

  if (zDb && zInput) {
    int rc;
    Parse sParse;
    u64 flags = db->flags;
    if (bNoDQS)
      db->flags &= ~(0x40000000 | 0x20000000);
    rc = renameParseSql(&sParse, zDb, db, zInput, bTemp);
    db->flags = flags;
    if (rc == 0) {
      if (isLegacy == 0 && sParse.pNewTable && ((sParse.pNewTable)->eTabType == 2)) {
        NameContext sNC;
        memset(&sNC, 0, sizeof(sNC));
        sNC.pParse = &sParse;
        sqlite3SelectPrep(&sParse, sParse.pNewTable->u.view.pSelect, &sNC);
        if (sParse.nErr)
          rc = sParse.rc;
      }

      else if (sParse.pNewTrigger) {
        if (isLegacy == 0) {
          rc = renameResolveTrigger(&sParse);
        }
        if (rc == 0) {
          int i1 = sqlite3SchemaToIndex(db, sParse.pNewTrigger->pTabSchema);
          int i2 = sqlite3FindDbName(db, zDb);
          if (i1 == i2) {

            sqlite3_result_int(context, 1);
          }
        }
      }
    }

    if (rc != 0 && zWhen && !sqlite3WritableSchema(db)) {

      renameColumnParseError(context, zWhen, argv[2], argv[3], &sParse);
    }
    renameParseCleanup(&sParse);
  }

  db->xAuth = xAuth;
}

void dropColumnFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  int iSchema = sqlite3_value_int(argv[0]);
  const char *zSql = (const char *)sqlite3_value_text(argv[1]);
  int iCol = sqlite3_value_int(argv[2]);
  const char *zDb = db->aDb[iSchema].zDbSName;
  int rc;
  Parse sParse;
  RenameToken *pCol;
  Table *pTab;
  const char *zEnd;
  char *zNew = 0;

  sqlite3_xauth xAuth = db->xAuth;
  db->xAuth = 0;

  (void)(NotUsed);
  rc = renameParseSql(&sParse, zDb, db, zSql, iSchema == 1);
  if (rc != 0)
    goto drop_column_done;
  pTab = sParse.pNewTable;
  if (pTab == 0 || pTab->nCol == 1 || iCol >= pTab->nCol) {

    rc = sqlite3CorruptError(122761);
    goto drop_column_done;
  }

  if (iCol < pTab->nCol - 1) {
    RenameToken *pEnd;
    pCol = renameTokenFind(&sParse, 0, (void *)pTab->aCol[iCol].zCnName);
    pEnd = renameTokenFind(&sParse, 0, (void *)pTab->aCol[iCol + 1].zCnName);
    zEnd = (const char *)pEnd->t.z;
  } else {
    int eTok;

    ((void)(0))

        ;

    ((void)(0))

        ;

    pCol = renameTokenFind(&sParse, 0, (void *)pTab->aCol[iCol - 1].zCnName);
    do {
      pCol->t.z += getConstraintToken((const u8 *)pCol->t.z, &eTok);
    } while (eTok != 25);
    pCol->t.z--;
    zEnd = (const char *)&zSql[pTab->u.tab.addColOffset];
  }

  zNew = sqlite3MPrintf(db, "%.*s%s", pCol->t.z - zSql, zSql, zEnd);
  sqlite3_result_text(context, zNew, -1, ((sqlite3_destructor_type)-1));
  sqlite3_free(zNew);

drop_column_done:
  renameParseCleanup(&sParse);

  db->xAuth = xAuth;

  if (rc != 0) {
    sqlite3_result_error_code(context, rc);
  }
}

int quotedCompare(sqlite3_context *ctx, int t, const u8 *zQuote, int nQuote, const u8 *zCmp, int *pRes) {
  char *zCopy = 0;

  if (t == 186) {
    *pRes = 1;
    return 0;
  }
  zCopy = sqlite3MallocZero(nQuote + 1);
  if (zCopy == 0) {
    sqlite3_result_error_nomem(ctx);
    return 7;
  }
  memcpy(zCopy, zQuote, nQuote);
  sqlite3Dequote(zCopy);
  *pRes = sqlite3_stricmp((const char *)zCopy, (const char *)zCmp);
  sqlite3_free(zCopy);
  return 0;
}

int skipCreateTable(sqlite3_context *ctx, const u8 *zSql, int *piOff) {
  int iOff = 0;

  if (zSql == 0)
    return 1;

  while (1) {
    int t = 0;
    iOff += sqlite3GetToken(&zSql[iOff], &t);
    if (t == 22)
      break;
    if (t == 186) {
      sqlite3_result_error_code(ctx, sqlite3CorruptError(123056));
      return 1;
    }
  }

  *piOff = iOff;
  return 0;
}

void dropConstraintFunc(sqlite3_context *ctx, int NotUsed, sqlite3_value **argv) {
  const u8 *zSql = sqlite3_value_text(argv[0]);
  const u8 *zCons = 0;
  int iNotNull = -1;
  int ii;
  int iOff = 0;
  int iStart = 0;
  int iEnd = 0;
  char *zNew = 0;
  int t = 0;
  sqlite3 *db;
  (void)(NotUsed);

  if (zSql == 0)
    return;

  if (skipCreateTable(ctx, zSql, &iOff))
    return;

  if (sqlite3_value_type(argv[1]) == 1) {
    iNotNull = sqlite3_value_int(argv[1]);
  } else {
    zCons = sqlite3_value_text(argv[1]);
  }

  for (ii = 0; iEnd == 0; ii++) {

    while (1) {
      iStart = iOff;
      iOff += getConstraintToken(&zSql[iOff], &t);
      if (t == 120 && (zCons || iNotNull == ii)) {

        int nTok = 0;
        int cmp = 1;

        iOff += getWhitespace(&zSql[iOff]);

        nTok = getConstraintToken(&zSql[iOff], &t);
        if (zCons) {
          if (quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp))
            return;
        }
        iOff += nTok;

        nTok = getConstraintToken(&zSql[iOff], &t);
        if (t == 120 || t == 121 || t == 114 || t == 25 || t == 23 || t == 96 || t == 24) {
          t = 125;
        } else {
          iOff += nTok;
          iOff += getConstraint(&zSql[iOff]);
        }

        if (cmp == 0 || (iNotNull >= 0 && t == 19)) {
          if (t != 19 && t != 125) {
            errorMPrintf(ctx, "constraint may not be dropped: %s", zCons);
            return;
          }
          iEnd = iOff;
          break;
        }

      } else if (t == 19 && iNotNull == ii) {
        iEnd = iOff + getConstraint(&zSql[iOff]);
        break;
      } else if (t == 23 || t == 186) {
        iEnd = -1;
        break;
      } else if (t == 25) {
        break;
      }
    }
  }

  if (iEnd <= 0) {
    if (zCons) {
      errorMPrintf(ctx, "no such constraint: %s", zCons);
    } else {

      sqlite3_result_text(ctx, (const char *)zSql, -1, ((sqlite3_destructor_type)-1));
    }
  } else {

    const char *zSpace = " ";
    iEnd += getWhitespace(&zSql[iEnd]);
    sqlite3GetToken(&zSql[iEnd], &t);
    if (t == 23 || t == 25) {
      zSpace = "";
      if (zSql[iStart - 1] == ',')
        iStart--;
    }

    db = sqlite3_context_db_handle(ctx);
    zNew = sqlite3MPrintf(db, "%.*s%s%s", iStart, zSql, zSpace, &zSql[iEnd]);
    sqlite3_result_text(ctx, zNew, -1, ((sqlite3_destructor_type)sqlite3RowSetClear));
  }
}

void addConstraintFunc(sqlite3_context *ctx, int NotUsed, sqlite3_value **argv) {
  const u8 *zSql = sqlite3_value_text(argv[0]);
  const char *zCons = (const char *)sqlite3_value_text(argv[1]);
  int iCol = sqlite3_value_int(argv[2]);
  int iOff = 0;
  int ii;
  char *zNew = 0;
  int t = 0;
  sqlite3 *db;
  (void)(NotUsed);

  if (skipCreateTable(ctx, zSql, &iOff))
    return;

  for (ii = 0; ii <= iCol || (iCol < 0 && t != 23); ii++) {
    iOff += getConstraintToken(&zSql[iOff], &t);
    while (1) {
      int nTok = getConstraintToken(&zSql[iOff], &t);
      if (t == 25 || t == 23)
        break;
      if (t == 186) {
        sqlite3_result_error_code(ctx, sqlite3CorruptError(123234));
        return;
      }
      iOff += nTok;
    }
  }

  iOff += getWhitespace(&zSql[iOff]);

  db = sqlite3_context_db_handle(ctx);
  if (iCol < 0) {
    zNew = sqlite3MPrintf(db, "%.*s, %s%s", iOff, zSql, zCons, &zSql[iOff]);
  } else {
    zNew = sqlite3MPrintf(db, "%.*s %s%s", iOff, zSql, zCons, &zSql[iOff]);
  }
  sqlite3_result_text(ctx, zNew, -1, ((sqlite3_destructor_type)sqlite3RowSetClear));
}

void failConstraintFunc(sqlite3_context *ctx, int NotUsed, sqlite3_value **argv) {
  const char *zText = (const char *)sqlite3_value_text(argv[0]);
  int err = sqlite3_value_int(argv[1]);
  (void)NotUsed;
  sqlite3_result_error(ctx, zText, -1);
  sqlite3_result_error_code(ctx, err);
}

void findConstraintFunc(sqlite3_context *ctx, int NotUsed, sqlite3_value **argv) {
  const u8 *zSql = 0;
  const u8 *zCons = 0;
  int iOff = 0;
  int t = 0;

  (void)NotUsed;
  zSql = sqlite3_value_text(argv[0]);
  zCons = sqlite3_value_text(argv[1]);

  if (zSql == 0 || zCons == 0)
    return;
  while (t != 22 && t != 186) {
    iOff += sqlite3GetToken(&zSql[iOff], &t);
  }

  while (1) {
    iOff += getConstraintToken(&zSql[iOff], &t);
    if (t == 120) {
      int nTok = 0;
      int cmp = 0;
      iOff += getWhitespace(&zSql[iOff]);
      nTok = getConstraintToken(&zSql[iOff], &t);
      if (quotedCompare(ctx, t, &zSql[iOff], nTok, zCons, &cmp))
        return;
      if (cmp == 0) {
        sqlite3_result_int(ctx, 1);
        return;
      }
    } else if (t == 186) {
      break;
    }
  }

  sqlite3_result_int(ctx, 0);
}

void statInit(sqlite3_context *context, int argc, sqlite3_value **argv) {
  StatAccum *p;
  int nCol;
  int nKeyCol;
  int nColUp;
  i64 n;
  sqlite3 *db = sqlite3_context_db_handle(context);

  (void)(argc);
  nCol = sqlite3_value_int(argv[0]);

  nColUp = sizeof(tRowcnt) < 8 ? (nCol + 1) & ~1 : nCol;
  nKeyCol = sqlite3_value_int(argv[1]);

  n = sizeof(*p) + sizeof(tRowcnt) * nColUp;

  p = sqlite3DbMallocZero(db, n);
  if (p == 0) {
    sqlite3_result_error_nomem(context);
    return;
  }

  p->db = db;
  p->nEst = sqlite3_value_int64(argv[2]);
  p->nRow = 0;
  p->nLimit = sqlite3_value_int(argv[3]);
  p->nCol = nCol;
  p->nKeyCol = nKeyCol;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt *)&p[1];

  sqlite3_result_blob(context, p, sizeof(*p), statAccumDestructor);
}

void statPush(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int i;

  StatAccum *p = (StatAccum *)sqlite3_value_blob(argv[0]);
  int iChng = sqlite3_value_int(argv[1]);

  (void)(argc);
  (void)(context);

  if (p->nRow == 0) {

  } else {

    for (i = iChng; i < p->nCol; i++) {
      p->current.anDLt[i]++;
    }
  }

  p->nRow++;

  if (p->nLimit && p->nRow > (tRowcnt)p->nLimit * (p->nSkipAhead + 1)) {
    p->nSkipAhead++;
    sqlite3_result_int(context, p->current.anDLt[0] > 0);
  }
}

void statGet(sqlite3_context *context, int argc, sqlite3_value **argv) {
  StatAccum *p = (StatAccum *)sqlite3_value_blob(argv[0]);

  {

    sqlite3_str sStat;
    int i;

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->nKeyCol + 1) * 100);
    sqlite3_str_appendf(&sStat, "%llu", p->nSkipAhead ? (u64)p->nEst : (u64)p->nRow);
    for (i = 0; i < p->nKeyCol; i++) {
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->nRow + nDistinct - 1) / nDistinct;
      if (iVal == 2 && p->nRow * 10 <= nDistinct * 11)
        iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
    }
    sqlite3ResultStrAccum(context, &sStat);
  }

  (void)(argc);
}

void attachFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  int i;
  int rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  const char *zName;
  const char *zFile;
  char *zPath = 0;
  char *zErr = 0;
  unsigned int flags;
  Db *aNew;
  Db *pNew = 0;
  char *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  (void)(NotUsed);
  zFile = (const char *)sqlite3_value_text(argv[0]);
  zName = (const char *)sqlite3_value_text(argv[1]);
  if (zFile == 0)
    zFile = "";
  if (zName == 0)
    zName = "";

  if ((db->init.reopenMemdb)) {

    Btree *pNewBt = 0;

    pNew = &db->aDb[db->init.iDb];

    ((void)(0))

        ;
    if (sqlite3BtreeTxnState(pNew->pBt) != 0 || sqlite3BtreeIsInBackup(pNew->pBt)) {
      rc = 5;
      goto attach_error;
    }

    pVfs = sqlite3_vfs_find("memdb");
    if (pVfs == 0)
      return;
    rc = sqlite3BtreeOpen(pVfs, "x\0", db, &pNewBt, 0, 0x00000100);
    if (rc == 0) {
      Schema *pNewSchema = sqlite3SchemaGet(db, pNewBt);
      if (pNewSchema) {

        sqlite3BtreeClose(pNew->pBt);
        pNew->pBt = pNewBt;
        pNew->pSchema = pNewSchema;
      } else {
        sqlite3BtreeClose(pNewBt);
        rc = 7;
      }
    }
    if (rc)
      goto attach_error;
  } else {

    if (db->nDb >= db->aLimit[7] + 2) {
      zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", db->aLimit[7]);
      goto attach_error;
    }
    for (i = 0; i < db->nDb; i++) {

      ((void)(0))

          ;
      if (sqlite3DbIsNamed(db, i, zName)) {
        zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
        goto attach_error;
      }
    }

    if (db->aDb == db->aDbStatic) {
      aNew = sqlite3DbMallocRawNN(db, sizeof(db->aDb[0]) * 3);
      if (aNew == 0)
        return;
      memcpy(aNew, db->aDb, sizeof(db->aDb[0]) * 2);
    } else {
      aNew = sqlite3DbRealloc(db, db->aDb, sizeof(db->aDb[0]) * (1 + (i64)db->nDb));
      if (aNew == 0)
        return;
    }
    db->aDb = aNew;
    pNew = &db->aDb[db->nDb];
    memset(pNew, 0, sizeof(*pNew));

    flags = db->openFlags;
    rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
    if (rc != 0) {
      if (rc == 7)
        sqlite3OomFault(db);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      return;
    }
    if ((db->flags & ((u64)(0x00020) << 32)) == 0) {
      flags &= ~(0x00000004 | 0x00000002);
      flags |= 0x00000001;
    } else if ((db->flags & ((u64)(0x00010) << 32)) == 0) {
      flags &= ~0x00000004;
    }

    ((void)(0))

        ;
    flags |= 0x00000100;
    rc = sqlite3BtreeOpen(pVfs, zPath, db, &pNew->pBt, 0, flags);
    db->nDb++;
    pNew->zDbSName = sqlite3DbStrDup(db, zName);
  }
  db->noSharedCache = 0;
  if (rc == 19) {
    rc = 1;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  } else if (rc == 0) {
    Pager *pPager;
    pNew->pSchema = sqlite3SchemaGet(db, pNew->pBt);
    if (!pNew->pSchema) {
      rc = 7;
    } else if (pNew->pSchema->file_format && pNew->pSchema->enc != ((db)->enc)) {
      zErrDyn = sqlite3MPrintf(db, "attached databases must use the same text encoding as main database");
      rc = 1;
    }
    sqlite3BtreeEnter(pNew->pBt);
    pPager = sqlite3BtreePager(pNew->pBt);
    sqlite3PagerLockingMode(pPager, db->dfltLockMode);
    sqlite3BtreeSecureDelete(pNew->pBt, sqlite3BtreeSecureDelete(db->aDb[0].pBt, -1));

    sqlite3BtreeSetPagerFlags(pNew->pBt, 0x03 | (db->flags & 0x38));

    sqlite3BtreeLeave(pNew->pBt);
  }
  pNew->safety_level = 2 + 1;
  if (rc == 0 && pNew->zDbSName == 0) {
    rc = 7;
  }
  sqlite3_free_filename(zPath);

  if (rc == 0) {
    sqlite3BtreeEnterAll(db);
    db->init.iDb = 0;
    db->mDbFlags &= ~(0x0010);

    if (!(db->init.reopenMemdb)) {
      rc = sqlite3Init(db, &zErrDyn);
    }
    sqlite3BtreeLeaveAll(db);

    ((void)(0))

        ;
  }
  if (rc) {
    if ((!(db->init.reopenMemdb))) {
      int iDb = db->nDb - 1;

      ((void)(0))

          ;
      if (db->aDb[iDb].pBt) {
        sqlite3BtreeClose(db->aDb[iDb].pBt);
        db->aDb[iDb].pBt = 0;
        db->aDb[iDb].pSchema = 0;
      }
      sqlite3ResetAllSchemasOfConnection(db);
      db->nDb = iDb;
      if (rc == 7 || rc == (10 | (12 << 8))) {
        sqlite3OomFault(db);
        sqlite3DbFree(db, zErrDyn);
        zErrDyn = sqlite3MPrintf(db, "out of memory");
      } else if (zErrDyn == 0) {
        zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
      }
    }
    goto attach_error;
  }

  return;

attach_error:

  if (zErrDyn) {
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  if (rc)
    sqlite3_result_error_code(context, rc);
}

void detachFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  const char *zName = (const char *)sqlite3_value_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  int i;
  Db *pDb = 0;
  HashElem *pEntry;
  char zErr[128];

  (void)(NotUsed);

  if (zName == 0)
    zName = "";
  for (i = 0; i < db->nDb; i++) {
    pDb = &db->aDb[i];
    if (pDb->pBt == 0)
      continue;
    if (sqlite3DbIsNamed(db, i, zName))
      break;
  }

  if (i >= db->nDb) {
    sqlite3_snprintf(sizeof(zErr), zErr, "no such database: %s", zName);
    goto detach_error;
  }
  if (i < 2) {
    sqlite3_snprintf(sizeof(zErr), zErr, "cannot detach database %s", zName);
    goto detach_error;
  }
  if (sqlite3BtreeTxnState(pDb->pBt) != 0 || sqlite3BtreeIsInBackup(pDb->pBt)) {
    sqlite3_snprintf(sizeof(zErr), zErr, "database %s is locked", zName);
    goto detach_error;
  }

  pEntry = ((&db->aDb[1].pSchema->trigHash)->first);
  while (pEntry) {
    Trigger *pTrig = (Trigger *)((pEntry)->data);
    if (pTrig->pTabSchema == pDb->pSchema) {
      pTrig->pTabSchema = pTrig->pSchema;
    }
    pEntry = ((pEntry)->next);
  }

  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3CollapseDatabaseArray(db);
  return;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context) {
  VdbeOp *pOp;

  pOp = &context->pVdbe->aOp[context->iOp - 1];

  return pOp->p4.pColl;
}

void sqlite3SkipAccumulatorLoad(sqlite3_context *context) {

  context->isError = -1;
  context->skipFlag = 1;
}

void minmaxFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int i;
  int mask;
  int iBest;
  CollSeq *pColl;

  mask = sqlite3_user_data(context) == 0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);

  iBest = 0;
  if (sqlite3_value_type(argv[0]) == 5)
    return;
  for (i = 1; i < argc; i++) {
    if (sqlite3_value_type(argv[i]) == 5)
      return;
    if ((sqlite3MemCompare(argv[iBest], argv[i], pColl) ^ mask) >= 0) {
      ;
      iBest = i;
    }
  }
  sqlite3_result_value(context, argv[iBest]);
}

void typeofFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  static const char *azType[] = {"integer", "real", "text", "blob", "null"};
  int i = sqlite3_value_type(argv[0]) - 1;
  (void)(NotUsed);

  sqlite3_result_text(context, azType[i], -1, ((sqlite3_destructor_type)0));
}

void subtypeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  (void)(argc);
  sqlite3_result_int(context, sqlite3_value_subtype(argv[0]));
}

void lengthFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {

  (void)(argc);
  switch (sqlite3_value_type(argv[0])) {
  case 4:
  case 1:
  case 2: {
    sqlite3_result_int(context, sqlite3_value_bytes(argv[0]));
    break;
  }
  case 3: {
    const unsigned char *z = sqlite3_value_text(argv[0]);
    const unsigned char *z0;
    unsigned char c;
    if (z == 0)
      return;
    z0 = z;
    while ((c = *z) != 0) {
      z++;
      if (c >= 0xc0) {
        while ((*z & 0xc0) == 0x80) {
          z++;
          z0++;
        }
      }
    }
    sqlite3_result_int(context, (int)(z - z0));
    break;
  }
  default: {
    sqlite3_result_null(context);
    break;
  }
  }
}

void bytelengthFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {

  (void)(argc);
  switch (sqlite3_value_type(argv[0])) {
  case 4: {
    sqlite3_result_int(context, sqlite3_value_bytes(argv[0]));
    break;
  }
  case 1:
  case 2: {
    i64 m = sqlite3_context_db_handle(context)->enc <= 1 ? 1 : 2;
    sqlite3_result_int64(context, sqlite3_value_bytes(argv[0]) * m);
    break;
  }
  case 3: {
    if (sqlite3_value_encoding(argv[0]) <= 1) {
      sqlite3_result_int(context, sqlite3_value_bytes(argv[0]));
    } else {
      sqlite3_result_int(context, sqlite3_value_bytes16(argv[0]));
    }
    break;
  }
  default: {
    sqlite3_result_null(context);
    break;
  }
  }
}

void absFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {

  (void)(argc);
  switch (sqlite3_value_type(argv[0])) {
  case 1: {
    i64 iVal = sqlite3_value_int64(argv[0]);
    if (iVal < 0) {
      if (iVal == (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32)))) {

        sqlite3_result_error(context, "integer overflow", -1);
        return;
      }
      iVal = -iVal;
    }
    sqlite3_result_int64(context, iVal);
    break;
  }
  case 5: {

    sqlite3_result_null(context);
    break;
  }
  default: {

    double rVal = sqlite3_value_double(argv[0]);
    if (rVal < 0)
      rVal = -rVal;
    sqlite3_result_double(context, rVal);
    break;
  }
  }
}

void instrFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const unsigned char *zHaystack;
  const unsigned char *zNeedle;
  int nHaystack;
  int nNeedle;
  int typeHaystack, typeNeedle;
  int N = 1;
  int isText;
  unsigned char firstChar;
  sqlite3_value *pC1 = 0;
  sqlite3_value *pC2 = 0;

  (void)(argc);
  typeHaystack = sqlite3_value_type(argv[0]);
  typeNeedle = sqlite3_value_type(argv[1]);
  if (typeHaystack == 5 || typeNeedle == 5)
    return;
  nHaystack = sqlite3_value_bytes(argv[0]);
  nNeedle = sqlite3_value_bytes(argv[1]);
  if (nNeedle > 0) {
    if (typeHaystack == 4 && typeNeedle == 4) {
      zHaystack = sqlite3_value_blob(argv[0]);
      zNeedle = sqlite3_value_blob(argv[1]);
      isText = 0;
    } else if (typeHaystack != 4 && typeNeedle != 4) {
      zHaystack = sqlite3_value_text(argv[0]);
      zNeedle = sqlite3_value_text(argv[1]);
      isText = 1;
    } else {
      pC1 = sqlite3_value_dup(argv[0]);
      zHaystack = sqlite3_value_text(pC1);
      if (zHaystack == 0)
        goto endInstrOOM;
      nHaystack = sqlite3_value_bytes(pC1);
      pC2 = sqlite3_value_dup(argv[1]);
      zNeedle = sqlite3_value_text(pC2);
      if (zNeedle == 0)
        goto endInstrOOM;
      nNeedle = sqlite3_value_bytes(pC2);
      isText = 1;
    }
    if (zNeedle == 0 || (nHaystack && zHaystack == 0))
      goto endInstrOOM;
    firstChar = zNeedle[0];
    while (nNeedle <= nHaystack && (zHaystack[0] != firstChar || memcmp(zHaystack, zNeedle, nNeedle) != 0)) {
      N++;
      do {
        nHaystack--;
        zHaystack++;
      } while (isText && (zHaystack[0] & 0xc0) == 0x80);
    }
    if (nNeedle > nHaystack)
      N = 0;
  }
  sqlite3_result_int(context, N);
endInstr:
  sqlite3_value_free(pC1);
  sqlite3_value_free(pC2);
  return;
endInstrOOM:
  sqlite3_result_error_nomem(context);
  goto endInstr;
}

void printfFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  PrintfArguments x;
  StrAccum str;
  const char *zFormat;
  int n;
  sqlite3 *db = sqlite3_context_db_handle(context);

  if (argc >= 1 && (zFormat = (const char *)sqlite3_value_text(argv[0])) != 0) {
    x.nArg = argc - 1;
    x.nUsed = 0;
    x.apArg = argv + 1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[0]);
    str.printfFlags = 0x02;
    sqlite3_str_appendf(&str, zFormat, &x);
    if (str.accError == 0) {
      n = str.nChar;
      sqlite3_result_text(context, sqlite3StrAccumFinish(&str), n, ((sqlite3_destructor_type)sqlite3RowSetClear));
    } else {
      if (str.accError == 7) {
        sqlite3_result_error_nomem(context);
      } else {
        sqlite3_result_error_toobig(context);
      }
      sqlite3_str_reset(&str);
    }
  }
}

void substrFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const unsigned char *z;
  const unsigned char *z2;
  int len;
  int p0type;
  i64 p1, p2;

  p0type = sqlite3_value_type(argv[0]);
  p1 = sqlite3_value_int64(argv[1]);
  if (p0type == 4) {
    len = sqlite3_value_bytes(argv[0]);
    z = sqlite3_value_blob(argv[0]);
    if (z == 0)
      return;

    ((void)(0))

        ;
  } else {
    z = sqlite3_value_text(argv[0]);
    if (z == 0)
      return;
    len = 0;
    if (p1 < 0) {
      for (z2 = z; *z2; len++) {
        {
          if ((*(z2++)) >= 0xc0) {
            while ((*z2 & 0xc0) == 0x80) {
              z2++;
            }
          }
        };
      }
    }
  }
  if (argc == 3) {
    p2 = sqlite3_value_int64(argv[2]);
    if (p2 == 0 && sqlite3_value_type(argv[2]) == 5)
      return;
  } else {
    p2 = sqlite3_context_db_handle(context)->aLimit[0];
  }
  if (p1 == 0) {

    if (sqlite3_value_type(argv[1]) == 5)
      return;
  }
  if (p1 < 0) {
    p1 += len;
    if (p1 < 0) {
      if (p2 < 0) {
        p2 = 0;
      } else {
        p2 += p1;
      }
      p1 = 0;
    }
  } else if (p1 > 0) {
    p1--;
  } else if (p2 > 0) {
    p2--;
  }
  if (p2 < 0) {
    if (p2 < -p1) {
      p2 = p1;
    } else {
      p2 = -p2;
    }
    p1 -= p2;
  }

  if (p0type != 4) {
    while (*z && p1) {
      {
        if ((*(z++)) >= 0xc0) {
          while ((*z & 0xc0) == 0x80) {
            z++;
          }
        }
      };
      p1--;
    }
    for (z2 = z; *z2 && p2; p2--) {
      {
        if ((*(z2++)) >= 0xc0) {
          while ((*z2 & 0xc0) == 0x80) {
            z2++;
          }
        }
      };
    }
    sqlite3_result_text64(context, (char *)z, z2 - z, ((sqlite3_destructor_type)-1), 1);
  } else {
    if (p1 >= len) {
      p1 = p2 = 0;
    } else if (p2 > len - p1) {
      p2 = len - p1;

      ((void)(0))

          ;
    }
    sqlite3_result_blob64(context, (char *)&z[p1], (u64)p2, ((sqlite3_destructor_type)-1));
  }
}

void roundFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  i64 n = 0;
  double r;
  char *zBuf;

  if (argc == 2) {
    if (5 == sqlite3_value_type(argv[1]))
      return;
    n = sqlite3_value_int64(argv[1]);
    if (n > 30)
      n = 30;
    if (n < 0)
      n = 0;
  }
  if (sqlite3_value_type(argv[0]) == 5)
    return;
  r = sqlite3_value_double(argv[0]);

  if (r < -4503599627370496.0 || r > +4503599627370496.0) {

  } else if (n == 0) {
    r = (double)((sqlite_int64)(r + (r < 0 ? -0.5 : +0.5)));
  } else {
    zBuf = sqlite3_mprintf("%!.*f", (int)n, r);
    if (zBuf == 0) {
      sqlite3_result_error_nomem(context);
      return;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_result_double(context, r);
}

void *contextMalloc(sqlite3_context *context, i64 nByte) {
  char *z;
  sqlite3 *db = sqlite3_context_db_handle(context);

  ;
  ;
  if (nByte > db->aLimit[0]) {
    sqlite3_result_error_toobig(context);
    z = 0;
  } else {
    z = sqlite3Malloc(nByte);
    if (!z) {
      sqlite3_result_error_nomem(context);
    }
  }
  return z;
}

void upperFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  char *z1;
  const char *z2;
  int i, n;
  (void)(argc);
  z2 = (char *)sqlite3_value_text(argv[0]);
  n = sqlite3_value_bytes(argv[0]);

  if (z2) {
    z1 = contextMalloc(context, ((i64)n) + 1);
    if (z1) {
      for (i = 0; i < n; i++) {
        z1[i] = (char)((z2[i]) & ~(sqlite3CtypeMap[(unsigned char)(z2[i])] & 0x20));
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}

void lowerFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  char *z1;
  const char *z2;
  int i, n;
  (void)(argc);
  z2 = (char *)sqlite3_value_text(argv[0]);
  n = sqlite3_value_bytes(argv[0]);

  if (z2) {
    z1 = contextMalloc(context, ((i64)n) + 1);
    if (z1) {
      for (i = 0; i < n; i++) {
        z1[i] = (sqlite3UpperToLower[(unsigned char)(z2[i])]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}

void randomFunc(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  sqlite_int64 r;
  (void)(NotUsed), (void)(NotUsed2);
  sqlite3_randomness(sizeof(r), &r);
  if (r < 0) {

    r = -(r & (0xffffffff | (((i64)0x7fffffff) << 32)));
  }
  sqlite3_result_int64(context, r);
}

void randomBlob(sqlite3_context *context, int argc, sqlite3_value **argv) {
  sqlite3_int64 n;
  unsigned char *p;

  (void)(argc);
  n = sqlite3_value_int64(argv[0]);
  if (n < 1) {
    n = 1;
  }
  p = contextMalloc(context, n);
  if (p) {
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (char *)p, n, sqlite3_free);
  }
}

void last_insert_rowid(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  (void)(NotUsed), (void)(NotUsed2);

  sqlite3_result_int64(context, sqlite3_last_insert_rowid(db));
}

void changes(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  (void)(NotUsed), (void)(NotUsed2);
  sqlite3_result_int64(context, sqlite3_changes64(db));
}

void total_changes(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  (void)(NotUsed), (void)(NotUsed2);

  sqlite3_result_int64(context, sqlite3_total_changes64(db));
}

void likeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const unsigned char *zA, *zB;
  u32 escape;
  int nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  struct compareInfo *pInfo = sqlite3_user_data(context);
  struct compareInfo backupInfo;

  nPat = sqlite3_value_bytes(argv[0]);
  ;
  ;
  if (nPat > db->aLimit[8]) {
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    return;
  }
  if (argc == 3) {

    const unsigned char *zEsc = sqlite3_value_text(argv[2]);
    if (zEsc == 0)
      return;
    if (sqlite3Utf8CharLen((char *)zEsc, -1) != 1) {
      sqlite3_result_error(context, "ESCAPE expression must be a single character", -1);
      return;
    }
    escape = sqlite3Utf8Read(&zEsc);
    if (escape == pInfo->matchAll || escape == pInfo->matchOne) {
      memcpy(&backupInfo, pInfo, sizeof(backupInfo));
      pInfo = &backupInfo;
      if (escape == pInfo->matchAll)
        pInfo->matchAll = 0;
      if (escape == pInfo->matchOne)
        pInfo->matchOne = 0;
    }
  } else {
    escape = pInfo->matchSet;
  }
  zB = sqlite3_value_text(argv[0]);
  zA = sqlite3_value_text(argv[1]);
  if (zA && zB) {

    sqlite3_result_int(context, patternCompare(zB, zA, pInfo, escape) == 0);
  }
}

void nullifFunc(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  (void)(NotUsed);
  if (sqlite3MemCompare(argv[0], argv[1], pColl) != 0) {
    sqlite3_result_value(context, argv[0]);
  }
}

void versionFunc(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);

  sqlite3_result_text(context, sqlite3_libversion(), -1, ((sqlite3_destructor_type)0));
}

void sourceidFunc(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);

  sqlite3_result_text(context, sqlite3_sourceid(), -1, ((sqlite3_destructor_type)0));
}

void errlogFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  (void)(argc);
  (void)(context);
  sqlite3_log(sqlite3_value_int(argv[0]), "%s", sqlite3_value_text(argv[1]));
}

void compileoptionusedFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const char *zOptName;

  (void)(argc);

  if ((zOptName = (const char *)sqlite3_value_text(argv[0])) != 0) {
    sqlite3_result_int(context, sqlite3_compileoption_used(zOptName));
  }
}

void compileoptiongetFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int n;

  (void)(argc);

  n = sqlite3_value_int(argv[0]);
  sqlite3_result_text(context, sqlite3_compileoption_get(n), -1, ((sqlite3_destructor_type)0));
}

void unistrFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  char *zOut;
  const char *zIn;
  int nIn;
  int i, j, n;
  u32 v;

  (void)(argc);
  zIn = (const char *)sqlite3_value_text(argv[0]);
  if (zIn == 0)
    return;
  nIn = sqlite3_value_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn + 1);
  if (zOut == 0) {
    sqlite3_result_error_nomem(context);
    return;
  }
  i = j = 0;
  while (i < nIn) {
    const char *z =

        _Generic(0 ? (

                         &zIn[i]

                         )
                   : (void *)1,
        const void *: (const char *)(strchr(

            &zIn[i]

            ,

            '\\'

            )),
        default: strchr(

                     &zIn[i]

                     ,

                     '\\'

                     ))

        ;
    if (z == 0) {
      n = nIn - i;
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      break;
    }
    n = z - &zIn[i];
    if (n > 0) {
      memmove(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    if (zIn[i + 1] == '\\') {
      i += 2;
      zOut[j++] = '\\';
    } else if ((sqlite3CtypeMap[(unsigned char)(zIn[i + 1])] & 0x08)) {
      if (!isNHex(&zIn[i + 1], 4, &v))
        goto unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    } else if (zIn[i + 1] == '+') {
      if (!isNHex(&zIn[i + 2], 6, &v))
        goto unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    } else if (zIn[i + 1] == 'u') {
      if (!isNHex(&zIn[i + 2], 4, &v))
        goto unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    } else if (zIn[i + 1] == 'U') {
      if (!isNHex(&zIn[i + 2], 8, &v))
        goto unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    } else {
      goto unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_result_text64(context, zOut, j, sqlite3_free, 16);
  return;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_result_error(context, "invalid Unicode escape", -1);
  return;
}

void quoteFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);

  (void)(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[0]);
  sqlite3QuoteValue(&str, argv[0], ((int)(intptr_t)(sqlite3_user_data(context))));
  sqlite3_result_text(context, sqlite3StrAccumFinish(&str), str.nChar, ((sqlite3_destructor_type)sqlite3RowSetClear));
  if (str.accError != 0) {
    sqlite3_result_null(context);
    sqlite3_result_error_code(context, str.accError);
  }
}

void unicodeFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const unsigned char *z = sqlite3_value_text(argv[0]);
  (void)argc;
  if (z && z[0])
    sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

void charFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  unsigned char *z, *zOut;
  int i;
  zOut = z = sqlite3_malloc64(argc * 4 + 1);
  if (z == 0) {
    sqlite3_result_error_nomem(context);
    return;
  }
  for (i = 0; i < argc; i++) {
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_value_int64(argv[i]);
    if (x < 0 || x > 0x10ffff)
      x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    if (c < 0x00080) {
      *zOut++ = (u8)(c & 0xFF);
    } else if (c < 0x00800) {
      *zOut++ = 0xC0 + (u8)((c >> 6) & 0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    } else if (c < 0x10000) {
      *zOut++ = 0xE0 + (u8)((c >> 12) & 0x0F);
      *zOut++ = 0x80 + (u8)((c >> 6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    } else {
      *zOut++ = 0xF0 + (u8)((c >> 18) & 0x07);
      *zOut++ = 0x80 + (u8)((c >> 12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c >> 6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }
  }
  *zOut = 0;
  sqlite3_result_text64(context, (char *)z, zOut - z, sqlite3_free, 16);
}

void hexFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int i, n;
  const unsigned char *pBlob;
  char *zHex, *z;

  (void)(argc);
  pBlob = sqlite3_value_blob(argv[0]);
  n = sqlite3_value_bytes(argv[0]);

  z = zHex = contextMalloc(context, ((i64)n) * 2 + 1);
  if (zHex) {
    for (i = 0; i < n; i++, pBlob++) {
      unsigned char c = *pBlob;
      *(z++) = hexdigits[(c >> 4) & 0xf];
      *(z++) = hexdigits[c & 0xf];
    }
    *z = 0;
    sqlite3_result_text64(context, zHex, (u64)(z - zHex), sqlite3_free, 16);
  }
}

void unhexFunc(sqlite3_context *pCtx, int argc, sqlite3_value **argv) {
  const u8 *zPass = (const u8 *)"";
  int nPass = 0;
  const u8 *zHex = sqlite3_value_text(argv[0]);
  int nHex = sqlite3_value_bytes(argv[0]);

  u8 *pBlob = 0;
  u8 *p = 0;

  if (argc == 2) {
    zPass = sqlite3_value_text(argv[1]);
    nPass = sqlite3_value_bytes(argv[1]);
  }
  if (!zHex || !zPass)
    return;

  p = pBlob = contextMalloc(pCtx, (nHex / 2) + 1);
  if (pBlob) {
    u8 c;
    u8 d;

    while ((c = *zHex) != 0x00) {
      while (!(sqlite3CtypeMap[(unsigned char)(c)] & 0x08)) {
        u32 ch = (zHex[0] < 0x80 ? *(zHex++) : sqlite3Utf8Read(&zHex));

        ((void)(0))

            ;
        if (!strContainsChar(zPass, nPass, ch))
          goto unhex_null;
        c = *zHex;
        if (c == 0x00)
          goto unhex_done;
      }
      zHex++;

      ((void)(0))

          ;

      ((void)(0))

          ;
      d = *(zHex++);
      if (!(sqlite3CtypeMap[(unsigned char)(d)] & 0x08))
        goto unhex_null;
      *(p++) = (sqlite3HexToInt(c) << 4) | sqlite3HexToInt(d);
    }
  }

unhex_done:
  sqlite3_result_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  return;

unhex_null:
  sqlite3_free(pBlob);
  return;
}

void zeroblobFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  i64 n;
  int rc;

  (void)(argc);
  n = sqlite3_value_int64(argv[0]);
  if (n < 0)
    n = 0;
  rc = sqlite3_result_zeroblob64(context, n);
  if (rc) {
    sqlite3_result_error_code(context, rc);
  }
}

void replaceFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const unsigned char *zStr;
  const unsigned char *zPattern;
  const unsigned char *zRep;
  unsigned char *zOut;
  int nStr;
  int nPattern;
  int nRep;
  i64 nOut;
  int loopLimit;
  int i, j;
  unsigned cntExpand;
  sqlite3 *db = sqlite3_context_db_handle(context);

  (void)(argc);
  zStr = sqlite3_value_text(argv[0]);
  if (zStr == 0)
    return;
  nStr = sqlite3_value_bytes(argv[0]);

  zPattern = sqlite3_value_text(argv[1]);
  if (zPattern == 0) {

    ((void)(0))

        ;
    return;
  }
  if (zPattern[0] == 0) {

    ((void)(0))

        ;
    sqlite3_result_text(context, (const char *)zStr, nStr, ((sqlite3_destructor_type)-1));
    return;
  }
  nPattern = sqlite3_value_bytes(argv[1]);

  zRep = sqlite3_value_text(argv[2]);
  if (zRep == 0)
    return;
  nRep = sqlite3_value_bytes(argv[2]);

  nOut = nStr + 1;

  zOut = contextMalloc(context, nOut);
  if (zOut == 0) {
    return;
  }
  loopLimit = nStr - nPattern;
  cntExpand = 0;
  for (i = j = 0; i <= loopLimit; i++) {
    if (zStr[i] != zPattern[0] || memcmp(&zStr[i], zPattern, nPattern)) {
      zOut[j++] = zStr[i];
    } else {
      if (nRep > nPattern) {
        nOut += nRep - nPattern;
        ;
        ;
        if (nOut - 1 > db->aLimit[0]) {
          sqlite3_result_error_toobig(context);
          sqlite3_free(zOut);
          return;
        }
        cntExpand++;
        if ((cntExpand & (cntExpand - 1)) == 0) {

          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (int)nOut + (nOut - nStr - 1));
          if (zOut == 0) {
            sqlite3_result_error_nomem(context);
            sqlite3_free(zOld);
            return;
          }
        }
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern - 1;
    }
  }

  memcpy(&zOut[j], &zStr[i], nStr - i);
  j += nStr - i;

  zOut[j] = 0;
  sqlite3_result_text(context, (char *)zOut, j, sqlite3_free);
}

void trimFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const unsigned char *zIn;
  const unsigned char *zCharSet;
  unsigned int nIn;
  int flags;
  int i;
  unsigned int *aLen = 0;
  unsigned char **azChar = 0;
  int nChar;

  if (sqlite3_value_type(argv[0]) == 5) {
    return;
  }
  zIn = sqlite3_value_text(argv[0]);
  if (zIn == 0)
    return;
  nIn = (unsigned)sqlite3_value_bytes(argv[0]);

  if (argc == 1) {
    static const unsigned lenOne[] = {1};
    static unsigned char *const azOne[] = {(u8 *)" "};
    nChar = 1;
    aLen = (unsigned *)lenOne;
    azChar = (unsigned char **)azOne;
    zCharSet = 0;
  } else if ((zCharSet = sqlite3_value_text(argv[1])) == 0) {
    return;
  } else {
    const unsigned char *z;
    for (z = zCharSet, nChar = 0; *z; nChar++) {
      {
        if ((*(z++)) >= 0xc0) {
          while ((*z & 0xc0) == 0x80) {
            z++;
          }
        }
      };
    }
    if (nChar > 0) {
      azChar = contextMalloc(context, ((i64)nChar) * (sizeof(char *) + sizeof(unsigned)));
      if (azChar == 0) {
        return;
      }
      aLen = (unsigned *)&azChar[nChar];
      for (z = zCharSet, nChar = 0; *z; nChar++) {
        azChar[nChar] = (unsigned char *)z;
        {
          if ((*(z++)) >= 0xc0) {
            while ((*z & 0xc0) == 0x80) {
              z++;
            }
          }
        };
        aLen[nChar] = (unsigned)(z - azChar[nChar]);
      }
    }
  }
  if (nChar > 0) {
    flags = ((int)(intptr_t)(sqlite3_user_data(context)));
    if (flags & 1) {
      while (nIn > 0) {
        unsigned int len = 0;
        for (i = 0; i < nChar; i++) {
          len = aLen[i];
          if (len <= nIn && memcmp(zIn, azChar[i], len) == 0)
            break;
        }
        if (i >= nChar)
          break;
        zIn += len;
        nIn -= len;
      }
    }
    if (flags & 2) {
      while (nIn > 0) {
        unsigned int len = 0;
        for (i = 0; i < nChar; i++) {
          len = aLen[i];
          if (len <= nIn && memcmp(&zIn[nIn - len], azChar[i], len) == 0)
            break;
        }
        if (i >= nChar)
          break;
        nIn -= len;
      }
    }
    if (zCharSet) {
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (char *)zIn, nIn, ((sqlite3_destructor_type)-1));
}

void concatFuncCore(sqlite3_context *context, int argc, sqlite3_value **argv, int nSep, const char *zSep) {
  i64 j, n = 0;
  int i;
  int bNotNull = 0;
  char *z;
  for (i = 0; i < argc; i++) {
    n += sqlite3_value_bytes(argv[i]);
  }
  n += (argc - 1) * (i64)nSep;
  z = sqlite3_malloc64(n + 1);
  if (z == 0) {
    sqlite3_result_error_nomem(context);
    return;
  }
  j = 0;
  for (i = 0; i < argc; i++) {
    if (sqlite3_value_type(argv[i]) != 5) {
      int k = sqlite3_value_bytes(argv[i]);
      const char *v = (const char *)sqlite3_value_text(argv[i]);
      if (v != 0) {
        if (bNotNull && nSep > 0) {
          memcpy(&z[j], zSep, nSep);
          j += nSep;
        }
        memcpy(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;

  sqlite3_result_text64(context, z, j, sqlite3_free, 16);
}

void concatFunc(sqlite3_context *context, int argc, sqlite3_value **argv) { concatFuncCore(context, argc, argv, 0, ""); }

void concatwsFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int nSep = sqlite3_value_bytes(argv[0]);
  const char *zSep = (const char *)sqlite3_value_text(argv[0]);
  if (zSep == 0)
    return;
  concatFuncCore(context, argc - 1, argv + 1, nSep, zSep);
}

void loadExt(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const char *zFile = (const char *)sqlite3_value_text(argv[0]);
  const char *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  char *zErrMsg = 0;

  if ((db->flags & 0x00020000) == 0) {
    sqlite3_result_error(context, "not authorized", -1);
    return;
  }

  if (argc == 2) {
    zProc = (const char *)sqlite3_value_text(argv[1]);
  } else {
    zProc = 0;
  }
  if (zFile && sqlite3_load_extension(db, zFile, zProc, &zErrMsg)) {
    sqlite3_result_error(context, zErrMsg, -1);
    sqlite3_free(zErrMsg);
  }
}

void sumStep(sqlite3_context *context, int argc, sqlite3_value **argv) {
  SumCtx *p;
  int type;

  (void)(argc);
  p = sqlite3_aggregate_context(context, sizeof(*p));
  type = sqlite3_value_numeric_type(argv[0]);
  if (p && type != 5) {
    p->cnt++;
    if (p->approx == 0) {
      if (type != 1) {
        kahanBabuskaNeumaierInit(p, p->iSum);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_value_double(argv[0]));
      } else {
        i64 x = p->iSum;
        if (sqlite3AddInt64(&x, sqlite3_value_int64(argv[0])) == 0) {
          p->iSum = x;
        } else {
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->iSum);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_value_int64(argv[0]));
        }
      }
    } else {
      if (type == 1) {
        kahanBabuskaNeumaierStepInt64(p, sqlite3_value_int64(argv[0]));
      } else {
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_value_double(argv[0]));
      }
    }
  }
}

void sumInverse(sqlite3_context *context, int argc, sqlite3_value **argv) {
  SumCtx *p;
  int type;

  (void)(argc);
  p = sqlite3_aggregate_context(context, sizeof(*p));
  type = sqlite3_value_numeric_type(argv[0]);

  if ((p) && type != 5) {

    ((void)(0))

        ;
    p->cnt--;
    if (!p->approx) {
      i64 x = p->iSum;
      if (sqlite3SubInt64(&x, sqlite3_value_int64(argv[0])) == 0) {
        p->iSum = x;
        return;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->iSum);
    }
    if (type == 1) {
      i64 iVal = sqlite3_value_int64(argv[0]);
      if (iVal != (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32)))) {
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      } else {
        kahanBabuskaNeumaierStepInt64(p, (0xffffffff | (((i64)0x7fffffff) << 32)));
        kahanBabuskaNeumaierStepInt64(p, 1);
      }
    } else {
      kahanBabuskaNeumaierStep(p, -sqlite3_value_double(argv[0]));
    }
  }
}

void sumFinalize(sqlite3_context *context) {
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if (p && p->cnt > 0) {
    if (p->approx) {
      if (p->ovrfl) {
        sqlite3_result_error(context, "integer overflow", -1);
      } else if (!sqlite3IsOverflow(p->rErr)) {
        sqlite3_result_double(context, p->rSum + p->rErr);
      } else {
        sqlite3_result_double(context, p->rSum);
      }
    } else {
      sqlite3_result_int64(context, p->iSum);
    }
  }
}

void avgFinalize(sqlite3_context *context) {
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if (p && p->cnt > 0) {
    double r;
    if (p->approx) {
      r = p->rSum;
      if (!sqlite3IsOverflow(p->rErr))
        r += p->rErr;
    } else {
      r = (double)(p->iSum);
    }
    sqlite3_result_double(context, r / (double)p->cnt);
  }
}

void totalFinalize(sqlite3_context *context) {
  SumCtx *p;
  double r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  if (p) {
    if (p->approx) {
      r = p->rSum;
      if (!sqlite3IsOverflow(p->rErr))
        r += p->rErr;
    } else {
      r = (double)(p->iSum);
    }
  }
  sqlite3_result_double(context, r);
}

void countStep(sqlite3_context *context, int argc, sqlite3_value **argv) {
  CountCtx *p;
  p = sqlite3_aggregate_context(context, sizeof(*p));
  if ((argc == 0 || 5 != sqlite3_value_type(argv[0])) && p) {
    p->n++;
  }
}

void countFinalize(sqlite3_context *context) {
  CountCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}

void countInverse(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  CountCtx *p;
  p = sqlite3_aggregate_context(ctx, sizeof(*p));

  if ((argc == 0 || 5 != sqlite3_value_type(argv[0])) && (p)) {
    p->n--;
  }
}

void minmaxStep(sqlite3_context *context, int NotUsed, sqlite3_value **argv) {
  Mem *pArg = (Mem *)argv[0];
  Mem *pBest;
  (void)(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, sizeof(*pBest));
  if (!pBest)
    return;

  if (sqlite3_value_type(pArg) == 5) {
    if (pBest->flags)
      sqlite3SkipAccumulatorLoad(context);
  } else if (pBest->flags) {
    int max;
    int cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);

    max = sqlite3_user_data(context) != 0;
    cmp = sqlite3MemCompare(pBest, pArg, pColl);
    if ((max && cmp < 0) || (!max && cmp > 0)) {
      sqlite3VdbeMemCopy(pBest, pArg);
    } else {
      sqlite3SkipAccumulatorLoad(context);
    }
  } else {
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}

void minMaxValueFinalize(sqlite3_context *context, int bValue) {
  sqlite3_value *pRes;
  pRes = (sqlite3_value *)sqlite3_aggregate_context(context, 0);
  if (pRes) {
    if (pRes->flags) {
      sqlite3_result_value(context, pRes);
    }
    if (bValue == 0)
      sqlite3VdbeMemRelease(pRes);
  }
}

void minMaxValue(sqlite3_context *context) { minMaxValueFinalize(context, 1); }

void minMaxFinalize(sqlite3_context *context) { minMaxValueFinalize(context, 0); }

void groupConcatStep(sqlite3_context *context, int argc, sqlite3_value **argv) {
  const char *zVal;
  GroupConcatCtx *pGCC;
  const char *zSep;
  int nVal, nSep;

  if (sqlite3_value_type(argv[0]) == 5)
    return;
  pGCC = (GroupConcatCtx *)sqlite3_aggregate_context(context, sizeof(*pGCC));
  if (pGCC) {
    sqlite3 *db = sqlite3_context_db_handle(context);
    int firstTerm = pGCC->str.mxAlloc == 0;
    pGCC->str.mxAlloc = db->aLimit[0];
    if (argc == 1) {
      if (!firstTerm) {
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }

      else {
        pGCC->nFirstSepLength = 1;
      }

    } else if (!firstTerm) {
      zSep = (char *)sqlite3_value_text(argv[1]);
      nSep = sqlite3_value_bytes(argv[1]);
      if (zSep) {
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }

      else {
        nSep = 0;
      }
      if (nSep != pGCC->nFirstSepLength || pGCC->pnSepLengths != 0) {
        int *pnsl = pGCC->pnSepLengths;
        if (pnsl == 0) {

          pnsl = (int *)sqlite3_malloc64((pGCC->nAccum + 1) * sizeof(int));
          if (pnsl != 0) {
            int i = 0, nA = pGCC->nAccum - 1;
            while (i < nA)
              pnsl[i++] = pGCC->nFirstSepLength;
          }
        } else {
          pnsl = (int *)sqlite3_realloc64(pnsl, pGCC->nAccum * sizeof(int));
        }
        if (pnsl != 0) {
          if ((pGCC->nAccum > 0)) {
            pnsl[pGCC->nAccum - 1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        } else {
          sqlite3StrAccumSetError(&pGCC->str, 7);
        }
      }

    }

    else {
      pGCC->nFirstSepLength = sqlite3_value_bytes(argv[1]);
    }
    pGCC->nAccum += 1;

    zVal = (char *)sqlite3_value_text(argv[0]);
    nVal = sqlite3_value_bytes(argv[0]);
    if (zVal)
      sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

void groupConcatInverse(sqlite3_context *context, int argc, sqlite3_value **argv) {
  GroupConcatCtx *pGCC;

  (void)argc;
  if (sqlite3_value_type(argv[0]) == 5)
    return;
  pGCC = (GroupConcatCtx *)sqlite3_aggregate_context(context, sizeof(*pGCC));

  if ((pGCC)) {
    int nVS;

    (void)sqlite3_value_text(argv[0]);
    nVS = sqlite3_value_bytes(argv[0]);
    pGCC->nAccum -= 1;
    if (pGCC->pnSepLengths != 0) {

      ((void)(0))

          ;
      if (pGCC->nAccum > 0) {
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths + 1, (pGCC->nAccum - 1) * sizeof(int));
      }
    } else {

      nVS += pGCC->nFirstSepLength;
    }
    if (nVS >= (int)pGCC->str.nChar) {
      pGCC->str.nChar = 0;
    } else {
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    if (pGCC->str.nChar == 0) {
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}

void groupConcatFinalize(sqlite3_context *context) {
  GroupConcatCtx *pGCC = (GroupConcatCtx *)sqlite3_aggregate_context(context, 0);
  if (pGCC) {
    sqlite3ResultStrAccum(context, &pGCC->str);

    sqlite3_free(pGCC->pnSepLengths);
  }
}

void groupConcatValue(sqlite3_context *context) {
  GroupConcatCtx *pGCC = (GroupConcatCtx *)sqlite3_aggregate_context(context, 0);
  if (pGCC) {
    StrAccum *pAccum = &pGCC->str;
    if (pAccum->accError == 18) {
      sqlite3_result_error_toobig(context);
    } else if (pAccum->accError == 7) {
      sqlite3_result_error_nomem(context);
    } else if (pGCC->nAccum > 0 && pAccum->nChar == 0) {
      sqlite3_result_text(context, "", 1, ((sqlite3_destructor_type)0));
    } else {
      const char *zText = sqlite3_str_value(pAccum);
      sqlite3_result_text(context, zText, pAccum->nChar, ((sqlite3_destructor_type)-1));
    }
  }
}

void ceilingFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {

  switch (sqlite3_value_numeric_type(argv[0])) {
  case 1: {
    sqlite3_result_int64(context, sqlite3_value_int64(argv[0]));
    break;
  }
  case 2: {
    double (*x)(double) = (double (*)(double))sqlite3_user_data(context);
    sqlite3_result_double(context, x(sqlite3_value_double(argv[0])));
    break;
  }
  default: {
    break;
  }
  }
}

void logFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  double x, b, ans;

  switch (sqlite3_value_numeric_type(argv[0])) {
  case 1:
  case 2:
    x = sqlite3_value_double(argv[0]);
    if (x <= 0.0)
      return;
    break;
  default:
    return;
  }
  if (argc == 2) {
    switch (sqlite3_value_numeric_type(argv[0])) {
    case 1:
    case 2:
      b = log(x);
      if (b <= 0.0)
        return;
      x = sqlite3_value_double(argv[1]);
      if (x <= 0.0)
        return;
      break;
    default:
      return;
    }
    ans = log(x) / b;
  } else {
    switch (((int)(intptr_t)(sqlite3_user_data(context)))) {
    case 1:
      ans = log10(x);
      break;
    case 2:
      ans = log2(x);
      break;
    default:
      ans = log(x);
      break;
    }
  }
  sqlite3_result_double(context, ans);
}

void math1Func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int type0;
  double v0, ans;
  double (*x)(double);

  type0 = sqlite3_value_numeric_type(argv[0]);
  if (type0 != 1 && type0 != 2)
    return;
  v0 = sqlite3_value_double(argv[0]);
  x = (double (*)(double))sqlite3_user_data(context);
  ans = x(v0);
  sqlite3_result_double(context, ans);
}

void math2Func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int type0, type1;
  double v0, v1, ans;
  double (*x)(double, double);

  type0 = sqlite3_value_numeric_type(argv[0]);
  if (type0 != 1 && type0 != 2)
    return;
  type1 = sqlite3_value_numeric_type(argv[1]);
  if (type1 != 1 && type1 != 2)
    return;
  v0 = sqlite3_value_double(argv[0]);
  v1 = sqlite3_value_double(argv[1]);
  x = (double (*)(double, double))sqlite3_user_data(context);
  ans = x(v0, v1);
  sqlite3_result_double(context, ans);
}

void piFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {

  (void)argv;
  sqlite3_result_double(context,

                        3.14159265358979323846

  );
}

void signFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  int type0;
  double x;
  (void)(argc);

  type0 = sqlite3_value_numeric_type(argv[0]);
  if (type0 != 1 && type0 != 2)
    return;
  x = sqlite3_value_double(argv[0]);
  sqlite3_result_int(context, x < 0.0 ? -1 : x > 0.0 ? +1 : 0);
}

void percentError(sqlite3_context *pCtx, const char *zFormat, ...) {
  char *zMsg1;
  char *zMsg2;
  va_list ap;

  va_start(

      ap, zFormat

  )

      ;
  zMsg1 = sqlite3_vmprintf(zFormat, ap);

  va_end(

      ap

  )

      ;
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_result_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

void percentStep(sqlite3_context *pCtx, int argc, sqlite3_value **argv) {
  Percentile *p;
  double rPct;
  int eType;
  double y;

  if (argc == 1) {

    rPct = 0.5;
  } else {

    double mxFrac = (((int)(intptr_t)(sqlite3_user_data(pCtx))) & 2) ? 100.0 : 1.0;
    eType = sqlite3_value_numeric_type(argv[1]);
    rPct = sqlite3_value_double(argv[1]) / mxFrac;
    if ((eType != 1 && eType != 2) || rPct < 0.0 || rPct > 1.0) {
      percentError(pCtx,
                   "the fraction argument to %%s()"
                   " is not between 0.0 and %.1f",
                   (double)mxFrac);
      return;
    }
  }

  p = (Percentile *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p == 0)
    return;

  if (!p->bPctValid) {
    p->rPct = rPct;
    p->bPctValid = 1;
  } else if (!percentSameValue(p->rPct, rPct)) {
    percentError(pCtx, "the fraction argument to %%s()"
                       " is not the same for all input rows");
    return;
  }

  eType = sqlite3_value_type(argv[0]);
  if (eType == 5)
    return;

  if (eType != 1 && eType != 2) {
    percentError(pCtx, "input to %%s() is not numeric");
    return;
  }

  y = sqlite3_value_double(argv[0]);
  if (percentIsInfinity(y)) {
    percentError(pCtx, "Inf input to %%s()");
    return;
  }

  if (p->nUsed >= p->nAlloc) {
    u64 n = p->nAlloc * 2 + 250;
    double *a = sqlite3_realloc64(p->a, sizeof(double) * n);
    if (a == 0) {
      sqlite3_free(p->a);
      memset(p, 0, sizeof(*p));
      sqlite3_result_error_nomem(pCtx);
      return;
    }
    p->nAlloc = n;
    p->a = a;
  }
  if (p->nUsed == 0) {
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  } else if (!p->bSorted || y >= p->a[p->nUsed - 1]) {
    p->a[p->nUsed++] = y;
  } else if (p->bKeepSorted) {
    i64 i;
    i = percentBinarySearch(p, y, 0);
    if (i < (int)p->nUsed) {
      memmove(&p->a[i + 1], &p->a[i], (p->nUsed - i) * sizeof(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  } else {
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

void percentInverse(sqlite3_context *pCtx, int argc, sqlite3_value **argv) {
  Percentile *p;
  int eType;
  double y;
  i64 i;

  p = (Percentile *)sqlite3_aggregate_context(pCtx, sizeof(*p));

  eType = sqlite3_value_type(argv[0]);
  if (eType == 5)
    return;

  if (eType != 1 && eType != 2) {
    return;
  }

  y = sqlite3_value_double(argv[0]);
  if (percentIsInfinity(y)) {
    return;
  }
  if (p->bSorted == 0) {

    ((void)(0))

        ;
    percentSort(p->a, p->nUsed);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  i = percentBinarySearch(p, y, 1);
  if (i >= 0) {
    p->nUsed--;
    if (i < (int)p->nUsed) {
      memmove(&p->a[i], &p->a[i + 1], (p->nUsed - i) * sizeof(p->a[0]));
    }
  }
}

void percentCompute(sqlite3_context *pCtx, int bIsFinal) {
  Percentile *p;
  int settings = ((int)(intptr_t)(sqlite3_user_data(pCtx))) & 1;
  unsigned i1, i2;
  double v1, v2;
  double ix, vx;
  p = (Percentile *)sqlite3_aggregate_context(pCtx, 0);
  if (p == 0)
    return;
  if (p->a == 0)
    return;
  if (p->nUsed) {
    if (p->bSorted == 0) {

      ((void)(0))

          ;
      percentSort(p->a, p->nUsed);
      p->bSorted = 1;
    }
    ix = p->rPct * (p->nUsed - 1);
    i1 = (unsigned)ix;
    if (settings & 1) {
      vx = p->a[i1];
    } else {
      i2 = ix == (double)i1 || i1 == p->nUsed - 1 ? i1 : i1 + 1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2 - v1) * (ix - i1);
    }
    sqlite3_result_double(pCtx, vx);
  }
  if (bIsFinal) {
    sqlite3_free(p->a);
    memset(p, 0, sizeof(*p));
  } else {
    p->bKeepSorted = 1;
  }
}

void percentFinal(sqlite3_context *pCtx) { percentCompute(pCtx, 1); }

void percentValue(sqlite3_context *pCtx) { percentCompute(pCtx, 0); }

void row_numberStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  i64 *p = (i64 *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p)
    (*p)++;
  (void)(nArg);
  (void)(apArg);
}

void row_numberValueFunc(sqlite3_context *pCtx) {
  i64 *p = (i64 *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  sqlite3_result_int64(pCtx, (p ? *p : 0));
}

void dense_rankStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct CallCount *p;
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p)
    p->nStep = 1;
  (void)(nArg);
  (void)(apArg);
}

void dense_rankValueFunc(sqlite3_context *pCtx) {
  struct CallCount *p;
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    if (p->nStep) {
      p->nValue++;
      p->nStep = 0;
    }
    sqlite3_result_int64(pCtx, p->nValue);
  }
}

void nth_valueStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct NthValueCtx *p;
  p = (struct NthValueCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    i64 iVal;
    switch (sqlite3_value_numeric_type(apArg[1])) {
    case 1:
      iVal = sqlite3_value_int64(apArg[1]);
      break;
    case 2: {
      double fVal = sqlite3_value_double(apArg[1]);
      if (sqlite3RealToI64(fVal) != fVal)
        goto error_out;
      iVal = (i64)fVal;
      break;
    }
    default:
      goto error_out;
    }
    if (iVal <= 0)
      goto error_out;

    p->nStep++;
    if (iVal == p->nStep) {
      p->pValue = sqlite3_value_dup(apArg[0]);
      if (!p->pValue) {
        sqlite3_result_error_nomem(pCtx);
      }
    }
  }
  (void)(nArg);
  (void)(apArg);
  return;

error_out:
  sqlite3_result_error(pCtx, "second argument to nth_value must be a positive integer", -1);
}

void nth_valueFinalizeFunc(sqlite3_context *pCtx) {
  struct NthValueCtx *p;
  p = (struct NthValueCtx *)sqlite3_aggregate_context(pCtx, 0);
  if (p && p->pValue) {
    sqlite3_result_value(pCtx, p->pValue);
    sqlite3_value_free(p->pValue);
    p->pValue = 0;
  }
}

void first_valueStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct NthValueCtx *p;
  p = (struct NthValueCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p && p->pValue == 0) {
    p->pValue = sqlite3_value_dup(apArg[0]);
    if (!p->pValue) {
      sqlite3_result_error_nomem(pCtx);
    }
  }
  (void)(nArg);
  (void)(apArg);
}

void first_valueFinalizeFunc(sqlite3_context *pCtx) {
  struct NthValueCtx *p;
  p = (struct NthValueCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p && p->pValue) {
    sqlite3_result_value(pCtx, p->pValue);
    sqlite3_value_free(p->pValue);
    p->pValue = 0;
  }
}

void rankStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct CallCount *p;
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    p->nStep++;
    if (p->nValue == 0) {
      p->nValue = p->nStep;
    }
  }
  (void)(nArg);
  (void)(apArg);
}

void rankValueFunc(sqlite3_context *pCtx) {
  struct CallCount *p;
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    sqlite3_result_int64(pCtx, p->nValue);
    p->nValue = 0;
  }
}

void percent_rankStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct CallCount *p;
  (void)(nArg);

  (void)(apArg);
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    p->nTotal++;
  }
}

void percent_rankInvFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct CallCount *p;
  (void)(nArg);

  (void)(apArg);
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  p->nStep++;
}

void percent_rankValueFunc(sqlite3_context *pCtx) {
  struct CallCount *p;
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    p->nValue = p->nStep;
    if (p->nTotal > 1) {
      double r = (double)p->nValue / (double)(p->nTotal - 1);
      sqlite3_result_double(pCtx, r);
    } else {
      sqlite3_result_double(pCtx, 0.0);
    }
  }
}

void cume_distStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct CallCount *p;
  (void)(nArg);

  (void)(apArg);
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    p->nTotal++;
  }
}

void cume_distInvFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct CallCount *p;
  (void)(nArg);

  (void)(apArg);
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  p->nStep++;
}

void cume_distValueFunc(sqlite3_context *pCtx) {
  struct CallCount *p;
  p = (struct CallCount *)sqlite3_aggregate_context(pCtx, 0);
  if (p) {
    double r = (double)(p->nStep) / (double)(p->nTotal);
    sqlite3_result_double(pCtx, r);
  }
}

void ntileStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct NtileCtx *p;

  (void)(nArg);
  p = (struct NtileCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    if (p->nTotal == 0) {
      p->nParam = sqlite3_value_int64(apArg[0]);
      if (p->nParam <= 0) {
        sqlite3_result_error(pCtx, "argument of ntile must be a positive integer", -1);
      }
    }
    p->nTotal++;
  }
}

void ntileInvFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct NtileCtx *p;

  (void)(nArg);
  (void)(apArg);
  p = (struct NtileCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  p->iRow++;
}

void ntileValueFunc(sqlite3_context *pCtx) {
  struct NtileCtx *p;
  p = (struct NtileCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p && p->nParam > 0) {
    int nSize = (p->nTotal / p->nParam);
    if (nSize == 0) {
      sqlite3_result_int64(pCtx, p->iRow + 1);
    } else {
      i64 nLarge = p->nTotal - p->nParam * nSize;
      i64 iSmall = nLarge * (nSize + 1);
      i64 iRow = p->iRow;

      ((void)(0))

          ;

      if (iRow < iSmall) {
        sqlite3_result_int64(pCtx, 1 + iRow / (nSize + 1));
      } else {
        sqlite3_result_int64(pCtx, 1 + nLarge + (iRow - iSmall) / nSize);
      }
    }
  }
}

void last_valueStepFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct LastValueCtx *p;
  (void)(nArg);
  p = (struct LastValueCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p) {
    sqlite3_value_free(p->pVal);
    p->pVal = sqlite3_value_dup(apArg[0]);
    if (p->pVal == 0) {
      sqlite3_result_error_nomem(pCtx);
    } else {
      p->nVal++;
    }
  }
}

void last_valueInvFunc(sqlite3_context *pCtx, int nArg, sqlite3_value **apArg) {
  struct LastValueCtx *p;
  (void)(nArg);
  (void)(apArg);
  p = (struct LastValueCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if ((p)) {
    p->nVal--;
    if (p->nVal == 0) {
      sqlite3_value_free(p->pVal);
      p->pVal = 0;
    }
  }
}

void last_valueValueFunc(sqlite3_context *pCtx) {
  struct LastValueCtx *p;
  p = (struct LastValueCtx *)sqlite3_aggregate_context(pCtx, 0);
  if (p && p->pVal) {
    sqlite3_result_value(pCtx, p->pVal);
  }
}

void last_valueFinalizeFunc(sqlite3_context *pCtx) {
  struct LastValueCtx *p;
  p = (struct LastValueCtx *)sqlite3_aggregate_context(pCtx, sizeof(*p));
  if (p && p->pVal) {
    sqlite3_result_value(pCtx, p->pVal);
    sqlite3_value_free(p->pVal);
    p->pVal = 0;
  }
}

void noopStepFunc(sqlite3_context *p, int n, sqlite3_value **a) {
  (void)(p);
  (void)(n);
  (void)(a);
}

void noopValueFunc(sqlite3_context *p) { (void)(p); }

void sqlite3InvalidFunction(sqlite3_context *context, int NotUsed, sqlite3_value **NotUsed2) {
  const char *zName = (const char *)sqlite3_user_data(context);
  char *zErr;
  (void)(NotUsed), (void)(NotUsed2);
  zErr = sqlite3_mprintf("unable to use function %s in the requested context", zName);
  sqlite3_result_error(context, zErr, -1);
  sqlite3_free(zErr);
}

int jsonCacheInsert(sqlite3_context *ctx, JsonParse *pParse) {
  JsonCache *p;

  p = sqlite3_get_auxdata(ctx, (-429938));
  if (p == 0) {
    sqlite3 *db = sqlite3_context_db_handle(ctx);
    p = sqlite3DbMallocZero(db, sizeof(*p));
    if (p == 0)
      return 7;
    p->db = db;
    sqlite3_set_auxdata(ctx, (-429938), p, jsonCacheDeleteGeneric);
    p = sqlite3_get_auxdata(ctx, (-429938));
    if (p == 0)
      return 7;
  }
  if (p->nUsed >= 4) {
    jsonParseFree(p->a[0]);
    memmove(p->a, &p->a[1], (4 - 1) * sizeof(p->a[0]));
    p->nUsed = 4 - 1;
  }

  pParse->eEdit = 0;
  pParse->nJPRef++;
  pParse->bReadOnly = 1;
  p->a[p->nUsed] = pParse;
  p->nUsed++;
  return 0;
}

JsonParse *jsonCacheSearch(sqlite3_context *ctx, sqlite3_value *pArg) {
  JsonCache *p;
  int i;
  const char *zJson;
  int nJson;

  if (sqlite3_value_type(pArg) != 3) {
    return 0;
  }
  zJson = (const char *)sqlite3_value_text(pArg);
  if (zJson == 0)
    return 0;
  nJson = sqlite3_value_bytes(pArg);

  p = sqlite3_get_auxdata(ctx, (-429938));
  if (p == 0) {
    return 0;
  }
  for (i = 0; i < p->nUsed; i++) {
    if (p->a[i]->zJson == zJson)
      break;
  }
  if (i >= p->nUsed) {
    for (i = 0; i < p->nUsed; i++) {
      if (p->a[i]->nJson != nJson)
        continue;
      if (memcmp(p->a[i]->zJson, zJson, nJson) == 0)
        break;
    }
  }
  if (i < p->nUsed) {
    if (i < p->nUsed - 1) {

      JsonParse *tmp = p->a[i];
      memmove(&p->a[i], &p->a[i + 1], (p->nUsed - i - 1) * sizeof(tmp));
      p->a[p->nUsed - 1] = tmp;
      i = p->nUsed - 1;
    }

    ((void)(0))

        ;
    return p->a[i];
  } else {
    return 0;
  }
}

void jsonWrongNumArgs(sqlite3_context *pCtx, const char *zFuncName) {
  char *zMsg = sqlite3_mprintf("json_%s() needs an odd number of arguments", zFuncName);
  sqlite3_result_error(pCtx, zMsg, -1);
  sqlite3_free(zMsg);
}

void jsonReturnTextJsonFromBlob(sqlite3_context *ctx, const u8 *aBlob, u32 nBlob) {
  JsonParse x;
  JsonString s;

  if ((aBlob == 0))
    return;
  memset(&x, 0, sizeof(x));
  x.aBlob = (u8 *)aBlob;
  x.nBlob = nBlob;
  jsonStringInit(&s, ctx);
  jsonTranslateBlobToText(&x, 0, &s);
  jsonReturnString(&s, 0, 0);
}

int jsonFunctionArgToBlob(sqlite3_context *ctx, sqlite3_value *pArg, JsonParse *pParse) {
  int eType = sqlite3_value_type(pArg);
  static u8 aNull[] = {0x00};
  memset(pParse, 0, sizeof(pParse[0]));
  pParse->db = sqlite3_context_db_handle(ctx);
  switch (eType) {
  default: {
    pParse->aBlob = aNull;
    pParse->nBlob = 1;
    return 0;
  }
  case 4: {
    if (!jsonArgIsJsonb(pArg, pParse)) {
      sqlite3_result_error(ctx, "JSON cannot hold BLOB values", -1);
      return 1;
    }
    break;
  }
  case 3: {
    const char *zJson = (const char *)sqlite3_value_text(pArg);
    int nJson = sqlite3_value_bytes(pArg);
    if (zJson == 0)
      return 1;
    if (sqlite3_value_subtype(pArg) == 74) {
      pParse->zJson = (char *)zJson;
      pParse->nJson = nJson;
      if (jsonConvertTextToBlob(pParse, ctx)) {
        sqlite3_result_error(ctx, "malformed JSON", -1);
        sqlite3DbFree(pParse->db, pParse->aBlob);
        memset(pParse, 0, sizeof(pParse[0]));
        return 1;
      }
    } else {
      jsonBlobAppendNode(pParse, 10, nJson, zJson);
    }
    break;
  }
  case 2: {
    double r = sqlite3_value_double(pArg);
    if ((sqlite3IsNaN(r))) {
      jsonBlobAppendNode(pParse, 0, 0, 0);
    } else {
      int n = sqlite3_value_bytes(pArg);
      const char *z = (const char *)sqlite3_value_text(pArg);
      if (z == 0)
        return 1;
      if (z[0] == 'I') {
        jsonBlobAppendNode(pParse, 5, 5, "9e999");
      } else if (z[0] == '-' && z[1] == 'I') {
        jsonBlobAppendNode(pParse, 5, 6, "-9e999");
      } else {
        jsonBlobAppendNode(pParse, 5, n, z);
      }
    }
    break;
  }
  case 1: {
    int n = sqlite3_value_bytes(pArg);
    const char *z = (const char *)sqlite3_value_text(pArg);
    if (z == 0)
      return 1;
    jsonBlobAppendNode(pParse, 3, n, z);
    break;
  }
  }
  if (pParse->oom) {
    sqlite3_result_error_nomem(ctx);
    return 1;
  } else {
    return 0;
  }
}

char *jsonBadPathError(sqlite3_context *ctx, const char *zPath, int rc) {
  char *zMsg;
  if (rc == (int)0xfffffffd) {
    zMsg = sqlite3_mprintf("not an array element: %Q", zPath);
  } else if (rc == (int)0xffffffff) {
    zMsg = sqlite3_mprintf("malformed JSON");
  } else if (rc == (int)0xfffffffc) {
    zMsg = sqlite3_mprintf("JSON path too deep");
  } else {
    zMsg = sqlite3_mprintf("bad JSON path: %Q", zPath);
  }
  if (ctx == 0)
    return zMsg;
  if (zMsg) {
    sqlite3_result_error(ctx, zMsg, -1);
    sqlite3_free(zMsg);
  } else {
    sqlite3_result_error_nomem(ctx);
  }
  return 0;
}

void jsonInsertIntoBlob(sqlite3_context *ctx, int argc, sqlite3_value **argv, int eEdit) {
  int i;
  u32 rc = 0;
  const char *zPath = 0;
  int flgs;
  JsonParse *p;
  JsonParse ax;

  flgs = argc == 1 ? 0 : 0x01;
  p = jsonParseFuncArg(ctx, argv[0], flgs);
  if (p == 0)
    return;
  for (i = 1; i < argc - 1; i += 2) {
    if (sqlite3_value_type(argv[i]) == 5)
      continue;
    zPath = (const char *)sqlite3_value_text(argv[i]);
    if (zPath == 0) {
      sqlite3_result_error_nomem(ctx);
      jsonParseFree(p);
      return;
    }
    if (zPath[0] != '$')
      goto jsonInsertIntoBlob_patherror;
    if (jsonFunctionArgToBlob(ctx, argv[i + 1], &ax)) {
      jsonParseReset(&ax);
      jsonParseFree(p);
      return;
    }
    if (zPath[1] == 0) {
      if (eEdit == 2 || eEdit == 4) {
        jsonBlobEdit(p, 0, p->nBlob, ax.aBlob, ax.nBlob);
      }
      rc = 0;
    } else {
      p->eEdit = eEdit;
      p->nIns = ax.nBlob;
      p->aIns = ax.aBlob;
      p->delta = 0;
      p->iDepth = 0;
      rc = jsonLookupStep(p, 0, zPath + 1, 0);
    }
    jsonParseReset(&ax);
    if (rc == 0xfffffffe)
      continue;
    if (((rc) >= 0xfffffffb))
      goto jsonInsertIntoBlob_patherror;
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  return;

jsonInsertIntoBlob_patherror:
  jsonParseFree(p);
  jsonBadPathError(ctx, zPath, rc);
  return;
}

JsonParse *jsonParseFuncArg(sqlite3_context *ctx, sqlite3_value *pArg, u32 flgs) {
  int eType;
  JsonParse *p = 0;
  JsonParse *pFromCache = 0;
  sqlite3 *db;

  eType = sqlite3_value_type(pArg);
  if (eType == 5) {
    return 0;
  }
  pFromCache = jsonCacheSearch(ctx, pArg);
  if (pFromCache) {
    pFromCache->nJPRef++;
    if ((flgs & 0x01) == 0) {
      return pFromCache;
    }
  }
  db = sqlite3_context_db_handle(ctx);
rebuild_from_cache:
  p = sqlite3DbMallocZero(db, sizeof(*p));
  if (p == 0)
    goto json_pfa_oom;
  memset(p, 0, sizeof(*p));
  p->db = db;
  p->nJPRef = 1;
  if (pFromCache != 0) {
    u32 nBlob = pFromCache->nBlob;
    p->aBlob = sqlite3DbMallocRaw(db, nBlob);
    if (p->aBlob == 0)
      goto json_pfa_oom;
    memcpy(p->aBlob, pFromCache->aBlob, nBlob);
    p->nBlobAlloc = p->nBlob = nBlob;
    p->hasNonstd = pFromCache->hasNonstd;
    jsonParseFree(pFromCache);
    return p;
  }
  if (eType == 4) {
    if (jsonArgIsJsonb(pArg, p)) {
      if ((flgs & 0x01) != 0 && jsonBlobMakeEditable(p, 0) == 0) {
        goto json_pfa_oom;
      }
      return p;
    }
  }
  p->zJson = (char *)sqlite3_value_text(pArg);
  p->nJson = sqlite3_value_bytes(pArg);
  if (db->mallocFailed)
    goto json_pfa_oom;
  if (p->nJson == 0)
    goto json_pfa_malformed;

  if (jsonConvertTextToBlob(p, (flgs & 0x02) ? 0 : ctx)) {
    if (flgs & 0x02) {
      p->nErr = 1;
      return p;
    } else {
      jsonParseFree(p);
      return 0;
    }
  } else {
    int isRCStr = sqlite3ValueIsOfClass(pArg, sqlite3RCStrUnref);
    int rc;
    if (!isRCStr) {
      char *zNew = sqlite3RCStrNew(p->nJson);
      if (zNew == 0)
        goto json_pfa_oom;
      memcpy(zNew, p->zJson, p->nJson);
      p->zJson = zNew;
      p->zJson[p->nJson] = 0;
    } else {
      sqlite3RCStrRef(p->zJson);
    }
    p->bJsonIsRCStr = 1;
    rc = jsonCacheInsert(ctx, p);
    if (rc == 7)
      goto json_pfa_oom;
    if (flgs & 0x01) {
      pFromCache = p;
      p = 0;
      goto rebuild_from_cache;
    }
  }
  return p;

json_pfa_malformed:
  if (flgs & 0x02) {
    p->nErr = 1;
    return p;
  } else {
    jsonParseFree(p);
    sqlite3_result_error(ctx, "malformed JSON", -1);
    return 0;
  }

json_pfa_oom:
  jsonParseFree(pFromCache);
  jsonParseFree(p);
  sqlite3_result_error_nomem(ctx);
  return 0;
}

void jsonReturnParse(sqlite3_context *ctx, JsonParse *p) {
  int flgs;
  if (p->oom) {
    sqlite3_result_error_nomem(ctx);
    return;
  }
  flgs = ((int)(intptr_t)(sqlite3_user_data(ctx)));
  if (flgs & 0x10) {
    if (p->nBlobAlloc > 0 && !p->bReadOnly) {
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, ((sqlite3_destructor_type)sqlite3RowSetClear));
      p->nBlobAlloc = 0;
    } else {
      sqlite3_result_blob(ctx, p->aBlob, p->nBlob, ((sqlite3_destructor_type)-1));
    }
  } else {
    JsonString s;
    jsonStringInit(&s, ctx);
    p->delta = 0;
    jsonTranslateBlobToText(p, 0, &s);
    jsonReturnString(&s, p, ctx);
    sqlite3_result_subtype(ctx, 74);
  }
}

void jsonQuoteFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonString jx;
  (void)(argc);

  jsonStringInit(&jx, ctx);
  jsonAppendSqlValue(&jx, argv[0]);
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, 74);
}

void jsonArrayFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  int i;
  JsonString jx;

  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '[');
  for (i = 0; i < argc; i++) {
    jsonAppendSeparator(&jx);
    jsonAppendSqlValue(&jx, argv[i]);
  }
  jsonAppendChar(&jx, ']');
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, 74);
}

void jsonArrayLengthFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonParse *p;
  sqlite3_int64 cnt = 0;
  u32 i;
  u8 eErr = 0;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  if (p == 0)
    return;
  if (argc == 2) {
    const char *zPath = (const char *)sqlite3_value_text(argv[1]);
    if (zPath == 0) {
      jsonParseFree(p);
      return;
    }
    i = jsonLookupStep(p, 0, zPath[0] == '$' ? zPath + 1 : "@", 0);
    if (((i) >= 0xfffffffb)) {
      if (i == 0xfffffffe) {

      } else {
        jsonBadPathError(ctx, zPath, i);
      }
      eErr = 1;
      i = 0;
    }
  } else {
    i = 0;
  }
  if ((p->aBlob[i] & 0x0f) == 11) {
    cnt = jsonbArrayCount(p, i);
  }
  if (!eErr)
    sqlite3_result_int64(ctx, cnt);
  jsonParseFree(p);
}

void jsonExtractFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonParse *p = 0;
  int flags;
  int i;
  JsonString jx;

  if (argc < 2)
    return;
  p = jsonParseFuncArg(ctx, argv[0], 0);
  if (p == 0)
    return;
  flags = ((int)(intptr_t)(sqlite3_user_data(ctx)));
  jsonStringInit(&jx, ctx);
  if (argc > 2) {
    jsonAppendChar(&jx, '[');
  }
  for (i = 1; i < argc; i++) {

    const char *zPath = (const char *)sqlite3_value_text(argv[i]);
    int nPath;
    u32 j;
    if (zPath == 0)
      goto json_extract_error;
    nPath = sqlite3Strlen30(zPath);
    if (zPath[0] == '$') {
      j = jsonLookupStep(p, 0, zPath + 1, 0);
    } else if ((flags & 0x03)) {

      jsonStringInit(&jx, ctx);
      if (sqlite3_value_type(argv[i]) == 1) {
        jsonAppendRawNZ(&jx, "[", 1);
        if (zPath[0] == '-')
          jsonAppendRawNZ(&jx, "#", 1);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "]", 2);
      } else if (jsonAllAlphanum(zPath, nPath)) {
        jsonAppendRawNZ(&jx, ".", 1);
        jsonAppendRaw(&jx, zPath, nPath);
      } else if (zPath[0] == '[' && nPath >= 3 && zPath[nPath - 1] == ']') {
        jsonAppendRaw(&jx, zPath, nPath);
      } else {
        jsonAppendRawNZ(&jx, ".\"", 2);
        jsonAppendRaw(&jx, zPath, nPath);
        jsonAppendRawNZ(&jx, "\"", 1);
      }
      jsonStringTerminate(&jx);
      j = jsonLookupStep(p, 0, jx.zBuf, 0);
      jsonStringReset(&jx);
    } else {
      jsonBadPathError(ctx, zPath, 0);
      goto json_extract_error;
    }
    if (j < p->nBlob) {
      if (argc == 2) {
        if (flags & 0x01) {
          jsonStringInit(&jx, ctx);
          jsonTranslateBlobToText(p, j, &jx);
          jsonReturnString(&jx, 0, 0);
          jsonStringReset(&jx);

          ((void)(0))

              ;
          sqlite3_result_subtype(ctx, 74);
        } else {
          jsonReturnFromBlob(p, j, ctx, 0);
          if ((flags & (0x02 | 0x10)) == 0 && (p->aBlob[j] & 0x0f) >= 11) {
            sqlite3_result_subtype(ctx, 74);
          }
        }
      } else {
        jsonAppendSeparator(&jx);
        jsonTranslateBlobToText(p, j, &jx);
      }
    } else if (j == 0xfffffffe) {
      if (argc == 2) {
        goto json_extract_error;
      } else {
        jsonAppendSeparator(&jx);
        jsonAppendRawNZ(&jx, "null", 4);
      }
    } else {
      jsonBadPathError(ctx, zPath, j);
      goto json_extract_error;
    }
  }
  if (argc > 2) {
    jsonAppendChar(&jx, ']');
    jsonReturnString(&jx, 0, 0);
    if ((flags & 0x10) == 0) {
      sqlite3_result_subtype(ctx, 74);
    }
  }
json_extract_error:
  jsonStringReset(&jx);
  jsonParseFree(p);
  return;
}

void jsonPatchFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonParse *pTarget;
  JsonParse *pPatch;
  int rc;

  (void)(argc);

  pTarget = jsonParseFuncArg(ctx, argv[0], 0x01);
  if (pTarget == 0)
    return;
  pPatch = jsonParseFuncArg(ctx, argv[1], 0);
  if (pPatch) {
    rc = jsonMergePatch(pTarget, 0, pPatch, 0, 0);
    if (rc == 0) {
      jsonReturnParse(ctx, pTarget);
    } else if (rc == 3) {
      sqlite3_result_error_nomem(ctx);
    } else if (rc == 4) {
      sqlite3_result_error(ctx, "JSON nested too deep", -1);
    } else {
      sqlite3_result_error(ctx, "malformed JSON", -1);
    }
    jsonParseFree(pPatch);
  }
  jsonParseFree(pTarget);
}

void jsonObjectFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  int i;
  JsonString jx;
  const char *z;
  u32 n;

  if (argc & 1) {
    sqlite3_result_error(ctx,
                         "json_object() requires an even number "
                         "of arguments",
                         -1);
    return;
  }
  jsonStringInit(&jx, ctx);
  jsonAppendChar(&jx, '{');
  for (i = 0; i < argc; i += 2) {
    if (sqlite3_value_type(argv[i]) != 3) {
      sqlite3_result_error(ctx, "json_object() labels must be TEXT", -1);
      jsonStringReset(&jx);
      return;
    }
    jsonAppendSeparator(&jx);
    z = (const char *)sqlite3_value_text(argv[i]);
    n = sqlite3_value_bytes(argv[i]);
    jsonAppendString(&jx, z, n);
    jsonAppendChar(&jx, ':');
    jsonAppendSqlValue(&jx, argv[i + 1]);
  }
  jsonAppendChar(&jx, '}');
  jsonReturnString(&jx, 0, 0);
  sqlite3_result_subtype(ctx, 74);
}

void jsonRemoveFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonParse *p;
  const char *zPath = 0;
  int i;
  u32 rc;

  if (argc < 1)
    return;
  p = jsonParseFuncArg(ctx, argv[0], argc > 1 ? 0x01 : 0);
  if (p == 0)
    return;
  for (i = 1; i < argc; i++) {
    zPath = (const char *)sqlite3_value_text(argv[i]);
    if (zPath == 0) {
      goto json_remove_done;
    }
    if (zPath[0] != '$') {
      goto json_remove_patherror;
    }
    if (zPath[1] == 0) {

      goto json_remove_done;
    }
    p->eEdit = 1;
    p->delta = 0;
    rc = jsonLookupStep(p, 0, zPath + 1, 0);
    if (((rc) >= 0xfffffffb)) {
      if (rc == 0xfffffffe) {
        continue;
      } else {
        jsonBadPathError(ctx, zPath, rc);
      }
      goto json_remove_done;
    }
  }
  jsonReturnParse(ctx, p);
  jsonParseFree(p);
  return;

json_remove_patherror:
  jsonBadPathError(ctx, zPath, 0);

json_remove_done:
  jsonParseFree(p);
  return;
}

void jsonReplaceFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  if (argc < 1)
    return;
  if ((argc & 1) == 0) {
    jsonWrongNumArgs(ctx, "replace");
    return;
  }
  jsonInsertIntoBlob(ctx, argc, argv, 2);
}

void jsonSetFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  int flags = ((int)(intptr_t)(sqlite3_user_data(ctx)));
  int eInsType = (((flags) & 0xC) >> 2);
  static const char *azInsType[] = {"insert", "set", "array_insert"};
  static const u8 aEditType[] = {3, 4, 5};

  if (argc < 1)
    return;

  if ((argc & 1) == 0) {
    jsonWrongNumArgs(ctx, azInsType[eInsType]);
    return;
  }
  jsonInsertIntoBlob(ctx, argc, argv, aEditType[eInsType]);
}

void jsonTypeFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonParse *p;
  const char *zPath = 0;
  u32 i;

  p = jsonParseFuncArg(ctx, argv[0], 0);
  if (p == 0)
    return;
  if (argc == 2) {
    zPath = (const char *)sqlite3_value_text(argv[1]);
    if (zPath == 0)
      goto json_type_done;
    if (zPath[0] != '$') {
      jsonBadPathError(ctx, zPath, 0);
      goto json_type_done;
    }
    i = jsonLookupStep(p, 0, zPath + 1, 0);
    if (((i) >= 0xfffffffb)) {
      if (i == 0xfffffffe) {

      } else {
        jsonBadPathError(ctx, zPath, i);
      }
      goto json_type_done;
    }
  } else {
    i = 0;
  }
  sqlite3_result_text(ctx, jsonbType[p->aBlob[i] & 0x0f], -1, ((sqlite3_destructor_type)0));
json_type_done:
  jsonParseFree(p);
}

void jsonPrettyFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonString s;
  JsonPretty x;

  memset(&x, 0, sizeof(x));
  x.pParse = jsonParseFuncArg(ctx, argv[0], 0);
  if (x.pParse == 0)
    return;
  x.pOut = &s;
  jsonStringInit(&s, ctx);
  if (argc == 1 || (x.zIndent = (const char *)sqlite3_value_text(argv[1])) == 0) {
    x.zIndent = "    ";
    x.szIndent = 4;
  } else {
    x.szIndent = (u32)strlen(x.zIndent);
  }
  jsonTranslateBlobToPrettyText(&x, 0);
  jsonReturnString(&s, 0, 0);
  jsonParseFree(x.pParse);
}

void jsonValidFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonParse *p;
  u8 flags = 1;
  u8 res = 0;
  if (argc == 2) {
    i64 f = sqlite3_value_int64(argv[1]);
    if (f < 1 || f > 15) {
      sqlite3_result_error(ctx,
                           "FLAGS parameter to json_valid() must be"
                           " between 1 and 15",
                           -1);
      return;
    }
    flags = f & 0x0f;
  }
  switch (sqlite3_value_type(argv[0])) {
  case 5: {

    return;
  }
  case 4: {
    JsonParse py;
    memset(&py, 0, sizeof(py));
    if (jsonArgIsJsonb(argv[0], &py)) {
      if (flags & 0x04) {

        res = 1;
      } else if (flags & 0x08) {

        res = 0 == jsonbValidityCheck(&py, 0, py.nBlob, 1);
      }
      break;
    }

    __attribute__((fallthrough));
  }
  default: {
    JsonParse px;
    if ((flags & 0x3) == 0)
      break;
    memset(&px, 0, sizeof(px));

    p = jsonParseFuncArg(ctx, argv[0], 0x02);
    if (p) {
      if (p->oom) {
        sqlite3_result_error_nomem(ctx);
      } else if (p->nErr) {

      } else if ((flags & 0x02) != 0 || p->hasNonstd == 0) {
        res = 1;
      }
      jsonParseFree(p);
    } else {
      sqlite3_result_error_nomem(ctx);
    }
    break;
  }
  }
  sqlite3_result_int(ctx, res);
}

void jsonErrorFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  i64 iErrPos = 0;
  JsonParse s;

  (void)(argc);
  memset(&s, 0, sizeof(s));
  s.db = sqlite3_context_db_handle(ctx);
  if (jsonArgIsJsonb(argv[0], &s)) {
    iErrPos = (i64)jsonbValidityCheck(&s, 0, s.nBlob, 1);
  } else {
    s.zJson = (char *)sqlite3_value_text(argv[0]);
    if (s.zJson == 0)
      return;
    s.nJson = sqlite3_value_bytes(argv[0]);
    if (jsonConvertTextToBlob(&s, 0)) {
      if (s.oom) {
        iErrPos = -1;
      } else {

        u32 k;

        ((void)(0))

            ;
        for (k = 0; k < s.iErr && (s.zJson[k]); k++) {
          if ((s.zJson[k] & 0xc0) != 0x80)
            iErrPos++;
        }
        iErrPos++;
      }
    }
  }
  jsonParseReset(&s);
  if (iErrPos < 0) {
    sqlite3_result_error_nomem(ctx);
  } else {
    sqlite3_result_int64(ctx, iErrPos);
  }
}

void jsonArrayStep(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonString *pStr;
  (void)(argc);
  pStr = (JsonString *)sqlite3_aggregate_context(ctx, sizeof(*pStr));
  if (pStr) {
    if (pStr->zBuf == 0) {
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '[');
    } else if (pStr->nUsed > 1) {
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    jsonAppendSqlValue(pStr, argv[0]);
  }
}

void jsonArrayCompute(sqlite3_context *ctx, int isFinal) {
  JsonString *pStr;
  int flags = ((int)(intptr_t)(sqlite3_user_data(ctx)));
  pStr = (JsonString *)sqlite3_aggregate_context(ctx, 0);
  if (pStr) {
    pStr->pCtx = ctx;
    jsonAppendRawNZ(pStr, "]", 2);
    jsonStringTrimOneChar(pStr);
    if (pStr->eErr) {
      jsonReturnString(pStr, 0, 0);
      return;
    } else if (flags & 0x10) {
      jsonReturnStringAsBlob(pStr);
      if (isFinal) {
        if (!pStr->bStatic)
          sqlite3RCStrUnref(pStr->zBuf);
      } else {
        jsonStringTrimOneChar(pStr);
      }
      return;
    } else if (isFinal) {
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed, pStr->bStatic ? ((sqlite3_destructor_type)-1) : sqlite3RCStrUnref);
      pStr->bStatic = 1;
    } else {
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed, ((sqlite3_destructor_type)-1));
      jsonStringTrimOneChar(pStr);
    }
  } else if (flags & 0x10) {
    static const u8 emptyArray = 0x0b;
    sqlite3_result_blob(ctx, &emptyArray, 1, ((sqlite3_destructor_type)0));
  } else {
    sqlite3_result_text(ctx, "[]", 2, ((sqlite3_destructor_type)0));
  }
  sqlite3_result_subtype(ctx, 74);
}

void jsonArrayValue(sqlite3_context *ctx) { jsonArrayCompute(ctx, 0); }

void jsonArrayFinal(sqlite3_context *ctx) { jsonArrayCompute(ctx, 1); }

void jsonGroupInverse(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  unsigned int i;
  int inStr = 0;
  int nNest = 0;
  char *z;
  char c;
  JsonString *pStr;
  (void)(argc);
  (void)(argv);
  pStr = (JsonString *)sqlite3_aggregate_context(ctx, 0);

  if ((!pStr))
    return;
  z = pStr->zBuf;
  for (i = 1; i < pStr->nUsed && ((c = z[i]) != ',' || inStr || nNest); i++) {
    if (c == '"') {
      inStr = !inStr;
    } else if (c == '\\') {
      i++;
    } else if (!inStr) {
      if (c == '{' || c == '[')
        nNest++;
      if (c == '}' || c == ']')
        nNest--;
    }
  }
  if (i < pStr->nUsed) {
    pStr->nUsed -= i;
    memmove(&z[1], &z[i + 1], (size_t)pStr->nUsed - 1);
    z[pStr->nUsed] = 0;
  } else {
    pStr->nUsed = 1;
  }
}

void jsonObjectStep(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  JsonString *pStr;
  const char *z;
  u32 n;
  (void)(argc);
  pStr = (JsonString *)sqlite3_aggregate_context(ctx, sizeof(*pStr));
  if (pStr) {
    z = (const char *)sqlite3_value_text(argv[0]);
    n = sqlite3Strlen30(z);
    if (pStr->zBuf == 0) {
      jsonStringInit(pStr, ctx);
      jsonAppendChar(pStr, '{');
    } else if (pStr->nUsed > 1) {
      jsonAppendChar(pStr, ',');
    }
    pStr->pCtx = ctx;
    if (z != 0) {
      jsonAppendString(pStr, z, n);
      jsonAppendChar(pStr, ':');
      jsonAppendSqlValue(pStr, argv[1]);
    } else {
      pStr->zBuf[0] = '@';
      jsonAppendRawNZ(pStr, "@", 1);
    }
  }
}

void jsonObjectCompute(sqlite3_context *ctx, int isFinal) {
  JsonString *pStr;
  int flags = ((int)(intptr_t)(sqlite3_user_data(ctx)));
  pStr = (JsonString *)sqlite3_aggregate_context(ctx, 0);
  if (pStr) {
    JsonString *pOgStr = pStr;
    JsonString tmpStr;
    jsonAppendRawNZ(pOgStr, "}", 2);
    jsonStringTrimOneChar(pOgStr);
    pStr->pCtx = ctx;
    if (pStr->eErr) {
      jsonReturnString(pStr, 0, 0);
      return;
    }
    if (pStr->zBuf[0] != '{') {

      u64 i, j;
      int inStr = 0;
      if (!isFinal) {

        jsonStringInit(&tmpStr, ctx);
        jsonAppendRawNZ(&tmpStr, pStr->zBuf, pStr->nUsed + 1);
        pStr = &tmpStr;
        if (pStr->eErr) {
          jsonReturnString(pStr, 0, 0);
          return;
        }
        jsonStringTrimOneChar(pStr);
      }

      pStr->zBuf[0] = '{';
      for (i = j = 1; i < pStr->nUsed; i++) {
        char c = pStr->zBuf[i];
        if (c == '"') {
          inStr = !inStr;
          pStr->zBuf[j++] = '"';
        } else if (c == '\\') {
          pStr->zBuf[j++] = '\\';
          pStr->zBuf[j++] = pStr->zBuf[++i];
        } else if (c == '@' && !inStr) {

          ((void)(0))

              ;
          if (pStr->zBuf[i + 1] == ',') {
            i++;
          } else if (pStr->zBuf[j - 1] == ',') {
            j--;
          }
        } else {
          pStr->zBuf[j++] = c;
        }
      }
      pStr->zBuf[j] = 0;
      pStr->nUsed = j;
    }
    if (flags & 0x10) {
      jsonReturnStringAsBlob(pStr);
      if (isFinal) {
        if (!pStr->bStatic)
          sqlite3RCStrUnref(pStr->zBuf);
      } else {
        jsonStringTrimOneChar(pOgStr);
      }
    } else if (isFinal) {
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed, pStr->bStatic ? ((sqlite3_destructor_type)-1) : sqlite3RCStrUnref);
      pStr->bStatic = 1;
    } else {
      sqlite3_result_text(ctx, pStr->zBuf, (int)pStr->nUsed, ((sqlite3_destructor_type)-1));
      jsonStringTrimOneChar(pOgStr);
    }
    if (pStr != pOgStr)
      jsonStringReset(pStr);
  } else if (flags & 0x10) {
    static const unsigned char emptyObject = 0x0c;
    sqlite3_result_blob(ctx, &emptyObject, 1, ((sqlite3_destructor_type)0));
  } else {
    sqlite3_result_text(ctx, "{}", 2, ((sqlite3_destructor_type)0));
  }
  sqlite3_result_subtype(ctx, 74);
}

void jsonObjectValue(sqlite3_context *ctx) { jsonObjectCompute(ctx, 0); }

void jsonObjectFinal(sqlite3_context *ctx) { jsonObjectCompute(ctx, 1); }