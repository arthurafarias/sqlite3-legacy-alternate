#define _GNU_SOURCE 1

#include "sqlite/sqlite3_complete.h"

#include "sqlite/sqlite3.h"
#include "sqlite/u8.h"

int sqlite3_complete(const char *zSql) {
  u8 state = 0;
  u8 token;

  static const u8 trans[8][8] = {

      {
          1,
          0,
          2,
          3,
          4,
          2,
          2,
          2,
      },
      {
          1,
          1,
          2,
          3,
          4,
          2,
          2,
          2,
      },
      {
          1,
          2,
          2,
          2,
          2,
          2,
          2,
          2,
      },
      {
          1,
          3,
          3,
          2,
          4,
          2,
          2,
          2,
      },
      {
          1,
          4,
          2,
          2,
          2,
          4,
          5,
          2,
      },
      {
          6,
          5,
          5,
          5,
          5,
          5,
          5,
          5,
      },
      {
          6,
          6,
          5,
          5,
          5,
          5,
          5,
          7,
      },
      {
          1,
          7,
          5,
          5,
          5,
          5,
          5,
          5,
      },
  };

  while (*zSql) {
    switch (*zSql) {
    case ';': {
      token = 0;
      break;
    }
    case ' ':
    case '\r':
    case '\t':
    case '\n':
    case '\f': {
      token = 1;
      break;
    }
    case '/': {
      if (zSql[1] != '*') {
        token = 2;
        break;
      }
      zSql += 2;
      while (zSql[0] && (zSql[0] != '*' || zSql[1] != '/')) {
        zSql++;
      }
      if (zSql[0] == 0)
        return 0;
      zSql++;
      token = 1;
      break;
    }
    case '-': {
      if (zSql[1] != '-') {
        token = 2;
        break;
      }
      while (*zSql && *zSql != '\n') {
        zSql++;
      }
      if (*zSql == 0)
        return state == 1;
      token = 1;
      break;
    }
    case '[': {
      zSql++;
      while (*zSql && *zSql != ']') {
        zSql++;
      }
      if (*zSql == 0)
        return 0;
      token = 2;
      break;
    }
    case '`':
    case '"':
    case '\'': {
      int c = *zSql;
      zSql++;
      while (*zSql && *zSql != c) {
        zSql++;
      }
      if (*zSql == 0)
        return 0;
      token = 2;
      break;
    }
    default: {

      if (((sqlite3CtypeMap[(unsigned char)(u8)*zSql] & 0x46) != 0)) {

        int nId;
        for (nId = 1; ((sqlite3CtypeMap[(unsigned char)zSql[nId]] & 0x46) != 0); nId++) {
        }

        switch (*zSql) {
        case 'c':
        case 'C': {
          if (nId == 6 && sqlite3_strnicmp(zSql, "create", 6) == 0) {
            token = 4;
          } else {
            token = 2;
          }
          break;
        }
        case 't':
        case 'T': {
          if (nId == 7 && sqlite3_strnicmp(zSql, "trigger", 7) == 0) {
            token = 6;
          } else if (nId == 4 && sqlite3_strnicmp(zSql, "temp", 4) == 0) {
            token = 5;
          } else if (nId == 9 && sqlite3_strnicmp(zSql, "temporary", 9) == 0) {
            token = 5;
          } else {
            token = 2;
          }
          break;
        }
        case 'e':
        case 'E': {
          if (nId == 3 && sqlite3_strnicmp(zSql, "end", 3) == 0) {
            token = 7;
          } else

              if (nId == 7 && sqlite3_strnicmp(zSql, "explain", 7) == 0) {
            token = 3;
          } else

          {
            token = 2;
          }
          break;
        }
        default: {
          token = 2;
          break;
        }
        }

        zSql += nId - 1;
      } else {

        token = 2;
      }
      break;
    }
    }
    state = trans[state][token];
    zSql++;
  }
  return state == 1;
}
