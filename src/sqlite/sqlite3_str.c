#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>

#include "sqlite/sqlite3_str.h"

#include "sqlite/Expr.h"
#include "sqlite/FpDecode.h"
#include "sqlite/PrintfArguments.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/StrAccum.h"
#include "sqlite/Subquery.h"
#include "sqlite/Token.h"
#include "sqlite/etByte.h"
#include "sqlite/et_info.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite_uint64.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
static const char aDigits[] = "0123456789ABCDEF0123456789abcdef";

static const char aPrefix[] = "-x0\000X0";

static const et_info fmtinfo[23] = {{'s', 0, 4, 5, 0, 0, 1}, {'E', 0, 1, 2, 14, 0, 0}, {'u', 10, 0, 16, 0, 0, 3}, {'G', 0, 1, 3, 14, 0, 0}, {'w', 0, 4, 14, 0, 0, 0}, {'x', 16, 0, 0, 16, 1, 0}, {'c', 0, 0, 8, 0, 0, 0}, {'z', 0, 4, 6, 0, 0, 6}, {'d', 10, 1, 16, 0, 0, 0}, {'e', 0, 1, 2, 30, 0, 0}, {'f', 0, 1, 1, 0, 0, 0}, {'g', 0, 1, 3, 30, 0, 0}, {'Q', 0, 4, 10, 0, 0, 0}, {'i', 10, 1, 16, 0, 0, 0}, {'%', 0, 0, 7, 0, 0, 16}, {'T', 0, 0, 11, 0, 0, 0}, {'S', 0, 0, 12, 0, 0, 0}, {'X', 16, 0, 0, 0, 4, 0}, {'n', 0, 0, 4, 0, 0, 0}, {'o', 8, 0, 0, 0, 2, 17}, {'p', 16, 0, 13, 0, 1, 0}, {'q', 0, 4, 9, 0, 0, 0}, {'r', 10, 1, 15, 0, 0, 0}};


char *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n) {
  char *z;
  if (pAccum->accError)
    return 0;
  if (n > pAccum->nAlloc && n > pAccum->mxAlloc) {
    sqlite3StrAccumSetError(pAccum, 18);
    return 0;
  }
  z = sqlite3_malloc(n);
  if (z == 0) {
    sqlite3StrAccumSetError(pAccum, 7);
  }
  return z;
}

