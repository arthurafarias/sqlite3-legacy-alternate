#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/yyParser.h"

#include "sqlite/Cte.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FrameBound.h"
#include "sqlite/IdList.h"
#include "sqlite/Index.h"
#include "sqlite/OnOrUsing.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/SelectDest.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Token.h"
#include "sqlite/TrigEvent.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/Upsert.h"
#include "sqlite/Vdbe.h"
#include "sqlite/Window.h"
#include "sqlite/With.h"
#include "sqlite/YYMINORTYPE.h"
#include "sqlite/YyAction.h"
#include "sqlite/bft.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/yyStackEntry.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static void *parserStackRealloc(void *pOld, sqlite3_uint64 newSize, Parse *pParse);
static unsigned short int yy_find_reduce_action(unsigned short int stateno, unsigned short int iLookAhead);

static void *parserStackRealloc(void *pOld, sqlite3_uint64 newSize, Parse *pParse) {
  void *p = sqlite3FaultSim(700) ? 0 : sqlite3_realloc(pOld, newSize);
  if (p == 0)
    sqlite3OomFault(pParse->db);
  return p;
}

static const short yy_reduce_ofst[] = {
    -67, 345, -64, -178, -181, 143, 435, -78, -183, 163, -185, 284, 384, -174, 189, 352, 440, 444, 493, -23, 227, -277, -1, 305, 561, 755, 759, 764, -189, 839, 857, 354, 484, 859, 631, 67, 734, 780, -187, 616, 581, 730, 891, 449, 588, 795, 836, -238, 287, -238, 287, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, 205, 582, 715, 958, 985, 1003, 1005, 1010, 1012, 1059, 1066, 1092, 1094, 1097, 1122, 1137, 1141, 1143, 1147, 1151, 1172, 1249, 1251, 1269, 1271, 1276, 1290, 1316, 1318, 1337, 1371, 1373, 1375, 1400, 1413, 1418, 1420, 1422, 1425, 1427, 1433, 1438, 1447, 1454, 1459, 1463, 1467, 1480, 1484, 1518, 1523, 1525, 1527, 1529, 1531, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, 155, 210, -220, 86, -130, 943, 996, 402, -256, -113, 981, 1095, 1135, 395, -256, -256, -256, -256, 568, 568, 568, -4, -153, -133, 259, 306, -166, 523, -303, -126, 503, 503, -37, -149, 164, 690, 292, 412, 492, 651, 784, 332, 786, 841, 1149, 833, 1236, 792, 162, 796, 1253, 777, 288, 381, 380, 709, 487, 1027, 972, 1030, 1084, 991, 1120, -152, 1062, 692, 1240, 1247, 1250, 1239, 1306, -207, -194, 57, 180, 74, 315, 355, 376, 452, 488, 630, 693, 965, 1004, 1025, 1099, 1154, 1289, 1305, 1310, 1469, 1489, 984, 1494, 1502, 1516, 1544, 1556, 1557, 1562, 1576, 1578, 1579, 1583, 1584, 1585, 1586, 1217, 1440, 1554, 1589, 1593, 1594, 1530, 1597, 1599, 1600, 1539, 1472, 1601, 1560, 1604, 355, 1605, 1609, 1610, 1612, 1613, 1614, 1503, 1512, 1520, 1567, 1548, 1558, 1559, 1561, 1530, 1567, 1567, 1569, 1596, 1622, 1519, 1521, 1547, 1565, 1581, 1568, 1534, 1582, 1563, 1566, 1591, 1570, 1606, 1536, 1619, 1615, 1616, 1624, 1626, 1633, 1590, 1595, 1603, 1617, 1618, 1621, 1572, 1623, 1628, 1663, 1644, 1577, 1647, 1648, 1673, 1675, 1588, 1627, 1678, 1630, 1629, 1634, 1651, 1650, 1652, 1653, 1654, 1688, 1701, 1655, 1635, 1636, 1658, 1639, 1672, 1661, 1677, 1666, 1715, 1717, 1632, 1642, 1724, 1726, 1712, 1728, 1736, 1737, 1739, 1718, 1722, 1723, 1725, 1719, 1720, 1721, 1727, 1731, 1734, 1735, 1738, 1740, 1742, 1643, 1656, 1669, 1674, 1753, 1759, 1645, 1646, 1711, 1713, 1748, 1744, 1709, 1789, 1716, 1749, 1752, 1755, 1758, 1807, 1816, 1818, 1823, 1824, 1825, 1745, 1754, 1714, 1811, 1806, 1808, 1809, 1810, 1813, 1802, 1803, 1812, 1815, 1817, 1820,
};

static unsigned short int yy_find_reduce_action(unsigned short int stateno, unsigned short int iLookAhead) {
  int i;

  i = yy_reduce_ofst[stateno];

  i += iLookAhead;

  return yy_action[i];
}

static const unsigned short int yyRuleInfoLhs[] = {
    191, 191, 190, 192, 193, 193, 193, 193, 192, 192, 192, 192, 192, 197, 199, 201, 201, 200, 200, 198, 198, 205, 205, 207, 207, 208, 210, 210, 210, 211, 215, 216, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 217, 226, 226, 222, 222, 224, 224, 227, 227, 227, 227, 228, 228, 228, 228, 228, 225, 225, 229, 229, 229, 204, 231, 232, 232, 232, 232, 232, 235, 220, 220, 236, 236, 237, 237, 192, 239, 239, 192, 192, 192, 206, 206, 206, 241, 244, 244, 244, 242, 242, 254, 242, 256, 256, 245, 245, 245, 257, 246, 246, 246, 258, 258, 247, 247, 260, 260, 259, 259, 259, 259, 259, 202, 202, 240, 240, 265, 265, 265, 265, 261, 261, 261, 261, 262, 262, 262, 267, 263, 263, 251, 251, 233, 233, 221, 221, 221, 268, 268, 268, 249, 249, 250, 250, 252, 252, 252, 252, 192, 248, 248, 270, 270, 270, 270, 192, 271, 271, 271, 271, 192, 192, 274, 274, 274, 274, 274, 274, 275, 272, 272, 273, 273, 266, 266, 219, 219, 219, 219, 218, 218, 218, 219, 219, 219, 219, 219, 219, 219, 219, 219, 218, 219, 219, 219, 219, 219, 219, 219, 219, 219, 277, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 278, 278, 219, 279, 279, 219, 219, 219, 219, 219, 219, 282, 282, 283, 283, 281, 264, 255, 255, 280, 280, 192, 284, 284, 223, 223, 234, 234, 285, 285, 192, 192, 192, 286, 286, 192, 192, 192, 192, 192, 213, 214, 192, 288, 290, 290, 290, 291, 291, 291, 293, 293, 289, 289, 295, 295, 294, 294, 294, 294, 219, 219, 238, 238, 238, 192, 192, 192, 297, 297, 192, 192, 192, 192, 192, 192, 298, 192, 192, 192, 192, 192, 192, 192, 192, 192, 300, 302, 303, 303, 304, 269, 269, 307, 307, 307, 306, 308, 243, 243, 309, 310, 311, 311, 311, 311, 311, 312, 312, 312, 316, 318, 318, 319, 319, 317, 317, 320, 320, 321, 321, 321, 253, 276, 276, 276, 315, 315, 314, 218, 187, 188, 188, 189, 189, 189, 194, 194, 194, 196, 196, 192, 205, 203, 203, 195, 195, 210, 211, 212, 212, 209, 209, 217, 217, 217, 204, 230, 230, 231, 235, 237, 241, 242, 257, 258, 267, 275, 219, 277, 281, 264, 287, 287, 287, 287, 287, 213, 292, 292, 295, 296, 296, 299, 299, 301, 301, 302, 305, 305, 305, 269, 309, 311,
};

const signed char yyRuleInfoNRhs[] = {
    -1, -3, -1, -3, 0, -1, -1, -1, -2, -2, -2, -3, -5, -6, -1, 0, -3, -1, 0, -5, -2, 0, -3, -2, -1, -2, 0, -4, -6, -2, 0, 0, -2, -3, -4, -4, -4, -3, -3, -5, -2, -4, -4, -1, -2, -3, -4, 0, -1, 0, -2, -2, -3, -3, -3, -2, -2, -1, -1, -2, -3, -2, 0, -2, -2, 0, -1, -2, -7, -5, -5, -10, 0, 0, -3, 0, -2, -1, -1, -4, -2, 0, -9, -4, -1, -3, -4, -1, -3, -1, -2, -1, -9, -10, -4, -1, -5, -5, -1, -1, 0, 0, -5, -3, -5, -2, 0, 0, -2, -2, 0, -5, -6, -8, -6, -6, 0, -2, -1, -3, -1, -3, -3, -5, -1, -2, -3, -4, -2, -4, 0, 0, -3, -2, 0, -3, -5, -3, -1, -1, 0, -2, -2, 0, 0, -3, 0, -2, 0, -2, -4, -4, -6, 0, -2, 0, -2, -2, -4, -9, -5, -7, -3, -5, -7, -8, 0, -2, -12, -9, -5, -8, -2, -2, -1, 0, -3, -3, -1, -3, -1, -3, -5, -1, -1, -1, -1, -3, -6, -5, -8, -4, -6, -9, -5, -1, -5, -3, -3, -3, -3, -3, -3, -3, -3, -2, -3, -5, -2, -3, -3, -4, -6, -5, -2, -2, -2, -3, -1, -2, -5, -1, -2, -5, -3, -5, -5, -4, -5, -5, -4, -2, 0, 0, 0, -3, -1, 0, -3, -12, -1, 0, 0, -3, -5, -3, 0, -2, -4, -2, -3, -2, 0, -3, -5, -6, -5, -6, -2, -2, -5, -11, -1, -2, 0, -1, -1, -3, 0, -2, -3, -2, -3, -2, -9, -8, -6, -3, -4, -6, -1, -1, -1, -4, -6, -3, 0, -2, -1, -3, -1, -3, -6, -2, -7, -6, -8, -6, -9, -10, -11, -9, -1, -4, -8, 0, -1, -3, -1, -2, -3, -1, -2, -3, -6, -1, -1, -3, -3, -5, -5, -6, -4, -5, -2, 0, -3, -6, -1, -1, -2, -1, -2, -2, -2, 0, -2, -2, -2, -1, -2, -2, -1, -1, -4, -2, -5, -1, -1, -2, -1, -1, -2, -3, 0, -1, -2, -1, 0, -2, -1, -4, -2, -1, -1, -1, -1, -1, -1, -2, 0, -2, -4, -2, -2, -3, -1, 0, -1, -1, -1, -1, -2, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -3, 0, -1, 0, 0, -1, -1, -3, -2, 0, -4, -2, 0, -1, -1,
};