void sqlite3_str_vappendf(sqlite3_str *pAccum, const char *fmt, va_list ap) {
  int c;
  char *bufpt;
  int precision;
  int length;
  int idx;
  int width;
  etByte flag_leftjustify;
  etByte flag_prefix;
  etByte flag_alternateform;
  etByte flag_altform2;
  etByte flag_zeropad;
  etByte flag_long;
  etByte done;
  etByte cThousand;
  etByte xtype = 17;
  u8 bArgList;
  char prefix;
  sqlite_uint64 longvalue;
  double realvalue;
  const et_info *infop;
  char *zOut;
  int nOut;
  char *zExtra = 0;
  int exp, e2;
  etByte flag_dp;
  etByte flag_rtz;

  PrintfArguments *pArgList = 0;
  char buf[70];

  bufpt = 0;
  if ((pAccum->printfFlags & 0x02) != 0) {
    pArgList =

        va_arg(

            ap

            ,

            PrintfArguments *

        )

        ;
    bArgList = 1;
  } else {
    bArgList = 0;
  }
  for (; (c = (*fmt)) != 0; ++fmt) {
    if (c != '%') {
      bufpt = (char *)fmt;

      fmt =

          _Generic(0 ? (

                           fmt

                           )
                     : (void *)1,
          const void *: (const char *)(strchr(

              fmt

              ,

              '%'

              )),
          default: strchr(

                       fmt

                       ,

                       '%'

                       ))

          ;
      if (fmt == 0) {
        fmt = bufpt + strlen(bufpt);
      }

      sqlite3_str_append(pAccum, bufpt, (int)(fmt - bufpt));
      if (*fmt == 0)
        break;
    }
    if ((c = (*++fmt)) == 0) {
      sqlite3_str_append(pAccum, "%", 1);
      break;
    }

    flag_leftjustify = flag_prefix = cThousand = flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    do {
      switch (c) {
      case '-':
        flag_leftjustify = 1;
        break;
      case '+':
        flag_prefix = '+';
        break;
      case ' ':
        flag_prefix = ' ';
        break;
      case '#':
        flag_alternateform = 1;
        break;
      case '!':
        flag_altform2 = 1;
        break;
      case '0':
        flag_zeropad = 1;
        break;
      case ',':
        cThousand = ',';
        break;
      default:
        done = 1;
        break;
      case 'l': {
        flag_long = 1;
        c = *++fmt;
        if (c == 'l') {
          c = *++fmt;
          flag_long = 2;
        }
        done = 1;
        break;
      }
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        unsigned wx = c - '0';
        while ((c = *++fmt) >= '0' && c <= '9') {
          wx = wx * 10 + c - '0';
        };
        width = wx & 0x7fffffff;

        if (c != '.' && c != 'l') {
          done = 1;
        } else {
          fmt--;
        }
        break;
      }
      case '*': {
        if (bArgList) {
          width = (int)getIntArg(pArgList);
        } else {
          width =

              va_arg(

                  ap

                  ,

                  int

              )

              ;
        }
        if (width < 0) {
          flag_leftjustify = 1;
          width = width >= -2147483647 ? -width : 0;
        }

        if ((c = fmt[1]) != '.' && c != 'l') {
          c = *++fmt;
          done = 1;
        }
        break;
      }
      case '.': {
        c = *++fmt;
        if (c == '*') {
          if (bArgList) {
            precision = (int)getIntArg(pArgList);
          } else {
            precision =

                va_arg(

                    ap

                    ,

                    int

                )

                ;
          }
          if (precision < 0) {
            precision = precision >= -2147483647 ? -precision : -1;
          }
          c = *++fmt;
        } else {
          unsigned px = 0;
          while (c >= '0' && c <= '9') {
            px = px * 10 + c - '0';
            c = *++fmt;
          };
          precision = px & 0x7fffffff;
        }

        if (c == 'l') {
          --fmt;
        } else {
          done = 1;
        }
        break;
      }
      }
    } while (!done && (c = (*++fmt)) != 0);


    idx = ((unsigned)c) % 23;
    if (fmtinfo[idx].fmttype == c || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype == c) {
      infop = &fmtinfo[idx];
      xtype = infop->type;
    } else {
      infop = &fmtinfo[0];
      xtype = 17;
    }




    switch (xtype) {
    case 13:
      flag_long = sizeof(char *) == sizeof(i64) ? 2 : sizeof(char *) == sizeof(long int) ? 1 : 0;
      __attribute__((fallthrough));
    case 15:
    case 0:
      cThousand = 0;
      __attribute__((fallthrough));
    case 16:
      if (infop->flags & 1) {
        i64 v;
        if (bArgList) {
          v = getIntArg(pArgList);
        } else if (flag_long) {
          if (flag_long == 2) {
            v =

                va_arg(

                    ap

                    ,

                    i64

                )

                ;
          } else {
            v =

                va_arg(

                    ap

                    ,

                    long int

                )

                ;
          }
        } else {
          v =

              va_arg(

                  ap

                  ,

                  int

              )

              ;
        }
        if (v < 0) {
          ;
          ;
          longvalue = ~v;
          longvalue++;
          prefix = '-';
        } else {
          longvalue = v;
          prefix = flag_prefix;
        }
      } else {
        if (bArgList) {
          longvalue = (u64)getIntArg(pArgList);
        } else if (flag_long) {
          if (flag_long == 2) {
            longvalue =

                va_arg(

                    ap

                    ,

                    u64

                )

                ;
          } else {
            longvalue =

                va_arg(

                    ap

                    ,

                    unsigned long int

                )

                ;
          }
        } else {
          longvalue =

              va_arg(

                  ap

                  ,

                  unsigned int

              )

              ;
        }
        prefix = 0;
      }

      if (longvalue == 0)
        flag_alternateform = 0;
      if (flag_zeropad && precision < width - (prefix != 0)) {
        precision = width - (prefix != 0);
      }
      if (precision < 70 - 10 - 70 / 3) {
        nOut = 70;
        zOut = buf;
      } else {
        u64 n;
        n = (u64)precision + 10;
        if (cThousand)
          n += precision / 3;
        zOut = zExtra = printfTempBuf(pAccum, n);
        if (zOut == 0)
          return;
        nOut = (int)n;
      }
      bufpt = &zOut[nOut - 1];
      if (xtype == 15) {
        static const char zOrd[] = "thstndrd";
        int x = (int)(longvalue % 10);
        if (x >= 4 || (longvalue / 10) % 10 == 1) {
          x = 0;
        }
        *(--bufpt) = zOrd[x * 2 + 1];
        *(--bufpt) = zOrd[x * 2];
      }
      {
        const char *cset = &aDigits[infop->charset];
        u8 base = infop->base;
        do {
          *(--bufpt) = cset[longvalue % base];
          longvalue = longvalue / base;
        } while (longvalue > 0);
      }
      length = (int)(&zOut[nOut - 1] - bufpt);
      if (precision > length) {
        int nn = precision - length;
        bufpt -= nn;
        memset(bufpt, '0', nn);
        length = precision;
      }
      if (cThousand) {
        int nn = (length - 1) / 3;
        int ix = (length - 1) % 3 + 1;
        bufpt -= nn;
        for (idx = 0; nn > 0; idx++) {
          bufpt[idx] = bufpt[idx + nn];
          ix--;
          if (ix == 0) {
            bufpt[++idx] = cThousand;
            nn--;
            ix = 3;
          }
        }
      }
      if (prefix)
        *(--bufpt) = prefix;
      if (flag_alternateform && infop->prefix) {
        const char *pre;
        char x;
        pre = &aPrefix[infop->prefix];
        for (; (x = (*pre)) != 0; pre++)
          *(--bufpt) = x;
      }
      length = (int)(&zOut[nOut - 1] - bufpt);
      break;
    case 1:
    case 2:
    case 3: {
      FpDecode s;
      int iRound;
      int j;
      i64 szBufNeeded;

      if (bArgList) {
        realvalue = getDoubleArg(pArgList);
      } else {
        realvalue =

            va_arg(

                ap

                ,

                double

            )

            ;
      }
      if (precision < 0)
        precision = 6;

      if (precision > 100000000) {
        precision = 100000000;
      }

      if (xtype == 1) {
        iRound = -precision;
      } else if (xtype == 3) {
        if (precision == 0)
          precision = 1;
        iRound = precision;
      } else {
        iRound = precision + 1;
      }
      sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
      if (s.isSpecial) {
        if (s.isSpecial == 2) {
          bufpt = flag_zeropad ? "null" : "NaN";
          length = sqlite3Strlen30(bufpt);
          break;
        } else if (flag_zeropad) {
          s.z[0] = '9';
          s.iDP = 1000;
          s.n = 1;
        } else {
          memcpy(buf, "-Inf", 5);
          bufpt = buf;
          if (s.sign == '-') {

          } else if (flag_prefix) {
            buf[0] = flag_prefix;
          } else {
            bufpt++;
          }
          length = sqlite3Strlen30(bufpt);
          break;
        }
      }
      if (s.sign == '-') {
        if (flag_alternateform && !flag_prefix && xtype == 1 && s.iDP <= iRound) {

          prefix = 0;
        } else {
          prefix = '-';
        }
      } else {
        prefix = flag_prefix;
      }

      exp = s.iDP - 1;

      if (xtype == 3) {


        precision--;
        flag_rtz = !flag_alternateform;
        if (exp < -4 || exp > precision) {
          xtype = 2;
        } else {
          precision = precision - exp;
          xtype = 1;
        }
      } else {
        flag_rtz = flag_altform2;
      }
      if (xtype == 2) {
        e2 = 0;
      } else {
        e2 = s.iDP - 1;
      }

      szBufNeeded = ((e2) > (0) ? (e2) : (0)) + (i64)precision + (i64)width + 10;
      if (cThousand && e2 > 0)
        szBufNeeded += (e2 + 2) / 3;
      if (szBufNeeded + pAccum->nChar >= pAccum->nAlloc) {
        if (pAccum->mxAlloc == 0 && pAccum->accError == 0) {

          bufpt = sqlite3_malloc(szBufNeeded);
          if (bufpt == 0) {
            sqlite3StrAccumSetError(pAccum, 7);
            return;
          }
          zExtra = bufpt;
        } else if (sqlite3StrAccumEnlarge(pAccum, szBufNeeded) < szBufNeeded) {
          width = length = 0;
          break;
        } else {
          bufpt = pAccum->zText + pAccum->nChar;
        }
      } else {
        bufpt = pAccum->zText + pAccum->nChar;
      }
      zOut = bufpt;

      flag_dp = (precision > 0 ? 1 : 0) | flag_alternateform | flag_altform2;

      if (prefix) {
        *(bufpt++) = prefix;
      }

      j = 0;


      if (e2 < 0) {
        *(bufpt++) = '0';
      } else if (cThousand) {
        for (; e2 >= 0; e2--) {
          *(bufpt++) = j < s.n ? s.z[j++] : '0';
          if ((e2 % 3) == 0 && e2 > 1)
            *(bufpt++) = ',';
        }
      } else {
        j = e2 + 1;
        if (j > s.n)
          j = s.n;
        memcpy(bufpt, s.z, j);
        bufpt += j;
        e2 -= j;
        if (e2 >= 0) {
          memset(bufpt, '0', e2 + 1);
          bufpt += e2 + 1;
          e2 = -1;
        }
      }

      if (flag_dp) {
        *(bufpt++) = '.';
      }

      if (e2 < (-1) && precision > 0) {
        int nn = -1 - e2;
        if (nn > precision)
          nn = precision;
        memset(bufpt, '0', nn);
        bufpt += nn;
        precision -= nn;
      }

      if (precision > 0) {
        int nn = s.n - j;
        if ((nn > precision))
          nn = precision;
        if (nn > 0) {
          memcpy(bufpt, s.z + j, nn);
          bufpt += nn;
          precision -= nn;
        }
        if (precision > 0 && !flag_rtz) {
          memset(bufpt, '0', precision);
          bufpt += precision;
        }
      }

      if (flag_rtz && flag_dp) {
        while (bufpt[-1] == '0')
          *(--bufpt) = 0;


        if (bufpt[-1] == '.') {
          if (flag_altform2) {
            *(bufpt++) = '0';
          } else {
            *(--bufpt) = 0;
          }
        }
      }

      if (xtype == 2) {
        exp = s.iDP - 1;
        *(bufpt++) = aDigits[infop->charset];
        if (exp < 0) {
          *(bufpt++) = '-';
          exp = -exp;
        } else {
          *(bufpt++) = '+';
        }
        if (exp >= 100) {
          *(bufpt++) = (char)((exp / 100) + '0');
          exp %= 100;
        }
        *(bufpt++) = (char)(exp / 10 + '0');
        *(bufpt++) = (char)(exp % 10 + '0');
      }

      length = (int)(bufpt - zOut);


      if (length < width) {
        i64 nPad = width - length;
        if (flag_leftjustify) {
          memset(bufpt, ' ', nPad);
        } else if (!flag_zeropad) {
          memmove(zOut + nPad, zOut, length);
          memset(zOut, ' ', nPad);
        } else {
          int adj = prefix != 0;
          memmove(zOut + nPad + adj, zOut + adj, length - adj);
          memset(zOut + adj, '0', nPad);
        }
        length = width;
      }

      if (zExtra == 0) {

        pAccum->nChar += length;
        zOut[length] = 0;
        continue;
      } else {

        bufpt[0] = 0;
        bufpt = zExtra;
        break;
      }
    }
    case 4:
      if (!bArgList) {
        *(

            va_arg(

                ap

                ,

                int *

                )

                ) = pAccum->nChar;
      }
      length = width = 0;
      break;
    case 7:
      buf[0] = '%';
      bufpt = buf;
      length = 1;
      break;
    case 8:
      if (bArgList) {
        bufpt = getTextArg(pArgList);
        length = 1;
        if (bufpt) {
          buf[0] = c = *(bufpt++);
          if ((c & 0xc0) == 0xc0) {
            while (length < 4 && (bufpt[0] & 0xc0) == 0x80) {
              buf[length++] = *(bufpt++);
            }
          }
        } else {
          buf[0] = 0;
        }
      } else {
        unsigned int ch =

            va_arg(

                ap

                ,

                unsigned int

            )

            ;
        length = sqlite3AppendOneUtf8Character(buf, ch);
      }
      if (precision > 1) {
        i64 nPrior = 1;
        width -= precision - 1;
        if (width > 1 && !flag_leftjustify) {
          sqlite3_str_appendchar(pAccum, width - 1, ' ');
          width = 0;
        }
        sqlite3_str_append(pAccum, buf, length);
        precision--;
        while (precision > 1) {
          i64 nCopyBytes;
          if (nPrior > precision - 1)
            nPrior = precision - 1;
          nCopyBytes = length * nPrior;
          if (sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes)) {
            break;
          }
          sqlite3_str_append(pAccum, &pAccum->zText[pAccum->nChar - nCopyBytes], nCopyBytes);
          precision -= nPrior;
          nPrior *= 2;
        }
      }
      bufpt = buf;
      flag_altform2 = 1;
      goto adjust_width_for_utf8;
    case 5:
    case 6:
      if (bArgList) {
        bufpt = getTextArg(pArgList);
        xtype = 5;
      } else {
        bufpt =

            va_arg(

                ap

                ,

                char *

            )

            ;
      }
      if (bufpt == 0) {
        bufpt = "";
      } else if (xtype == 6) {
        if (pAccum->nChar == 0 && pAccum->mxAlloc && width == 0 && precision < 0 && pAccum->accError == 0) {


          pAccum->zText = bufpt;
          pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
          pAccum->nChar = 0x7fffffff & (int)strlen(bufpt);
          pAccum->printfFlags |= 0x04;
          length = 0;
          break;
        }
        zExtra = bufpt;
      }
      if (precision >= 0) {
        if (flag_altform2) {

          unsigned char *z = (unsigned char *)bufpt;
          while (precision-- > 0 && z[0]) {
            {
              if ((*(z++)) >= 0xc0) {
                while ((*z & 0xc0) == 0x80) {
                  z++;
                }
              }
            };
          }
          length = (int)(z - (unsigned char *)bufpt);
        } else {
          for (length = 0; length < precision && bufpt[length]; length++) {
          }
        }
      } else {
        length = 0x7fffffff & (int)strlen(bufpt);
      }
    adjust_width_for_utf8:
      if (flag_altform2 && width > 0) {

        int ii = length - 1;
        while (ii >= 0)
          if ((bufpt[ii--] & 0xc0) == 0x80)
            width++;
      }
      break;
    case 9:
    case 10:
    case 14: {
      i64 i, j, k, n;
      int needQuote = 0;
      char ch;
      char *escarg;
      char q;

      if (bArgList) {
        escarg = getTextArg(pArgList);
      } else {
        escarg =

            va_arg(

                ap

                ,

                char *

            )

            ;
      }
      if (escarg == 0) {
        escarg = (xtype == 10 ? "NULL" : "(NULL)");
      } else if (xtype == 10) {
        needQuote = 1;
      }
      if (xtype == 14) {
        q = '"';
        flag_alternateform = 0;
      } else {
        q = '\'';
      }

      k = precision;
      for (i = n = 0; k != 0 && (ch = escarg[i]) != 0; i++, k--) {
        if (ch == q)
          n++;
        if (flag_altform2 && (ch & 0xc0) == 0xc0) {
          while ((escarg[i + 1] & 0xc0) == 0x80) {
            i++;
          }
        }
      }
      if (flag_alternateform) {

        i64 nBack = 0;
        i64 nCtrl = 0;
        for (k = 0; k < i; k++) {
          if (escarg[k] == '\\') {
            nBack++;
          } else if (((u8 *)escarg)[k] <= 0x1f) {
            nCtrl++;
          }
        }
        if (nCtrl || xtype == 9) {
          n += nBack + 5 * nCtrl;
          if (xtype == 10) {
            n += 10;
            needQuote = 2;
          }
        } else {
          flag_alternateform = 0;
        }
      }
      n += i + 3;
      if (n > 70) {
        bufpt = zExtra = printfTempBuf(pAccum, n);
        if (bufpt == 0)
          return;
      } else {
        bufpt = buf;
      }
      j = 0;
      if (needQuote) {
        if (needQuote == 2) {
          memcpy(&bufpt[j], "unistr('", 8);
          j += 8;
        } else {
          bufpt[j++] = '\'';
        }
      }
      k = i;
      if (flag_alternateform) {
        for (i = 0; i < k; i++) {
          bufpt[j++] = ch = escarg[i];
          if (ch == q) {
            bufpt[j++] = ch;
          } else if (ch == '\\') {
            bufpt[j++] = '\\';
          } else if (((unsigned char)ch) <= 0x1f) {
            bufpt[j - 1] = '\\';
            bufpt[j++] = 'u';
            bufpt[j++] = '0';
            bufpt[j++] = '0';
            bufpt[j++] = ch >= 0x10 ? '1' : '0';
            bufpt[j++] = "0123456789abcdef"[ch & 0xf];
          }
        }
      } else {
        for (i = 0; i < k; i++) {
          bufpt[j++] = ch = escarg[i];
          if (ch == q)
            bufpt[j++] = ch;
        }
      }
      if (needQuote) {
        bufpt[j++] = '\'';
        if (needQuote == 2)
          bufpt[j++] = ')';
      }
      bufpt[j] = 0;
      length = j;
      goto adjust_width_for_utf8;
    }
    case 11: {
      if ((pAccum->printfFlags & 0x01) == 0)
        return;
      if (flag_alternateform) {

        Expr *pExpr =

            va_arg(

                ap

                ,

                Expr *

            )

            ;
        if ((pExpr) && (!(((pExpr)->flags & (u32)(0x000800)) != 0))) {
          sqlite3_str_appendall(pAccum, (const char *)pExpr->u.zToken);
          sqlite3RecordErrorOffsetOfExpr(pAccum->db, pExpr);
        }
      } else {

        Token *pToken =

            va_arg(

                ap

                ,

                Token *

            )

            ;


        if (pToken && pToken->n) {
          sqlite3_str_append(pAccum, (const char *)pToken->z, pToken->n);
          sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
        }
      }
      length = width = 0;
      break;
    }
    case 12: {
      SrcItem *pItem;
      if ((pAccum->printfFlags & 0x01) == 0)
        return;
      pItem =

          va_arg(

              ap

              ,

              SrcItem *

          )

          ;


      if (pItem->zAlias && !flag_altform2) {
        sqlite3_str_appendall(pAccum, pItem->zAlias);
      } else if (pItem->zName) {
        if (pItem->fg.fixedSchema == 0 && pItem->fg.isSubquery == 0 && pItem->u4.zDatabase != 0) {
          sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
          sqlite3_str_append(pAccum, ".", 1);
        }
        sqlite3_str_appendall(pAccum, pItem->zName);
      } else if (pItem->zAlias) {
        sqlite3_str_appendall(pAccum, pItem->zAlias);
      } else if ((pItem->fg.isSubquery)) {
        Select *pSel = pItem->u4.pSubq->pSelect;


        if (pSel->selFlags & 0x0000800) {
          sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
        } else if (pSel->selFlags & 0x0000400) {


          sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE", pItem->u1.nRow);
        } else {
          sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
        }
      }
      length = width = 0;
      break;
    }
    default: {


      return;
    }
    }

    width -= length;
    if (width > 0) {
      if (!flag_leftjustify)
        sqlite3_str_appendchar(pAccum, width, ' ');
      sqlite3_str_append(pAccum, bufpt, length);
      if (flag_leftjustify)
        sqlite3_str_appendchar(pAccum, width, ' ');
    } else {
      sqlite3_str_append(pAccum, bufpt, length);
    }

    if (zExtra) {
      sqlite3DbFree(pAccum->db, zExtra);
      zExtra = 0;
    }
  }
}

void sqlite3_str_appendchar(sqlite3_str *p, int N, char c) {
  ;
  if (p->nChar + (i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N)) <= 0) {
    return;
  }
  while ((N--) > 0)
    p->zText[p->nChar++] = c;
}

void sqlite3_str_append(sqlite3_str *p, const char *z, int N) {

  if (p->nChar + N >= p->nAlloc) {
    enlargeAndAppend(p, z, N);
  } else if (N) {


    p->nChar += N;
    memcpy(&p->zText[p->nChar - N], z, N);
  }
}

void sqlite3_str_appendall(sqlite3_str *p, const char *z) { sqlite3_str_append(p, z, sqlite3Strlen30(z)); }

sqlite3_str sqlite3OomStr = {0, 0, 0, 0, 0, 7, 0};

char *sqlite3_str_finish(sqlite3_str *p) {
  char *z;
  if (p != 0 && p != &sqlite3OomStr) {
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  } else {
    z = 0;
  }
  return z;
}

int sqlite3_str_errcode(sqlite3_str *p) { return p ? p->accError : 7; }

int sqlite3_str_length(sqlite3_str *p) { return p ? p->nChar : 0; }

void sqlite3_str_truncate(sqlite3_str *p, int N) {
  if (p != 0 && N >= 0 && (u32)N < p->nChar) {
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

char *sqlite3_str_value(sqlite3_str *p) {
  if (p == 0 || p->nChar == 0)
    return 0;
  p->zText[p->nChar] = 0;
  return p->zText;
}

void sqlite3_str_free(sqlite3_str *p) {
  if (p != 0 && p != &sqlite3OomStr) {
    sqlite3_str_reset(p);
    sqlite3_free(p);
  }
}