int yyGrowStack(yyParser *p) {
  int oldSize = 1 + (int)(p->yystackEnd - p->yystack);
  int newSize;
  int idx;
  yyStackEntry *pNew;

  int nLimit = parserStackSizeLimit(((p)->pParse));

  newSize = oldSize * 2 + 100;

  if (newSize > nLimit) {
    newSize = nLimit;
    if (newSize <= oldSize)
      return 1;
  }

  idx = (int)(p->yytos - p->yystack);
  if (p->yystack == p->yystk0) {
    pNew = parserStackRealloc(0, newSize * sizeof(pNew[0]), ((p)->pParse));
    if (pNew == 0)
      return 1;
    memcpy(pNew, p->yystack, oldSize * sizeof(pNew[0]));
  } else {
    pNew = parserStackRealloc(p->yystack, newSize * sizeof(pNew[0]), ((p)->pParse));
    if (pNew == 0)
      return 1;
  }
  p->yystack = pNew;
  p->yytos = &p->yystack[idx];

  p->yystackEnd = &p->yystack[newSize - 1];
  return 0;
}

void yy_destructor(yyParser *yypParser, unsigned short int yymajor, YYMINORTYPE *yypminor) {

  Parse *pParse = yypParser->pParse;
  switch (yymajor) {

  case 206:
  case 241:
  case 242:
  case 254:
  case 256: {
    sqlite3SelectDelete(pParse->db, (yypminor->yy555));
  } break;
  case 218:
  case 219:
  case 248:
  case 250:
  case 270:
  case 281:
  case 283:
  case 286:
  case 293:
  case 297:
  case 314: {
    sqlite3ExprDelete(pParse->db, (yypminor->yy454));
  } break;
  case 223:
  case 233:
  case 234:
  case 246:
  case 249:
  case 251:
  case 255:
  case 257:
  case 264:
  case 271:
  case 280:
  case 282:
  case 313: {
    sqlite3ExprListDelete(pParse->db, (yypminor->yy14));
  } break;
  case 240:
  case 247:
  case 259:
  case 260:
  case 265: {
    sqlite3SrcListDelete(pParse->db, (yypminor->yy203));
  } break;
  case 243: {
    sqlite3WithDelete(pParse->db, (yypminor->yy59));
  } break;
  case 253:
  case 309: {
    sqlite3WindowListDelete(pParse->db, (yypminor->yy211));
  } break;
  case 266:
  case 273: {
    sqlite3IdListDelete(pParse->db, (yypminor->yy132));
  } break;
  case 276:
  case 310:
  case 311:
  case 312:
  case 315: {
    sqlite3WindowDelete(pParse->db, (yypminor->yy211));
  } break;
  case 289:
  case 294: {
    sqlite3DeleteTriggerStep(pParse->db, (yypminor->yy427));
  } break;
  case 291: {
    sqlite3IdListDelete(pParse->db, (yypminor->yy286).b);
  } break;
  case 317:
  case 318:
  case 319: {
    sqlite3ExprDelete(pParse->db, (yypminor->yy509).pExpr);
  } break;

  default:
    break;
  }
}

void yy_pop_parser_stack(yyParser *pParser) {
  yyStackEntry *yytos;

  yytos = pParser->yytos--;

  yy_destructor(pParser, yytos->major, &yytos->minor);
}

void yyStackOverflow(yyParser *yypParser) {

  Parse *pParse = yypParser->pParse;

  while (yypParser->yytos > yypParser->yystack)
    yy_pop_parser_stack(yypParser);

  if (pParse->nErr == 0)
    sqlite3ErrorMsg(pParse, "Recursion limit");

  yypParser->pParse = pParse;
}

void yy_shift(yyParser *yypParser, unsigned short int yyNewState, unsigned short int yyMajor, Token yyMinor) {
  yyStackEntry *yytos;
  yypParser->yytos++;

  yytos = yypParser->yytos;
  if (yytos > yypParser->yystackEnd) {
    if (yyGrowStack(yypParser)) {
      yypParser->yytos--;
      yyStackOverflow(yypParser);
      return;
    }
    yytos = yypParser->yytos;

    ((void)(0))

        ;
  }
  if (yyNewState > 599) {
    yyNewState += 1282 - 867;
  }
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor.yy0 = yyMinor;
  ;
}

unsigned short int yy_reduce(yyParser *yypParser, unsigned int yyruleno, int yyLookahead, Token yyLookaheadToken, Parse *pParse) {
  int yygoto;
  unsigned short int yyact;
  yyStackEntry *yymsp;
  int yysize;

  (void)yyLookahead;
  (void)yyLookaheadToken;
  yymsp = yypParser->yytos;

  switch (yyruleno) {

    YYMINORTYPE yylhsminor;
  case 0: {
    if (pParse->pReprepare == 0)
      pParse->explain = 1;
  } break;
  case 1: {
    if (pParse->pReprepare == 0)
      pParse->explain = 2;
  } break;
  case 2: {
    sqlite3FinishCoding(pParse);
  } break;
  case 3: {
    sqlite3BeginTransaction(pParse, yymsp[-1].minor.yy144);
  } break;
  case 4: {
    yymsp[1].minor.yy144 = 7;
  } break;
  case 5:
  case 6:;
  case 7:;
  case 328:;
    {
      yymsp[0].minor.yy144 = yymsp[0].major;
    }
    break;
  case 8:
  case 9:;
    {
      sqlite3EndTransaction(pParse, yymsp[-1].major);
    }
    break;
  case 10: {
    sqlite3Savepoint(pParse, 0, &yymsp[0].minor.yy0);
  } break;
  case 11: {
    sqlite3Savepoint(pParse, 1, &yymsp[0].minor.yy0);
  } break;
  case 12: {
    sqlite3Savepoint(pParse, 2, &yymsp[0].minor.yy0);
  } break;
  case 13: {
    sqlite3StartTable(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0, yymsp[-4].minor.yy144, 0, 0, yymsp[-2].minor.yy144);
  } break;
  case 14: {
    disableLookaside(pParse);
  } break;
  case 15:
  case 18:;
  case 47:;
  case 62:;
  case 72:;
  case 81:;
  case 100:;
  case 246:;
    {
      yymsp[1].minor.yy144 = 0;
    }
    break;
  case 16: {
    yymsp[-2].minor.yy144 = 1;
  } break;
  case 17: {
    yymsp[0].minor.yy144 = pParse->db->init.busy == 0;
  } break;
  case 19: {
    sqlite3EndTable(pParse, &yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0, yymsp[0].minor.yy391, 0);
  } break;
  case 20: {
    sqlite3EndTable(pParse, 0, 0, 0, yymsp[0].minor.yy555);
    sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy555);
  } break;
  case 21: {
    yymsp[1].minor.yy391 = 0;
  } break;
  case 22: {
    yylhsminor.yy391 = yymsp[-2].minor.yy391 | yymsp[0].minor.yy391;
  }
    yymsp[-2].minor.yy391 = yylhsminor.yy391;
    break;
  case 23: {
    if (yymsp[0].minor.yy0.n == 5 && sqlite3_strnicmp(yymsp[0].minor.yy0.z, "rowid", 5) == 0) {
      yymsp[-1].minor.yy391 = 0x00000080 | 0x00000200;
    } else {
      yymsp[-1].minor.yy391 = 0;
      sqlite3ErrorMsg(pParse, "unknown table option: %.*s", yymsp[0].minor.yy0.n, yymsp[0].minor.yy0.z);
    }
  } break;
  case 24: {
    if (yymsp[0].minor.yy0.n == 6 && sqlite3_strnicmp(yymsp[0].minor.yy0.z, "strict", 6) == 0) {
      yylhsminor.yy391 = 0x00010000;
    } else {
      yylhsminor.yy391 = 0;
      sqlite3ErrorMsg(pParse, "unknown table option: %.*s", yymsp[0].minor.yy0.n, yymsp[0].minor.yy0.z);
    }
  }
    yymsp[0].minor.yy391 = yylhsminor.yy391;
    break;
  case 25: {
    sqlite3AddColumn(pParse, yymsp[-1].minor.yy0, yymsp[0].minor.yy0);
  } break;
  case 26:
  case 65:;
  case 106:;
    {
      yymsp[1].minor.yy0.n = 0;
      yymsp[1].minor.yy0.z = 0;
    }
    break;
  case 27: {
    yymsp[-3].minor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-3].minor.yy0.z);
  } break;
  case 28: {
    yymsp[-5].minor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-5].minor.yy0.z);
  } break;
  case 29: {
    yymsp[-1].minor.yy0.n = yymsp[0].minor.yy0.n + (int)(yymsp[0].minor.yy0.z - yymsp[-1].minor.yy0.z);
  } break;
  case 30: {

    ((void)(0))

        ;
    yymsp[1].minor.yy168 = yyLookaheadToken.z;
  } break;
  case 31: {

    ((void)(0))

        ;
    yymsp[1].minor.yy0 = yyLookaheadToken;
  } break;
  case 32:
  case 67:;
    {

      ((void)(0))

          ;
      pParse->u1.cr.constraintName = yymsp[0].minor.yy0;
    }
    break;
  case 33: {
    sqlite3AddDefaultValue(pParse, yymsp[0].minor.yy454, yymsp[-1].minor.yy0.z, &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n]);
  } break;
  case 34: {
    sqlite3AddDefaultValue(pParse, yymsp[-1].minor.yy454, yymsp[-2].minor.yy0.z + 1, yymsp[0].minor.yy0.z);
  } break;
  case 35: {
    sqlite3AddDefaultValue(pParse, yymsp[0].minor.yy454, yymsp[-2].minor.yy0.z, &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n]);
  } break;
  case 36: {
    Expr *p = sqlite3PExpr(pParse, 174, yymsp[0].minor.yy454, 0);
    sqlite3AddDefaultValue(pParse, p, yymsp[-2].minor.yy0.z, &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n]);
  } break;
  case 37: {
    Expr *p = tokenExpr(pParse, 118, yymsp[0].minor.yy0);
    if (p) {
      sqlite3ExprIdToTrueFalse(p);
      ;
    }
    sqlite3AddDefaultValue(pParse, p, yymsp[0].minor.yy0.z, yymsp[0].minor.yy0.z + yymsp[0].minor.yy0.n);
  } break;
  case 38: {
    sqlite3AddNotNull(pParse, yymsp[0].minor.yy144);
  } break;
  case 39: {
    sqlite3AddPrimaryKey(pParse, 0, yymsp[-1].minor.yy144, yymsp[0].minor.yy144, yymsp[-2].minor.yy144);
  } break;
  case 40: {
    sqlite3CreateIndex(pParse, 0, 0, 0, 0, yymsp[0].minor.yy144, 0, 0, 0, 0, 1);
  } break;
  case 41: {
    sqlite3AddCheckConstraint(pParse, yymsp[-1].minor.yy454, yymsp[-2].minor.yy0.z, yymsp[0].minor.yy0.z);
  } break;
  case 42: {
    sqlite3CreateForeignKey(pParse, 0, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy14, yymsp[0].minor.yy144);
  } break;
  case 43: {
    sqlite3DeferForeignKey(pParse, yymsp[0].minor.yy144);
  } break;
  case 44: {
    sqlite3AddCollateType(pParse, &yymsp[0].minor.yy0);
  } break;
  case 45: {
    sqlite3AddGenerated(pParse, yymsp[-1].minor.yy454, 0);
  } break;
  case 46: {
    sqlite3AddGenerated(pParse, yymsp[-2].minor.yy454, &yymsp[0].minor.yy0);
  } break;
  case 48: {
    yymsp[0].minor.yy144 = 1;
  } break;
  case 49: {
    yymsp[1].minor.yy144 = 0 * 0x0101;
  } break;
  case 50: {
    yymsp[-1].minor.yy144 = (yymsp[-1].minor.yy144 & ~yymsp[0].minor.yy383.mask) | yymsp[0].minor.yy383.value;
  } break;
  case 51: {
    yymsp[-1].minor.yy383.value = 0;
    yymsp[-1].minor.yy383.mask = 0x000000;
  } break;
  case 52: {
    yymsp[-2].minor.yy383.value = 0;
    yymsp[-2].minor.yy383.mask = 0x000000;
  } break;
  case 53: {
    yymsp[-2].minor.yy383.value = yymsp[0].minor.yy144;
    yymsp[-2].minor.yy383.mask = 0x0000ff;
  } break;
  case 54: {
    yymsp[-2].minor.yy383.value = yymsp[0].minor.yy144 << 8;
    yymsp[-2].minor.yy383.mask = 0x00ff00;
  } break;
  case 55: {
    yymsp[-1].minor.yy144 = 8;
  } break;
  case 56: {
    yymsp[-1].minor.yy144 = 9;
  } break;
  case 57: {
    yymsp[0].minor.yy144 = 10;
  } break;
  case 58: {
    yymsp[0].minor.yy144 = 7;
  } break;
  case 59: {
    yymsp[-1].minor.yy144 = 0;
  } break;
  case 60: {
    yymsp[-2].minor.yy144 = 0;
  } break;
  case 61:
  case 76:;
  case 173:;
    {
      yymsp[-1].minor.yy144 = yymsp[0].minor.yy144;
    }
    break;
  case 63:
  case 80:;
  case 219:;
  case 222:;
  case 247:;
    {
      yymsp[-1].minor.yy144 = 1;
    }
    break;
  case 64: {
    yymsp[-1].minor.yy144 = 0;
  } break;
  case 66: {

    ((void)(0))

        ;
    pParse->u1.cr.constraintName.n = 0;
  } break;
  case 68: {
    sqlite3AddPrimaryKey(pParse, yymsp[-3].minor.yy14, yymsp[0].minor.yy144, yymsp[-2].minor.yy144, 0);
  } break;
  case 69: {
    sqlite3CreateIndex(pParse, 0, 0, 0, yymsp[-2].minor.yy14, yymsp[0].minor.yy144, 0, 0, 0, 0, 1);
  } break;
  case 70: {
    sqlite3AddCheckConstraint(pParse, yymsp[-2].minor.yy454, yymsp[-3].minor.yy0.z, yymsp[-1].minor.yy0.z);
  } break;
  case 71: {
    sqlite3CreateForeignKey(pParse, yymsp[-6].minor.yy14, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy14, yymsp[-1].minor.yy144);
    sqlite3DeferForeignKey(pParse, yymsp[0].minor.yy144);
  } break;
  case 73:
  case 75:;
    {
      yymsp[1].minor.yy144 = 11;
    }
    break;
  case 74: {
    yymsp[-2].minor.yy144 = yymsp[0].minor.yy144;
  } break;
  case 77: {
    yymsp[0].minor.yy144 = 4;
  } break;
  case 78:
  case 174:;
    {
      yymsp[0].minor.yy144 = 5;
    }
    break;
  case 79: {
    sqlite3DropTable(pParse, yymsp[0].minor.yy203, 0, yymsp[-1].minor.yy144);
  } break;
  case 82: {
    sqlite3CreateView(pParse, &yymsp[-8].minor.yy0, &yymsp[-4].minor.yy0, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy14, yymsp[0].minor.yy555, yymsp[-7].minor.yy144, yymsp[-5].minor.yy144);
  } break;
  case 83: {
    sqlite3DropTable(pParse, yymsp[0].minor.yy203, 1, yymsp[-1].minor.yy144);
  } break;
  case 84: {
    SelectDest dest = {7, 0, 0, 0, 0, 0, 0};
    if ((pParse->db->mDbFlags & 0x0040) != 0 || sqlite3ReadSchema(pParse) == 0) {
      sqlite3Select(pParse, yymsp[0].minor.yy555, &dest);
    }
    sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy555);
  } break;
  case 85: {
    yymsp[-2].minor.yy555 = attachWithToSelect(pParse, yymsp[0].minor.yy555, yymsp[-1].minor.yy59);
  } break;
  case 86: {
    yymsp[-3].minor.yy555 = attachWithToSelect(pParse, yymsp[0].minor.yy555, yymsp[-1].minor.yy59);
  } break;
  case 87: {
    Select *p = yymsp[0].minor.yy555;
    if (p) {
      parserDoubleLinkSelect(pParse, p);
    }
  } break;
  case 88: {
    Select *pRhs = yymsp[0].minor.yy555;
    Select *pLhs = yymsp[-2].minor.yy555;
    if (pRhs && pRhs->pPrior) {
      SrcList *pFrom;
      Token x;
      x.n = 0;
      parserDoubleLinkSelect(pParse, pRhs);
      pFrom = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &x, pRhs, 0);
      pRhs = sqlite3SelectNew(pParse, 0, pFrom, 0, 0, 0, 0, 0, 0);
    }
    if (pRhs) {
      pRhs->op = (u8)yymsp[-1].minor.yy144;
      pRhs->pPrior = pLhs;
      if ((pLhs))
        pLhs->selFlags &= ~(u32)0x0000400;
      pRhs->selFlags &= ~(u32)0x0000400;
      if (yymsp[-1].minor.yy144 != 136)
        pParse->hasCompound = 1;
    } else {
      sqlite3SelectDelete(pParse->db, pLhs);
    }
    yymsp[-2].minor.yy555 = pRhs;
  } break;
  case 89:
  case 91:;
    {
      yymsp[0].minor.yy144 = yymsp[0].major;
    }
    break;
  case 90: {
    yymsp[-1].minor.yy144 = 136;
  } break;
  case 92: {
    yymsp[-8].minor.yy555 = sqlite3SelectNew(pParse, yymsp[-6].minor.yy14, yymsp[-5].minor.yy203, yymsp[-4].minor.yy454, yymsp[-3].minor.yy14, yymsp[-2].minor.yy454, yymsp[-1].minor.yy14, yymsp[-7].minor.yy144, yymsp[0].minor.yy454);
  } break;
  case 93: {
    yymsp[-9].minor.yy555 = sqlite3SelectNew(pParse, yymsp[-7].minor.yy14, yymsp[-6].minor.yy203, yymsp[-5].minor.yy454, yymsp[-4].minor.yy14, yymsp[-3].minor.yy454, yymsp[-1].minor.yy14, yymsp[-8].minor.yy144, yymsp[0].minor.yy454);
    if (yymsp[-9].minor.yy555) {
      yymsp[-9].minor.yy555->pWinDefn = yymsp[-2].minor.yy211;
    } else {
      sqlite3WindowListDelete(pParse->db, yymsp[-2].minor.yy211);
    }
  } break;
  case 94: {
    yymsp[-3].minor.yy555 = sqlite3SelectNew(pParse, yymsp[-1].minor.yy14, 0, 0, 0, 0, 0, 0x0000200, 0);
  } break;
  case 95: {
    sqlite3MultiValuesEnd(pParse, yymsp[0].minor.yy555);
  } break;
  case 96:
  case 97:;
    {
      yymsp[-4].minor.yy555 = sqlite3MultiValues(pParse, yymsp[-4].minor.yy555, yymsp[-1].minor.yy14);
    }
    break;
  case 98: {
    yymsp[0].minor.yy144 = 0x0000001;
  } break;
  case 99: {
    yymsp[0].minor.yy144 = 0x0000002;
  } break;
  case 101:
  case 134:;
  case 144:;
  case 234:;
  case 237:;
  case 242:;
    {
      yymsp[1].minor.yy14 = 0;
    }
    break;
  case 102: {
    yymsp[-4].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[-2].minor.yy454);
    if (yymsp[0].minor.yy0.n > 0)
      sqlite3ExprListSetName(pParse, yymsp[-4].minor.yy14, &yymsp[0].minor.yy0, 1);
    sqlite3ExprListSetSpan(pParse, yymsp[-4].minor.yy14, yymsp[-3].minor.yy168, yymsp[-1].minor.yy168);
  } break;
  case 103: {
    Expr *p = sqlite3Expr(pParse->db, 180, 0);
    sqlite3ExprSetErrorOffset(p, (int)(yymsp[0].minor.yy0.z - pParse->zTail));
    yymsp[-2].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy14, p);
  } break;
  case 104: {
    Expr *pRight, *pLeft, *pDot;
    pRight = sqlite3PExpr(pParse, 180, 0, 0);
    sqlite3ExprSetErrorOffset(pRight, (int)(yymsp[0].minor.yy0.z - pParse->zTail));
    pLeft = tokenExpr(pParse, 60, yymsp[-2].minor.yy0);
    pDot = sqlite3PExpr(pParse, 142, pLeft, pRight);
    yymsp[-4].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, pDot);
  } break;
  case 105:
  case 117:;
  case 258:;
  case 259:;
    {
      yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
    }
    break;
  case 107:
  case 110:;
    {
      yymsp[1].minor.yy203 = 0;
    }
    break;
  case 108: {
    yymsp[-1].minor.yy203 = yymsp[0].minor.yy203;
    sqlite3SrcListShiftJoinType(pParse, yymsp[-1].minor.yy203);
  } break;
  case 109: {
    if ((yymsp[-1].minor.yy203 && yymsp[-1].minor.yy203->nSrc > 0))
      yymsp[-1].minor.yy203->a[yymsp[-1].minor.yy203->nSrc - 1].fg.jointype = (u8)yymsp[0].minor.yy144;
  } break;
  case 111: {
    yymsp[-4].minor.yy203 = sqlite3SrcListAppendFromTerm(pParse, yymsp[-4].minor.yy203, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0, 0, &yymsp[0].minor.yy269);
  } break;
  case 112: {
    yymsp[-5].minor.yy203 = sqlite3SrcListAppendFromTerm(pParse, yymsp[-5].minor.yy203, &yymsp[-4].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, 0, &yymsp[0].minor.yy269);
    sqlite3SrcListIndexedBy(pParse, yymsp[-5].minor.yy203, &yymsp[-1].minor.yy0);
  } break;
  case 113: {
    yymsp[-7].minor.yy203 = sqlite3SrcListAppendFromTerm(pParse, yymsp[-7].minor.yy203, &yymsp[-6].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-1].minor.yy0, 0, &yymsp[0].minor.yy269);
    sqlite3SrcListFuncArgs(pParse, yymsp[-7].minor.yy203, yymsp[-3].minor.yy14);
  } break;
  case 114: {
    yymsp[-5].minor.yy203 = sqlite3SrcListAppendFromTerm(pParse, yymsp[-5].minor.yy203, 0, 0, &yymsp[-1].minor.yy0, yymsp[-3].minor.yy555, &yymsp[0].minor.yy269);
  } break;
  case 115: {
    if (yymsp[-5].minor.yy203 == 0 && yymsp[-1].minor.yy0.n == 0 && yymsp[0].minor.yy269.pOn == 0 && yymsp[0].minor.yy269.pUsing == 0) {
      yymsp[-5].minor.yy203 = yymsp[-3].minor.yy203;
    } else if ((yymsp[-3].minor.yy203 != 0) && yymsp[-3].minor.yy203->nSrc == 1) {
      yymsp[-5].minor.yy203 = sqlite3SrcListAppendFromTerm(pParse, yymsp[-5].minor.yy203, 0, 0, &yymsp[-1].minor.yy0, 0, &yymsp[0].minor.yy269);
      if (yymsp[-5].minor.yy203) {
        SrcItem *pNew = &yymsp[-5].minor.yy203->a[yymsp[-5].minor.yy203->nSrc - 1];
        SrcItem *pOld = yymsp[-3].minor.yy203->a;

        ((void)(0))

            ;
        pNew->zName = pOld->zName;

        ((void)(0))

            ;
        if (pOld->fg.isSubquery) {
          pNew->fg.isSubquery = 1;
          pNew->u4.pSubq = pOld->u4.pSubq;
          pOld->u4.pSubq = 0;
          pOld->fg.isSubquery = 0;

          ((void)(0))

              ;
          if ((pNew->u4.pSubq->pSelect->selFlags & 0x0000800) != 0) {
            pNew->fg.isNestedFrom = 1;
          }
        } else {
          pNew->u4.zDatabase = pOld->u4.zDatabase;
          pOld->u4.zDatabase = 0;
        }
        if (pOld->fg.isTabFunc) {
          pNew->u1.pFuncArg = pOld->u1.pFuncArg;
          pOld->u1.pFuncArg = 0;
          pOld->fg.isTabFunc = 0;
          pNew->fg.isTabFunc = 1;
        }
        pOld->zName = 0;
      }
      sqlite3SrcListDelete(pParse->db, yymsp[-3].minor.yy203);
    } else {
      Select *pSubquery;
      sqlite3SrcListShiftJoinType(pParse, yymsp[-3].minor.yy203);
      pSubquery = sqlite3SelectNew(pParse, 0, yymsp[-3].minor.yy203, 0, 0, 0, 0, 0x0000800, 0);
      yymsp[-5].minor.yy203 = sqlite3SrcListAppendFromTerm(pParse, yymsp[-5].minor.yy203, 0, 0, &yymsp[-1].minor.yy0, pSubquery, &yymsp[0].minor.yy269);
    }
  } break;
  case 116:
  case 131:;
    {
      yymsp[1].minor.yy0.z = 0;
      yymsp[1].minor.yy0.n = 0;
    }
    break;
  case 118:
  case 120:;
    {
      yylhsminor.yy203 = sqlite3SrcListAppend(pParse, 0, &yymsp[0].minor.yy0, 0);
      if ((pParse->eParseMode >= 2) && yylhsminor.yy203)
        sqlite3RenameTokenMap(pParse, yylhsminor.yy203->a[0].zName, &yymsp[0].minor.yy0);
    }
    yymsp[0].minor.yy203 = yylhsminor.yy203;
    break;
  case 119:
  case 121:;
    {
      yylhsminor.yy203 = sqlite3SrcListAppend(pParse, 0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0);
      if ((pParse->eParseMode >= 2) && yylhsminor.yy203)
        sqlite3RenameTokenMap(pParse, yylhsminor.yy203->a[0].zName, &yymsp[0].minor.yy0);
    }
    yymsp[-2].minor.yy203 = yylhsminor.yy203;
    break;
  case 122: {
    yylhsminor.yy203 = sqlite3SrcListAppend(pParse, 0, &yymsp[-2].minor.yy0, 0);
    if (yylhsminor.yy203) {
      if ((pParse->eParseMode >= 2)) {
        sqlite3RenameTokenMap(pParse, yylhsminor.yy203->a[0].zName, &yymsp[-2].minor.yy0);
      } else {
        yylhsminor.yy203->a[0].zAlias = sqlite3NameFromToken(pParse->db, &yymsp[0].minor.yy0);
      }
    }
  }
    yymsp[-2].minor.yy203 = yylhsminor.yy203;
    break;
  case 123: {
    yylhsminor.yy203 = sqlite3SrcListAppend(pParse, 0, &yymsp[-4].minor.yy0, &yymsp[-2].minor.yy0);
    if (yylhsminor.yy203) {
      if ((pParse->eParseMode >= 2)) {
        sqlite3RenameTokenMap(pParse, yylhsminor.yy203->a[0].zName, &yymsp[-2].minor.yy0);
      } else {
        yylhsminor.yy203->a[0].zAlias = sqlite3NameFromToken(pParse->db, &yymsp[0].minor.yy0);
      }
    }
  }
    yymsp[-4].minor.yy203 = yylhsminor.yy203;
    break;
  case 124: {
    yymsp[0].minor.yy144 = 0x01;
  } break;
  case 125: {
    yymsp[-1].minor.yy144 = sqlite3JoinType(pParse, &yymsp[-1].minor.yy0, 0, 0);
  } break;
  case 126: {
    yymsp[-2].minor.yy144 = sqlite3JoinType(pParse, &yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0, 0);
  } break;
  case 127: {
    yymsp[-3].minor.yy144 = sqlite3JoinType(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0);
  } break;
  case 128: {
    yymsp[-1].minor.yy269.pOn = yymsp[0].minor.yy454;
    yymsp[-1].minor.yy269.pUsing = 0;
  } break;
  case 129: {
    yymsp[-3].minor.yy269.pOn = 0;
    yymsp[-3].minor.yy269.pUsing = yymsp[-1].minor.yy132;
  } break;
  case 130: {
    yymsp[1].minor.yy269.pOn = 0;
    yymsp[1].minor.yy269.pUsing = 0;
  } break;
  case 132: {
    yymsp[-2].minor.yy0 = yymsp[0].minor.yy0;
  } break;
  case 133: {
    yymsp[-1].minor.yy0.z = 0;
    yymsp[-1].minor.yy0.n = 1;
  } break;
  case 135:
  case 145:;
    {
      yymsp[-2].minor.yy14 = yymsp[0].minor.yy14;
    }
    break;
  case 136: {
    yymsp[-4].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[-2].minor.yy454);
    sqlite3ExprListSetSortOrder(yymsp[-4].minor.yy14, yymsp[-1].minor.yy144, yymsp[0].minor.yy144);
  } break;
  case 137: {
    yymsp[-2].minor.yy14 = sqlite3ExprListAppend(pParse, 0, yymsp[-2].minor.yy454);
    sqlite3ExprListSetSortOrder(yymsp[-2].minor.yy14, yymsp[-1].minor.yy144, yymsp[0].minor.yy144);
  } break;
  case 138: {
    yymsp[0].minor.yy144 = 0;
  } break;
  case 139: {
    yymsp[0].minor.yy144 = 1;
  } break;
  case 140:
  case 143:;
    {
      yymsp[1].minor.yy144 = -1;
    }
    break;
  case 141: {
    yymsp[-1].minor.yy144 = 0;
  } break;
  case 142: {
    yymsp[-1].minor.yy144 = 1;
  } break;
  case 146:
  case 148:;
  case 153:;
  case 155:;
  case 232:;
  case 233:;
  case 252:;
    {
      yymsp[1].minor.yy454 = 0;
    }
    break;
  case 147:
  case 154:;
  case 156:;
  case 231:;
  case 251:;
    {
      yymsp[-1].minor.yy454 = yymsp[0].minor.yy454;
    }
    break;
  case 149: {
    yymsp[-1].minor.yy454 = sqlite3PExpr(pParse, 149, yymsp[0].minor.yy454, 0);
  } break;
  case 150: {
    yymsp[-3].minor.yy454 = sqlite3PExpr(pParse, 149, yymsp[-2].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 151: {
    yymsp[-3].minor.yy454 = sqlite3PExpr(pParse, 149, yymsp[0].minor.yy454, yymsp[-2].minor.yy454);
  } break;
  case 152: {
    sqlite3SrcListIndexedBy(pParse, yymsp[-2].minor.yy203, &yymsp[-1].minor.yy0);
    sqlite3DeleteFrom(pParse, yymsp[-2].minor.yy203, yymsp[0].minor.yy454, 0, 0);
  } break;
  case 157: {
    sqlite3AddReturning(pParse, yymsp[0].minor.yy14);
    yymsp[-1].minor.yy454 = 0;
  } break;
  case 158: {
    sqlite3AddReturning(pParse, yymsp[0].minor.yy14);
    yymsp[-3].minor.yy454 = yymsp[-2].minor.yy454;
  } break;
  case 159: {
    sqlite3SrcListIndexedBy(pParse, yymsp[-5].minor.yy203, &yymsp[-4].minor.yy0);
    sqlite3ExprListCheckLength(pParse, yymsp[-2].minor.yy14, "set list");
    if (yymsp[-1].minor.yy203) {
      SrcList *pFromClause = yymsp[-1].minor.yy203;
      if (pFromClause->nSrc > 1) {
        Select *pSubquery;
        Token as;
        pSubquery = sqlite3SelectNew(pParse, 0, pFromClause, 0, 0, 0, 0, 0x0000800, 0);
        as.n = 0;
        as.z = 0;
        pFromClause = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSubquery, 0);
      }
      yymsp[-5].minor.yy203 = sqlite3SrcListAppendList(pParse, yymsp[-5].minor.yy203, pFromClause);
    }
    sqlite3Update(pParse, yymsp[-5].minor.yy203, yymsp[-2].minor.yy14, yymsp[0].minor.yy454, yymsp[-6].minor.yy144, 0, 0, 0);
  } break;
  case 160: {
    yymsp[-4].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[0].minor.yy454);
    sqlite3ExprListSetName(pParse, yymsp[-4].minor.yy14, &yymsp[-2].minor.yy0, 1);
  } break;
  case 161: {
    yymsp[-6].minor.yy14 = sqlite3ExprListAppendVector(pParse, yymsp[-6].minor.yy14, yymsp[-3].minor.yy132, yymsp[0].minor.yy454);
  } break;
  case 162: {
    yylhsminor.yy14 = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy454);
    sqlite3ExprListSetName(pParse, yylhsminor.yy14, &yymsp[-2].minor.yy0, 1);
  }
    yymsp[-2].minor.yy14 = yylhsminor.yy14;
    break;
  case 163: {
    yymsp[-4].minor.yy14 = sqlite3ExprListAppendVector(pParse, 0, yymsp[-3].minor.yy132, yymsp[0].minor.yy454);
  } break;
  case 164: {
    sqlite3Insert(pParse, yymsp[-3].minor.yy203, yymsp[-1].minor.yy555, yymsp[-2].minor.yy132, yymsp[-5].minor.yy144, yymsp[0].minor.yy122);
  } break;
  case 165: {
    sqlite3Insert(pParse, yymsp[-4].minor.yy203, 0, yymsp[-3].minor.yy132, yymsp[-6].minor.yy144, 0);
  } break;
  case 166: {
    yymsp[1].minor.yy122 = 0;
  } break;
  case 167: {
    yymsp[-1].minor.yy122 = 0;
    sqlite3AddReturning(pParse, yymsp[0].minor.yy14);
  } break;
  case 168: {
    yymsp[-11].minor.yy122 = sqlite3UpsertNew(pParse->db, yymsp[-8].minor.yy14, yymsp[-6].minor.yy454, yymsp[-2].minor.yy14, yymsp[-1].minor.yy454, yymsp[0].minor.yy122);
  } break;
  case 169: {
    yymsp[-8].minor.yy122 = sqlite3UpsertNew(pParse->db, yymsp[-5].minor.yy14, yymsp[-3].minor.yy454, 0, 0, yymsp[0].minor.yy122);
  } break;
  case 170: {
    yymsp[-4].minor.yy122 = sqlite3UpsertNew(pParse->db, 0, 0, 0, 0, 0);
  } break;
  case 171: {
    yymsp[-7].minor.yy122 = sqlite3UpsertNew(pParse->db, 0, 0, yymsp[-2].minor.yy14, yymsp[-1].minor.yy454, 0);
  } break;
  case 172: {
    sqlite3AddReturning(pParse, yymsp[0].minor.yy14);
  } break;
  case 175: {
    yymsp[1].minor.yy132 = 0;
  } break;
  case 176: {
    yymsp[-2].minor.yy132 = yymsp[-1].minor.yy132;
  } break;
  case 177: {
    yymsp[-2].minor.yy132 = sqlite3IdListAppend(pParse, yymsp[-2].minor.yy132, &yymsp[0].minor.yy0);
  } break;
  case 178: {
    yymsp[0].minor.yy132 = sqlite3IdListAppend(pParse, 0, &yymsp[0].minor.yy0);
  } break;
  case 179: {
    yymsp[-2].minor.yy454 = yymsp[-1].minor.yy454;
  } break;
  case 180: {
    yymsp[0].minor.yy454 = tokenExpr(pParse, 60, yymsp[0].minor.yy0);
  } break;
  case 181: {
    Expr *temp1 = tokenExpr(pParse, 60, yymsp[-2].minor.yy0);
    Expr *temp2 = tokenExpr(pParse, 60, yymsp[0].minor.yy0);
    yylhsminor.yy454 = sqlite3PExpr(pParse, 142, temp1, temp2);
  }
    yymsp[-2].minor.yy454 = yylhsminor.yy454;
    break;
  case 182: {
    Expr *temp1 = tokenExpr(pParse, 60, yymsp[-4].minor.yy0);
    Expr *temp2 = tokenExpr(pParse, 60, yymsp[-2].minor.yy0);
    Expr *temp3 = tokenExpr(pParse, 60, yymsp[0].minor.yy0);
    Expr *temp4 = sqlite3PExpr(pParse, 142, temp2, temp3);
    if ((pParse->eParseMode >= 2)) {
      sqlite3RenameTokenRemap(pParse, 0, temp1);
    }
    yylhsminor.yy454 = sqlite3PExpr(pParse, 142, temp1, temp4);
  }
    yymsp[-4].minor.yy454 = yylhsminor.yy454;
    break;
  case 183:
  case 184:;
    {
      yymsp[0].minor.yy454 = tokenExpr(pParse, yymsp[0].major, yymsp[0].minor.yy0);
    }
    break;
  case 185: {
    int iValue;
    if (sqlite3GetInt32(yymsp[0].minor.yy0.z, &iValue) == 0) {
      yylhsminor.yy454 = sqlite3ExprAlloc(pParse->db, 156, &yymsp[0].minor.yy0, 0);
    } else {
      yylhsminor.yy454 = sqlite3ExprInt32(pParse->db, iValue);
    }
    if (yylhsminor.yy454)
      yylhsminor.yy454->w.iOfst = (int)(yymsp[0].minor.yy0.z - pParse->zTail);
  }
    yymsp[0].minor.yy454 = yylhsminor.yy454;
    break;
  case 186: {
    if (!(yymsp[0].minor.yy0.z[0] == '#' && (sqlite3CtypeMap[(unsigned char)(yymsp[0].minor.yy0.z[1])] & 0x04))) {
      u32 n = yymsp[0].minor.yy0.n;
      yymsp[0].minor.yy454 = tokenExpr(pParse, 157, yymsp[0].minor.yy0);
      sqlite3ExprAssignVarNumber(pParse, yymsp[0].minor.yy454, n);
    } else {

      Token t = yymsp[0].minor.yy0;

      ((void)(0))

          ;
      if (pParse->nested == 0) {
        parserSyntaxError(pParse, &t);
        yymsp[0].minor.yy454 = 0;
      } else {
        yymsp[0].minor.yy454 = sqlite3PExpr(pParse, 176, 0, 0);
        if (yymsp[0].minor.yy454)
          sqlite3GetInt32(&t.z[1], &yymsp[0].minor.yy454->iTable);
      }
    }
  } break;
  case 187: {
    yymsp[-2].minor.yy454 = sqlite3ExprAddCollateToken(pParse, yymsp[-2].minor.yy454, &yymsp[0].minor.yy0, 1);
  } break;
  case 188: {
    yymsp[-5].minor.yy454 = sqlite3ExprAlloc(pParse->db, 36, &yymsp[-1].minor.yy0, 1);
    sqlite3ExprAttachSubtrees(pParse->db, yymsp[-5].minor.yy454, yymsp[-3].minor.yy454, 0);
  } break;
  case 189: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, yymsp[-1].minor.yy14, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy144);
  }
    yymsp[-4].minor.yy454 = yylhsminor.yy454;
    break;
  case 190: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, yymsp[-4].minor.yy14, &yymsp[-7].minor.yy0, yymsp[-5].minor.yy144);
    sqlite3ExprAddFunctionOrderBy(pParse, yylhsminor.yy454, yymsp[-1].minor.yy14);
  }
    yymsp[-7].minor.yy454 = yylhsminor.yy454;
    break;
  case 191: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, 0, &yymsp[-3].minor.yy0, 0);
  }
    yymsp[-3].minor.yy454 = yylhsminor.yy454;
    break;
  case 192: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, yymsp[-2].minor.yy14, &yymsp[-5].minor.yy0, yymsp[-3].minor.yy144);
    sqlite3WindowAttach(pParse, yylhsminor.yy454, yymsp[0].minor.yy211);
  }
    yymsp[-5].minor.yy454 = yylhsminor.yy454;
    break;
  case 193: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, yymsp[-5].minor.yy14, &yymsp[-8].minor.yy0, yymsp[-6].minor.yy144);
    sqlite3WindowAttach(pParse, yylhsminor.yy454, yymsp[0].minor.yy211);
    sqlite3ExprAddFunctionOrderBy(pParse, yylhsminor.yy454, yymsp[-2].minor.yy14);
  }
    yymsp[-8].minor.yy454 = yylhsminor.yy454;
    break;
  case 194: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, 0, &yymsp[-4].minor.yy0, 0);
    sqlite3WindowAttach(pParse, yylhsminor.yy454, yymsp[0].minor.yy211);
  }
    yymsp[-4].minor.yy454 = yylhsminor.yy454;
    break;
  case 195: {
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, 0, &yymsp[0].minor.yy0, 0);
  }
    yymsp[0].minor.yy454 = yylhsminor.yy454;
    break;
  case 196: {
    ExprList *pList = sqlite3ExprListAppend(pParse, yymsp[-3].minor.yy14, yymsp[-1].minor.yy454);
    yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 177, 0, 0);
    if (yymsp[-4].minor.yy454) {
      int i;
      yymsp[-4].minor.yy454->x.pList = pList;
      for (i = 0; i < pList->nExpr; i++) {

        ((void)(0))

            ;
        yymsp[-4].minor.yy454->flags |= pList->a[i].pExpr->flags & (0x000200 | 0x400000 | 0x000008);
      }
    } else {
      sqlite3ExprListDelete(pParse->db, pList);
    }
  } break;
  case 197: {
    yymsp[-2].minor.yy454 = sqlite3ExprAnd(pParse, yymsp[-2].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 198:
  case 199:;
  case 200:;
  case 201:;
  case 202:;
  case 203:;
  case 204:;
    {
      yymsp[-2].minor.yy454 = sqlite3PExpr(pParse, yymsp[-1].major, yymsp[-2].minor.yy454, yymsp[0].minor.yy454);
    }
    break;
  case 205: {
    yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;
    yymsp[-1].minor.yy0.n |= 0x80000000;
  } break;
  case 206: {
    ExprList *pList;
    int bNot = yymsp[-1].minor.yy0.n & 0x80000000;
    yymsp[-1].minor.yy0.n &= 0x7fffffff;
    pList = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy454);
    pList = sqlite3ExprListAppend(pParse, pList, yymsp[-2].minor.yy454);
    yymsp[-2].minor.yy454 = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy0, 0);
    if (bNot)
      yymsp[-2].minor.yy454 = sqlite3PExpr(pParse, 19, yymsp[-2].minor.yy454, 0);
    if (yymsp[-2].minor.yy454)
      yymsp[-2].minor.yy454->flags |= 0x000100;
  } break;
  case 207: {
    ExprList *pList;
    int bNot = yymsp[-3].minor.yy0.n & 0x80000000;
    yymsp[-3].minor.yy0.n &= 0x7fffffff;
    pList = sqlite3ExprListAppend(pParse, 0, yymsp[-2].minor.yy454);
    pList = sqlite3ExprListAppend(pParse, pList, yymsp[-4].minor.yy454);
    pList = sqlite3ExprListAppend(pParse, pList, yymsp[0].minor.yy454);
    yymsp[-4].minor.yy454 = sqlite3ExprFunction(pParse, pList, &yymsp[-3].minor.yy0, 0);
    if (bNot)
      yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 19, yymsp[-4].minor.yy454, 0);
    if (yymsp[-4].minor.yy454)
      yymsp[-4].minor.yy454->flags |= 0x000100;
  } break;
  case 208: {
    yymsp[-1].minor.yy454 = sqlite3PExprIsNull(pParse, yymsp[0].major, yymsp[-1].minor.yy454);
  } break;
  case 209: {
    yymsp[-2].minor.yy454 = sqlite3PExprIsNull(pParse, 52, yymsp[-2].minor.yy454);
  } break;
  case 210: {
    yymsp[-2].minor.yy454 = sqlite3PExprIs(pParse, 45, yymsp[-2].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 211: {
    yymsp[-3].minor.yy454 = sqlite3PExprIs(pParse, 46, yymsp[-3].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 212: {
    yymsp[-5].minor.yy454 = sqlite3PExprIs(pParse, 45, yymsp[-5].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 213: {
    yymsp[-4].minor.yy454 = sqlite3PExprIs(pParse, 46, yymsp[-4].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 214:
  case 215:;
    {
      yymsp[-1].minor.yy454 = sqlite3PExpr(pParse, yymsp[-1].major, yymsp[0].minor.yy454, 0);
    }
    break;
  case 216: {
    Expr *p = yymsp[0].minor.yy454;
    u8 op = yymsp[-1].major + (173 - 107);

    ((void)(0))

        ;

    ((void)(0))

        ;
    if (p && p->op == 173) {
      p->op = op;
      yymsp[-1].minor.yy454 = p;
    } else {
      yymsp[-1].minor.yy454 = sqlite3PExpr(pParse, op, p, 0);
    }
  } break;
  case 217: {
    ExprList *pList = sqlite3ExprListAppend(pParse, 0, yymsp[-2].minor.yy454);
    pList = sqlite3ExprListAppend(pParse, pList, yymsp[0].minor.yy454);
    yylhsminor.yy454 = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy0, 0);
  }
    yymsp[-2].minor.yy454 = yylhsminor.yy454;
    break;
  case 218:
  case 221:;
    {
      yymsp[0].minor.yy144 = 0;
    }
    break;
  case 220: {
    ExprList *pList = sqlite3ExprListAppend(pParse, 0, yymsp[-2].minor.yy454);
    pList = sqlite3ExprListAppend(pParse, pList, yymsp[0].minor.yy454);
    yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 49, yymsp[-4].minor.yy454, 0);
    if (yymsp[-4].minor.yy454) {
      yymsp[-4].minor.yy454->x.pList = pList;
      sqlite3ExprSetHeightAndFlags(pParse, yymsp[-4].minor.yy454);
    } else {
      sqlite3ExprListDelete(pParse->db, pList);
    }
    if (yymsp[-3].minor.yy144)
      yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 19, yymsp[-4].minor.yy454, 0);
  } break;
  case 223: {
    if (yymsp[-1].minor.yy14 == 0) {

      Expr *pB = sqlite3Expr(pParse->db, 118, yymsp[-3].minor.yy144 ? "true" : "false");
      if (pB)
        sqlite3ExprIdToTrueFalse(pB);
      if (!(((yymsp[-4].minor.yy454)->flags & (u32)(0x000008)) != 0)) {
        sqlite3ExprUnmapAndDelete(pParse, yymsp[-4].minor.yy454);
        yymsp[-4].minor.yy454 = pB;
      } else {
        yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, yymsp[-3].minor.yy144 ? 43 : 44, pB, yymsp[-4].minor.yy454);
      }
    } else {
      Expr *pRHS = yymsp[-1].minor.yy14->a[0].pExpr;
      if (yymsp[-1].minor.yy14->nExpr == 1 && sqlite3ExprIsConstant(pParse, pRHS) && yymsp[-4].minor.yy454->op != 177) {
        yymsp[-1].minor.yy14->a[0].pExpr = 0;
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
        pRHS = sqlite3PExpr(pParse, 173, pRHS, 0);
        yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 54, yymsp[-4].minor.yy454, pRHS);
      } else if (yymsp[-1].minor.yy14->nExpr == 1 && pRHS->op == 139) {
        yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 50, yymsp[-4].minor.yy454, 0);
        sqlite3PExprAddSelect(pParse, yymsp[-4].minor.yy454, pRHS->x.pSelect);
        pRHS->x.pSelect = 0;
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
      } else {
        yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 50, yymsp[-4].minor.yy454, 0);
        if (yymsp[-4].minor.yy454 == 0) {
          sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
        } else if (yymsp[-4].minor.yy454->pLeft->op == 177) {
          int nExpr = yymsp[-4].minor.yy454->pLeft->x.pList->nExpr;
          Select *pSelectRHS = sqlite3ExprListToValues(pParse, nExpr, yymsp[-1].minor.yy14);
          if (pSelectRHS) {
            parserDoubleLinkSelect(pParse, pSelectRHS);
            sqlite3PExprAddSelect(pParse, yymsp[-4].minor.yy454, pSelectRHS);
          }
        } else {
          yymsp[-4].minor.yy454->x.pList = yymsp[-1].minor.yy14;
          sqlite3ExprSetHeightAndFlags(pParse, yymsp[-4].minor.yy454);
        }
      }
      if (yymsp[-3].minor.yy144)
        yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 19, yymsp[-4].minor.yy454, 0);
    }
  } break;
  case 224: {
    yymsp[-2].minor.yy454 = sqlite3PExpr(pParse, 139, 0, 0);
    sqlite3PExprAddSelect(pParse, yymsp[-2].minor.yy454, yymsp[-1].minor.yy555);
  } break;
  case 225: {
    yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 50, yymsp[-4].minor.yy454, 0);
    sqlite3PExprAddSelect(pParse, yymsp[-4].minor.yy454, yymsp[-1].minor.yy555);
    if (yymsp[-3].minor.yy144)
      yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 19, yymsp[-4].minor.yy454, 0);
  } break;
  case 226: {
    SrcList *pSrc = sqlite3SrcListAppend(pParse, 0, &yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0);
    Select *pSelect = sqlite3SelectNew(pParse, 0, pSrc, 0, 0, 0, 0, 0, 0);
    if (yymsp[0].minor.yy14)
      sqlite3SrcListFuncArgs(pParse, pSelect ? pSrc : 0, yymsp[0].minor.yy14);
    yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 50, yymsp[-4].minor.yy454, 0);
    sqlite3PExprAddSelect(pParse, yymsp[-4].minor.yy454, pSelect);
    if (yymsp[-3].minor.yy144)
      yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 19, yymsp[-4].minor.yy454, 0);
  } break;
  case 227: {
    Expr *p;
    p = yymsp[-3].minor.yy454 = sqlite3PExpr(pParse, 20, 0, 0);
    sqlite3PExprAddSelect(pParse, p, yymsp[-1].minor.yy555);
  } break;
  case 228: {
    yymsp[-4].minor.yy454 = sqlite3PExpr(pParse, 158, yymsp[-3].minor.yy454, 0);
    if (yymsp[-4].minor.yy454) {
      yymsp[-4].minor.yy454->x.pList = yymsp[-1].minor.yy454 ? sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy14, yymsp[-1].minor.yy454) : yymsp[-2].minor.yy14;
      sqlite3ExprSetHeightAndFlags(pParse, yymsp[-4].minor.yy454);
    } else {
      sqlite3ExprListDelete(pParse->db, yymsp[-2].minor.yy14);
      sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy454);
    }
  } break;
  case 229: {
    yymsp[-4].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[-2].minor.yy454);
    yymsp[-4].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[0].minor.yy454);
  } break;
  case 230: {
    yymsp[-3].minor.yy14 = sqlite3ExprListAppend(pParse, 0, yymsp[-2].minor.yy454);
    yymsp[-3].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-3].minor.yy14, yymsp[0].minor.yy454);
  } break;
  case 235: {
    yymsp[-2].minor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy14, yymsp[0].minor.yy454);
  } break;
  case 236: {
    yymsp[0].minor.yy14 = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy454);
  } break;
  case 238:
  case 243:;
    {
      yymsp[-2].minor.yy14 = yymsp[-1].minor.yy14;
    }
    break;
  case 239: {
    sqlite3CreateIndex(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, sqlite3SrcListAppend(pParse, 0, &yymsp[-4].minor.yy0, 0), yymsp[-2].minor.yy14, yymsp[-10].minor.yy144, &yymsp[-11].minor.yy0, yymsp[0].minor.yy454, 0, yymsp[-8].minor.yy144, 0);
    if ((pParse->eParseMode >= 2) && pParse->pNewIndex) {
      sqlite3RenameTokenMap(pParse, pParse->pNewIndex->zName, &yymsp[-4].minor.yy0);
    }
  } break;
  case 240:
  case 281:;
    {
      yymsp[0].minor.yy144 = 2;
    }
    break;
  case 241: {
    yymsp[1].minor.yy144 = 0;
  } break;
  case 244: {
    yymsp[-4].minor.yy14 = parserAddExprIdListTerm(pParse, yymsp[-4].minor.yy14, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy144, yymsp[0].minor.yy144);
  } break;
  case 245: {
    yymsp[-2].minor.yy14 = parserAddExprIdListTerm(pParse, 0, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy144, yymsp[0].minor.yy144);
  } break;
  case 248: {
    sqlite3DropIndex(pParse, yymsp[0].minor.yy203, yymsp[-1].minor.yy144);
  } break;
  case 249: {
    sqlite3Vacuum(pParse, 0, yymsp[0].minor.yy454);
  } break;
  case 250: {
    sqlite3Vacuum(pParse, &yymsp[-1].minor.yy0, yymsp[0].minor.yy454);
  } break;
  case 253: {
    sqlite3Pragma(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0, 0, 0);
  } break;
  case 254: {
    sqlite3Pragma(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, 0);
  } break;
  case 255: {
    sqlite3Pragma(pParse, &yymsp[-4].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-1].minor.yy0, 0);
  } break;
  case 256: {
    sqlite3Pragma(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, 1);
  } break;
  case 257: {
    sqlite3Pragma(pParse, &yymsp[-4].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-1].minor.yy0, 1);
  } break;
  case 260: {
    Token all;
    all.z = yymsp[-3].minor.yy0.z;
    all.n = (int)(yymsp[0].minor.yy0.z - yymsp[-3].minor.yy0.z) + yymsp[0].minor.yy0.n;
    sqlite3FinishTrigger(pParse, yymsp[-1].minor.yy427, &all);
  } break;
  case 261: {
    sqlite3BeginTrigger(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, yymsp[-5].minor.yy144, yymsp[-4].minor.yy286.a, yymsp[-4].minor.yy286.b, yymsp[-2].minor.yy203, yymsp[0].minor.yy454, yymsp[-10].minor.yy144, yymsp[-8].minor.yy144);
    yymsp[-10].minor.yy0 = (yymsp[-6].minor.yy0.n == 0 ? yymsp[-7].minor.yy0 : yymsp[-6].minor.yy0);

  } break;
  case 262: {
    yymsp[0].minor.yy144 = yymsp[0].major;
  } break;
  case 263: {
    yymsp[-1].minor.yy144 = 66;
  } break;
  case 264: {
    yymsp[1].minor.yy144 = 33;
  } break;
  case 265:
  case 266:;
    {
      yymsp[0].minor.yy286.a = yymsp[0].major;
      yymsp[0].minor.yy286.b = 0;
    }
    break;
  case 267: {
    yymsp[-2].minor.yy286.a = 130;
    yymsp[-2].minor.yy286.b = yymsp[0].minor.yy132;
  } break;
  case 268:
  case 286:;
    {
      yymsp[1].minor.yy454 = 0;
    }
    break;
  case 269:
  case 287:;
    {
      yymsp[-1].minor.yy454 = yymsp[0].minor.yy454;
    }
    break;
  case 270: {
    yymsp[-2].minor.yy427->pLast->pNext = yymsp[-1].minor.yy427;
    yymsp[-2].minor.yy427->pLast = yymsp[-1].minor.yy427;
  } break;
  case 271: {
    yymsp[-1].minor.yy427->pLast = yymsp[-1].minor.yy427;
  } break;
  case 272: {
    sqlite3ErrorMsg(pParse, "the INDEXED BY clause is not allowed on UPDATE or DELETE statements "
                            "within triggers");
  } break;
  case 273: {
    sqlite3ErrorMsg(pParse, "the NOT INDEXED clause is not allowed on UPDATE or DELETE statements "
                            "within triggers");
  } break;
  case 274: {
    yylhsminor.yy427 = sqlite3TriggerUpdateStep(pParse, yymsp[-6].minor.yy203, yymsp[-2].minor.yy203, yymsp[-3].minor.yy14, yymsp[-1].minor.yy454, yymsp[-7].minor.yy144, yymsp[-8].minor.yy0.z, yymsp[0].minor.yy168);
  }
    yymsp[-8].minor.yy427 = yylhsminor.yy427;
    break;
  case 275: {
    yylhsminor.yy427 = sqlite3TriggerInsertStep(pParse, yymsp[-4].minor.yy203, yymsp[-3].minor.yy132, yymsp[-2].minor.yy555, yymsp[-6].minor.yy144, yymsp[-1].minor.yy122, yymsp[-7].minor.yy168, yymsp[0].minor.yy168);
  }
    yymsp[-7].minor.yy427 = yylhsminor.yy427;
    break;
  case 276: {
    yylhsminor.yy427 = sqlite3TriggerDeleteStep(pParse, yymsp[-3].minor.yy203, yymsp[-1].minor.yy454, yymsp[-5].minor.yy0.z, yymsp[0].minor.yy168);
  }
    yymsp[-5].minor.yy427 = yylhsminor.yy427;
    break;
  case 277: {
    yylhsminor.yy427 = sqlite3TriggerSelectStep(pParse->db, yymsp[-1].minor.yy555, yymsp[-2].minor.yy168, yymsp[0].minor.yy168);
  }
    yymsp[-2].minor.yy427 = yylhsminor.yy427;
    break;
  case 278: {
    yymsp[-3].minor.yy454 = sqlite3PExpr(pParse, 72, 0, 0);
    if (yymsp[-3].minor.yy454) {
      yymsp[-3].minor.yy454->affExpr = 4;
    }
  } break;
  case 279: {
    yymsp[-5].minor.yy454 = sqlite3PExpr(pParse, 72, yymsp[-1].minor.yy454, 0);
    if (yymsp[-5].minor.yy454) {
      yymsp[-5].minor.yy454->affExpr = (char)yymsp[-3].minor.yy144;
    }
  } break;
  case 280: {
    yymsp[0].minor.yy144 = 1;
  } break;
  case 282: {
    yymsp[0].minor.yy144 = 3;
  } break;
  case 283: {
    sqlite3DropTrigger(pParse, yymsp[0].minor.yy203, yymsp[-1].minor.yy144);
  } break;
  case 284: {
    sqlite3Attach(pParse, yymsp[-3].minor.yy454, yymsp[-1].minor.yy454, yymsp[0].minor.yy454);
  } break;
  case 285: {
    sqlite3Detach(pParse, yymsp[0].minor.yy454);
  } break;
  case 288: {
    sqlite3Reindex(pParse, 0, 0);
  } break;
  case 289: {
    sqlite3Reindex(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);
  } break;
  case 290: {
    sqlite3Analyze(pParse, 0, 0);
  } break;
  case 291: {
    sqlite3Analyze(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);
  } break;
  case 292: {
    sqlite3AlterRenameTable(pParse, yymsp[-3].minor.yy203, &yymsp[0].minor.yy0);
  } break;
  case 293: {
    yymsp[-1].minor.yy0.n = (int)(pParse->sLastToken.z - yymsp[-1].minor.yy0.z) + pParse->sLastToken.n;
    sqlite3AlterFinishAddColumn(pParse, &yymsp[-1].minor.yy0);
  } break;
  case 294: {
    disableLookaside(pParse);
    sqlite3AlterBeginAddColumn(pParse, yymsp[-4].minor.yy203);
    sqlite3AddColumn(pParse, yymsp[-1].minor.yy0, yymsp[0].minor.yy0);
    yymsp[-6].minor.yy0 = yymsp[-1].minor.yy0;
  } break;
  case 295: {
    sqlite3AlterDropColumn(pParse, yymsp[-3].minor.yy203, &yymsp[0].minor.yy0);
  } break;
  case 296: {
    sqlite3AlterRenameColumn(pParse, yymsp[-5].minor.yy203, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0);
  } break;
  case 297: {
    sqlite3AlterDropConstraint(pParse, yymsp[-3].minor.yy203, &yymsp[0].minor.yy0, 0);
  } break;
  case 298: {
    sqlite3AlterDropConstraint(pParse, yymsp[-6].minor.yy203, 0, &yymsp[-3].minor.yy0);
  } break;
  case 299: {
    sqlite3AlterSetNotNull(pParse, yymsp[-7].minor.yy203, &yymsp[-4].minor.yy0, &yymsp[-2].minor.yy0);
  } break;
  case 300: {
    sqlite3AlterAddConstraint(pParse, yymsp[-8].minor.yy203, &yymsp[-6].minor.yy0, &yymsp[-5].minor.yy0, yymsp[-3].minor.yy0.z + 1, (yymsp[-1].minor.yy0.z - yymsp[-3].minor.yy0.z - 1), yymsp[-2].minor.yy454);
  } break;
  case 301: {
    sqlite3AlterAddConstraint(pParse, yymsp[-6].minor.yy203, &yymsp[-4].minor.yy0, 0, yymsp[-3].minor.yy0.z + 1, (yymsp[-1].minor.yy0.z - yymsp[-3].minor.yy0.z - 1), yymsp[-2].minor.yy454);
  } break;
  case 302: {
    sqlite3VtabFinishParse(pParse, 0);
  } break;
  case 303: {
    sqlite3VtabFinishParse(pParse, &yymsp[0].minor.yy0);
  } break;
  case 304: {
    sqlite3VtabBeginParse(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, yymsp[-4].minor.yy144);
  } break;
  case 305: {
    sqlite3VtabArgInit(pParse);
  } break;
  case 306:
  case 307:;
  case 308:;
    {
      sqlite3VtabArgExtend(pParse, &yymsp[0].minor.yy0);
    }
    break;
  case 309:
  case 310:;
    {
      sqlite3WithPush(pParse, yymsp[0].minor.yy59, 1);
    }
    break;
  case 311: {
    yymsp[0].minor.yy462 = 1;
  } break;
  case 312: {
    yymsp[-1].minor.yy462 = 0;
  } break;
  case 313: {
    yymsp[-2].minor.yy462 = 2;
  } break;
  case 314: {
    yymsp[-5].minor.yy67 = sqlite3CteNew(pParse, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy14, yymsp[-1].minor.yy555, yymsp[-3].minor.yy462);
  } break;
  case 315: {
    pParse->bHasWith = 1;
  } break;
  case 316: {
    yymsp[0].minor.yy59 = sqlite3WithAdd(pParse, 0, yymsp[0].minor.yy67);
  } break;
  case 317: {
    yymsp[-2].minor.yy59 = sqlite3WithAdd(pParse, yymsp[-2].minor.yy59, yymsp[0].minor.yy67);
  } break;
  case 318: {

    ((void)(0))

        ;
    sqlite3WindowChain(pParse, yymsp[0].minor.yy211, yymsp[-2].minor.yy211);
    yymsp[0].minor.yy211->pNextWin = yymsp[-2].minor.yy211;
    yylhsminor.yy211 = yymsp[0].minor.yy211;
  }
    yymsp[-2].minor.yy211 = yylhsminor.yy211;
    break;
  case 319: {
    if ((yymsp[-1].minor.yy211)) {
      yymsp[-1].minor.yy211->zName = sqlite3DbStrNDup(pParse->db, yymsp[-4].minor.yy0.z, yymsp[-4].minor.yy0.n);
    }
    yylhsminor.yy211 = yymsp[-1].minor.yy211;
  }
    yymsp[-4].minor.yy211 = yylhsminor.yy211;
    break;
  case 320: {
    yymsp[-4].minor.yy211 = sqlite3WindowAssemble(pParse, yymsp[0].minor.yy211, yymsp[-2].minor.yy14, yymsp[-1].minor.yy14, 0);
  } break;
  case 321: {
    yylhsminor.yy211 = sqlite3WindowAssemble(pParse, yymsp[0].minor.yy211, yymsp[-2].minor.yy14, yymsp[-1].minor.yy14, &yymsp[-5].minor.yy0);
  }
    yymsp[-5].minor.yy211 = yylhsminor.yy211;
    break;
  case 322: {
    yymsp[-3].minor.yy211 = sqlite3WindowAssemble(pParse, yymsp[0].minor.yy211, 0, yymsp[-1].minor.yy14, 0);
  } break;
  case 323: {
    yylhsminor.yy211 = sqlite3WindowAssemble(pParse, yymsp[0].minor.yy211, 0, yymsp[-1].minor.yy14, &yymsp[-4].minor.yy0);
  }
    yymsp[-4].minor.yy211 = yylhsminor.yy211;
    break;
  case 324: {
    yylhsminor.yy211 = sqlite3WindowAssemble(pParse, yymsp[0].minor.yy211, 0, 0, &yymsp[-1].minor.yy0);
  }
    yymsp[-1].minor.yy211 = yylhsminor.yy211;
    break;
  case 325: {
    yymsp[1].minor.yy211 = sqlite3WindowAlloc(pParse, 0, 91, 0, 86, 0, 0);
  } break;
  case 326: {
    yylhsminor.yy211 = sqlite3WindowAlloc(pParse, yymsp[-2].minor.yy144, yymsp[-1].minor.yy509.eType, yymsp[-1].minor.yy509.pExpr, 86, 0, yymsp[0].minor.yy462);
  }
    yymsp[-2].minor.yy211 = yylhsminor.yy211;
    break;
  case 327: {
    yylhsminor.yy211 = sqlite3WindowAlloc(pParse, yymsp[-5].minor.yy144, yymsp[-3].minor.yy509.eType, yymsp[-3].minor.yy509.pExpr, yymsp[-1].minor.yy509.eType, yymsp[-1].minor.yy509.pExpr, yymsp[0].minor.yy462);
  }
    yymsp[-5].minor.yy211 = yylhsminor.yy211;
    break;
  case 329:
  case 331:;
    {
      yylhsminor.yy509 = yymsp[0].minor.yy509;
    }
    yymsp[0].minor.yy509 = yylhsminor.yy509;
    break;
  case 330:
  case 332:;
  case 334:;
    {
      yylhsminor.yy509.eType = yymsp[-1].major;
      yylhsminor.yy509.pExpr = 0;
    }
    yymsp[-1].minor.yy509 = yylhsminor.yy509;
    break;
  case 333: {
    yylhsminor.yy509.eType = yymsp[0].major;
    yylhsminor.yy509.pExpr = yymsp[-1].minor.yy454;
  }
    yymsp[-1].minor.yy509 = yylhsminor.yy509;
    break;
  case 335: {
    yymsp[1].minor.yy462 = 0;
  } break;
  case 336: {
    yymsp[-1].minor.yy462 = yymsp[0].minor.yy462;
  } break;
  case 337:
  case 338:;
    {
      yymsp[-1].minor.yy462 = yymsp[-1].major;
    }
    break;
  case 339: {
    yymsp[0].minor.yy462 = yymsp[0].major;
  } break;
  case 340: {
    yymsp[-1].minor.yy211 = yymsp[0].minor.yy211;
  } break;
  case 341: {
    if (yymsp[0].minor.yy211) {
      yymsp[0].minor.yy211->pFilter = yymsp[-1].minor.yy454;
    } else {
      sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy454);
    }
    yylhsminor.yy211 = yymsp[0].minor.yy211;
  }
    yymsp[-1].minor.yy211 = yylhsminor.yy211;
    break;
  case 342: {
    yylhsminor.yy211 = yymsp[0].minor.yy211;
  }
    yymsp[0].minor.yy211 = yylhsminor.yy211;
    break;
  case 343: {
    yylhsminor.yy211 = (Window *)sqlite3DbMallocZero(pParse->db, sizeof(Window));
    if (yylhsminor.yy211) {
      yylhsminor.yy211->eFrmType = 167;
      yylhsminor.yy211->pFilter = yymsp[0].minor.yy454;
    } else {
      sqlite3ExprDelete(pParse->db, yymsp[0].minor.yy454);
    }
  }
    yymsp[0].minor.yy211 = yylhsminor.yy211;
    break;
  case 344: {
    yymsp[-3].minor.yy211 = yymsp[-1].minor.yy211;

    ((void)(0))

        ;
  } break;
  case 345: {
    yymsp[-1].minor.yy211 = (Window *)sqlite3DbMallocZero(pParse->db, sizeof(Window));
    if (yymsp[-1].minor.yy211) {
      yymsp[-1].minor.yy211->zName = sqlite3DbStrNDup(pParse->db, yymsp[0].minor.yy0.z, yymsp[0].minor.yy0.n);
    }
  } break;
  case 346: {
    yymsp[-4].minor.yy454 = yymsp[-1].minor.yy454;
  } break;
  case 347: {
    yylhsminor.yy454 = tokenExpr(pParse, yymsp[0].major, yymsp[0].minor.yy0);
    sqlite3DequoteNumber(pParse, yylhsminor.yy454);
  }
    yymsp[0].minor.yy454 = yylhsminor.yy454;
    break;
  default:;
    ;

    ((void)(0))

        ;
    ;
    ;

    ((void)(0))

        ;
    ;
    ;
    ;
    ;
    ;
    ;

    ((void)(0))

        ;
    ;
    ;
    ;
    ;
    ;
    ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;

    ((void)(0))

        ;
    ;

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    ;
    ;
    ;

    ((void)(0))

        ;
    ;

    ((void)(0))

        ;
    ;
    ;
    ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    break;
  };

  yygoto = yyRuleInfoLhs[yyruleno];
  yysize = yyRuleInfoNRhs[yyruleno];
  yyact = yy_find_reduce_action(yymsp[yysize].stateno, (unsigned short int)yygoto);

  yymsp += yysize + 1;
  yypParser->yytos = yymsp;
  yymsp->stateno = (unsigned short int)yyact;
  yymsp->major = (unsigned short int)yygoto;
  ;
  return yyact;
}

void yy_syntax_error(yyParser *yypParser, int yymajor, Token yyminor) {

  Parse *pParse = yypParser->pParse;

  (void)(yymajor);
  if (yyminor.z[0]) {
    parserSyntaxError(pParse, &yyminor);
  } else {
    sqlite3ErrorMsg(pParse, "incomplete input");
  }

  yypParser->pParse = pParse;
}

void yy_accept(yyParser *yypParser) {

  Parse *pParse = yypParser->pParse;

  yypParser->pParse = pParse;
}