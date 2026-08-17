#define _GNU_SOURCE 1
#include <math.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite/sqlite3.h"
#include "sqlite/AggInfo.h"
#include "sqlite/AuxData.h"
#include "sqlite/BenignMallocHooks.h"
#include "sqlite/Bitvec.h"
#include "sqlite/BtCursor.h"
#include "sqlite/BtShared.h"
#include "sqlite/Btree.h"
#include "sqlite/BusyHandler.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/Cte.h"
#include "sqlite/CteUse.h"
#include "sqlite/Db.h"
#include "sqlite/DbClientData.h"
#include "sqlite/DbPage.h"
#include "sqlite/EdupBuf.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FKey.h"
#include "sqlite/FpDecode.h"
#include "sqlite/FuncDef.h"
#include "sqlite/FuncDefHash.h"
#include "sqlite/FuncDestructor.h"
#include "sqlite/Hash.h"
#include "sqlite/HashElem.h"
#include "sqlite/HiddenIndexInfo.h"
#include "sqlite/IdList.h"
#include "sqlite/Incrblob.h"
#include "sqlite/Index.h"
#include "sqlite/IndexedExpr.h"
#include "sqlite/InitData.h"
#include "sqlite/IntegrityCk.h"
#include "sqlite/JsonEachConnection.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/LOGFUNC_t.h"
#include "sqlite/LogEst.h"
#include "sqlite/Lookaside.h"
#include "sqlite/LookasideSlot.h"
#include "sqlite/Mem.h"
#include "sqlite/Mem0Global.h"
#include "sqlite/sqlite3DigitPairs_t.h"
#include "sqlite/sqlite3PrngType.h"
#include "sqlite/MemFile.h"
#include "sqlite/MemPage.h"
#include "sqlite/MemStore.h"
#include "sqlite/MergeEngine.h"
#include "sqlite/Module.h"
#include "sqlite/NameContext.h"
#include "sqlite/OnOrUsing.h"
#include "sqlite/Op.h"
#include "sqlite/PCache.h"
#include "sqlite/PCache1.h"
#include "sqlite/PCacheGlobal.h"
#include "sqlite/Pager.h"
#include "sqlite/Parse.h"
#include "sqlite/PgHdr.h"
#include "sqlite/PgHdr1.h"
#include "sqlite/Pgno.h"
#include "sqlite/PmaReader.h"
#include "sqlite/PragmaName.h"
#include "sqlite/PragmaVtab.h"
#include "sqlite/RenameToken.h"
#include "sqlite/Returning.h"
#include "sqlite/RowSet.h"
#include "sqlite/RowSetChunk.h"
#include "sqlite/RowSetEntry.h"
#include "sqlite/Savepoint.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterFile.h"
#include "sqlite/SorterList.h"
#include "sqlite/SorterRecord.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/StrAccum.h"
#include "sqlite/SubProgram.h"
#include "sqlite/Subquery.h"
#include "sqlite/SubrtnSig.h"
#include "sqlite/TabResult.h"
#include "sqlite/Table.h"
#include "sqlite/Token.h"
#include "sqlite/Trigger.h"
#include "sqlite/TriggerPrg.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/Upsert.h"
#include "sqlite/VList.h"
#include "sqlite/VTable.h"
#include "sqlite/ValueNewStat4Ctx.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeCursor.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/VdbeOpList.h"
#include "sqlite/VdbeSorter.h"
#include "sqlite/VtabCtx.h"
#include "sqlite/Wal.h"
#include "sqlite/WhereAndInfo.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/WhereMemBlock.h"
#include "sqlite/WhereOrInfo.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/Window.h"
#include "sqlite/With.h"
#include "sqlite/YYMINORTYPE.h"
#include "sqlite/YyAction.h"
#include "sqlite/analysisInfo.h"
#include "sqlite/bft.h"
#include "sqlite/compareInfo.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3AutoExtList.h"
#include "sqlite/sqlite3FaultFuncType.h"
#include "sqlite/sqlite3LocaltimeType.h"
#include "sqlite/sqlite3StatType.h"
#include "sqlite/sqlite3StatValueType.h"
#include "sqlite/sqlite3_api_routines.h"
#include "sqlite/sqlite3_backup.h"
#include "sqlite/sqlite3_blob.h"
#include "sqlite/sqlite3_callback.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_index_info.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/sqlite3_loadext_entry.h"
#include "sqlite/sqlite3_mem_methods.h"
#include "sqlite/sqlite3_module.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_mutex_methods.h"
#include "sqlite/sqlite3_pcache_methods2.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite3_version.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/sqlite3_xauth.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/sqlite_uint64.h"
#include "sqlite/tRowcnt.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/unixFile.h"
#include "sqlite/uptr.h"
#include "sqlite/void_function.h"
#include "sqlite/ynVar.h"
#include "sqlite/yyParser.h"
#include "sqlite/yyStackEntry.h"
#include "sqlite/SqliteAccessFlags.h"
#include "sqlite/SqliteAuthorizerReturnCode.h"
#include "sqlite/SqliteCheckpointMode.h"
#include "sqlite/SqliteConfigOption.h"
#include "sqlite/SqliteConflictResolution.h"
#include "sqlite/SqliteDbConfigOption.h"
#include "sqlite/SqliteDbStatusParameter.h"
#include "sqlite/SqliteDeserializeFlags.h"
#include "sqlite/SqliteFileControlOpcode.h"
#include "sqlite/SqliteFunctionFlags.h"
#include "sqlite/SqliteFundamentalDatatype.h"
#include "sqlite/SqliteIndexConstraintOp.h"
#include "sqlite/SqliteIoCap.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteOpenFlags.h"
#include "sqlite/SqlitePrepareFlags.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteSerializeFlags.h"
#include "sqlite/SqliteStatusParameter.h"
#include "sqlite/SqliteSyncFlags.h"
#include "sqlite/SqliteTestCtrlOpcode.h"
#include "sqlite/SqliteTextEncoding.h"
#include "sqlite/SqliteTraceEventCode.h"
#include "sqlite/SqliteTxnState.h"
#include "sqlite/SqliteVtabConfigOption.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int analysisLoader(void *pData, int argc, char **argv, char **NotUsed);
static char *appendText(char *p, const char *z);
static int compare2pow63(const char *zNum, int incr);
static const char *databaseName(const char *zName);
static void decodeIntArray(char *zIntArray, int nOut, tRowcnt *aOut, LogEst *aLog, Index *pIndex);
static double degToRad(double x);
static i64 identLength(const char *z);
static void identPut(char *z, int *pIdx, char *zSignedIdent);
static i64 keywordCode(const char *z, i64 n, int *pType);
static void logBadConnection(const char *zType);
static void mallocWithAlarm(int n, void **pp);
static int nocaseCollatingFunc(void *NotUsed, int nKey1, const void *pKey1, int nKey2, const void *pKey2);
static sqlite3_mutex *noopMutexAlloc(int id);
static int noopMutexEnd(void);
static int noopMutexInit(void);
static int openDatabase(const char *zFilename, sqlite3 **ppDb, unsigned int flags, const char *zVfs);
static void parserStackFree(void *pOld, Parse *pParse);
static u64 powerOfTen(int p, u32 *pLo);
static sqlite3_mutex *pthreadMutexAlloc(int iType);
static int pthreadMutexEnd(void);
static int pthreadMutexInit(void);
static int __attribute__((noinline)) putVarint64(unsigned char *p, u64 v);
static int pwr10to2(int p);
static int pwr2to10(int p);
static double radToDeg(double x);
static void renderLogMsg(int iErrCode, const char *zFormat, va_list ap);
static int rtrimCollFunc(void *pUser, int nKey1, const void *pKey1, int nKey2, const void *pKey2);
static int sqliteDefaultBusyCallback(void *ptr, int count);
static int sqlite3KeywordCode(const unsigned char *, int);
static void sqlite3MallocAlarm(int nByte);
static u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo);
static void sqlite3RegisterJsonFunctions(void);
static sqlite3_int64 sqlite3StatusValue(int);
static void unixTempFileInit(void);
static const char *uriParameter(const char *zFilename, const char *zParam);
static double xCeil(double x);
static double xFloor(double x);
static unsigned short int yy_find_shift_action(unsigned short int iLookAhead, unsigned short int stateno);

static const char *const sqlite3azCompileOpt[] = {
    "ATOMIC_INTRINSICS="
    "1",
    "COMPILER=gcc-"
    "16.2.1 20260810",
    "DEFAULT_AUTOVACUUM",
    "DEFAULT_CACHE_SIZE="
    "-2000",
    "DEFAULT_FILE_FORMAT="
    "4",
    "DEFAULT_JOURNAL_SIZE_LIMIT="
    "-1",
    "DEFAULT_MMAP_SIZE="
    "0",
    "DEFAULT_PAGE_SIZE="
    "4096",
    "DEFAULT_PCACHE_INITSZ="
    "20",
    "DEFAULT_RECURSIVE_TRIGGERS",
    "DEFAULT_SECTOR_SIZE="
    "4096",
    "DEFAULT_SYNCHRONOUS="
    "2",
    "DEFAULT_WAL_AUTOCHECKPOINT="
    "1000",
    "DEFAULT_WAL_SYNCHRONOUS="
    "2",
    "DEFAULT_WORKER_THREADS="
    "0",
    "DIRECT_OVERFLOW_READ",
    "ENABLE_MATH_FUNCTIONS",
    "ENABLE_PERCENTILE",
    "HAVE_ISNAN",
    "MALLOC_SOFT_LIMIT="
    "1024",
    "MAX_ATTACHED="
    "10",
    "MAX_COLUMN="
    "2000",
    "MAX_COMPOUND_SELECT="
    "500",
    "MAX_DEFAULT_PAGE_SIZE="
    "8192",
    "MAX_EXPR_DEPTH="
    "1000",
    "MAX_FUNCTION_ARG="
    "1000",
    "MAX_LENGTH="
    "1000000000",
    "MAX_LIKE_PATTERN_LENGTH="
    "50000",
    "MAX_MMAP_SIZE="
    "0x7fff0000",
    "MAX_PAGE_COUNT="
    "0xfffffffe",
    "MAX_PAGE_SIZE="
    "65536",
    "MAX_SQL_LENGTH="
    "1000000000",
    "MAX_TRIGGER_DEPTH="
    "1000",
    "MAX_VARIABLE_NUMBER="
    "32766",
    "MAX_VDBE_OP="
    "250000000",
    "MAX_WORKER_THREADS="
    "8",
    "MUTEX_PTHREADS",
    "SYSTEM_MALLOC",
    "TEMP_STORE="
    "1",
    "THREADSAFE="
    "1",
};

static const char statMutex[] = {
    0, 1, 1, 0, 0, 0, 0, 1, 0, 0,
};

static int noopMutexInit(void) {
  return SQLITE_OK;
}

static int noopMutexEnd(void) {
  return SQLITE_OK;
}

static sqlite3_mutex *noopMutexAlloc(int id) {
  (void)(id);
  return (sqlite3_mutex *)8;
}

static int pthreadMutexInit(void) {
  return SQLITE_OK;
}

static int pthreadMutexEnd(void) {
  return SQLITE_OK;
}

static sqlite3_mutex *pthreadMutexAlloc(int iType) {
  static sqlite3_mutex staticMutexes[] = {{

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          },
                                          {

                                              {{0, 0, 0, 0, PTHREAD_MUTEX_TIMED_NP, 0, 0, {((void *)0), ((void *)0)}}}

                                          }};
  sqlite3_mutex *p;
  switch (iType) {
    case SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero(sizeof(*p));
      if (p) {
        pthread_mutexattr_t recursiveAttr;
        pthread_mutexattr_init(&recursiveAttr);
        pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&p->mutex, &recursiveAttr);
        pthread_mutexattr_destroy(&recursiveAttr);
      }
      break;
    }
    case SQLITE_MUTEX_FAST: {
      p = sqlite3MallocZero(sizeof(*p));
      if (p) {
        pthread_mutex_init(&p->mutex, 0);
      }
      break;
    }
    default: {
      p = &staticMutexes[iType - 2];
      break;
    }
  }

  return p;
}

static void mallocWithAlarm(int n, void **pp) {
  void *p;
  int nFull;

  nFull = sqlite3Config.m.xRoundup(n);

  sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, n);
  if (mem0.alarmThreshold > 0) {
    sqlite3_int64 nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
    if (nUsed >= mem0.alarmThreshold - nFull) {
      __atomic_store_n((&mem0.nearlyFull), (1), 0);
      sqlite3MallocAlarm(nFull);
      if (mem0.hardLimit) {
        nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
        if (nUsed >= mem0.hardLimit - nFull) {
          *pp = 0;
          return;
        }
      }
    } else {
      __atomic_store_n((&mem0.nearlyFull), (0), 0);
    }
  }
  p = sqlite3Config.m.xMalloc(nFull);

  if (p) {
    nFull = sqlite3MallocSize(p);
    sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nFull);
    sqlite3StatusUp(SQLITE_STATUS_MALLOC_COUNT, 1);
  }
  *pp = p;
}

static void renderLogMsg(int iErrCode, const char *zFormat, va_list ap) {
  StrAccum acc;
  char zMsg[(70 * 10)];

  sqlite3StrAccumInit(&acc, 0, zMsg, sizeof(zMsg), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3Config.xLog(sqlite3Config.pLogArg, iErrCode, sqlite3StrAccumFinish(&acc));
}

static u64 powerOfTen(int p, u32 *pLo) {
  static const u64 aBase[] = {
      0x8000000000000000UL, 0xa000000000000000UL, 0xc800000000000000UL, 0xfa00000000000000UL, 0x9c40000000000000UL,
      0xc350000000000000UL, 0xf424000000000000UL, 0x9896800000000000UL, 0xbebc200000000000UL, 0xee6b280000000000UL,
      0x9502f90000000000UL, 0xba43b74000000000UL, 0xe8d4a51000000000UL, 0x9184e72a00000000UL, 0xb5e620f480000000UL,
      0xe35fa931a0000000UL, 0x8e1bc9bf04000000UL, 0xb1a2bc2ec5000000UL, 0xde0b6b3a76400000UL, 0x8ac7230489e80000UL,
      0xad78ebc5ac620000UL, 0xd8d726b7177a8000UL, 0x878678326eac9000UL, 0xa968163f0a57b400UL, 0xd3c21bcecceda100UL,
      0x84595161401484a0UL, 0xa56fa5b99019a5c8UL,
  };
  static const u64 aScale[] = {
      0x8049a4ac0c5811aeUL, 0xcf42894a5dce35eaUL, 0xa76c582338ed2621UL, 0x873e4f75e2224e68UL, 0xda7f5bf590966848UL,
      0xb080392cc4349decUL, 0x8e938662882af53eUL, 0xe65829b3046b0afaUL, 0xba121a4650e4ddebUL, 0x964e858c91ba2655UL,
      0xf2d56790ab41c2a2UL, 0xc428d05aa4751e4cUL, 0x9e74d1b791e07e48UL, 0xccccccccccccccccUL, 0xcecb8f27f4200f3aUL,
      0xa70c3c40a64e6c51UL, 0x86f0ac99b4e8dafdUL, 0xda01ee641a708de9UL, 0xb01ae745b101e9e4UL, 0x8e41ade9fbebc27dUL,
      0xe5d3ef282a242e81UL, 0xb9a74a0637ce2ee1UL, 0x95f83d0a1fb69cd9UL, 0xf24a01a73cf2dccfUL, 0xc3b8358109e84f07UL,
      0x9e19db92b4e31ba9UL,
  };
  static const unsigned int aScaleLo[] = {
      0x205b896d, 0x52064cad, 0xaf2af2b8, 0x5a7744a7, 0xaf39a475, 0xbd8d794e, 0x547eb47b, 0x0cb4a5a3, 0x92f34d62,
      0x3a6a07f9, 0xfae27299, 0xaa97e14c, 0x775ea265, 0xcccccccc, 0x00000000, 0x999090b6, 0x69a028bb, 0xe80e6f48,
      0x5ec05dd0, 0x14588f14, 0x8f1668c9, 0x6d953e2c, 0x4abdaf10, 0xbc633b39, 0x0a862f81, 0x6c07a2c2,
  };
  int g, n;
  u64 s, x;
  u32 lo;

  if (p < 0) {
    if (p == (-1)) {
      *pLo = aScaleLo[13];
      return aScale[13];
    }
    g = p / 27;
    n = p % 27;
    if (n) {
      g--;
      n += 27;
    }
  } else if (p < 27) {
    *pLo = 0;
    return aBase[p];
  } else {
    g = p / 27;
    n = p % 27;
  }
  s = aScale[g + 13];
  if (n == 0) {
    *pLo = aScaleLo[g + 13];
    return s;
  }
  x = sqlite3Multiply160(s, aScaleLo[g + 13], aBase[n], &lo);
  if (((((u64)1) << (63)) & x) == 0) {
    x = x << 1 | ((lo >> 31) & 1);
    lo = (lo << 1) | 1;
  }
  *pLo = lo;
  return x;
}

static int pwr10to2(int p) {
  return (p * 108853) >> 15;
}

static int pwr2to10(int p) {
  return (p * 78913) >> 18;
}

static int compare2pow63(const char *zNum, int incr) {
  int c = 0;
  int i;

  const char *pow63 = "922337203685477580";
  for (i = 0; c == 0 && i < 18; i++) {
    c = (zNum[i * incr] - pow63[i]) * 10;
  }
  if (c == 0) {
    c = zNum[18 * incr] - '8';
  }
  return c;
}

static int __attribute__((noinline)) putVarint64(unsigned char *p, u64 v) {
  int i, j, n;
  u8 buf[10];
  if (v & (((u64)0xff000000) << 32)) {
    p[8] = (u8)v;
    v >>= 8;
    for (i = 7; i >= 0; i--) {
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    return 9;
  }
  n = 0;
  do {
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  } while (v != 0);
  buf[0] &= 0x7f;

  for (i = 0, j = n - 1; j >= 0; j--, i++) {
    p[i] = buf[j];
  }
  return n;
}

static void logBadConnection(const char *zType) {
  sqlite3_log(SQLITE_MISUSE, "API call with %s database connection pointer", zType);
}

static void decodeIntArray(char *zIntArray, int nOut, tRowcnt *aOut, LogEst *aLog, Index *pIndex) {
  char *z = zIntArray;
  int c;
  int i;
  tRowcnt v;

  for (i = 0; *z && i < nOut; i++) {
    v = 0;
    while ((c = z[0]) >= '0' && c <= '9') {
      v = v * 10 + c - '0';
      z++;
    }

    (void)(aOut);

    aLog[i] = sqlite3LogEst(v);

    if (*z == ' ')
      z++;
  }

  {
    pIndex->bUnordered = 0;
    pIndex->noSkipScan = 0;
    while (z[0]) {
      if (sqlite3_strglob("unordered*", z) == 0) {
        pIndex->bUnordered = 1;
      } else if (sqlite3_strglob("sz=[0-9]*", z) == 0) {
        int sz = sqlite3Atoi(z + 3);
        if (sz < 2)
          sz = 2;
        pIndex->szIdxRow = sqlite3LogEst(sz);
      } else if (sqlite3_strglob("noskipscan*", z) == 0) {
        pIndex->noSkipScan = 1;
      }

      while (z[0] != 0 && z[0] != ' ')
        z++;
      while (z[0] == ' ')
        z++;
    }
  }
}

static int analysisLoader(void *pData, int argc, char **argv, char **NotUsed) {
  analysisInfo *pInfo = (analysisInfo *)pData;
  Index *pIndex;
  Table *pTable;
  const char *z;

  (void)(NotUsed), (void)(argc);

  if (argv == 0 || argv[0] == 0 || argv[2] == 0) {
    return 0;
  }
  pTable = sqlite3FindTable(pInfo->db, argv[0], pInfo->zDatabase);
  if (pTable == 0) {
    return 0;
  }
  if (argv[1] == 0) {
    pIndex = 0;
  } else if (sqlite3_stricmp(argv[0], argv[1]) == 0) {
    pIndex = sqlite3PrimaryKeyIndex(pTable);
  } else {
    pIndex = sqlite3FindIndex(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  if (pIndex) {
    tRowcnt *aiRowEst = 0;
    int nCol = pIndex->nKeyCol + 1;

    pIndex->bUnordered = 0;
    decodeIntArray((char *)z, nCol, aiRowEst, pIndex->aiRowLogEst, pIndex);
    pIndex->hasStat1 = 1;
    if (pIndex->pPartIdxWhere == 0) {
      pTable->nRowLogEst = pIndex->aiRowLogEst[0];
      pTable->tabFlags |= 0x00000010;
    }
  } else {
    Index fakeIdx;
    fakeIdx.szIdxRow = pTable->szTabRow;

    decodeIntArray((char *)z, 1, 0, &pTable->nRowLogEst, &fakeIdx);
    pTable->szTabRow = fakeIdx.szIdxRow;
    pTable->tabFlags |= 0x00000010;
  }

  return 0;
}

static i64 identLength(const char *z) {
  i64 n;
  for (n = 0; *z; n++, z++) {
    if (*z == '"') {
      n++;
    }
  }
  return n + 2;
}

static void identPut(char *z, int *pIdx, char *zSignedIdent) {
  unsigned char *zIdent = (unsigned char *)zSignedIdent;
  int i, j, needQuote;
  i = *pIdx;

  for (j = 0; zIdent[j]; j++) {
    if (!(sqlite3CtypeMap[(unsigned char)(zIdent[j])] & 0x06) && zIdent[j] != '_')
      break;
  }
  needQuote = (sqlite3CtypeMap[(unsigned char)(zIdent[0])] & 0x04) || sqlite3KeywordCode(zIdent, j) != 60 ||
              zIdent[j] != 0 || j == 0;

  if (needQuote)
    z[i++] = '"';
  for (j = 0; zIdent[j]; j++) {
    z[i++] = zIdent[j];
    if (zIdent[j] == '"')
      z[i++] = '"';
  }
  if (needQuote)
    z[i++] = '"';
  z[i] = 0;
  *pIdx = i;
}

static double xCeil(double x) {
  return ceil(x);
}

static double xFloor(double x) {
  return floor(x);
}

static double degToRad(double x) {
  return x * (3.14159265358979323846 / 180.0);
}

static double radToDeg(double x) {
  return x * (180.0 / 3.14159265358979323846);
}

static void parserStackFree(void *pOld, Parse *pParse) {
  (void)pParse;
  sqlite3_free(pOld);
}

static const unsigned short int yy_lookahead[] = {
    277, 278, 279, 241, 242, 225, 195, 227, 195, 312, 195, 218, 195, 316, 195, 235, 254, 195, 256, 19,  297, 277, 278,
    279, 218, 206, 213, 214, 206, 218, 219, 31,  206, 218, 219, 218, 219, 218, 219, 39,  218, 219, 195, 43,  44,  45,
    195, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  19,  241, 242, 195, 241, 242, 195, 255, 241, 242,
    195, 255, 237, 238, 254, 255, 256, 254, 255, 256, 264, 254, 207, 256, 43,  44,  45,  264, 47,  48,  49,  50,  51,
    52,  53,  54,  55,  56,  57,  58,  251, 287, 253, 215, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    82,  265, 195, 271, 11,  187, 188, 189, 190, 191, 192, 190, 87,  192, 89,  197, 19,  199, 197, 317, 199, 319, 25,
    271, 206, 218, 219, 206, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 43,  44,  45,  195, 47,  48,
    49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  60,  139, 140, 241, 242, 289, 241, 242, 309, 310, 294, 70,  47,
    48,  49,  50,  254, 77,  256, 254, 195, 256, 55,  56,  57,  58,  59,  221, 88,  109, 90,  269, 240, 93,  269, 107,
    108, 109, 110, 111, 112, 113, 114, 215, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 136, 117, 118,
    119, 298, 141, 300, 298, 19,  300, 129, 130, 317, 318, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    114, 277, 278, 279, 146, 122, 43,  44,  45,  195, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  218,
    277, 278, 279, 19,  19,  195, 286, 23,  68,  218, 219, 55,  56,  57,  58,  103, 104, 105, 106, 107, 108, 109, 110,
    111, 112, 113, 114, 43,  44,  45,  232, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  103, 104, 105,
    106, 107, 108, 109, 110, 111, 112, 113, 114, 135, 60,  137, 138, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 82,  281, 206, 195, 109, 110, 111, 112, 113, 114, 195, 195, 195, 205, 22,  207, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 195, 60,  116, 117, 107, 108, 218, 219, 19,  241, 242, 121, 23,  116, 117, 118,
    119, 306, 121, 308, 206, 234, 254, 15,  256, 195, 129, 259, 260, 139, 140, 145, 43,  44,  45,  200, 47,  48,  49,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  218, 219, 60,  154, 19,  156, 265, 241, 242, 24,  117, 118, 119, 120,
    21,  73,  123, 124, 125, 74,  254, 61,  256, 107, 108, 221, 133, 82,  43,  44,  45,  195, 47,  48,  49,  50,  51,
    52,  53,  54,  55,  56,  57,  58,  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 317, 318, 117,
    118, 119, 22,  120, 195, 22,  123, 124, 125, 19,  20,  284, 22,  128, 81,  288, 133, 195, 195, 218, 219, 277, 278,
    279, 139, 140, 36,  195, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 218, 219, 62,  60,  195, 241,
    242, 271, 19,  240, 60,  189, 190, 191, 192, 233, 255, 124, 254, 197, 256, 199, 72,  129, 130, 264, 195, 195, 206,
    22,  23,  60,  43,  44,  45,  206, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  195, 218, 219, 101,
    195, 60,  271, 162, 195, 107, 108, 109, 117, 118, 119, 241, 242, 115, 73,  117, 118, 119, 241, 242, 122, 60,  195,
    266, 254, 312, 256, 218, 219, 316, 203, 254, 195, 256, 255, 208, 117, 118, 119, 269, 103, 104, 105, 106, 107, 108,
    109, 110, 111, 112, 113, 114, 154, 155, 156, 157, 158, 102, 117, 118, 119, 19,  242, 144, 255, 23,  206, 24,  298,
    195, 300, 206, 195, 264, 254, 206, 256, 240, 117, 118, 119, 183, 22,  22,  23,  43,  44,  45,  151, 47,  48,  49,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  241, 242, 60,  195, 19,  241, 242, 195, 23,  241, 242, 195, 152, 254,
    310, 256, 243, 312, 254, 60,  256, 316, 254, 206, 256, 60,  218, 219, 43,  44,  45,  272, 47,  48,  49,  50,  51,
    52,  53,  54,  55,  56,  57,  58,  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 240, 60,  241, 242,
    118, 25,  102, 255, 166, 167, 101, 22,  26,  19,  20,  254, 22,  256, 139, 140, 117, 118, 119, 306, 195, 308, 117,
    118, 237, 238, 36,  122, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 195, 60,  218, 219, 60,
    109, 195, 19,  217, 60,  25,  23,  77,  117, 118, 119, 225, 233, 154, 155, 156, 72,  312, 218, 219, 90,  316, 22,
    93,  303, 304, 43,  44,  45,  195, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  183, 195, 195, 101,
    19,  213, 214, 243, 23,  107, 108, 117, 118, 119, 117, 118, 119, 115, 60,  117, 118, 119, 195, 60,  122, 218, 219,
    22,  43,  44,  45,  35,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  103, 104, 105, 106, 107, 108,
    109, 110, 111, 112, 113, 114, 154, 155, 156, 157, 158, 195, 255, 67,  195, 60,  101, 240, 311, 312, 306, 75,  308,
    316, 29,  117, 118, 119, 33,  287, 117, 118, 119, 118, 146, 183, 195, 122, 103, 104, 105, 106, 107, 108, 109, 110,
    111, 112, 113, 114, 215, 195, 77,  60,  25,  195, 122, 144, 19,  218, 219, 66,  23,  88,  246, 90,  132, 25,  93,
    154, 155, 156, 117, 118, 119, 257, 195, 131, 218, 219, 195, 265, 43,  44,  45,  195, 47,  48,  49,  50,  51,  52,
    53,  54,  55,  56,  57,  58,  183, 218, 219, 195, 19,  218, 219, 195, 23,  195, 218, 219, 117, 118, 119, 195, 233,
    255, 195, 195, 233, 22,  23,  146, 25,  233, 218, 219, 43,  44,  45,  294, 47,  48,  49,  50,  51,  52,  53,  54,
    55,  56,  57,  58,  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 12,  234, 195, 240, 74,  195,
    255, 195, 60,  243, 262, 263, 311, 312, 25,  27,  19,  316, 107, 108, 265, 24,  265, 195, 150, 195, 139, 140, 218,
    219, 42,  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 233, 102, 67,  218, 219, 218, 219, 243, 19,
    64,  22,  23,  23,  25,  195, 128, 129, 130, 233, 74,  233, 86,  154, 118, 156, 130, 265, 208, 19,  306, 95,  308,
    43,  44,  45,  266, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  153, 230, 96,  232, 43,  44,  45,
    19,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  114, 22,  306, 24,  308, 127, 120, 121, 122, 123,
    124, 125, 126, 195, 147, 212, 213, 214, 132, 23,  195, 25,  102, 100, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 218, 219, 19,  60,  195, 12,  210, 211, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    27,  134, 195, 195, 195, 210, 211, 218, 219, 195, 47,  195, 212, 213, 214, 42,  16,  130, 19,  112, 113, 114, 23,
    77,  195, 218, 219, 218, 219, 117, 163, 164, 218, 219, 218, 219, 90,  64,  19,  93,  153, 118, 43,  44,  45,  160,
    47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  195, 119, 272, 276, 43,  44,  45,  195, 47,  48,  49,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  78,  116, 80,  218, 219, 116, 144, 128, 129, 130, 218, 219, 61,  195,
    47,  195, 16,  132, 195, 263, 195, 314, 315, 267, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 218,
    219, 218, 219, 151, 218, 219, 195, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 210, 211, 195, 7,
    8,   9,   195, 60,  195, 312, 218, 219, 195, 316, 195, 120, 195, 263, 19,  195, 125, 267, 78,  24,  80,  218, 219,
    116, 162, 218, 219, 218, 219, 301, 302, 218, 219, 195, 19,  218, 219, 276, 43,  44,  45,  160, 47,  48,  49,  50,
    51,  52,  53,  54,  55,  56,  57,  58,  19,  146, 218, 219, 43,  44,  45,  118, 47,  48,  49,  50,  51,  52,  53,
    54,  55,  56,  57,  58,  165, 314, 315, 276, 43,  44,  45,  266, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
    57,  58,  128, 129, 130, 195, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 228, 195, 61,  195,
    314, 315, 25,  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 22,  195, 218, 219, 218, 219, 195,
    103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 195, 246, 218, 219, 218, 219, 25,  19,  246, 218,
    219, 246, 257, 259, 260, 195, 22,  266, 60,  257, 195, 120, 257, 218, 219, 116, 195, 19,  195, 150, 151, 25,  44,
    45,  266, 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  195, 54,  218, 219, 218, 219, 45,  145, 47,
    48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  246, 121, 122, 218, 219, 19,  23,  31,  25,  118, 159, 257,
    161, 24,  195, 39,  195, 143, 195, 19,  20,  22,  22,  24,  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
    114, 36,  218, 219, 218, 219, 218, 219, 195, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 195, 143,
    119, 136, 60,  195, 22,  195, 141, 195, 218, 219, 195, 23,  195, 25,  72,  23,  131, 25,  195, 134, 23,  218, 219,
    195, 82,  144, 218, 219, 218, 219, 218, 219, 195, 218, 219, 218, 219, 60,  23,  195, 25,  218, 219, 101, 195, 117,
    218, 219, 195, 107, 108, 23,  195, 25,  195, 218, 219, 115, 228, 117, 118, 119, 218, 219, 122, 195, 19,  218, 219,
    195, 60,  218, 219, 142, 195, 218, 219, 19,  20,  195, 22,  139, 140, 23,  23,  25,  25,  195, 218, 219, 7,   8,
    218, 219, 36,  118, 154, 155, 156, 157, 158, 195, 23,  195, 25,  84,  85,  49,  195, 23,  195, 25,  195, 23,  195,
    25,  195, 23,  60,  25,  23,  23,  25,  25,  142, 183, 218, 219, 118, 195, 72,  218, 219, 218, 219, 218, 219, 218,
    219, 218, 219, 195, 195, 146, 86,  98,  23,  195, 25,  91,  19,  20,  154, 22,  156, 154, 23,  156, 25,  101, 23,
    195, 25,  195, 195, 107, 108, 36,  195, 195, 195, 195, 228, 115, 195, 117, 118, 119, 195, 195, 122, 261, 195, 321,
    195, 195, 195, 258, 238, 195, 195, 60,  299, 291, 195, 195, 258, 195, 195, 195, 290, 244, 216, 72,  245, 193, 258,
    258, 299, 258, 299, 274, 154, 155, 156, 157, 158, 86,  247, 295, 248, 295, 91,  19,  20,  270, 22,  274, 270, 248,
    274, 222, 101, 227, 274, 221, 231, 221, 107, 108, 36,  183, 262, 247, 221, 283, 115, 262, 117, 118, 119, 198, 116,
    122, 220, 262, 61,  220, 220, 251, 247, 142, 251, 245, 60,  202, 299, 202, 38,  262, 202, 22,  152, 151, 296, 43,
    72,  236, 18,  239, 202, 239, 239, 239, 18,  154, 155, 156, 157, 158, 86,  150, 201, 248, 275, 91,  248, 273, 236,
    248, 275, 275, 273, 236, 248, 101, 286, 202, 201, 159, 63,  107, 108, 296, 183, 293, 202, 201, 22,  115, 202, 117,
    118, 119, 292, 223, 122, 201, 65,  202, 201, 223, 220, 220, 22,  220, 226, 226, 229, 127, 223, 220, 166, 24,  285,
    220, 222, 114, 315, 285, 220, 202, 220, 307, 92,  320, 320, 229, 154, 155, 156, 157, 158, 0,   1,   2,   223, 83,
    5,   268, 149, 268, 146, 10,  11,  12,  13,  14,  22,  280, 17,  202, 159, 19,  20,  251, 22,  183, 282, 148, 252,
    252, 250, 30,  249, 32,  248, 147, 25,  13,  36,  204, 196, 40,  196, 6,   302, 194, 194, 194, 209, 215, 209, 215,
    215, 215, 224, 224, 216, 209, 4,   216, 215, 3,   60,  22,  122, 19,  122, 19,  125, 22,  15,  22,  71,  16,  72,
    23,  23,  140, 305, 152, 79,  25,  131, 82,  143, 20,  16,  305, 1,   143, 145, 131, 131, 62,  54,  131, 37,  54,
    54,  152, 99,  117, 34,  101, 54,  24,  1,   5,   22,  107, 108, 116, 76,  25,  162, 41,  142, 115, 24,  117, 118,
    119, 116, 20,  122, 19,  126, 23,  132, 19,  20,  69,  22,  69,  22,  134, 22,  68,  22,  22,  139, 140, 60,  141,
    68,  24,  36,  28,  97,  22,  37,  68,  23,  150, 34,  22,  154, 155, 156, 157, 158, 23,  23,  22,  163, 25,  23,
    142, 23,  98,  60,  23,  22,  144, 25,  76,  34,  117, 34,  89,  34,  34,  72,  87,  76,  183, 34,  94,  34,  23,
    22,  24,  34,  23,  25,  44,  25,  23,  23,  23,  22,  22,  25,  11,  143, 25,  143, 23,  22,  22,  22,  101, 23,
    23,  136, 22,  25,  107, 108, 142, 25,  142, 142, 23,  15,  115, 1,   117, 118, 119, 1,   2,   122, 1,   5,   322,
    322, 322, 322, 10,  11,  12,  13,  14,  322, 322, 17,  322, 5,   322, 322, 141, 322, 10,  11,  12,  13,  14,  322,
    30,  17,  32,  322, 322, 154, 155, 156, 157, 158, 40,  322, 322, 322, 30,  322, 32,  322, 322, 322, 322, 322, 322,
    322, 40,  322, 322, 322, 322, 322, 322, 322, 322, 322, 183, 322, 322, 322, 322, 322, 322, 71,  322, 322, 322, 322,
    322, 322, 322, 79,  322, 322, 82,  322, 322, 71,  322, 322, 322, 322, 322, 322, 322, 79,  322, 322, 82,  322, 322,
    99,  322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 99,  322, 322, 322, 322, 322, 322, 322, 322,
    322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 134, 322, 322, 322, 322, 139, 140, 322, 322, 322, 322,
    322, 322, 322, 134, 322, 322, 322, 322, 139, 140, 322, 322, 322, 322, 322, 322, 322, 322, 163, 322, 322, 322, 322,
    322, 322, 322, 322, 322, 322, 322, 322, 322, 163, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322,
    322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 322, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
    187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187, 187,
};

static const unsigned short int yy_shift_ofst[] = {
    2201, 1973, 2215, 1552, 1552, 33,   368,  1668, 1741, 1814, 726,  726,  726,  265,  33,   33,   33,   33,   33,
    0,    0,    216,  1349, 726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,
    272,  272,  111,  111,  316,  365,  516,  867,  867,  916,  916,  916,  916,  40,   112,  260,  364,  408,  512,
    617,  661,  765,  809,  913,  957,  1061, 1081, 1195, 1215, 1329, 1349, 1349, 1349, 1349, 1349, 1349, 1349, 1349,
    1349, 1349, 1349, 1349, 1349, 1349, 1349, 1349, 1349, 1349, 1369, 1349, 1473, 1493, 1493, 473,  1974, 2082, 726,
    726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,
    726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,
    726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  726,  138,  232,  232,  232,  232,  232,
    232,  232,  188,  99,   242,  718,  416,  1159, 867,  867,  940,  940,  867,  1103, 417,  574,  574,  574,  611,
    139,  139,  2379, 2379, 1026, 1026, 1026, 536,  466,  466,  466,  466,  1017, 1017, 849,  718,  971,  1060, 867,
    867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  261,
    712,  712,  867,  108,  1142, 1142, 977,  1108, 1108, 977,  977,  1243, 2379, 2379, 2379, 2379, 2379, 2379, 2379,
    641,  789,  789,  635,  366,  721,  673,  782,  494,  787,  829,  867,  867,  867,  867,  867,  867,  867,  867,
    867,  867,  867,  959,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  867,  820,
    820,  820,  867,  867,  867,  1136, 867,  867,  867,  1119, 1007, 867,  1169, 867,  867,  867,  867,  867,  867,
    867,  867,  1225, 1153, 869,  196,  618,  618,  618,  618,  1491, 196,  196,  91,   339,  1326, 1386, 383,  1163,
    1364, 1426, 1364, 1538, 903,  1163, 1163, 903,  1163, 1426, 1538, 1018, 1535, 1241, 1528, 1528, 1528, 1394, 1394,
    1394, 1394, 762,  762,  1403, 1466, 1475, 1551, 1746, 1805, 1746, 1746, 1729, 1729, 1840, 1840, 1729, 1730, 1732,
    1859, 1842, 1870, 1870, 1870, 1870, 1729, 1876, 1751, 1732, 1732, 1751, 1859, 1842, 1751, 1842, 1751, 1729, 1876,
    1760, 1857, 1729, 1876, 1906, 1729, 1876, 1729, 1876, 1906, 1746, 1746, 1746, 1873, 1922, 1922, 1906, 1746, 1822,
    1746, 1873, 1746, 1746, 1786, 1929, 1843, 1843, 1906, 1729, 1872, 1872, 1894, 1894, 1831, 1836, 1966, 1729, 1833,
    1831, 1851, 1860, 1751, 1983, 1996, 1996, 2009, 2009, 2009, 2379, 2379, 2379, 2379, 2379, 2379, 2379, 2379, 2379,
    2379, 2379, 2379, 2379, 2379, 2379, 136,  1063, 1196, 530,  636,  1274, 1300, 1443, 1598, 1495, 1479, 967,  1083,
    1602, 463,  1625, 1638, 1670, 1541, 1671, 1689, 1696, 1277, 1432, 1693, 808,  1700, 1607, 1657, 1587, 1704, 1707,
    1631, 1708, 1733, 1608, 1611, 1743, 1747, 1620, 1592, 2026, 2030, 2013, 1914, 2018, 1916, 2020, 2019, 2021, 1915,
    2027, 2029, 2024, 2025, 1909, 1899, 1923, 2028, 2028, 1913, 2037, 1917, 2042, 2059, 1918, 1932, 2028, 1933, 2003,
    2031, 2028, 1919, 2012, 2015, 2016, 2022, 1936, 1956, 2040, 2053, 2077, 2074, 2058, 1967, 1924, 2034, 2060, 2036,
    2008, 2046, 1946, 1978, 2066, 2075, 2078, 1968, 1972, 2084, 2041, 2086, 2088, 2076, 2089, 2048, 2054, 2093, 2023,
    2091, 2099, 2055, 2085, 2101, 2092, 1975, 2105, 2110, 2111, 2112, 2115, 2113, 2043, 1997, 2117, 2120, 2032, 2114,
    2122, 2001, 2121, 2116, 2118, 2119, 2124, 2062, 2071, 2068, 2123, 2080, 2065, 2126, 2138, 2140, 2139, 2141, 2143,
    2130, 2033, 2035, 2142, 2121, 2146, 2147, 2148, 2150, 2149, 2152, 2156, 2151, 2164, 2158, 2159, 2161, 2162, 2160,
    2165, 2163, 2050, 2049, 2051, 2052, 2167, 2172, 2181, 2197, 2204,
};

static const unsigned short int yy_default[] = {
    1691, 1691, 1691, 1516, 1279, 1392, 1279, 1279, 1279, 1279, 1516, 1516, 1516, 1279, 1279, 1279, 1279, 1279, 1279,
    1422, 1422, 1568, 1312, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1515, 1279, 1279,
    1279, 1279, 1607, 1607, 1279, 1279, 1279, 1279, 1279, 1592, 1591, 1279, 1279, 1279, 1431, 1279, 1279, 1279, 1438,
    1279, 1279, 1279, 1279, 1279, 1517, 1518, 1279, 1279, 1279, 1279, 1567, 1569, 1533, 1445, 1444, 1443, 1442, 1551,
    1410, 1436, 1429, 1433, 1512, 1513, 1511, 1670, 1518, 1517, 1279, 1432, 1480, 1496, 1479, 1279, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1488, 1495, 1494, 1493, 1502, 1492,
    1489, 1482, 1481, 1483, 1484, 1303, 1300, 1354, 1279, 1279, 1279, 1279, 1279, 1485, 1312, 1473, 1472, 1471, 1279,
    1499, 1486, 1498, 1497, 1575, 1644, 1643, 1534, 1279, 1279, 1279, 1279, 1279, 1279, 1607, 1279, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1412,
    1607, 1607, 1279, 1312, 1607, 1607, 1308, 1413, 1413, 1308, 1308, 1416, 1587, 1383, 1383, 1383, 1383, 1392, 1383,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1572, 1570, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1388, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1637, 1683, 1279, 1546, 1368, 1388, 1388, 1388, 1388, 1390, 1369, 1367, 1382, 1313, 1286, 1683, 1683, 1448,
    1437, 1389, 1437, 1680, 1435, 1448, 1448, 1435, 1448, 1389, 1680, 1329, 1659, 1324, 1422, 1422, 1422, 1412, 1412,
    1412, 1412, 1416, 1416, 1514, 1389, 1382, 1279, 1355, 1683, 1355, 1355, 1398, 1398, 1682, 1682, 1398, 1534, 1667,
    1457, 1357, 1363, 1363, 1363, 1363, 1398, 1297, 1435, 1667, 1667, 1435, 1457, 1357, 1435, 1357, 1435, 1398, 1297,
    1550, 1678, 1398, 1297, 1524, 1398, 1297, 1398, 1297, 1524, 1355, 1355, 1355, 1344, 1279, 1279, 1524, 1355, 1329,
    1355, 1344, 1355, 1355, 1625, 1279, 1528, 1528, 1524, 1398, 1617, 1617, 1425, 1425, 1430, 1416, 1519, 1398, 1279,
    1430, 1428, 1426, 1435, 1347, 1640, 1640, 1636, 1636, 1636, 1688, 1688, 1587, 1652, 1312, 1312, 1312, 1312, 1652,
    1331, 1331, 1313, 1313, 1312, 1652, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1647, 1279, 1279, 1535, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1402, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1593, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1462, 1279, 1282, 1584, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1439, 1440, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1454, 1279, 1279,
    1279, 1449, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1403, 1279, 1279, 1279, 1279, 1279, 1279, 1549, 1548,
    1279, 1279, 1400, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1327, 1279, 1279,
    1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1279, 1427, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1622, 1417,
    1279, 1279, 1279, 1279, 1671, 1279, 1279, 1279, 1279, 1377, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279, 1279,
    1279, 1663, 1371, 1463, 1279, 1466, 1301, 1279, 1291, 1279, 1279,
};

static const unsigned short int yyFallback[] = {
    0,  0,  60, 60, 60, 60, 0,  60, 60, 60, 0,  60, 60, 60, 60, 0,  0,  0,  60, 0,  0,  60, 0,  0,  0,  0,  60,
    60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 0,  0,  0,  0,  60, 60, 0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
    60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

static unsigned short int yy_find_shift_action(unsigned short int iLookAhead, unsigned short int stateno) {
  int i;

  if (stateno > 599)
    return stateno;

  do {
    i = yy_shift_ofst[stateno];

    i += iLookAhead;

    if (yy_lookahead[i] != iLookAhead) {
      unsigned short int iFallback;

      iFallback = yyFallback[iLookAhead];
      if (iFallback != 0) {
        iLookAhead = iFallback;
        continue;
      }

      {
        int j = i - iLookAhead + 102;

        if (yy_lookahead[j] == 102 && iLookAhead > 0) {
          return yy_action[j];
        }
      }

      return yy_default[stateno];
    } else {
      return yy_action[i];
    }
  } while (1);
}

static const unsigned char aiClass[] = {

    29, 28, 28, 28, 28, 28, 28, 28, 28, 7,  7,  28, 7,  7,  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 28, 28, 7,  15, 8,  5,  4,  22, 24, 8,  17, 18, 21, 20, 23, 11, 26, 16, 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    5,  19, 12, 14, 13, 6,  5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  0,  2,  2,  9,  28, 28, 28, 2,  8,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  0,  2,  2,  28, 10, 28, 25, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 30, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27};

static const char zKWText[666] = {
    'R', 'E', 'I', 'N', 'D', 'E', 'X', 'E', 'D', 'E', 'S', 'C', 'A', 'P', 'E', 'A', 'C', 'H', 'E', 'C', 'K', 'E', 'Y',
    'B', 'E', 'F', 'O', 'R', 'E', 'I', 'G', 'N', 'O', 'R', 'E', 'G', 'E', 'X', 'P', 'L', 'A', 'I', 'N', 'S', 'T', 'E',
    'A', 'D', 'D', 'A', 'T', 'A', 'B', 'A', 'S', 'E', 'L', 'E', 'C', 'T', 'A', 'B', 'L', 'E', 'F', 'T', 'H', 'E', 'N',
    'D', 'E', 'F', 'E', 'R', 'R', 'A', 'B', 'L', 'E', 'L', 'S', 'E', 'X', 'C', 'L', 'U', 'D', 'E', 'L', 'E', 'T', 'E',
    'M', 'P', 'O', 'R', 'A', 'R', 'Y', 'I', 'S', 'N', 'U', 'L', 'L', 'S', 'A', 'V', 'E', 'P', 'O', 'I', 'N', 'T', 'E',
    'R', 'S', 'E', 'C', 'T', 'I', 'E', 'S', 'N', 'O', 'T', 'N', 'U', 'L', 'L', 'I', 'K', 'E', 'X', 'C', 'E', 'P', 'T',
    'R', 'A', 'N', 'S', 'A', 'C', 'T', 'I', 'O', 'N', 'A', 'T', 'U', 'R', 'A', 'L', 'T', 'E', 'R', 'A', 'I', 'S', 'E',
    'X', 'C', 'L', 'U', 'S', 'I', 'V', 'E', 'X', 'I', 'S', 'T', 'S', 'C', 'O', 'N', 'S', 'T', 'R', 'A', 'I', 'N', 'T',
    'O', 'F', 'F', 'S', 'E', 'T', 'R', 'I', 'G', 'G', 'E', 'R', 'A', 'N', 'G', 'E', 'N', 'E', 'R', 'A', 'T', 'E', 'D',
    'E', 'T', 'A', 'C', 'H', 'A', 'V', 'I', 'N', 'G', 'L', 'O', 'B', 'E', 'G', 'I', 'N', 'N', 'E', 'R', 'E', 'F', 'E',
    'R', 'E', 'N', 'C', 'E', 'S', 'U', 'N', 'I', 'Q', 'U', 'E', 'R', 'Y', 'W', 'I', 'T', 'H', 'O', 'U', 'T', 'E', 'R',
    'E', 'L', 'E', 'A', 'S', 'E', 'A', 'T', 'T', 'A', 'C', 'H', 'B', 'E', 'T', 'W', 'E', 'E', 'N', 'O', 'T', 'H', 'I',
    'N', 'G', 'R', 'O', 'U', 'P', 'S', 'C', 'A', 'S', 'C', 'A', 'D', 'E', 'F', 'A', 'U', 'L', 'T', 'C', 'A', 'S', 'E',
    'C', 'O', 'L', 'L', 'A', 'T', 'E', 'C', 'R', 'E', 'A', 'T', 'E', 'C', 'U', 'R', 'R', 'E', 'N', 'T', '_', 'D', 'A',
    'T', 'E', 'I', 'M', 'M', 'E', 'D', 'I', 'A', 'T', 'E', 'J', 'O', 'I', 'N', 'S', 'E', 'R', 'T', 'M', 'A', 'T', 'C',
    'H', 'P', 'L', 'A', 'N', 'A', 'L', 'Y', 'Z', 'E', 'P', 'R', 'A', 'G', 'M', 'A', 'T', 'E', 'R', 'I', 'A', 'L', 'I',
    'Z', 'E', 'D', 'E', 'F', 'E', 'R', 'R', 'E', 'D', 'I', 'S', 'T', 'I', 'N', 'C', 'T', 'U', 'P', 'D', 'A', 'T', 'E',
    'V', 'A', 'L', 'U', 'E', 'S', 'V', 'I', 'R', 'T', 'U', 'A', 'L', 'W', 'A', 'Y', 'S', 'W', 'H', 'E', 'N', 'W', 'H',
    'E', 'R', 'E', 'C', 'U', 'R', 'S', 'I', 'V', 'E', 'A', 'B', 'O', 'R', 'T', 'A', 'F', 'T', 'E', 'R', 'E', 'N', 'A',
    'M', 'E', 'A', 'N', 'D', 'R', 'O', 'P', 'A', 'R', 'T', 'I', 'T', 'I', 'O', 'N', 'A', 'U', 'T', 'O', 'I', 'N', 'C',
    'R', 'E', 'M', 'E', 'N', 'T', 'C', 'A', 'S', 'T', 'C', 'O', 'L', 'U', 'M', 'N', 'C', 'O', 'M', 'M', 'I', 'T', 'C',
    'O', 'N', 'F', 'L', 'I', 'C', 'T', 'C', 'R', 'O', 'S', 'S', 'C', 'U', 'R', 'R', 'E', 'N', 'T', '_', 'T', 'I', 'M',
    'E', 'S', 'T', 'A', 'M', 'P', 'R', 'E', 'C', 'E', 'D', 'I', 'N', 'G', 'F', 'A', 'I', 'L', 'A', 'S', 'T', 'F', 'I',
    'L', 'T', 'E', 'R', 'E', 'P', 'L', 'A', 'C', 'E', 'F', 'I', 'R', 'S', 'T', 'F', 'O', 'L', 'L', 'O', 'W', 'I', 'N',
    'G', 'F', 'R', 'O', 'M', 'F', 'U', 'L', 'L', 'I', 'M', 'I', 'T', 'I', 'F', 'O', 'R', 'D', 'E', 'R', 'E', 'S', 'T',
    'R', 'I', 'C', 'T', 'O', 'T', 'H', 'E', 'R', 'S', 'O', 'V', 'E', 'R', 'E', 'T', 'U', 'R', 'N', 'I', 'N', 'G', 'R',
    'I', 'G', 'H', 'T', 'R', 'O', 'L', 'L', 'B', 'A', 'C', 'K', 'R', 'O', 'W', 'S', 'U', 'N', 'B', 'O', 'U', 'N', 'D',
    'E', 'D', 'U', 'N', 'I', 'O', 'N', 'U', 'S', 'I', 'N', 'G', 'V', 'A', 'C', 'U', 'U', 'M', 'V', 'I', 'E', 'W', 'I',
    'N', 'D', 'O', 'W', 'B', 'Y', 'I', 'N', 'I', 'T', 'I', 'A', 'L', 'L', 'Y', 'P', 'R', 'I', 'M', 'A', 'R', 'Y',
};

static const unsigned char aKWHash[127] = {
    84,  92,  134, 82,  105, 29,  0,   0,   94, 0,   85,  72, 0,   53,  35, 86,  15, 0,  42,  97,  54,  89,
    135, 19,  0,   0,   140, 0,   40,  129, 0,  22,  107, 0,  9,   0,   0,  123, 80, 0,  78,  6,   0,   65,
    103, 147, 0,   136, 115, 0,   0,   48,  0,  90,  24,  0,  17,  0,   27, 70,  23, 26, 5,   60,  142, 110,
    122, 0,   73,  91,  71,  145, 61,  120, 74, 0,   49,  0,  11,  41,  0,  113, 0,  0,  0,   109, 10,  111,
    116, 125, 14,  50,  124, 0,   100, 0,   18, 121, 144, 56, 130, 139, 88, 83,  37, 30, 126, 0,   0,   108,
    51,  131, 128, 0,   34,  0,   0,   132, 0,  98,  38,  39, 0,   20,  45, 117, 93,
};

static const unsigned char aKWNext[148] = {
    0,   0,  0,   0,  0,  4,   0,  43, 0,  0,   106, 114, 0, 0,  0,   2,   0,   0,   143, 0,   0,  0,  13, 0,   0,
    0,   0,  141, 0,  0,  119, 52, 0,  0,  137, 12,  0,   0, 62, 0,   138, 0,   133, 0,   0,   36, 0,  0,  28,  77,
    0,   0,  0,   0,  59, 0,   47, 0,  0,  0,   0,   0,   0, 0,  0,   0,   0,   69,  0,   0,   0,  0,  0,  146, 3,
    0,   58, 0,   1,  75, 0,   0,  0,  31, 0,   0,   0,   0, 0,  127, 0,   104, 0,   64,  66,  63, 0,  0,  0,   0,
    0,   46, 0,   16, 8,  0,   0,  0,  0,  0,   0,   0,   0, 0,  0,   81,  101, 0,   112, 21,  7,  67, 0,  79,  96,
    118, 0,  0,   68, 0,  0,   99, 44, 0,  55,  0,   76,  0, 95, 32,  33,  57,  25,  0,   102, 0,  0,  87,
};

static const unsigned char aKWLen[148] = {
    0, 7, 7, 5,  4, 6, 4, 5, 3, 6, 7, 3, 6,  6, 7, 7, 3,  8, 2, 6, 5,  4,  4, 3, 10, 4,  7, 6, 9, 4,
    2, 6, 5, 9,  9, 4, 7, 3, 2, 4, 4, 6, 11, 6, 2, 7, 5,  5, 9, 6, 10, 4,  6, 2, 3,  7,  5, 9, 6, 6,
    4, 5, 5, 10, 6, 5, 7, 4, 5, 7, 6, 7, 7,  6, 5, 7, 3,  7, 4, 7, 6,  12, 9, 4, 6,  5,  4, 7, 6, 12,
    8, 8, 2, 6,  6, 7, 6, 4, 5, 9, 5, 5, 6,  3, 4, 9, 13, 2, 2, 4, 6,  6,  8, 5, 17, 12, 7, 9, 4, 4,
    6, 7, 5, 9,  4, 4, 5, 2, 5, 8, 6, 4, 9,  5, 8, 4, 3,  9, 5, 5, 6,  4,  6, 2, 2,  9,  3, 7,
};

static const unsigned short int aKWOffset[148] = {
    0,   0,   2,   2,   8,   9,   14,  16,  20,  23,  25,  25,  29,  33,  36,  41,  46,  48,  53,  54,  59,  62,
    65,  67,  69,  78,  81,  86,  90,  90,  94,  99,  101, 105, 111, 119, 123, 123, 123, 126, 129, 132, 137, 142,
    146, 147, 152, 156, 160, 168, 174, 181, 184, 184, 187, 189, 195, 198, 206, 211, 216, 219, 222, 226, 236, 239,
    244, 244, 248, 252, 259, 265, 271, 277, 277, 283, 284, 288, 295, 299, 306, 312, 324, 333, 335, 341, 346, 348,
    355, 359, 370, 377, 378, 385, 391, 397, 402, 408, 412, 415, 424, 429, 433, 439, 441, 444, 453, 455, 457, 466,
    470, 476, 482, 490, 495, 495, 495, 511, 520, 523, 527, 532, 539, 544, 553, 557, 560, 565, 567, 571, 579, 585,
    588, 597, 602, 610, 610, 614, 623, 628, 633, 639, 642, 645, 648, 650, 655, 659,
};

static const unsigned char aKWCode[148] = {
    0,   99,  117, 162, 39,  59,  41,  125, 68,  33,  133, 63,  64,  48,  2,   66,  164, 38,  24,  139, 16,  119,
    160, 11,  132, 161, 92,  129, 21,  21,  43,  51,  83,  13,  138, 95,  52,  19,  67,  122, 48,  137, 6,   28,
    116, 119, 163, 72,  9,   20,  120, 152, 70,  69,  131, 78,  90,  96,  40,  148, 48,  5,   119, 126, 124, 3,
    26,  82,  119, 14,  32,  49,  153, 93,  147, 35,  31,  121, 158, 114, 17,  101, 8,   144, 128, 47,  4,   30,
    71,  98,  7,   141, 45,  130, 140, 81,  97,  159, 150, 73,  27,  29,  100, 44,  134, 88,  127, 15,  50,  36,
    61,  10,  37,  119, 101, 101, 86,  89,  42,  85,  167, 74,  84,  87,  143, 119, 149, 18,  146, 75,  94,  166,
    151, 119, 12,  77,  76,  91,  135, 145, 79,  80,  165, 62,  34,  65,  136, 123,
};

static i64 keywordCode(const char *z, i64 n, int *pType) {
  i64 i, j;
  const char *zKW;

  i = ((sqlite3UpperToLower[(unsigned char)z[0]] * 4) ^ (sqlite3UpperToLower[(unsigned char)z[n - 1]] * 3) ^ n * 1) %
      127;
  for (i = (int)aKWHash[i]; i > 0; i = aKWNext[i]) {
    if (aKWLen[i] != n)
      continue;
    zKW = &zKWText[aKWOffset[i]];

    if ((z[0] & ~0x20) != zKW[0])
      continue;
    if ((z[1] & ~0x20) != zKW[1])
      continue;
    j = 2;
    while (j < n && (z[j] & ~0x20) == zKW[j]) {
      j++;
    }

    if (j < n)
      continue;
    *pType = aKWCode[i];
    break;
  }
  return n;
}

static int rtrimCollFunc(void *pUser, int nKey1, const void *pKey1, int nKey2, const void *pKey2) {
  const u8 *pK1 = (const u8 *)pKey1;
  const u8 *pK2 = (const u8 *)pKey2;
  while (nKey1 && pK1[nKey1 - 1] == ' ')
    nKey1--;
  while (nKey2 && pK2[nKey2 - 1] == ' ')
    nKey2--;
  return binCollFunc(pUser, nKey1, pKey1, nKey2, pKey2);
}

static int nocaseCollatingFunc(void *NotUsed, int nKey1, const void *pKey1, int nKey2, const void *pKey2) {
  int r = sqlite3_strnicmp((const char *)pKey1, (const char *)pKey2, (nKey1 < nKey2) ? nKey1 : nKey2);
  (void)(NotUsed);
  if (0 == r) {
    r = nKey1 - nKey2;
  }
  return r;
}

static int sqliteDefaultBusyCallback(void *ptr, int count) {
  static const u8 delays[] = {1, 2, 5, 10, 15, 20, 25, 25, 25, 50, 50, 100};
  static const u8 totals[] = {0, 1, 3, 8, 18, 33, 53, 78, 103, 128, 178, 228};

  sqlite3 *db = (sqlite3 *)ptr;
  int tmout = db->busyTimeout;
  int delay, prior;

  if (count < ((int)(sizeof(delays) / sizeof(delays[0])))) {
    delay = delays[count];
    prior = totals[count];
  } else {
    delay = delays[((int)(sizeof(delays) / sizeof(delays[0]))) - 1];
    prior = totals[((int)(sizeof(delays) / sizeof(delays[0]))) - 1] +
            delay * (count - (((int)(sizeof(delays) / sizeof(delays[0]))) - 1));
  }
  if (prior + delay > tmout) {
    delay = tmout - prior;
    if (delay <= 0)
      return 0;
  }
  sqlite3OsSleep(db->pVfs, delay * 1000);
  return 1;
}

static const int aHardLimit[] = {
    1000000000, 1000000000, 2000, 1000, 500, 250000000, 1000, 10, 50000, 32766, 1000, 8, 2500,
};

static const char *uriParameter(const char *zFilename, const char *zParam) {
  zFilename += sqlite3Strlen30(zFilename) + 1;
  while ((zFilename != 0) && zFilename[0]) {
    int x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    if (x == 0)
      return zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  return 0;
}

static int openDatabase(const char *zFilename, sqlite3 **ppDb, unsigned int flags, const char *zVfs) {
  sqlite3 *db;
  int rc;
  int isThreadsafe;
  char *zOpen = 0;
  char *zErrMsg = 0;
  int i;

  *ppDb = 0;

  rc = sqlite3_initialize();
  if (rc)
    return rc;

  if (sqlite3Config.bCoreMutex == 0) {
    isThreadsafe = 0;
  } else if (flags & SQLITE_OPEN_NOMUTEX) {
    isThreadsafe = 0;
  } else if (flags & SQLITE_OPEN_FULLMUTEX) {
    isThreadsafe = 1;
  } else {
    isThreadsafe = sqlite3Config.bFullMutex;
  }

  if (flags & SQLITE_OPEN_PRIVATECACHE) {
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  } else if (sqlite3Config.sharedCacheEnabled) {
    flags |= SQLITE_OPEN_SHAREDCACHE;
  }

  flags &= ~(SQLITE_OPEN_DELETEONCLOSE | SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_MAIN_DB | SQLITE_OPEN_TEMP_DB |
             SQLITE_OPEN_TRANSIENT_DB | SQLITE_OPEN_MAIN_JOURNAL | SQLITE_OPEN_TEMP_JOURNAL | SQLITE_OPEN_SUBJOURNAL |
             SQLITE_OPEN_SUPER_JOURNAL | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_WAL);

  db = sqlite3MallocZero(sizeof(sqlite3));
  if (db == 0)
    goto opendb_out;
  if (isThreadsafe) {
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    if (db->mutex == 0) {
      sqlite3_free(db);
      db = 0;
      goto opendb_out;
    }
    if (isThreadsafe == 0) {
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->errMask = (flags & SQLITE_OPEN_EXRESCODE) != 0 ? 0xffffffff : 0xff;
  db->nDb = 2;
  db->eOpenState = 0x6d;
  db->aDb = db->aDbStatic;
  db->lookaside.bDisable = 1;
  db->lookaside.sz = 0;
  db->nFpDigit = 17;

  memcpy(db->aLimit, aHardLimit, sizeof(db->aLimit));
  db->aLimit[SQLITE_LIMIT_WORKER_THREADS] = 0;
  db->autoCommit = 1;
  db->nextAutovac = -1;
  db->szMmap = sqlite3Config.szMmap;
  db->nextPagesize = 0;
  db->init.azInit = sqlite3StdType;

  db->flags |= 0x00000040 | 0x00040000 | 0x80000000 | 0x00000020 | ((u64)(0x00010) << 32) | ((u64)(0x00020) << 32) |
               ((u64)(0x00040) << 32) | 0x00000080 | 0x40000000 | 0x20000000 | 0x00008000;
  sqlite3HashInit(&db->aCollSeq);

  sqlite3HashInit(&db->aModule);

  createCollation(db, sqlite3StrBINARY, SQLITE_UTF8, 0, binCollFunc, 0);
  createCollation(db, sqlite3StrBINARY, SQLITE_UTF16BE, 0, binCollFunc, 0);
  createCollation(db, sqlite3StrBINARY, SQLITE_UTF16LE, 0, binCollFunc, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, 0, rtrimCollFunc, 0);
  if (db->mallocFailed) {
    goto opendb_out;
  }

  db->openFlags = flags;

  if (((1 << (flags & 7)) & 0x46) == 0) {
    rc = sqlite3MisuseError(190964);
  } else {
    if (zFilename == 0)
      zFilename = ":memory:";
    rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &zErrMsg);
  }
  if (rc != SQLITE_OK) {
    if (rc == SQLITE_NOMEM)
      sqlite3OomFault(db);
    sqlite3ErrorWithMsg(db, rc, zErrMsg ? "%s" : 0, zErrMsg);
    sqlite3_free(zErrMsg);
    goto opendb_out;
  }

  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0, flags | SQLITE_OPEN_MAIN_DB);
  if (rc != SQLITE_OK) {
    if (rc == (10 | (12 << 8))) {
      rc = 7;
    }
    sqlite3Error(db, rc);
    goto opendb_out;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  if (!db->mallocFailed) {
    sqlite3SetTextEncoding(db, ((db)->aDb[0].pSchema->enc));
  }
  sqlite3BtreeLeave(db->aDb[0].pBt);
  db->aDb[1].pSchema = sqlite3SchemaGet(db, 0);

  db->aDb[0].zDbSName = "main";
  db->aDb[0].safety_level = 2 + 1;
  db->aDb[1].zDbSName = "temp";
  db->aDb[1].safety_level = 0x01;

  db->eOpenState = 0x76;
  if (db->mallocFailed) {
    goto opendb_out;
  }

  sqlite3Error(db, SQLITE_OK);
  sqlite3RegisterPerConnectionBuiltinFunctions(db);
  rc = sqlite3_errcode(db);

  for (i = 0; rc == SQLITE_OK && i < ((int)(sizeof(sqlite3BuiltinExtensions) / sizeof(sqlite3BuiltinExtensions[0])));
       i++) {
    rc = sqlite3BuiltinExtensions[i](db);
  }

  if (rc == SQLITE_OK) {
    sqlite3AutoLoadExtensions(db);
    rc = sqlite3_errcode(db);
    if (rc != SQLITE_OK) {
      goto opendb_out;
    }
  }

  if (rc)
    sqlite3Error(db, rc);

  setupLookaside(db, 0, sqlite3Config.szLookaside, sqlite3Config.nLookaside);

  sqlite3_wal_autocheckpoint(db, 1000);

opendb_out:
  if (db) {
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);

  if ((rc & 0xff) == SQLITE_NOMEM) {
    sqlite3_close(db);
    db = 0;
  } else if (rc != SQLITE_OK) {
    db->eOpenState = 0xba;
  }
  *ppDb = db;

  sqlite3_free_filename(zOpen);
  return rc;
}

static const char *databaseName(const char *zName) {
  while (zName[-1] != 0 || zName[-2] != 0 || zName[-3] != 0 || zName[-4] != 0) {
    zName--;
  }
  return zName;
}

static char *appendText(char *p, const char *z) {
  size_t n = strlen(z);
  memcpy(p, z, n + 1);
  return p + n + 1;
}

const sqlite3_io_methods *posixIoFinderImpl(const char *z, unixFile *p);
extern const sqlite3_io_methods *(*const posixIoFinder)(const char *, unixFile *);
const sqlite3_io_methods *nolockIoFinderImpl(const char *z, unixFile *p);
static const sqlite3_io_methods *(*const nolockIoFinder)(const char *, unixFile *);
const sqlite3_io_methods *dotlockIoFinderImpl(const char *z, unixFile *p);
static const sqlite3_io_methods *(*const dotlockIoFinder)(const char *, unixFile *);

extern const sqlite3_io_methods *(*const posixIoFinder)(const char *, unixFile *);
const sqlite3_io_methods *(*const posixIoFinder)(const char *, unixFile *p) = posixIoFinderImpl;

const sqlite3_io_methods *posixIoFinderImpl(const char *z, unixFile *p) {
  (void)(z);
  (void)(p);
  return &posixIoMethods;
}

const sqlite3_io_methods *nolockIoFinderImpl(const char *z, unixFile *p) {
  (void)(z);
  (void)(p);
  return &nolockIoMethods;
}

static const sqlite3_io_methods *(*const nolockIoFinder)(const char *, unixFile *p) = nolockIoFinderImpl;

const sqlite3_io_methods *dotlockIoFinderImpl(const char *z, unixFile *p) {
  (void)(z);
  (void)(p);
  return &dotlockIoMethods;
}

static const sqlite3_io_methods *(*const dotlockIoFinder)(const char *, unixFile *p) = dotlockIoFinderImpl;

const char *azTempDirs[] = {0, 0, "/var/tmp", "/usr/tmp", "/tmp", "."};

static void unixTempFileInit(void) {
  azTempDirs[0] = getenv("SQLITE_TMPDIR");
  azTempDirs[1] = getenv("TMPDIR");
}

const char **sqlite3CompileOptions(int *pnOpt) {
  *pnOpt = sizeof(sqlite3azCompileOpt) / sizeof(sqlite3azCompileOpt[0]);
  return (const char **)sqlite3azCompileOpt;
}

// TODO: Inspect sqlite code and bring all macros as enumerations
// TODO: Inspect sqlite3 code and replace appropriate macros
const unsigned char sqlite3UpperToLower[] = {

    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
    46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  97,  98,  99,  100,
    101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91,
    92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
    138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
    161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206,
    207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
    230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
    253, 254, 255, 1,   0,   0,   1,   1,   0,   0,   1,   0,   1,   0,   1,   1,   0,   1,   0,   0,   1};

const unsigned char sqlite3CtypeMap[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x40, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
    0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x80, 0x00, 0x00, 0x00,
    0x40, 0x80, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};

int sqlite3PendingByte = 0x40000000;

u32 sqlite3TreeTrace = 0;

u32 sqlite3WhereTrace = 0;

const unsigned char sqlite3OpcodeProperty[] = {
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x41, 0x00, 0x81, 0x01, 0x01, 0x81, 0x83, 0x83, 0x01, 0x01, 0x03, 0x03,
    0x01, 0x12, 0x01, 0xc9, 0xc9, 0xc9, 0xc9, 0x01, 0x49, 0x49, 0x49, 0x49, 0xc9, 0x49, 0xc1, 0x01, 0x41, 0x41,
    0xc1, 0x01, 0x01, 0x41, 0x41, 0x41, 0x41, 0x26, 0x26, 0x41, 0x41, 0x09, 0x23, 0x0b, 0x81, 0x03, 0x03, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x01, 0x01, 0x03, 0x03, 0x03, 0x01, 0x41, 0x01, 0x00, 0x00, 0x02, 0x02, 0x08,
    0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x02, 0x02,
    0x02, 0x00, 0x00, 0x12, 0x1e, 0x20, 0x40, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x26, 0x26, 0x26, 0x26, 0x26,
    0x26, 0x26, 0x26, 0x26, 0x26, 0x40, 0x40, 0x12, 0x00, 0x40, 0x10, 0x40, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00,
    0x40, 0x40, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x50, 0x00, 0x40, 0x04, 0x04, 0x00, 0x40,
    0x50, 0x40, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x06, 0x10, 0x00, 0x04,
    0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x10, 0x50, 0x40, 0x00,
    0x10, 0x10, 0x02, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const char sqlite3StrBINARY[] = "BINARY";

const unsigned char sqlite3StdTypeLen[] = {3, 4, 3, 7, 4, 4};

const char sqlite3StdTypeAffinity[] = {0x43, 0x41, 0x44, 0x44, 0x45, 0x42};

const char *sqlite3StdType[] = {"ANY", "BLOB", "INT", "INTEGER", "REAL", "TEXT"};

static sqlite3_int64 sqlite3StatusValue(int op) {
  return sqlite3Stat.nowValue[op];
}

void sqlite3StatusUp(int op, int N) {
  sqlite3Stat.nowValue[op] += N;
  if (sqlite3Stat.nowValue[op] > sqlite3Stat.mxValue[op]) {
    sqlite3Stat.mxValue[op] = sqlite3Stat.nowValue[op];
  }
}

void sqlite3StatusDown(int op, int N) {
  sqlite3Stat.nowValue[op] -= N;
}

void sqlite3StatusHighwater(int op, int X) {
  sqlite3StatValueType newValue;

  newValue = (sqlite3StatValueType)X;

  if (newValue > sqlite3Stat.mxValue[op]) {
    sqlite3Stat.mxValue[op] = newValue;
  }
}

int sqlite3_status64(int op, sqlite3_int64 *pCurrent, sqlite3_int64 *pHighwater, int resetFlag) {
  sqlite3_mutex *pMutex;
  if (op < 0 || op >= ((int)(sizeof(sqlite3Stat.nowValue) / sizeof(sqlite3Stat.nowValue[0])))) {
    return sqlite3MisuseError(25154);
  }

  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = sqlite3Stat.nowValue[op];
  *pHighwater = sqlite3Stat.mxValue[op];
  if (resetFlag) {
    sqlite3Stat.mxValue[op] = sqlite3Stat.nowValue[op];
  }
  sqlite3_mutex_leave(pMutex);
  (void)pMutex;
  return SQLITE_OK;
}

int sqlite3_status(int op, int *pCurrent, int *pHighwater, int resetFlag) {
  sqlite3_int64 iCur = 0, iHwtr = 0;
  int rc;

  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  if (rc == 0) {
    *pCurrent = (int)iCur;
    *pHighwater = (int)iHwtr;
  }
  return rc;
}

int sqlite3LookasideUsed(sqlite3 *db, int *pHighwater) {
  u32 nInit = countLookasideSlots(db->lookaside.pInit);
  u32 nFree = countLookasideSlots(db->lookaside.pFree);

  nInit += countLookasideSlots(db->lookaside.pSmallInit);
  nFree += countLookasideSlots(db->lookaside.pSmallFree);

  if (pHighwater)
    *pHighwater = (int)(db->lookaside.nSlot - nInit);
  return (int)(db->lookaside.nSlot - (nInit + nFree));
}

int sqlite3_db_status64(sqlite3 *db, int op, sqlite3_int64 *pCurrent, sqlite3_int64 *pHighwtr, int resetFlag) {
  int rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  switch (op) {
    case SQLITE_DBSTATUS_LOOKASIDE_USED: {
      int H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      if (resetFlag) {
        LookasideSlot *p = db->lookaside.pFree;
        if (p) {
          while (p->pNext)
            p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }

        p = db->lookaside.pSmallFree;
        if (p) {
          while (p->pNext)
            p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
      }
      break;
    }

    case SQLITE_DBSTATUS_LOOKASIDE_HIT:
    case SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    case SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT];
      if (resetFlag) {
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      break;
    }

    case SQLITE_DBSTATUS_CACHE_USED_SHARED:
    case SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      int i;
      sqlite3BtreeEnterAll(db);
      for (i = 0; i < db->nDb; i++) {
        Btree *pBt = db->aDb[i].pBt;
        if (pBt) {
          Pager *pPager = sqlite3BtreePager(pBt);
          int nByte = sqlite3PagerMemUsed(pPager);
          if (op == SQLITE_DBSTATUS_CACHE_USED_SHARED) {
            nByte = nByte / sqlite3BtreeConnectionCount(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      break;
    }

    case SQLITE_DBSTATUS_SCHEMA_USED: {
      int i;
      int nByte = 0;

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;

      db->lookaside.pEnd = db->lookaside.pStart;
      for (i = 0; i < db->nDb; i++) {
        Schema *pSchema = db->aDb[i].pSchema;
        if ((pSchema != 0)) {
          HashElem *p;

          nByte += sqlite3Config.m.xRoundup(sizeof(HashElem)) * (pSchema->tblHash.count + pSchema->trigHash.count +
                                                                 pSchema->idxHash.count + pSchema->fkeyHash.count);
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          for (p = ((&pSchema->trigHash)->first); p; p = ((p)->next)) {
            sqlite3DeleteTrigger(db, (Trigger *)((p)->data));
          }
          for (p = ((&pSchema->tblHash)->first); p; p = ((p)->next)) {
            sqlite3DeleteTable(db, (Table *)((p)->data));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      break;
    }

    case SQLITE_DBSTATUS_STMT_USED: {
      struct Vdbe *pVdbe;
      int nByte = 0;

      db->pnBytesFreed = &nByte;

      db->lookaside.pEnd = db->lookaside.pStart;
      for (pVdbe = db->pVdbe; pVdbe; pVdbe = pVdbe->pVNext) {
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;
      *pCurrent = nByte;

      break;
    }

    case SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE + 1;
      __attribute__((fallthrough));
    case SQLITE_DBSTATUS_CACHE_HIT:
    case SQLITE_DBSTATUS_CACHE_MISS:
    case SQLITE_DBSTATUS_CACHE_WRITE: {
      int i;
      u64 nRet = 0;

      for (i = 0; i < db->nDb; i++) {
        if (db->aDb[i].pBt) {
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0;

      *pCurrent = nRet;
      break;
    }

    case SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      if (db->aDb[1].pBt) {
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE, resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      if (resetFlag)
        db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      break;
    }

    case SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;
      *pCurrent = db->nDeferredImmCons > 0 || db->nDeferredCons > 0;
      break;
    }

    default: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_db_status(sqlite3 *db, int op, int *pCurrent, int *pHighwtr, int resetFlag) {
  sqlite3_int64 C = 0, H = 0;
  int rc;

  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  if (rc == 0) {
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  return rc;
}

static void sqlite3RegisterDateTimeFunctions(void) {
  static FuncDef aDateTimeFuncs[] = {

      {-1, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, juliandayFunc, 0, 0, 0, "julianday", {0}},
      {-1, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, unixepochFunc, 0, 0, 0, "unixepoch", {0}},
      {-1, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, dateFunc, 0, 0, 0, "date", {0}},
      {-1, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, timeFunc, 0, 0, 0, "time", {0}},
      {-1, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, datetimeFunc, 0, 0, 0, "datetime", {0}},
      {-1, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, strftimeFunc, 0, 0, 0, "strftime", {0}},
      {2, 0x00800000 | 0x2000 | 1 | 0x0800, (void *)&sqlite3Config, 0, timediffFunc, 0, 0, 0, "timediff", {0}},
      {0, 0x00800000 | 0x2000 | 1, 0, 0, ctimeFunc, 0, 0, 0, "current_time", {0}},
      {0, 0x00800000 | 0x2000 | 1, 0, 0, ctimestampFunc, 0, 0, 0, "current_timestamp", {0}},
      {0, 0x00800000 | 0x2000 | 1, 0, 0, cdateFunc, 0, 0, 0, "current_date", {0}},
  };
  sqlite3InsertBuiltinFuncs(aDateTimeFuncs, ((int)(sizeof(aDateTimeFuncs) / sizeof(aDateTimeFuncs[0]))));
}

static int sqlite3OsInit(void) {
  void *p = sqlite3_malloc(10);
  if (p == 0)
    return 7;
  sqlite3_free(p);
  return sqlite3_os_init();
}

void sqlite3BeginBenignMalloc(void) {
  if (sqlite3Hooks.xBenignBegin) {
    sqlite3Hooks.xBenignBegin();
  }
}

void sqlite3EndBenignMalloc(void) {
  if (sqlite3Hooks.xBenignEnd) {
    sqlite3Hooks.xBenignEnd();
  }
}

int sqlite3MutexInit(void) {
  int rc = SQLITE_OK;
  if (!sqlite3Config.mutex.xMutexAlloc) {
    sqlite3_mutex_methods const *pFrom;
    sqlite3_mutex_methods *pTo = &sqlite3Config.mutex;

    if (sqlite3Config.bCoreMutex) {
      pFrom = sqlite3DefaultMutex();

    } else {
      pFrom = sqlite3NoopMutex();
    }
    pTo->xMutexInit = pFrom->xMutexInit;
    pTo->xMutexEnd = pFrom->xMutexEnd;
    pTo->xMutexFree = pFrom->xMutexFree;
    pTo->xMutexEnter = pFrom->xMutexEnter;
    pTo->xMutexTry = pFrom->xMutexTry;
    pTo->xMutexLeave = pFrom->xMutexLeave;
    pTo->xMutexHeld = pFrom->xMutexHeld;
    pTo->xMutexNotheld = pFrom->xMutexNotheld;
    sqlite3MemoryBarrier();
    pTo->xMutexAlloc = pFrom->xMutexAlloc;
  }

  rc = sqlite3Config.mutex.xMutexInit();

  sqlite3MemoryBarrier();
  return rc;
}

static int sqlite3MutexEnd(void) {
  int rc = SQLITE_OK;
  if (sqlite3Config.mutex.xMutexEnd) {
    rc = sqlite3Config.mutex.xMutexEnd();
  }

  return rc;
}

sqlite3_mutex *sqlite3MutexAlloc(int id) {
  if (!sqlite3Config.bCoreMutex) {
    return 0;
  }

  return sqlite3Config.mutex.xMutexAlloc(id);
}

sqlite3_mutex_methods const *sqlite3NoopMutex(void) {
  static const sqlite3_mutex_methods sMutex = {
      noopMutexInit, noopMutexEnd, noopMutexAlloc, noopMutexFree, noopMutexEnter, noopMutexTry, noopMutexLeave, 0, 0,
  };

  return &sMutex;
}

void sqlite3MemoryBarrier(void) {
  __sync_synchronize();
}

sqlite3_mutex_methods const *sqlite3DefaultMutex(void) {
  static const sqlite3_mutex_methods sMutex = {pthreadMutexInit,
                                               pthreadMutexEnd,
                                               pthreadMutexAlloc,
                                               pthreadMutexFree,
                                               pthreadMutexEnter,
                                               pthreadMutexTry,
                                               pthreadMutexLeave,
                                               0,
                                               0};

  return &sMutex;
}

int sqlite3_release_memory(int n) {
  (void)(n);
  return 0;
}

sqlite3_mutex *sqlite3MallocMutex(void) {
  return mem0.mutex;
}

int sqlite3_memory_alarm(void (*xCallback)(void *pArg, sqlite3_int64 used, int N), void *pArg,
                         sqlite3_int64 iThreshold) {
  (void)xCallback;
  (void)pArg;
  (void)iThreshold;
  return SQLITE_OK;
}

sqlite3_int64 sqlite3_soft_heap_limit64(sqlite3_int64 n) {
  sqlite3_int64 priorLimit;
  sqlite3_int64 excess;
  sqlite3_int64 nUsed;

  int rc = sqlite3_initialize();
  if (rc)
    return -1;

  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.alarmThreshold;
  if (n < 0) {
    sqlite3_mutex_leave(mem0.mutex);
    return priorLimit;
  }
  if (mem0.hardLimit > 0 && (n > mem0.hardLimit || n == 0)) {
    n = mem0.hardLimit;
  }
  mem0.alarmThreshold = n;
  nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED);
  __atomic_store_n((&mem0.nearlyFull), (n > 0 && n <= nUsed), 0);
  sqlite3_mutex_leave(mem0.mutex);
  excess = sqlite3_memory_used() - n;
  if (excess > 0)
    sqlite3_release_memory((int)(excess & 0x7fffffff));
  return priorLimit;
}

void sqlite3_soft_heap_limit(int n) {
  if (n < 0)
    n = 0;
  sqlite3_soft_heap_limit64(n);
}

sqlite3_int64 sqlite3_hard_heap_limit64(sqlite3_int64 n) {
  sqlite3_int64 priorLimit;

  int rc = sqlite3_initialize();
  if (rc)
    return -1;

  sqlite3_mutex_enter(mem0.mutex);
  priorLimit = mem0.hardLimit;
  if (n >= 0) {
    mem0.hardLimit = n;
    if (n < mem0.alarmThreshold || mem0.alarmThreshold == 0) {
      mem0.alarmThreshold = n;
    }
  }
  sqlite3_mutex_leave(mem0.mutex);
  return priorLimit;
}

static int sqlite3MallocInit(void) {
  int rc;
  if (sqlite3Config.m.xMalloc == 0) {
    sqlite3MemSetDefault();
  }
  mem0.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM);
  if (sqlite3Config.pPage == 0 || sqlite3Config.szPage < 512 || sqlite3Config.nPage <= 0) {
    sqlite3Config.pPage = 0;
    sqlite3Config.szPage = 0;
  }
  rc = sqlite3Config.m.xInit(sqlite3Config.m.pAppData);
  if (rc != SQLITE_OK)
    memset(&mem0, 0, sizeof(mem0));
  return rc;
}

int sqlite3HeapNearlyFull(void) {
  return __atomic_load_n((&mem0.nearlyFull), 0);
}

static void sqlite3MallocEnd(void) {
  if (sqlite3Config.m.xShutdown) {
    sqlite3Config.m.xShutdown(sqlite3Config.m.pAppData);
  }
  memset(&mem0, 0, sizeof(mem0));
}

static void sqlite3MallocAlarm(int nByte) {
  if (mem0.alarmThreshold <= 0)
    return;
  sqlite3_mutex_leave(mem0.mutex);
  sqlite3_release_memory(nByte);
  sqlite3_mutex_enter(mem0.mutex);
}

void *sqlite3Malloc(u64 n) {
  void *p;
  if (n == 0 || n > 2147483391) {
    p = 0;
  } else if (sqlite3Config.bMemstat) {
    sqlite3_mutex_enter(mem0.mutex);
    mallocWithAlarm((int)n, &p);
    sqlite3_mutex_leave(mem0.mutex);
  } else {
    p = sqlite3Config.m.xMalloc((int)n);
  }

  return p;
}

int isLookaside(sqlite3 *db, const void *p) {
  return (((uptr)(p) >= (uptr)(db->lookaside.pStart)) && ((uptr)(p) < (uptr)(db->lookaside.pTrueEnd)));
}

int sqlite3MallocSize(const void *p) {
  return sqlite3Config.m.xSize((void *)p);
}

int lookasideMallocSize(sqlite3 *db, const void *p) {
  return p < db->lookaside.pMiddle ? db->lookaside.szTrue : 128;
}

int sqlite3DbMallocSize(sqlite3 *db, const void *p) {
  if (db) {
    if (((uptr)p) < (uptr)(db->lookaside.pTrueEnd)) {
      if (((uptr)p) >= (uptr)(db->lookaside.pMiddle)) {
        return 128;
      }

      if (((uptr)p) >= (uptr)(db->lookaside.pStart)) {
        return db->lookaside.szTrue;
      }
    }
  }
  return sqlite3Config.m.xSize((void *)p);
}

__attribute__((noinline)) void measureAllocationSize(sqlite3 *db, void *p) {
  *db->pnBytesFreed += sqlite3DbMallocSize(db, p);
}

void sqlite3DbFreeNN(sqlite3 *db, void *p) {
  if (db) {
    if (((uptr)p) < (uptr)(db->lookaside.pEnd)) {
      if (((uptr)p) >= (uptr)(db->lookaside.pMiddle)) {
        LookasideSlot *pBuf = (LookasideSlot *)p;

        pBuf->pNext = db->lookaside.pSmallFree;
        db->lookaside.pSmallFree = pBuf;
        return;
      }

      if (((uptr)p) >= (uptr)(db->lookaside.pStart)) {
        LookasideSlot *pBuf = (LookasideSlot *)p;

        pBuf->pNext = db->lookaside.pFree;
        db->lookaside.pFree = pBuf;
        return;
      }
    }
    if (db->pnBytesFreed) {
      measureAllocationSize(db, p);
      return;
    }
  }

  sqlite3_free(p);
}

void sqlite3DbNNFreeNN(sqlite3 *db, void *p) {
  if (((uptr)p) < (uptr)(db->lookaside.pEnd)) {
    if (((uptr)p) >= (uptr)(db->lookaside.pMiddle)) {
      LookasideSlot *pBuf = (LookasideSlot *)p;

      pBuf->pNext = db->lookaside.pSmallFree;
      db->lookaside.pSmallFree = pBuf;
      return;
    }

    if (((uptr)p) >= (uptr)(db->lookaside.pStart)) {
      LookasideSlot *pBuf = (LookasideSlot *)p;

      pBuf->pNext = db->lookaside.pFree;
      db->lookaside.pFree = pBuf;
      return;
    }
  }
  if (db->pnBytesFreed) {
    measureAllocationSize(db, p);
    return;
  }

  sqlite3_free(p);
}

void sqlite3DbFree(sqlite3 *db, void *p) {
  if (p)
    sqlite3DbFreeNN(db, p);
}

void *sqlite3Realloc(void *pOld, u64 nBytes) {
  int nOld, nNew, nDiff;
  void *pNew;

  if (pOld == 0) {
    return sqlite3Malloc(nBytes);
  }
  if (nBytes == 0) {
    sqlite3_free(pOld);
    return 0;
  }
  if (nBytes > 2147483391) {
    return 0;
  }
  nOld = sqlite3MallocSize(pOld);

  nNew = sqlite3Config.m.xRoundup((int)nBytes);
  if (nOld == nNew) {
    pNew = pOld;
  } else if (sqlite3Config.bMemstat) {
    sqlite3_int64 nUsed;
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusHighwater(SQLITE_STATUS_MALLOC_SIZE, (int)nBytes);
    nDiff = nNew - nOld;
    if (nDiff > 0 && (nUsed = sqlite3StatusValue(SQLITE_STATUS_MEMORY_USED)) >= mem0.alarmThreshold - nDiff) {
      sqlite3MallocAlarm(nDiff);
      if (mem0.hardLimit > 0 && nUsed >= mem0.hardLimit - nDiff) {
        sqlite3_mutex_leave(mem0.mutex);
        return 0;
      }
    }
    pNew = sqlite3Config.m.xRealloc(pOld, nNew);

    if (pNew) {
      nNew = sqlite3MallocSize(pNew);
      sqlite3StatusUp(SQLITE_STATUS_MEMORY_USED, nNew - nOld);
    }
    sqlite3_mutex_leave(mem0.mutex);
  } else {
    pNew = sqlite3Config.m.xRealloc(pOld, nNew);
  }

  return pNew;
}

void *sqlite3MallocZero(u64 n) {
  void *p = sqlite3Malloc(n);
  if (p) {
    memset(p, 0, (size_t)n);
  }
  return p;
}

void *sqlite3DbMallocZero(sqlite3 *db, u64 n) {
  void *p;
  p = sqlite3DbMallocRaw(db, n);
  if (p)
    memset(p, 0, (size_t)n);
  return p;
}

__attribute__((noinline)) void *dbMallocRawFinish(sqlite3 *db, u64 n) {
  void *p;

  p = sqlite3Malloc(n);
  if (!p)
    sqlite3OomFault(db);

  return p;
}

void *sqlite3DbMallocRaw(sqlite3 *db, u64 n) {
  void *p;
  if (db)
    return sqlite3DbMallocRawNN(db, n);
  p = sqlite3Malloc(n);
  return p;
}

void *sqlite3DbMallocRawNN(sqlite3 *db, u64 n) {
  LookasideSlot *pBuf;

  if (n > db->lookaside.sz) {
    if (!db->lookaside.bDisable) {
      db->lookaside.anStat[1]++;
    } else if (db->mallocFailed) {
      return 0;
    }
    return dbMallocRawFinish(db, n);
  }

  if (n <= 128) {
    if ((pBuf = db->lookaside.pSmallFree) != 0) {
      db->lookaside.pSmallFree = pBuf->pNext;
      db->lookaside.anStat[0]++;
      return (void *)pBuf;
    } else if ((pBuf = db->lookaside.pSmallInit) != 0) {
      db->lookaside.pSmallInit = pBuf->pNext;
      db->lookaside.anStat[0]++;
      return (void *)pBuf;
    }
  }

  if ((pBuf = db->lookaside.pFree) != 0) {
    db->lookaside.pFree = pBuf->pNext;
    db->lookaside.anStat[0]++;
    return (void *)pBuf;
  } else if ((pBuf = db->lookaside.pInit) != 0) {
    db->lookaside.pInit = pBuf->pNext;
    db->lookaside.anStat[0]++;
    return (void *)pBuf;
  } else {
    db->lookaside.anStat[2]++;
  }

  return dbMallocRawFinish(db, n);
}

void *sqlite3DbRealloc(sqlite3 *db, void *p, u64 n) {
  if (p == 0)
    return sqlite3DbMallocRawNN(db, n);

  if (((uptr)p) < (uptr)db->lookaside.pEnd) {
    if (((uptr)p) >= (uptr)db->lookaside.pMiddle) {
      if (n <= 128)
        return p;
    } else if (((uptr)p) >= (uptr)db->lookaside.pStart) {
      if (n <= db->lookaside.szTrue)
        return p;
    }
  }
  return dbReallocFinish(db, p, n);
}

__attribute__((noinline)) void *dbReallocFinish(sqlite3 *db, void *p, u64 n) {
  void *pNew = 0;

  if (db->mallocFailed == 0) {
    if (isLookaside(db, p)) {
      pNew = sqlite3DbMallocRawNN(db, n);
      if (pNew) {
        memcpy(pNew, p, lookasideMallocSize(db, p));
        sqlite3DbFree(db, p);
      }
    } else {
      pNew = sqlite3Realloc(p, n);
      if (!pNew) {
        sqlite3OomFault(db);
      }
    }
  }
  return pNew;
}

void *sqlite3DbReallocOrFree(sqlite3 *db, void *p, u64 n) {
  void *pNew;
  pNew = sqlite3DbRealloc(db, p, n);
  if (!pNew) {
    sqlite3DbFree(db, p);
  }
  return pNew;
}

char *sqlite3DbStrDup(sqlite3 *db, const char *z) {
  char *zNew;
  size_t n;
  if (z == 0) {
    return 0;
  }
  n = strlen(z) + 1;
  zNew = sqlite3DbMallocRaw(db, n);
  if (zNew) {
    memcpy(zNew, z, n);
  }
  return zNew;
}

char *sqlite3DbStrNDup(sqlite3 *db, const char *z, u64 n) {
  char *zNew;

  zNew = z ? sqlite3DbMallocRawNN(db, n + 1) : 0;
  if (zNew) {
    memcpy(zNew, z, (size_t)n);
    zNew[n] = 0;
  }
  return zNew;
}

char *sqlite3DbSpanDup(sqlite3 *db, const char *zStart, const char *zEnd) {
  int n;

  while ((sqlite3CtypeMap[(unsigned char)(zStart[0])] & 0x01))
    zStart++;
  n = (int)(zEnd - zStart);
  while ((sqlite3CtypeMap[(unsigned char)(zStart[n - 1])] & 0x01))
    n--;
  return sqlite3DbStrNDup(db, zStart, n);
}

void sqlite3SetString(char **pz, sqlite3 *db, const char *zNew) {
  char *z = sqlite3DbStrDup(db, zNew);
  sqlite3DbFree(db, *pz);
  *pz = z;
}

void *sqlite3OomFault(sqlite3 *db) {
  if (db->mallocFailed == 0 && db->bBenignMalloc == 0) {
    db->mallocFailed = 1;
    if (db->nVdbeExec > 0) {
      __atomic_store_n((&db->u1.isInterrupted), (1), 0);
    }
    db->lookaside.bDisable++;
    db->lookaside.sz = 0;
    if (db->pParse) {
      Parse *pParse;
      sqlite3ErrorMsg(db->pParse, "out of memory");
      db->pParse->rc = 7;
      for (pParse = db->pParse->pOuterParse; pParse; pParse = pParse->pOuterParse) {
        pParse->nErr++;
        pParse->rc = SQLITE_NOMEM;
      }
    }
  }
  return 0;
}

void sqlite3OomClear(sqlite3 *db) {
  if (db->mallocFailed && db->nVdbeExec == 0) {
    db->mallocFailed = 0;
    __atomic_store_n((&db->u1.isInterrupted), (0), 0);

    db->lookaside.bDisable--;
    db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
  }
}

__attribute__((noinline)) int apiHandleError(sqlite3 *db, int rc) {
  if (db->mallocFailed || rc == (10 | (12 << 8))) {
    sqlite3OomClear(db);
    sqlite3Error(db, SQLITE_NOMEM);
    return 7;
  }
  return rc & db->errMask;
}

int sqlite3ApiExit(sqlite3 *db, int rc) {
  if (db->mallocFailed || rc) {
    return apiHandleError(db, rc);
  }
  return 0;
}

void sqlite3RecordErrorByteOffset(sqlite3 *db, const char *z) {
  const Parse *pParse;
  const char *zText;
  const char *zEnd;

  if (db == 0)
    return;
  if (db->errByteOffset != (-2))
    return;
  pParse = db->pParse;
  if (pParse == 0)
    return;
  zText = pParse->zTail;
  if (zText == 0)
    return;
  zEnd = &zText[strlen(zText)];
  if ((((uptr)(z) >= (uptr)(zText)) && ((uptr)(z) < (uptr)(zEnd)))) {
    db->errByteOffset = (int)(z - zText);
  }
}

void sqlite3RecordErrorOffsetOfExpr(sqlite3 *db, const Expr *pExpr) {
  while (pExpr && ((((pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0) || pExpr->w.iOfst <= 0)) {
    pExpr = pExpr->pLeft;
  }
  if (pExpr == 0)
    return;
  if ((((pExpr)->flags & (u32)(0x40000000)) != 0))
    return;
  db->errByteOffset = pExpr->w.iOfst;
}

sqlite3_str *sqlite3_str_new(sqlite3 *db) {
  sqlite3_str *p = sqlite3_malloc64(sizeof(*p));
  if (p) {
    sqlite3StrAccumInit(p, 0, 0, 0, db ? db->aLimit[SQLITE_LIMIT_LENGTH] : 1000000000);
  } else {
    p = &sqlite3OomStr;
  }
  return p;
}

char *sqlite3VMPrintf(sqlite3 *db, const char *zFormat, va_list ap) {
  char *z;
  char zBase[70];
  StrAccum acc;

  sqlite3StrAccumInit(&acc, db, zBase, sizeof(zBase), db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = 0x01;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  if (acc.accError == SQLITE_NOMEM) {
    sqlite3OomFault(db);
  }
  return z;
}

char *sqlite3MPrintf(sqlite3 *db, const char *zFormat, ...) {
  va_list ap;
  char *z;

  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);

  va_end(ap);
  return z;
}

void sqlite3_log(int iErrCode, const char *zFormat, ...) {
  va_list ap;
  if (sqlite3Config.xLog) {
    va_start(ap, zFormat);
    renderLogMsg(iErrCode, zFormat, ap);

    va_end(ap);
  }
}

struct sqlite3PrngType sqlite3Prng;

struct sqlite3PrngType sqlite3SavedPrng;

static void sqlite3PrngSaveState(void) {
  memcpy(&sqlite3SavedPrng, &sqlite3Prng, sizeof(sqlite3Prng));
}

static void sqlite3PrngRestoreState(void) {
  memcpy(&sqlite3Prng, &sqlite3SavedPrng, sizeof(sqlite3Prng));
}

const unsigned char sqlite3Utf8Trans1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

int sqlite3AppendOneUtf8Character(char *zOut, u32 v) {
  if (v < 0x00080) {
    zOut[0] = (u8)(v & 0xff);
    return 1;
  }
  if (v < 0x00800) {
    zOut[0] = 0xc0 + (u8)((v >> 6) & 0x1f);
    zOut[1] = 0x80 + (u8)(v & 0x3f);
    return 2;
  }
  if (v < 0x10000) {
    zOut[0] = 0xe0 + (u8)((v >> 12) & 0x0f);
    zOut[1] = 0x80 + (u8)((v >> 6) & 0x3f);
    zOut[2] = 0x80 + (u8)(v & 0x3f);
    return 3;
  }
  zOut[0] = 0xf0 + (u8)((v >> 18) & 0x07);
  zOut[1] = 0x80 + (u8)((v >> 12) & 0x3f);
  zOut[2] = 0x80 + (u8)((v >> 6) & 0x3f);
  zOut[3] = 0x80 + (u8)(v & 0x3f);
  return 4;
}

u32 sqlite3Utf8Read(const unsigned char **pz) {
  unsigned int c;

  c = *((*pz)++);
  if (c >= 0xc0) {
    c = sqlite3Utf8Trans1[c - 0xc0];
    while ((*(*pz) & 0xc0) == 0x80) {
      c = (c << 6) + (0x3f & *((*pz)++));
    }
    if (c < 0x80 || (c & 0xFFFFF800) == 0xD800 || (c & 0xFFFFFFFE) == 0xFFFE) {
      c = 0xFFFD;
    }
  }
  return c;
}

int sqlite3Utf8ReadLimited(const u8 *z, int n, u32 *piOut) {
  u32 c;
  int i = 1;

  c = z[0];
  if (c >= 0xc0) {
    c = sqlite3Utf8Trans1[c - 0xc0];
    if (n > 4)
      n = 4;
    while (i < n && (z[i] & 0xc0) == 0x80) {
      c = (c << 6) + (0x3f & z[i]);
      i++;
    }
  }
  *piOut = c;
  return i;
}

int sqlite3Utf8CharLen(const char *zIn, int nByte) {
  int r = 0;
  const u8 *z = (const u8 *)zIn;
  const u8 *zTerm;
  if (nByte >= 0) {
    zTerm = &z[nByte];
  } else {
    zTerm = (const u8 *)(-1);
  }

  while (*z != 0 && z < zTerm) {
    {
      if ((*(z++)) >= 0xc0) {
        while ((*z & 0xc0) == 0x80) {
          z++;
        }
      }
    };
    r++;
  }
  return r;
}

char *sqlite3Utf16to8(sqlite3 *db, const void *z, int nByte, u8 enc) {
  Mem m;
  memset(&m, 0, sizeof(m));
  m.db = db;
  sqlite3VdbeMemSetStr(&m, z, nByte, enc, ((sqlite3_destructor_type)0));
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  if (db->mallocFailed) {
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }

  return m.z;
}

static int sqlite3Utf16ByteLen(const void *zIn, int nByte, int nChar) {
  int c;
  unsigned char const *z = zIn;
  unsigned char const *zEnd = &z[nByte - 1];
  int n = 0;

  if (2 == SQLITE_UTF16LE)
    z++;
  while (n < nChar && z <= zEnd) {
    c = z[0];
    z += 2;
    if (c >= 0xd8 && c < 0xdc && z <= zEnd && z[0] >= 0xdc && z[0] < 0xe0)
      z += 2;
    n++;
  }
  return (int)(z - (unsigned char const *)zIn) - (2 == SQLITE_UTF16LE);
}

int sqlite3FaultSim(int iTest) {
  int (*xCallback)(int) = sqlite3Config.xTestCallback;
  return xCallback ? xCallback(iTest) : SQLITE_OK;
}

int sqlite3IsNaN(double x) {
  int rc;

  rc = isnan(x);

  return rc;
}

int sqlite3IsOverflow(double x) {
  int rc;
  u64 y;
  memcpy(&y, &x, sizeof(y));
  rc = (((y) & (((u64)0x7ff) << 52)) == (((u64)0x7ff) << 52));
  return rc;
}

int sqlite3Strlen30(const char *z) {
  if (z == 0)
    return 0;
  return 0x3fffffff & (int)strlen(z);
}

__attribute__((noinline)) void sqlite3ErrorFinish(sqlite3 *db, int err_code) {
  if (db->pErr)
    sqlite3ValueSetNull(db->pErr);
  sqlite3SystemError(db, err_code);
}

void sqlite3Error(sqlite3 *db, int err_code) {
  db->errCode = err_code;
  if (err_code || db->pErr) {
    sqlite3ErrorFinish(db, err_code);
  } else {
    db->errByteOffset = -1;
  }
}

void sqlite3ErrorClear(sqlite3 *db) {
  db->errCode = SQLITE_OK;
  db->errByteOffset = -1;
  if (db->pErr)
    sqlite3ValueSetNull(db->pErr);
}

void sqlite3SystemError(sqlite3 *db, int rc) {
  if (rc == (10 | (12 << 8)))
    return;

  rc &= 0xff;
  if (rc == SQLITE_CANTOPEN || rc == SQLITE_IOERR) {
    db->iSysErrno = sqlite3OsGetLastError(db->pVfs);
  }
}

void sqlite3ErrorWithMsg(sqlite3 *db, int err_code, const char *zFormat, ...) {
  db->errCode = err_code;
  sqlite3SystemError(db, err_code);
  if (zFormat == 0) {
    sqlite3Error(db, err_code);
  } else if (db->pErr || (db->pErr = sqlite3ValueNew(db)) != 0) {
    char *z;
    va_list ap;

    va_start(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);

    va_end(ap);
    sqlite3ValueSetStr(db->pErr, -1, z, SQLITE_UTF8, ((sqlite3_destructor_type)sqlite3RowSetClear));
  }
}

int sqlite3ErrorToParser(sqlite3 *db, int errCode) {
  Parse *pParse;
  if (db == 0 || (pParse = db->pParse) == 0)
    return errCode;
  pParse->rc = errCode;
  pParse->nErr++;
  return errCode;
}

void sqlite3Dequote(char *z) {
  char quote;
  int i, j;
  if (z == 0)
    return;
  quote = z[0];
  if (!(sqlite3CtypeMap[(unsigned char)(quote)] & 0x80))
    return;
  if (quote == '[')
    quote = ']';
  for (i = 1, j = 0;; i++) {
    if (z[i] == quote) {
      if (z[i + 1] == quote) {
        z[j++] = quote;
        i++;
      } else {
        break;
      }
    } else {
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}

int sqlite3_stricmp(const char *zLeft, const char *zRight) {
  if (zLeft == 0) {
    return zRight ? -1 : 0;
  } else if (zRight == 0) {
    return 1;
  }
  return sqlite3StrICmp(zLeft, zRight);
}

int sqlite3StrICmp(const char *zLeft, const char *zRight) {
  unsigned char *a, *b;
  int c, x;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  for (;;) {
    c = *a;
    x = *b;
    if (c == x) {
      if (c == 0)
        break;
    } else {
      c = (int)sqlite3UpperToLower[c] - (int)sqlite3UpperToLower[x];
      if (c)
        break;
    }
    a++;
    b++;
  }
  return c;
}

int sqlite3_strnicmp(const char *zLeft, const char *zRight, int N) {
  register unsigned char *a, *b;
  if (zLeft == 0) {
    return zRight ? -1 : 0;
  } else if (zRight == 0) {
    return 1;
  }
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while (N-- > 0 && *a != 0 && sqlite3UpperToLower[*a] == sqlite3UpperToLower[*b]) {
    a++;
    b++;
  }
  return N < 0 ? 0 : sqlite3UpperToLower[*a] - sqlite3UpperToLower[*b];
}

u8 sqlite3StrIHash(const char *z) {
  u8 h = 0;
  if (z == 0)
    return 0;
  while (z[0]) {
    h += sqlite3UpperToLower[(unsigned char)z[0]];
    z++;
  }
  return h;
}

static u64 sqlite3Multiply128(u64 a, u64 b, u64 *pLo) {
  __uint128_t r = (__uint128_t)a * b;
  *pLo = (u64)r;
  return (u64)(r >> 64);
}

static u64 sqlite3Multiply160(u64 a, u32 aLo, u64 b, u32 *pLo) {
  __uint128_t r = (__uint128_t)a * b;
  r += ((__uint128_t)aLo * b) >> 32;
  *pLo = (r >> 32) & 0xffffffff;
  return r >> 64;
}

void sqlite3Fp2Convert10(u64 m, int e, int n, u64 *pD, int *pP) {
  int p;
  u64 h, d1;
  u32 d2;

  p = n - 1 - pwr2to10(e + 63);
  h = sqlite3Multiply128(m, powerOfTen(p, &d2), &d1);

  if (n == 18) {
    h >>= -(e + pwr10to2(p) + 2);
    *pD = (h + ((h << 1) & 2)) >> 1;
  } else {
    *pD = h >> -(e + pwr10to2(p) + 1);
  }
  *pP = -p;
}

double sqlite3Fp10Convert2(u64 d, int p) {
  int b, lp, e, adj, s;
  u32 pwr10l, mid1;
  u64 pwr10h, x, hi, lo, sticky, u, m;
  double r;
  if (p < (-348))
    return 0.0;
  if (p > (+347))
    return (INFINITY);
  b = 64 - countLeadingZeros(d);
  lp = pwr10to2(p);
  e = 53 - b - lp;
  if (e > 1074) {
    if (e >= 1130)
      return 0.0;
    e = 1074;
  }
  s = -(e - (64 - b) + lp + 3);
  pwr10h = powerOfTen(p, &pwr10l);
  if (pwr10l != 0) {
    pwr10h++;
    pwr10l = ~pwr10l;
  }
  x = d << (64 - b);
  hi = sqlite3Multiply128(x, pwr10h, &lo);
  mid1 = lo >> 32;
  sticky = 1;
  if ((hi & ((((u64)1) << (s)) - 1)) == 0) {
    u32 mid2 = sqlite3Multiply128(x, ((u64)pwr10l) << 32, &lo) >> 32;
    sticky = (mid1 - mid2 > 1);
    hi -= mid1 < mid2;
  }
  u = (hi >> s) | sticky;
  adj = (u >= (((u64)1) << (55)) - 2);
  if (adj) {
    u = (u >> adj) | (u & 1);
    e -= adj;
  }
  m = (u + 1 + ((u >> 2) & 1)) >> 2;
  if (e <= (-972))
    return (INFINITY);
  if ((m & (((u64)1) << (52))) != 0) {
    m = (m & ~(((u64)1) << (52))) | ((u64)(1075 - e) << 52);
  }
  memcpy(&r, &m, 8);
  return r;
}

int sqlite3AtoF(const char *zIn, double *pResult) {
  const unsigned char *z = (const unsigned char *)zIn;
  int neg = 0;
  u64 s = 0;
  int d = 0;
  int mState = 0;
  unsigned v;

start_of_text:
  if ((v = (unsigned)z[0] - '0') < 10) {
  parse_integer_part:
    mState = 1;
    s = v;
    z++;
    while ((v = (unsigned)z[0] - '0') < 10) {
      s = s * 10 + v;
      z++;
      if (s >= ((0xffffffff | (((u64)0xffffffff) << 32)) - 9) / 10) {
        mState = 9;
        while ((sqlite3CtypeMap[(unsigned char)(z[0])] & 0x04)) {
          z++;
          d++;
        }
        break;
      }
    }
  } else if (z[0] == '-') {
    neg = 1;
    z++;
    if ((v = (unsigned)z[0] - '0') < 10)
      goto parse_integer_part;
  } else if (z[0] == '+') {
    z++;
    if ((v = (unsigned)z[0] - '0') < 10)
      goto parse_integer_part;
  } else if ((sqlite3CtypeMap[(unsigned char)(z[0])] & 0x01)) {
    do {
      z++;
    } while ((sqlite3CtypeMap[(unsigned char)(z[0])] & 0x01));
    goto start_of_text;
  } else {
    s = 0;
  }

  if (*z == '.') {
    z++;
    if ((sqlite3CtypeMap[(unsigned char)(z[0])] & 0x04)) {
      mState |= 1;
      do {
        if (s < ((0xffffffff | (((u64)0xffffffff) << 32)) - 9) / 10) {
          s = s * 10 + z[0] - '0';
          d--;
        } else {
          mState = 11;
        }
      } while ((sqlite3CtypeMap[(unsigned char)(*++z)] & 0x04));
    } else if (mState == 0) {
      *pResult = 0.0;
      return 0;
    }
    mState |= 2;
  } else if (mState == 0) {
    *pResult = 0.0;
    return 0;
  }

  if (*z == 'e' || *z == 'E') {
    int esign;
    z++;

    if (*z == '-') {
      esign = -1;
      z++;
    } else {
      esign = +1;
      if (*z == '+') {
        z++;
      }
    }

    if ((v = (unsigned)z[0] - '0') < 10) {
      int exp = v;
      z++;
      mState |= 2;
      while ((v = (unsigned)z[0] - '0') < 10) {
        exp = exp < 10000 ? (exp * 10 + v) : 10000;
        z++;
      }
      d += esign * exp;
    } else {
      z--;
    }
  }

  if (s == 0) {
    *pResult = 0.0;
    mState |= 4;
  } else {
    *pResult = sqlite3Fp10Convert2(s, d);
  }
  if (neg)
    *pResult = -*pResult;

  if (z[0] == 0) {
    return mState;
  }
  if ((sqlite3CtypeMap[(unsigned char)(z[0])] & 0x01)) {
    do {
      z++;
    } while ((sqlite3CtypeMap[(unsigned char)(*z)] & 0x01));
    if (z[0] == 0) {
      return mState;
    }
  }
  return 0xfffffff0 | mState;
}

const sqlite3DigitPairs_t sqlite3DigitPairs = {
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899"};

int sqlite3Int64ToText(i64 v, char *zOut) {
  int i;
  u64 x;
  union {
    char a[20 + 1];
    u16 forceAlignment;
  } u;
  if (v > 0) {
    x = v;
  } else if (v == 0) {
    zOut[0] = '0';
    zOut[1] = 0;
    return 1;
  } else {
    x = (v == (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32)))) ? ((u64)1) << 63 : (u64)-v;
  }

  i = sizeof(u.a) - 1;
  u.a[i] = 0;
  while (x >= 10) {
    int kk = (x % 100) * 2;

    *(u16 *)(&u.a[i - 2]) = *(u16 *)&sqlite3DigitPairs.a[kk];
    i -= 2;
    x /= 100;
  }
  if (x) {
    u.a[--i] = x + '0';
  }

  if (v < 0)
    u.a[--i] = '-';
  memcpy(zOut, &u.a[i], sizeof(u.a) - i);
  return sizeof(u.a) - 1 - i;
}

int sqlite3Atoi64(const char *zNum, i64 *pNum, int length, u8 enc) {
  int incr;
  u64 u = 0;
  int neg = 0;
  int i, j;
  unsigned int c = 0;
  int nonNum = 0;
  int rc;
  const char *zStart;
  const char *zEnd = zNum + length;

  if (enc == SQLITE_UTF8) {
    incr = 1;
  } else {
    incr = 2;
    length &= ~1;

    for (i = 3 - enc; i < length && zNum[i] == 0; i += 2) {
    }
    nonNum = i < length;
    zEnd = &zNum[i ^ 1];
    zNum += (enc & 1);
  }
  while (zNum < zEnd && (sqlite3CtypeMap[(unsigned char)(*zNum)] & 0x01))
    zNum += incr;
  if (zNum < zEnd) {
    if (*zNum == '-') {
      neg = 1;
      zNum += incr;
    } else if (*zNum == '+') {
      zNum += incr;
    }
  }
  zStart = zNum;
  while (zNum < zEnd && zNum[0] == '0') {
    zNum += incr;
  }
  for (i = 0; &zNum[i] < zEnd && (c = (unsigned)zNum[i] - '0') <= 9; i += incr) {
    u = u * 10 + c;
  };
  if (u > (0xffffffff | (((i64)0x7fffffff) << 32))) {
    *pNum = neg ? (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))) : (0xffffffff | (((i64)0x7fffffff) << 32));
  } else if (neg) {
    *pNum = -(i64)u;
  } else {
    *pNum = (i64)u;
  }
  rc = 0;
  if (i == 0 && zStart == zNum) {
    rc = -1;
  } else if (nonNum) {
    rc = 1;
  } else if (&zNum[i] < zEnd) {
    int jj = i;
    do {
      if (!(sqlite3CtypeMap[(unsigned char)(zNum[jj])] & 0x01)) {
        rc = 1;
        break;
      }
      jj += incr;
    } while (&zNum[jj] < zEnd);
  }
  if (i < 19 * incr) {
    return rc;
  } else {
    j = i > 19 * incr ? 1 : compare2pow63(zNum, incr);
    if (j < 0) {
      return rc;
    } else {
      *pNum = neg ? (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))) : (0xffffffff | (((i64)0x7fffffff) << 32));
      if (j > 0) {
        return 2;
      } else {
        return neg ? rc : 3;
      }
    }
  }
}

int sqlite3DecOrHexToI64(const char *z, i64 *pOut) {
  if (z[0] == '0' && (z[1] == 'x' || z[1] == 'X')) {
    u64 u = 0;
    int i, k;
    for (i = 2; z[i] == '0'; i++) {
    }
    for (k = i; (sqlite3CtypeMap[(unsigned char)(z[k])] & 0x08); k++) {
      u = u * 16 + sqlite3HexToInt(z[k]);
    }
    memcpy(pOut, &u, 8);
    if (k - i > 16)
      return 2;
    if (z[k] != 0)
      return 1;
    return 0;
  } else {
    int n = (int)(0x3fffffff & strspn(z, "+- \n\t0123456789"));
    if (z[n])
      n++;
    return sqlite3Atoi64(z, pOut, n, SQLITE_UTF8);
  }
}

int sqlite3GetInt32(const char *zNum, int *pValue) {
  sqlite_int64 v = 0;
  int i, c;
  int neg = 0;
  if (zNum[0] == '-') {
    neg = 1;
    zNum++;
  } else if (zNum[0] == '+') {
    zNum++;
  }

  else if (zNum[0] == '0' && (zNum[1] == 'x' || zNum[1] == 'X') && (sqlite3CtypeMap[(unsigned char)(zNum[2])] & 0x08)) {
    u32 u = 0;
    zNum += 2;
    while (zNum[0] == '0')
      zNum++;
    for (i = 0; i < 8 && (sqlite3CtypeMap[(unsigned char)(zNum[i])] & 0x08); i++) {
      u = u * 16 + sqlite3HexToInt(zNum[i]);
    }
    if ((u & 0x80000000) == 0 && (sqlite3CtypeMap[(unsigned char)(zNum[i])] & 0x08) == 0) {
      memcpy(pValue, &u, 4);
      return 1;
    } else {
      return 0;
    }
  }

  if (!(sqlite3CtypeMap[(unsigned char)(zNum[0])] & 0x04))
    return 0;
  while (zNum[0] == '0')
    zNum++;
  for (i = 0; i < 11 && (c = zNum[i] - '0') >= 0 && c <= 9; i++) {
    v = v * 10 + c;
  }

  if (i > 10) {
    return 0;
  };
  if (v - neg > 2147483647) {
    return 0;
  }
  if (neg) {
    v = -v;
  }
  *pValue = (int)v;
  return 1;
}

int sqlite3Atoi(const char *z) {
  int x = 0;
  sqlite3GetInt32(z, &x);
  return x;
}

static int sqlite3GetUInt32(const char *z, u32 *pI) {
  u64 v = 0;
  int i;
  for (i = 0; (sqlite3CtypeMap[(unsigned char)(z[i])] & 0x04); i++) {
    v = v * 10 + z[i] - '0';
    if (v > 4294967296LL) {
      *pI = 0;
      return 0;
    }
  }
  if (i == 0 || z[i] != 0) {
    *pI = 0;
    return 0;
  }
  *pI = (u32)v;
  return 1;
}

int sqlite3PutVarint(unsigned char *p, u64 v) {
  if (v <= 0x7f) {
    p[0] = v & 0x7f;
    return 1;
  }
  if (v <= 0x3fff) {
    p[0] = ((v >> 7) & 0x7f) | 0x80;
    p[1] = v & 0x7f;
    return 2;
  }
  return putVarint64(p, v);
}

u8 sqlite3GetVarint(const unsigned char *p, u64 *v) {
  u32 a, b, s;

  if (((signed char *)p)[0] >= 0) {
    *v = *p;
    return 1;
  }
  if (((signed char *)p)[1] >= 0) {
    *v = ((u32)(p[0] & 0x7f) << 7) | p[1];
    return 2;
  }

  a = ((u32)p[0]) << 14;
  b = p[1];
  p += 2;
  a |= *p;

  if (!(a & 0x80)) {
    a &= 0x001fc07f;
    b &= 0x7f;
    b = b << 7;
    a |= b;
    *v = a;
    return 3;
  }

  a &= 0x001fc07f;
  p++;
  b = b << 14;
  b |= *p;

  if (!(b & 0x80)) {
    b &= 0x001fc07f;

    a = a << 7;
    a |= b;
    *v = a;
    return 4;
  }

  b &= 0x001fc07f;
  s = a;

  p++;
  a = a << 14;
  a |= *p;

  if (!(a & 0x80)) {
    b = b << 7;
    a |= b;
    s = s >> 18;
    *v = ((u64)s) << 32 | a;
    return 5;
  }

  s = s << 7;
  s |= b;

  p++;
  b = b << 14;
  b |= *p;

  if (!(b & 0x80)) {
    a &= 0x001fc07f;
    a = a << 7;
    a |= b;
    s = s >> 18;
    *v = ((u64)s) << 32 | a;
    return 6;
  }

  p++;
  a = a << 14;
  a |= *p;

  if (!(a & 0x80)) {
    a &= 0xf01fc07f;
    b &= 0x001fc07f;
    b = b << 7;
    a |= b;
    s = s >> 11;
    *v = ((u64)s) << 32 | a;
    return 7;
  }

  a &= 0x001fc07f;
  p++;
  b = b << 14;
  b |= *p;

  if (!(b & 0x80)) {
    b &= 0xf01fc07f;

    a = a << 7;
    a |= b;
    s = s >> 4;
    *v = ((u64)s) << 32 | a;
    return 8;
  }

  p++;
  a = a << 15;
  a |= *p;

  b &= 0x001fc07f;
  b = b << 8;
  a |= b;

  s = s << 4;
  b = p[-4];
  b &= 0x7f;
  b = b >> 3;
  s |= b;

  *v = ((u64)s) << 32 | a;

  return 9;
}

u8 sqlite3GetVarint32(const unsigned char *p, u32 *v) {
  u64 v64;
  u8 n;

  if ((p[1] & 0x80) == 0) {
    *v = ((p[0] & 0x7f) << 7) | p[1];
    return 2;
  }
  if ((p[2] & 0x80) == 0) {
    *v = ((p[0] & 0x7f) << 14) | ((p[1] & 0x7f) << 7) | p[2];
    return 3;
  }

  n = sqlite3GetVarint(p, &v64);

  if ((v64 & ((((u64)1) << 32) - 1)) != v64) {
    *v = 0xffffffff;
  } else {
    *v = (u32)v64;
  }
  return n;
}

int sqlite3VarintLen(u64 v) {
  int i;
  for (i = 1; (v >>= 7) != 0; i++) {
  }
  return i;
}

u32 sqlite3Get4byte(const u8 *p) {
  u32 x;
  memcpy(&x, p, 4);
  return __builtin_bswap32(x);
}

void sqlite3Put4byte(unsigned char *p, u32 v) {
  u32 x = __builtin_bswap32(v);
  memcpy(p, &x, 4);
}

u8 sqlite3HexToInt(int h) {
  h += 9 * (1 & (h >> 6));

  return (u8)(h & 0xf);
}

void *sqlite3HexToBlob(sqlite3 *db, const char *z, int n) {
  char *zBlob;
  int i;

  zBlob = (char *)sqlite3DbMallocRawNN(db, n / 2 + 1);
  n--;
  if (zBlob) {
    for (i = 0; i < n; i += 2) {
      zBlob[i / 2] = (sqlite3HexToInt(z[i]) << 4) | sqlite3HexToInt(z[i + 1]);
    }
    zBlob[i / 2] = 0;
  }
  return zBlob;
}

int sqlite3SafetyCheckOk(sqlite3 *db) {
  u8 eOpenState;
  if (db == 0) {
    logBadConnection("NULL");
    return 0;
  }
  eOpenState = db->eOpenState;
  if (eOpenState != 0x76) {
    if (sqlite3SafetyCheckSickOrOk(db)) {
      logBadConnection("unopened");
    }
    return 0;
  } else {
    return 1;
  }
}

int sqlite3SafetyCheckSickOrOk(sqlite3 *db) {
  u8 eOpenState;
  eOpenState = db->eOpenState;
  if (eOpenState != 0xba && eOpenState != 0x76 && eOpenState != 0x6d) {
    logBadConnection("invalid");
    return 0;
  } else {
    return 1;
  }
}

int sqlite3AddInt64(i64 *pA, i64 iB) {
  return __builtin_add_overflow(*pA, iB, pA);
}

int sqlite3SubInt64(i64 *pA, i64 iB) {
  return __builtin_sub_overflow(*pA, iB, pA);
}

int sqlite3MulInt64(i64 *pA, i64 iB) {
  return __builtin_mul_overflow(*pA, iB, pA);
}

int sqlite3AbsInt32(int x) {
  if (x >= 0)
    return x;
  if (x == (int)0x80000000)
    return 0x7fffffff;
  return -x;
}

LogEst sqlite3LogEstAdd(LogEst a, LogEst b) {
  static const unsigned char x[] = {
      10, 10, 9, 9, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2,
  };
  if (a >= b) {
    if (a > b + 49)
      return a;
    if (a > b + 31)
      return a + 1;
    return a + x[a - b];
  } else {
    if (b > a + 49)
      return b;
    if (b > a + 31)
      return b + 1;
    return b + x[b - a];
  }
}

LogEst sqlite3LogEst(u64 x) {
  static LogEst a[] = {0, 2, 3, 5, 6, 7, 8, 9};
  LogEst y = 40;
  if (x < 8) {
    if (x < 2)
      return 0;
    while (x < 8) {
      y -= 10;
      x <<= 1;
    }
  } else {
    int i = 60 - __builtin_clzll(x);
    y += i * 10;
    x >>= i;
  }
  return a[x & 7] + y - 10;
}

LogEst sqlite3LogEstFromDouble(double x) {
  u64 a;
  LogEst e;

  if (x <= 1)
    return 0;
  if (x <= 2000000000)
    return sqlite3LogEst((u64)x);
  memcpy(&a, &x, 8);
  e = (a >> 52) - 1022;
  return e * 10;
}

u64 sqlite3LogEstToInt(LogEst x) {
  u64 n;
  n = x % 10;
  x /= 10;
  if (n >= 5)
    n -= 2;
  else if (n >= 1)
    n -= 1;
  if (x > 60)
    return (u64)(0xffffffff | (((i64)0x7fffffff) << 32));
  return x >= 3 ? (n + 8) << (x - 3) : (n + 8) >> (3 - x);
}

VList *sqlite3VListAdd(sqlite3 *db, VList *pIn, const char *zName, int nName, int iVal) {
  int nInt;
  char *z;
  int i;

  nInt = nName / 4 + 3;

  if (pIn == 0 || pIn[1] + nInt > pIn[0]) {
    sqlite3_int64 nAlloc = (pIn ? 2 * (sqlite3_int64)pIn[0] : 10) + nInt;
    VList *pOut = sqlite3DbRealloc(db, pIn, nAlloc * sizeof(int));
    if (pOut == 0)
      return pIn;
    if (pIn == 0)
      pOut[1] = 2;
    pIn = pOut;
    pIn[0] = nAlloc;
  }
  i = pIn[1];
  pIn[i] = iVal;
  pIn[i + 1] = nInt;
  z = (char *)&pIn[i + 2];
  pIn[1] = i + nInt;

  memcpy(z, zName, nName);
  z[nName] = 0;
  return pIn;
}

const char *sqlite3OpcodeName(int i) {
  static const char *const azName[] = {
      "Savepoint",
      "AutoCommit",
      "Transaction",
      "Checkpoint",
      "JournalMode",
      "Vacuum",
      "VFilter",
      "VUpdate",
      "Init",
      "Goto",
      "Gosub",
      "InitCoroutine",
      "Yield",
      "MustBeInt",
      "Jump",
      "Once",
      "If",
      "IfNot",
      "IsType",
      "Not",
      "IfNullRow",
      "SeekLT",
      "SeekLE",
      "SeekGE",
      "SeekGT",
      "IfNotOpen",
      "IfNoHope",
      "NoConflict",
      "NotFound",
      "Found",
      "SeekRowid",
      "NotExists",
      "Last",
      "IfSizeBetween",
      "SorterSort",
      "Sort",
      "Rewind",
      "IfEmpty",
      "SorterNext",
      "Prev",
      "Next",
      "IdxLE",
      "IdxGT",
      "Or",
      "And",
      "IdxLT",
      "IdxGE",
      "IFindKey",
      "RowSetRead",
      "RowSetTest",
      "Program",
      "IsNull",
      "NotNull",
      "Ne",
      "Eq",
      "Gt",
      "Le",
      "Lt",
      "Ge",
      "ElseEq",
      "FkIfZero",
      "IfPos",
      "IfNotZero",
      "DecrJumpZero",
      "IncrVacuum",
      "VNext",
      "Filter",
      "PureFunc",
      "Function",
      "Return",
      "EndCoroutine",
      "HaltIfNull",
      "Halt",
      "Integer",
      "Int64",
      "String",
      "BeginSubrtn",
      "Null",
      "SoftNull",
      "Blob",
      "Variable",
      "Move",
      "Copy",
      "SCopy",
      "IntCopy",
      "FkCheck",
      "ResultRow",
      "CollSeq",
      "AddImm",
      "RealAffinity",
      "Cast",
      "Permutation",
      "Compare",
      "IsTrue",
      "ZeroOrNull",
      "Offset",
      "Column",
      "TypeCheck",
      "Affinity",
      "MakeRecord",
      "Count",
      "ReadCookie",
      "SetCookie",
      "BitAnd",
      "BitOr",
      "ShiftLeft",
      "ShiftRight",
      "Add",
      "Subtract",
      "Multiply",
      "Divide",
      "Remainder",
      "Concat",
      "ReopenIdx",
      "OpenRead",
      "BitNot",
      "OpenWrite",
      "OpenDup",
      "String8",
      "OpenAutoindex",
      "OpenEphemeral",
      "SorterOpen",
      "SequenceTest",
      "OpenPseudo",
      "Close",
      "ColumnsUsed",
      "SeekScan",
      "SeekHit",
      "Sequence",
      "NewRowid",
      "Insert",
      "RowCell",
      "Delete",
      "ResetCount",
      "SorterCompare",
      "SorterData",
      "RowData",
      "Rowid",
      "NullRow",
      "SeekEnd",
      "IdxInsert",
      "SorterInsert",
      "IdxDelete",
      "DeferredSeek",
      "IdxRowid",
      "FinishSeek",
      "Destroy",
      "Clear",
      "ResetSorter",
      "CreateBtree",
      "SqlExec",
      "ParseSchema",
      "LoadAnalysis",
      "DropTable",
      "Real",
      "DropIndex",
      "DropTrigger",
      "IntegrityCk",
      "RowSetAdd",
      "Param",
      "FkCounter",
      "MemMax",
      "OffsetLimit",
      "AggInverse",
      "AggStep",
      "AggStep1",
      "AggValue",
      "AggFinal",
      "Expire",
      "CursorLock",
      "CursorUnlock",
      "TableLock",
      "VBegin",
      "VCreate",
      "VDestroy",
      "VOpen",
      "VCheck",
      "VInitIn",
      "VColumn",
      "VRename",
      "Pagecount",
      "MaxPgcnt",
      "ClrSubtype",
      "GetSubtype",
      "SetSubtype",
      "FilterAdd",
      "Trace",
      "CursorHint",
      "ReleaseReg",
      "Noop",
      "Explain",
      "Abortable",
  };
  return azName[i];
}

int sqlite3_os_init(void) {
  static sqlite3_vfs aVfs[] = {

      {
          3,
          sizeof(unixFile),
          512,
          0,
          "unix",
          (void *)&posixIoFinder,
          unixOpen,
          unixDelete,
          unixAccess,
          unixFullPathname,
          unixDlOpen,
          unixDlError,
          unixDlSym,
          unixDlClose,
          unixRandomness,
          unixSleep,
          unixCurrentTime,
          unixGetLastError,
          unixCurrentTimeInt64,
          unixSetSystemCall,
          unixGetSystemCall,
          unixNextSystemCall,
      },
      {
          3,
          sizeof(unixFile),
          512,
          0,
          "unix-none",
          (void *)&nolockIoFinder,
          unixOpen,
          unixDelete,
          unixAccess,
          unixFullPathname,
          unixDlOpen,
          unixDlError,
          unixDlSym,
          unixDlClose,
          unixRandomness,
          unixSleep,
          unixCurrentTime,
          unixGetLastError,
          unixCurrentTimeInt64,
          unixSetSystemCall,
          unixGetSystemCall,
          unixNextSystemCall,
      },
      {
          3,
          sizeof(unixFile),
          512,
          0,
          "unix-dotfile",
          (void *)&dotlockIoFinder,
          unixOpen,
          unixDelete,
          unixAccess,
          unixFullPathname,
          unixDlOpen,
          unixDlError,
          unixDlSym,
          unixDlClose,
          unixRandomness,
          unixSleep,
          unixCurrentTime,
          unixGetLastError,
          unixCurrentTimeInt64,
          unixSetSystemCall,
          unixGetSystemCall,
          unixNextSystemCall,
      },
      {
          3,
          sizeof(unixFile),
          512,
          0,
          "unix-excl",
          (void *)&posixIoFinder,
          unixOpen,
          unixDelete,
          unixAccess,
          unixFullPathname,
          unixDlOpen,
          unixDlError,
          unixDlSym,
          unixDlClose,
          unixRandomness,
          unixSleep,
          unixCurrentTime,
          unixGetLastError,
          unixCurrentTimeInt64,
          unixSetSystemCall,
          unixGetSystemCall,
          unixNextSystemCall,
      },
  };
  unsigned int i;

  for (i = 0; i < (sizeof(aVfs) / sizeof(sqlite3_vfs)); i++) {
    sqlite3_vfs_register(&aVfs[i], i == 0);
  }

  unixBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);

  unixTempFileInit();

  return SQLITE_OK;
}

int sqlite3_os_end(void) {
  unixBigLock = 0;

  return 0;
}

MemFile *memdbFromDbSchema(sqlite3 *db, const char *zSchema) {
  MemFile *p = 0;
  MemStore *pStore;
  int rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  if (rc)
    return 0;
  if (p->base.pMethods != &memdb_io_methods)
    return 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  if (pStore->zFName != 0)
    p = 0;
  memdbLeave(pStore);
  return p;
}

unsigned char *sqlite3_serialize(sqlite3 *db, const char *zSchema, sqlite3_int64 *piSize, unsigned int mFlags) {
  MemFile *p;
  int iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  int szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned char *pOut = 0;
  char *zSql;
  int rc;

  sqlite3_mutex_enter(db->mutex);

  if (zSchema == 0)
    zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  if (piSize)
    *piSize = -1;
  if (iDb < 0)
    goto serialize_out;
  if (p) {
    MemStore *pStore = p->pStore;

    if (piSize)
      *piSize = pStore->sz;
    if (mFlags & SQLITE_SERIALIZE_NOCOPY) {
      pOut = pStore->aData;
    } else {
      pOut = sqlite3_malloc64(pStore->sz);
      if (pOut)
        memcpy(pOut, pStore->aData, pStore->sz);
    }
    goto serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  if (pBt == 0)
    goto serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  if (rc)
    goto serialize_out;
  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_ROW) {
    sz = sqlite3_column_int64(pStmt, 0) * szPage;
    if (sz == 0) {
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      if (rc == SQLITE_ROW) {
        sz = sqlite3_column_int64(pStmt, 0) * szPage;
      }
    }
    if (piSize)
      *piSize = sz;
    if (mFlags & SQLITE_SERIALIZE_NOCOPY) {
      pOut = 0;
    } else {
      pOut = sqlite3_malloc64(sz);
      if (pOut) {
        int nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        int pgno;
        for (pgno = 1; pgno <= nPage; pgno++) {
          DbPage *pPage = 0;
          unsigned char *pTo = pOut + szPage * (sqlite3_int64)(pgno - 1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage **)&pPage, 0);
          if (rc == SQLITE_OK) {
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          } else {
            memset(pTo, 0, szPage);
          }
          sqlite3PagerUnref(pPage);
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

serialize_out:
  sqlite3_mutex_leave(db->mutex);
  return pOut;
}

int sqlite3_deserialize(sqlite3 *db, const char *zSchema, unsigned char *pData, sqlite3_int64 szDb, sqlite3_int64 szBuf,
                        unsigned mFlags) {
  MemFile *p;
  char *zSql;
  sqlite3_stmt *pStmt = 0;
  int rc;
  int iDb;

  sqlite3_mutex_enter(db->mutex);
  if (zSchema == 0)
    zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  if (iDb < 2 && iDb != 0) {
    rc = SQLITE_ERROR;
    goto end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  if (zSql == 0) {
    rc = SQLITE_NOMEM;
  } else {
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  if (rc)
    goto end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  if (rc != SQLITE_OK) {
    goto end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  if (p == 0) {
    rc = SQLITE_ERROR;
  } else {
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    if (pStore->szMax < sqlite3Config.mxMemdbSize) {
      pStore->szMax = sqlite3Config.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  if (pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE) != 0) {
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

static int sqlite3MemdbInit(void) {
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned int sz;
  if (pLower == 0)
    return SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;

  if (sz < sizeof(MemFile))
    sz = sizeof(MemFile);
  memdb_vfs.szOsFile = sz;
  return sqlite3_vfs_register(&memdb_vfs, 0);
}

static int sqlite3PcacheInitialize(void) {
  if (sqlite3Config.pcache2.xInit == 0) {
    sqlite3PCacheSetDefault();
  }
  return sqlite3Config.pcache2.xInit(sqlite3Config.pcache2.pArg);
}

static void sqlite3PcacheShutdown(void) {
  if (sqlite3Config.pcache2.xShutdown) {
    sqlite3Config.pcache2.xShutdown(sqlite3Config.pcache2.pArg);
  }
}

int sqlite3PcacheSize(void) {
  return sizeof(PCache);
}

int sqlite3PcacheOpen(int szPage, int szExtra, int bPurgeable, int (*xStress)(void *, PgHdr *), void *pStress,
                      PCache *p) {
  memset(p, 0, sizeof(PCache));
  p->szPage = 1;
  p->szExtra = szExtra;

  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  return sqlite3PcacheSetPageSize(p, szPage);
}

static int sqlite3HeaderSizePcache(void) {
  return (((sizeof(PgHdr)) + 7) & ~7);
}

void *sqlite3PageMalloc(int sz) {
  return pcache1Alloc(sz);
}

void sqlite3PageFree(void *p) {
  pcache1Free(p);
}

static int sqlite3HeaderSizePcache1(void) {
  return (((sizeof(PgHdr1)) + 7) & ~7);
}

sqlite3_mutex *sqlite3Pcache1Mutex(void) {
  return (pcache1_g).mutex;
}

RowSet *sqlite3RowSetInit(sqlite3 *db) {
  RowSet *p = sqlite3DbMallocRawNN(db, sizeof(*p));
  if (p) {
    int N = sqlite3DbMallocSize(db, p);
    p->pChunk = 0;
    p->db = db;
    p->pEntry = 0;
    p->pLast = 0;
    p->pForest = 0;
    p->pFresh = (struct RowSetEntry *)((((sizeof(*p)) + 7) & ~7) + (char *)p);
    p->nFresh = (u16)((N - (((sizeof(*p)) + 7) & ~7)) / sizeof(struct RowSetEntry));
    p->rsFlags = 0x01;
    p->iBatch = 0;
  }
  return p;
}

sqlite3_file *sqlite3_database_file_object(const char *zName) {
  Pager *pPager;
  const char *p;
  while (zName[-1] != 0 || zName[-2] != 0 || zName[-3] != 0 || zName[-4] != 0) {
    zName--;
  }
  p = zName - 4 - sizeof(Pager *);

  pPager = *(Pager **)p;
  return pPager->fd;
}

void __attribute__((noinline)) btreeEnterAll(sqlite3 *db) {
  int i;
  u8 skipOk = 1;
  Btree *p;

  for (i = 0; i < db->nDb; i++) {
    p = db->aDb[i].pBt;
    if (p && p->sharable) {
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}

void sqlite3BtreeEnterAll(sqlite3 *db) {
  if (db->noSharedCache == 0)
    btreeEnterAll(db);
}

void __attribute__((noinline)) btreeLeaveAll(sqlite3 *db) {
  int i;
  Btree *p;

  for (i = 0; i < db->nDb; i++) {
    p = db->aDb[i].pBt;
    if (p)
      sqlite3BtreeLeave(p);
  }
}

void sqlite3BtreeLeaveAll(sqlite3 *db) {
  if (db->noSharedCache == 0)
    btreeLeaveAll(db);
}

int sqlite3_enable_shared_cache(int enable) {
  sqlite3Config.sharedCacheEnabled = enable;
  return SQLITE_OK;
}

int sqlite3BtreeCount(sqlite3 *db, BtCursor *pCur, i64 *pnEntry) {
  i64 nEntry = 0;
  int rc;

  rc = moveToRoot(pCur);
  if (rc == SQLITE_EMPTY) {
    *pnEntry = 0;
    return SQLITE_OK;
  }

  while (rc == SQLITE_OK && !__atomic_load_n((&db->u1.isInterrupted), 0)) {
    int iIdx;
    MemPage *pPage;

    pPage = pCur->pPage;
    if (pPage->leaf || !pPage->intKey) {
      nEntry += pPage->nCell;
    }

    if (pPage->leaf) {
      do {
        if (pCur->iPage == 0) {
          *pnEntry = nEntry;
          return moveToRoot(pCur);
        }
        moveToParent(pCur);
      } while (pCur->ix >= pCur->pPage->nCell);

      pCur->ix++;
      pPage = pCur->pPage;
    }

    iIdx = pCur->ix;
    if (iIdx == pPage->nCell) {
      rc = moveToChild(pCur, sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]));
    } else {
      rc = moveToChild(
          pCur, sqlite3Get4byte(((pPage)->aData +
                                 ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (iIdx)]))))));
    }
  }

  return rc;
}

int sqlite3BtreeIntegrityCheck(sqlite3 *db, Btree *p, Pgno *aRoot, Mem *aCnt, int nRoot, int mxErr, int *pnErr,
                               char **pzOut) {
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  char zErr[100];
  int bPartial = 0;
  int bCkFreelist = 1;

  if (aRoot[0] == 0) {
    bPartial = 1;
    if (aRoot[1] != 1)
      bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);

  memset(&sCheck, 0, sizeof(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.nCkPage = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.errMsg, 0, zErr, sizeof(zErr), 1000000000);
  sCheck.errMsg.printfFlags = 0x01;
  if (sCheck.nCkPage == 0) {
    goto integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.nCkPage / 8) + 1);
  if (!sCheck.aPgRef) {
    checkOom(&sCheck);
    goto integrity_ck_cleanup;
  }
  sCheck.heap = (u32 *)sqlite3PageMalloc(pBt->pageSize);

  if (sCheck.heap == 0) {
    checkOom(&sCheck);
    goto integrity_ck_cleanup;
  }

  i = ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1));
  if (i <= sCheck.nCkPage)
    setPageReferenced(&sCheck, i);

  if (bCkFreelist) {
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, sqlite3Get4byte(&pBt->pPage1->aData[32]), sqlite3Get4byte(&pBt->pPage1->aData[36]));
    sCheck.zPfx = 0;
  }

  if (!bPartial) {
    if (pBt->autoVacuum) {
      Pgno mx = 0;
      Pgno mxInHdr;
      for (i = 0; (int)i < nRoot; i++)
        if (mx < aRoot[i])
          mx = aRoot[i];
      mxInHdr = sqlite3Get4byte(&pBt->pPage1->aData[52]);
      if (mx != mxInHdr) {
        checkAppendMsg(&sCheck, "max rootpage (%u) disagrees with header (%u)", mx, mxInHdr);
      }
    } else if (sqlite3Get4byte(&pBt->pPage1->aData[64]) != 0) {
      checkAppendMsg(&sCheck, "incremental_vacuum enabled with a max rootpage of zero");
    }
  }

  pBt->db->flags &= ~(u64)0x00200000;
  for (i = 0; (int)i < nRoot && sCheck.mxErr; i++) {
    sCheck.nRow = 0;
    if (aRoot[i]) {
      i64 notUsed;

      if (pBt->autoVacuum && aRoot[i] > 1 && !bPartial) {
        checkPtrmap(&sCheck, aRoot[i], 1, 0);
      }

      sCheck.v0 = aRoot[i];
      checkTreePage(&sCheck, aRoot[i], &notUsed, (0xffffffff | (((i64)0x7fffffff) << 32)));
    }
    sqlite3MemSetArrayInt64(aCnt, i, sCheck.nRow);
  }
  pBt->db->flags = savedDbFlags;

  if (!bPartial) {
    for (i = 1; i <= sCheck.nCkPage && sCheck.mxErr; i++) {
      if (getPageReferenced(&sCheck, i) == 0 && (ptrmapPageno(pBt, i) != i || !pBt->autoVacuum)) {
        checkAppendMsg(&sCheck, "Page %u: never used", i);
      }
      if (getPageReferenced(&sCheck, i) != 0 && (ptrmapPageno(pBt, i) == i && pBt->autoVacuum)) {
        checkAppendMsg(&sCheck, "Page %u: pointer map referenced", i);
      }
    }
  }

integrity_ck_cleanup:
  sqlite3PageFree(sCheck.heap);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  if (sCheck.nErr == 0) {
    sqlite3_str_reset(&sCheck.errMsg);
    *pzOut = 0;
  } else {
    *pzOut = sqlite3StrAccumFinish(&sCheck.errMsg);
  }

  sqlite3BtreeLeave(p);
  return sCheck.rc;
}

static int sqlite3HeaderSizeBtree(void) {
  return (((sizeof(MemPage)) + 7) & ~7);
}

Btree *findBtree(sqlite3 *pErrorDb, sqlite3 *pDb, const char *zDb) {
  int i = sqlite3FindDbName(pDb, zDb);

  if (i == 1) {
    Parse sParse;
    int rc = 0;
    sqlite3ParseObjectInit(&sParse, pDb);
    if (sqlite3OpenTempDatabase(&sParse)) {
      sqlite3ErrorWithMsg(pErrorDb, sParse.rc, "%s", sParse.zErrMsg);
      rc = SQLITE_ERROR;
    }
    sqlite3DbFree(pErrorDb, sParse.zErrMsg);
    sqlite3ParseObjectReset(&sParse);
    if (rc) {
      return 0;
    }
  }

  if (i < 0) {
    sqlite3ErrorWithMsg(pErrorDb, SQLITE_ERROR, "unknown database %s", zDb);
    return 0;
  }

  return pDb->aDb[i].pBt;
}

int checkReadTransaction(sqlite3 *db, Btree *p) {
  if (sqlite3BtreeTxnState(p) != SQLITE_TXN_NONE) {
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, "destination database is in use");
    return SQLITE_ERROR;
  }
  return SQLITE_OK;
}

sqlite3_backup *sqlite3_backup_init(sqlite3 *pDestDb, const char *zDestDb, sqlite3 *pSrcDb, const char *zSrcDb) {
  sqlite3_backup *p;

  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3_mutex_enter(pDestDb->mutex);

  if (pSrcDb == pDestDb) {
    sqlite3ErrorWithMsg(pDestDb, SQLITE_ERROR, "source and destination must be distinct");
    p = 0;
  } else {
    int nDest = sqlite3Strlen30(zDestDb);

    p = (sqlite3_backup *)sqlite3MallocZero(sizeof(sqlite3_backup) + nDest + 1);
    if (!p) {
      sqlite3Error(pDestDb, 7);
    } else {
      p->zDestDb = (char *)&p[1];
      memcpy(p->zDestDb, zDestDb, nDest);
    }
  }

  if (p) {
    Btree *pDest = findBtree(pDestDb, pDestDb, zDestDb);
    p->pSrc = findBtree(pDestDb, pSrcDb, zSrcDb);
    p->pDestDb = pDestDb;
    p->pSrcDb = pSrcDb;
    p->iNext = 1;
    p->isAttached = 0;

    if (0 == p->pSrc || 0 == pDest || checkReadTransaction(pDestDb, pDest) != SQLITE_OK) {
      sqlite3_free(p);
      p = 0;
    }
  }
  if (p) {
    p->pSrc->nBackup++;
  }

  sqlite3_mutex_leave(pDestDb->mutex);
  sqlite3_mutex_leave(pSrcDb->mutex);
  return p;
}

int sqlite3RealSameAsInt(double r1, sqlite3_int64 i) {
  double r2 = (double)i;
  return r1 == 0.0 || (memcmp(&r1, &r2, sizeof(r1)) == 0 && i >= -2251799813685248LL && i < 2251799813685248LL);
}

i64 sqlite3RealToI64(double r) {
  if (r < -9223372036854774784.0)
    return (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32)));
  if (r > +9223372036854774784.0)
    return (0xffffffff | (((i64)0x7fffffff) << 32));
  return (i64)r;
}

void sqlite3NoopDestructor(void *p) {
  (void)(p);
}

sqlite3_value *sqlite3ValueNew(sqlite3 *db) {
  Mem *p = sqlite3DbMallocZero(db, sizeof(*p));
  if (p) {
    p->flags = 0x0001;
    p->db = db;
  }
  return p;
}

sqlite3_value *valueNew(sqlite3 *db, struct ValueNewStat4Ctx *p) {
  (void)(p);

  return sqlite3ValueNew(db);
}

int valueFromExpr(sqlite3 *db, const Expr *pExpr, u8 enc, u8 affinity, sqlite3_value **ppVal,
                  struct ValueNewStat4Ctx *pCtx) {
  int op;
  char *zVal = 0;
  sqlite3_value *pVal = 0;
  int negInt = 1;
  const char *zNeg = "";
  int rc = 0;

  while ((op = pExpr->op) == 173 || op == 181)
    pExpr = pExpr->pLeft;
  if (op == 176)
    op = pExpr->op2;

  if (op == 36) {
    u8 aff;

    aff = sqlite3AffinityType(pExpr->u.zToken, 0);
    rc = valueFromExpr(db, pExpr->pLeft, enc, aff, ppVal, pCtx);
    if (*ppVal) {
      sqlite3VdbeMemCast(*ppVal, aff, enc);
      sqlite3ValueApplyAffinity(*ppVal, affinity, enc);
    }
    return rc;
  }

  if (op == 174) {
    Expr *pLeft = pExpr->pLeft;
    if ((pLeft->op == 156 || pLeft->op == 154)) {
      if ((((pLeft)->flags & (u32)(0x000800)) != 0) || pLeft->u.zToken[0] != '0' ||
          (pLeft->u.zToken[1] & ~0x20) != 'X') {
        pExpr = pLeft;
        op = pExpr->op;
        negInt = -1;
        zNeg = "-";
      }
    }
  }

  if (op == 118 || op == 154 || op == 156) {
    pVal = valueNew(db, pCtx);
    if (pVal == 0)
      goto no_mem;
    if ((((pExpr)->flags & (u32)(0x000800)) != 0)) {
      sqlite3VdbeMemSetInt64(pVal, (i64)pExpr->u.iValue * negInt);
    } else {
      i64 iVal;
      if (op == 156 && 0 == sqlite3DecOrHexToI64(pExpr->u.zToken, &iVal)) {
        sqlite3VdbeMemSetInt64(pVal, iVal * negInt);
      } else {
        zVal = sqlite3MPrintf(db, "%s%s", zNeg, pExpr->u.zToken);
        if (zVal == 0)
          goto no_mem;
        sqlite3ValueSetStr(pVal, -1, zVal, SQLITE_UTF8, ((sqlite3_destructor_type)sqlite3RowSetClear));
      }
    }
    if (affinity == 0x41) {
      if (op == 154) {
        sqlite3AtoF(pVal->z, &pVal->u.r);
        pVal->flags = 0x0008;
      } else if (op == 156) {
        sqlite3ValueApplyAffinity(pVal, 0x43, SQLITE_UTF8);
      }
    } else {
      sqlite3ValueApplyAffinity(pVal, affinity, SQLITE_UTF8);
    }

    if (pVal->flags & (0x0004 | 0x0020 | 0x0008)) {
      pVal->flags &= ~0x0002;
    }
    if (enc != SQLITE_UTF8) {
      rc = sqlite3VdbeChangeEncoding(pVal, enc);
    }
  } else if (op == 174) {
    if (SQLITE_OK == valueFromExpr(db, pExpr->pLeft, enc, affinity, &pVal, pCtx) && pVal != 0) {
      sqlite3VdbeMemNumerify(pVal);
      if (pVal->flags & 0x0008) {
        pVal->u.r = -pVal->u.r;
      } else if (pVal->u.i == (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32)))) {
        pVal->u.r = -(double)(((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32)));

        ((pVal)->flags = ((pVal)->flags & ~(0x0dbf | 0x0400)) | 0x0008);
      } else {
        pVal->u.i = -pVal->u.i;
      }
      sqlite3ValueApplyAffinity(pVal, affinity, enc);
    }
  } else if (op == 122) {
    pVal = valueNew(db, pCtx);
    if (pVal == 0)
      goto no_mem;
    sqlite3VdbeMemSetNull(pVal);
  }

  else if (op == 155) {
    int nVal;

    pVal = valueNew(db, pCtx);
    if (!pVal)
      goto no_mem;
    zVal = &pExpr->u.zToken[2];
    nVal = sqlite3Strlen30(zVal) - 1;

    sqlite3VdbeMemSetStr(pVal, sqlite3HexToBlob(db, zVal, nVal), nVal / 2, 0,
                         ((sqlite3_destructor_type)sqlite3RowSetClear));
  }

  else if (op == 171) {
    pVal = valueNew(db, pCtx);
    if (pVal) {
      pVal->flags = 0x0004;
      pVal->u.i = pExpr->u.zToken[4] == 0;
      sqlite3ValueApplyAffinity(pVal, affinity, enc);
    }
  }

  *ppVal = pVal;
  return rc;

no_mem:
  sqlite3OomFault(db);
  sqlite3DbFree(db, zVal);

  sqlite3ValueFree(pVal);

  return 7;
}

int sqlite3ValueFromExpr(sqlite3 *db, const Expr *pExpr, u8 enc, u8 affinity, sqlite3_value **ppVal) {
  return pExpr ? valueFromExpr(db, pExpr, enc, affinity, ppVal, 0) : 0;
}

void freeEphemeralFunction(sqlite3 *db, FuncDef *pDef) {
  if ((pDef->funcFlags & 0x0010) != 0) {
    sqlite3DbNNFreeNN(db, pDef);
  }
}

__attribute__((noinline)) void freeP4Mem(sqlite3 *db, Mem *p) {
  if (p->szMalloc)
    sqlite3DbFree(db, p->zMalloc);
  sqlite3DbNNFreeNN(db, p);
}

__attribute__((noinline)) void freeP4FuncCtx(sqlite3 *db, sqlite3_context *p) {
  freeEphemeralFunction(db, p->pFunc);
  sqlite3DbNNFreeNN(db, p);
}

void freeP4(sqlite3 *db, int p4type, void *p4) {
  switch (p4type) {
    case (-16): {
      freeP4FuncCtx(db, (sqlite3_context *)p4);
      break;
    }
    case (-13):
    case (-14):
    case (-7):
    case (-15): {
      if (p4)
        sqlite3DbNNFreeNN(db, p4);
      break;
    }
    case (-9): {
      if (db->pnBytesFreed == 0)
        sqlite3KeyInfoUnref((KeyInfo *)p4);
      break;
    }

    case (-8): {
      freeEphemeralFunction(db, (FuncDef *)p4);
      break;
    }
    case (-11): {
      if (db->pnBytesFreed == 0) {
        sqlite3ValueFree((sqlite3_value *)p4);
      } else {
        freeP4Mem(db, (Mem *)p4);
      }
      break;
    }
    case (-12): {
      if (db->pnBytesFreed == 0)
        sqlite3VtabUnlock((VTable *)p4);
      break;
    }
    case (-17): {
      if (db->pnBytesFreed == 0)
        sqlite3DeleteTable(db, (Table *)p4);
      break;
    }
    case (-18): {
      SubrtnSig *pSig = (SubrtnSig *)p4;
      sqlite3DbFree(db, pSig->zAff);
      sqlite3DbFree(db, pSig);
      break;
    }
  }
}

void vdbeFreeOpArray(sqlite3 *db, Op *aOp, int nOp) {
  if (aOp) {
    Op *pOp = &aOp[nOp - 1];
    while (1) {
      if (pOp->p4type <= (-7))
        freeP4(db, pOp->p4type, pOp->p4.p);

      if (pOp == aOp)
        break;
      pOp--;
    }
    sqlite3DbNNFreeNN(db, aOp);
  }
}

char *sqlite3VdbeDisplayP4(sqlite3 *db, Op *pOp) {
  char *zP4 = 0;
  StrAccum x;

  sqlite3StrAccumInit(&x, 0, 0, 0, 1000000000);
  switch (pOp->p4type) {
    case (-9): {
      int j;
      KeyInfo *pKeyInfo = pOp->p4.pKeyInfo;

      sqlite3_str_appendf(&x, "k(%d", pKeyInfo->nKeyField);
      for (j = 0; j < pKeyInfo->nKeyField; j++) {
        CollSeq *pColl = pKeyInfo->aColl[j];
        const char *zColl = pColl ? pColl->zName : "";
        if (strcmp(zColl, "BINARY") == 0)
          zColl = "B";
        sqlite3_str_appendf(&x, ",%s%s%s", (pKeyInfo->aSortFlags[j] & 0x01) ? "-" : "",
                            (pKeyInfo->aSortFlags[j] & 0x02) ? "N." : "", zColl);
      }
      sqlite3_str_append(&x, ")", 1);
      break;
    }

    case (-2): {
      static const char *const encnames[] = {"?", "8", "16LE", "16BE"};
      CollSeq *pColl = pOp->p4.pColl;

      sqlite3_str_appendf(&x, "%.18s-%s", pColl->zName, encnames[pColl->enc]);
      break;
    }
    case (-8): {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      break;
    }
    case (-16): {
      FuncDef *pDef = pOp->p4.pCtx->pFunc;
      sqlite3_str_appendf(&x, "%s(%d)", pDef->zName, pDef->nArg);
      break;
    }
    case (-14): {
      sqlite3_str_appendf(&x, "%lld", *pOp->p4.pI64);
      break;
    }
    case (-3): {
      sqlite3_str_appendf(&x, "%d", pOp->p4.i);
      break;
    }
    case (-13): {
      sqlite3_str_appendf(&x, "%.16g", *pOp->p4.pReal);
      break;
    }
    case (-11): {
      Mem *pMem = pOp->p4.pMem;
      if (pMem->flags & 0x0002) {
        zP4 = pMem->z;
      } else if (pMem->flags & (0x0004 | 0x0020)) {
        sqlite3_str_appendf(&x, "%lld", pMem->u.i);
      } else if (pMem->flags & 0x0008) {
        sqlite3_str_appendf(&x, "%.16g", pMem->u.r);
      } else if (pMem->flags & 0x0001) {
        zP4 = "NULL";
      } else {
        zP4 = "(blob)";
      }
      break;
    }

    case (-12): {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_str_appendf(&x, "vtab:%p", pVtab);
      break;
    }

    case (-15): {
      u32 i;
      u32 *ai = pOp->p4.ai;
      u32 n = ai[0];

      for (i = 1; i <= n; i++) {
        sqlite3_str_appendf(&x, "%c%u", (i == 1 ? '[' : ','), ai[i]);
      }
      sqlite3_str_append(&x, "]", 1);
      break;
    }
    case (-4): {
      zP4 = "program";
      break;
    }
    case (-5): {
      zP4 = pOp->p4.pTab->zName;
      break;
    }
    case (-6): {
      zP4 = pOp->p4.pIdx->zName;
      break;
    }
    case (-18): {
      SubrtnSig *pSig = pOp->p4.pSubrtnSig;
      sqlite3_str_appendf(&x, "subrtnsig:%d,%s", pSig->selId, pSig->zAff);
      break;
    }
    default: {
      zP4 = pOp->p4.z;
    }
  }
  if (zP4)
    sqlite3_str_appendall(&x, zP4);
  if ((x.accError & SQLITE_NOMEM) != 0) {
    sqlite3OomFault(db);
  }
  return sqlite3StrAccumFinish(&x);
}

int vdbeCommit(sqlite3 *db, Vdbe *p) {
  int i;
  int nTrans = 0;

  int rc = SQLITE_OK;
  int needXcommit = 0;

  rc = sqlite3VtabSync(db, p);

  for (i = 0; rc == SQLITE_OK && i < db->nDb; i++) {
    Btree *pBt = db->aDb[i].pBt;
    if (sqlite3BtreeTxnState(pBt) == SQLITE_TXN_WRITE) {
      static const u8 aMJNeeded[] = {1, 1, 0, 1, 0, 0};
      Pager *pPager;
      needXcommit = 1;
      sqlite3BtreeEnter(pBt);
      pPager = sqlite3BtreePager(pBt);
      if (db->aDb[i].safety_level != 0x01 && aMJNeeded[sqlite3PagerGetJournalMode(pPager)] &&
          sqlite3PagerIsMemdb(pPager) == 0) {
        nTrans++;
      }
      rc = sqlite3PagerExclusiveLock(pPager);
      sqlite3BtreeLeave(pBt);
    }
  }
  if (rc != SQLITE_OK) {
    return rc;
  }

  if (needXcommit && db->xCommitCallback) {
    rc = db->xCommitCallback(db->pCommitArg);
    if (rc) {
      return (19 | (2 << 8));
    }
  }

  if (0 == sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt)) || nTrans <= 1) {
    if (needXcommit) {
      for (i = 0; rc == SQLITE_OK && i < db->nDb; i++) {
        Btree *pBt = db->aDb[i].pBt;
        if (sqlite3BtreeTxnState(pBt) >= SQLITE_TXN_WRITE) {
          rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
        }
      }
    }

    for (i = 0; rc == SQLITE_OK && i < db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      int txn = sqlite3BtreeTxnState(pBt);
      if (txn != SQLITE_TXN_NONE) {
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    if (rc == SQLITE_OK) {
      sqlite3VtabCommit(db);
    }
  }

  else {
    sqlite3_vfs *pVfs = db->pVfs;
    char *zSuper = 0;
    char const *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pSuperJrnl = 0;
    i64 offset = 0;
    int res;
    int retryCount = 0;
    int nMainFile;

    nMainFile = sqlite3Strlen30(zMainFile);
    zSuper = sqlite3MPrintf(db, "%.4c%s%.16c", 0, zMainFile, 0);
    if (zSuper == 0)
      return 7;
    zSuper += 4;
    do {
      u32 iRandom;
      if (retryCount) {
        if (retryCount > 100) {
          sqlite3_log(SQLITE_FULL, "MJ delete: %s", zSuper);
          sqlite3OsDelete(pVfs, zSuper, 0);
          break;
        } else if (retryCount == 1) {
          sqlite3_log(SQLITE_FULL, "MJ collide: %s", zSuper);
        }
      }
      retryCount++;
      sqlite3_randomness(sizeof(iRandom), &iRandom);
      sqlite3_snprintf(13, &zSuper[nMainFile], "-mj%06X9%02X", (iRandom >> 8) & 0xffffff, iRandom & 0xff);

      rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
    } while (rc == SQLITE_OK && res);
    if (rc == SQLITE_OK) {
      rc = sqlite3OsOpenMalloc(
          pVfs, zSuper, &pSuperJrnl,
          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_SUPER_JOURNAL, 0);
    }
    if (rc != SQLITE_OK) {
      sqlite3DbFree(db, zSuper - 4);
      return rc;
    }

    for (i = 0; i < db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      if (sqlite3BtreeTxnState(pBt) == SQLITE_TXN_WRITE) {
        char const *zFile = sqlite3BtreeGetJournalname(pBt);
        if (zFile == 0) {
          continue;
        }

        rc = sqlite3OsWrite(pSuperJrnl, zFile, sqlite3Strlen30(zFile) + 1, offset);
        offset += sqlite3Strlen30(zFile) + 1;
        if (rc != SQLITE_OK) {
          sqlite3OsCloseFree(pSuperJrnl);
          sqlite3OsDelete(pVfs, zSuper, 0);
          sqlite3DbFree(db, zSuper - 4);
          return rc;
        }
      }
    }

    if (0 == (sqlite3OsDeviceCharacteristics(pSuperJrnl) & SQLITE_IOCAP_SEQUENTIAL) &&
        SQLITE_OK != (rc = sqlite3OsSync(pSuperJrnl, SQLITE_SYNC_NORMAL))) {
      sqlite3OsCloseFree(pSuperJrnl);
      sqlite3OsDelete(pVfs, zSuper, 0);
      sqlite3DbFree(db, zSuper - 4);
      return rc;
    }

    for (i = 0; rc == SQLITE_OK && i < db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      if (pBt) {
        rc = sqlite3BtreeCommitPhaseOne(pBt, zSuper);
      }
    }
    sqlite3OsCloseFree(pSuperJrnl);

    if (rc != SQLITE_OK) {
      sqlite3DbFree(db, zSuper - 4);
      return rc;
    }

    rc = sqlite3OsDelete(pVfs, zSuper, 1);
    sqlite3DbFree(db, zSuper - 4);
    zSuper = 0;
    if (rc) {
      return rc;
    }

    sqlite3BeginBenignMalloc();
    for (i = 0; i < db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      if (pBt) {
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3EndBenignMalloc();

    sqlite3VtabCommit(db);
  }

  return rc;
}

void sqlite3VdbeDeleteAuxData(sqlite3 *db, AuxData **pp, int iOp, int mask) {
  while (*pp) {
    AuxData *pAux = *pp;
    if ((iOp < 0) || (pAux->iAuxOp == iOp && pAux->iAuxArg >= 0 &&
                      (pAux->iAuxArg > 31 || !(mask & (((unsigned int)1) << (pAux->iAuxArg)))))) {
      if (pAux->xDeleteAux) {
        pAux->xDeleteAux(pAux->pAux);
      }
      *pp = pAux->pNextAux;
      sqlite3DbFree(db, pAux);
    } else {
      pp = &pAux->pNextAux;
    }
  }
}

void sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p) {
  SubProgram *pSub, *pNext;

  if (p->aColName) {
    releaseMemArray(p->aColName, p->nResAlloc * 2);
    sqlite3DbNNFreeNN(db, p->aColName);
  }
  for (pSub = p->pProgram; pSub; pSub = pNext) {
    pNext = pSub->pNext;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  if (p->eVdbeState != 0) {
    releaseMemArray(p->aVar, p->nVar);
    if (p->pVList)
      sqlite3DbNNFreeNN(db, p->pVList);
    if (p->pFree)
      sqlite3DbNNFreeNN(db, p->pFree);
  }
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  if (p->zSql)
    sqlite3DbNNFreeNN(db, p->zSql);
}

const u8 sqlite3SmallTypeSizes[128] = {

    0,  1,  2,  3,  4,  6,  8,  8,  0,  0,  0,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,
    7,  7,  8,  8,  9,  9,  10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
    20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32,
    33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45,
    46, 46, 47, 47, 48, 48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57};

int sqlite3IntFloatCompare(i64 i, double r) {
  if (sqlite3IsNaN(r)) {
    return 1;
  } else {
    i64 y;
    if (r < -9223372036854775808.0)
      return +1;
    if (r >= 9223372036854775808.0)
      return -1;
    y = (i64)r;
    if (i < y)
      return -1;
    if (i > y)
      return +1;
    return (((double)i) < r) ? -1 : (((double)i) > r);
  }
}

int sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid) {
  i64 nCellKey = 0;
  int rc;
  u32 szHdr;
  u32 typeRowid;
  u32 lenRowid;
  Mem m, v;

  nCellKey = sqlite3BtreePayloadSize(pCur);

  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)nCellKey, &m);
  if (rc) {
    return rc;
  }

  szHdr = (u32) * ((u8 *)m.z);
  if (szHdr >= 0x80)
    sqlite3GetVarint32(((u8 *)m.z), (u32 *)&(szHdr));

  if ((szHdr < 3 || szHdr > (unsigned)m.n)) {
    goto idx_rowid_corruption;
  }

  typeRowid = (u32) * ((u8 *)&m.z[szHdr - 1]);
  if (typeRowid >= 0x80)
    sqlite3GetVarint32(((u8 *)&m.z[szHdr - 1]), (u32 *)&(typeRowid));
  if ((typeRowid < 1 || typeRowid > 9 || typeRowid == 7)) {
    goto idx_rowid_corruption;
  }
  lenRowid = sqlite3SmallTypeSizes[typeRowid];
  if (((u32)m.n < szHdr + lenRowid)) {
    goto idx_rowid_corruption;
  }

  sqlite3VdbeSerialGet((u8 *)&m.z[m.n - lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemReleaseMalloc(&m);
  return SQLITE_OK;

idx_rowid_corruption:;
  sqlite3VdbeMemReleaseMalloc(&m);
  return sqlite3CorruptError(93130);
}

int sqlite3VdbeIdxKeyCompare(sqlite3 *db, VdbeCursor *pC, UnpackedRecord *pUnpacked, int *res) {
  i64 nCellKey = 0;
  int rc;
  BtCursor *pCur;
  Mem m;

  pCur = pC->uc.pCursor;

  nCellKey = sqlite3BtreePayloadSize(pCur);

  if (nCellKey <= 0 || nCellKey > 0x7fffffff) {
    *res = 0;
    return sqlite3CorruptError(93163);
  }
  sqlite3VdbeMemInit(&m, db, 0);
  rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)nCellKey, &m);
  if (rc) {
    return rc;
  }
  *res = sqlite3VdbeRecordCompareWithSkip(m.n, m.z, pUnpacked, 0);
  sqlite3VdbeMemReleaseMalloc(&m);
  return SQLITE_OK;
}

void sqlite3VdbeSetChanges(sqlite3 *db, i64 nChange) {
  db->nChange = nChange;
  db->nTotalChange += nChange;
}

void sqlite3ExpirePreparedStatements(sqlite3 *db, int iCode) {
  Vdbe *p;
  for (p = db->pVdbe; p; p = p->pVNext) {
    p->expired = iCode + 1;
  }
}

__attribute__((noinline)) void invokeProfileCallback(sqlite3 *db, Vdbe *p) {
  sqlite3_int64 iNow;
  sqlite3_int64 iElapse;

  sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
  iElapse = (iNow - p->startTime) * 1000000;

  if (db->xProfile) {
    db->xProfile(db->pProfileArg, p->zSql, iElapse);
  }

  if (db->mTrace & SQLITE_TRACE_PROFILE) {
    db->trace.xV2(SQLITE_TRACE_PROFILE, db->pTraceArg, p, (void *)&iElapse);
  }
  p->startTime = 0;
}

int doWalCallbacks(sqlite3 *db) {
  int rc = SQLITE_OK;

  int i;
  for (i = 0; i < db->nDb; i++) {
    Btree *pBt = db->aDb[i].pBt;
    if (pBt) {
      int nEntry;
      sqlite3BtreeEnter(pBt);
      nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      sqlite3BtreeLeave(pBt);
      if (nEntry > 0 && db->xWalCallback && rc == SQLITE_OK) {
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zDbSName, nEntry);
      }
    }
  }

  return rc;
}

sqlite3_stmt *sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt) {
  sqlite3_stmt *pNext;

  sqlite3_mutex_enter(pDb->mutex);
  if (pStmt == 0) {
    pNext = (sqlite3_stmt *)pDb->pVdbe;
  } else {
    pNext = (sqlite3_stmt *)((Vdbe *)pStmt)->pVNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  return pNext;
}

int sqlite3_blob_open(sqlite3 *db, const char *zDb, const char *zTable, const char *zColumn, sqlite_int64 iRow,
                      int wrFlag, sqlite3_blob **ppBlob) {
  int nAttempt = 0;
  int iCol;
  int rc = SQLITE_OK;
  char *zErr = 0;
  Table *pTab;
  Incrblob *pBlob = 0;
  int iDb;
  Parse sParse;

  *ppBlob = 0;

  wrFlag = !!wrFlag;

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, sizeof(Incrblob));
  while (1) {
    sqlite3ParseObjectInit(&sParse, db);
    if (!pBlob)
      goto blob_open_out;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3LocateTable(&sParse, 0, zTable, zDb);
    if (pTab && ((pTab)->eTabType == 1)) {
      pTab = 0;
      sqlite3ErrorMsg(&sParse, "cannot open virtual table: %s", zTable);
    }
    if (pTab && !(((pTab)->tabFlags & 0x00000080) == 0)) {
      pTab = 0;
      sqlite3ErrorMsg(&sParse, "cannot open table without rowid: %s", zTable);
    }
    if (pTab && (pTab->tabFlags & 0x00000060) != 0) {
      pTab = 0;
      sqlite3ErrorMsg(&sParse, "cannot open table with generated columns: %s", zTable);
    }

    if (pTab && ((pTab)->eTabType == 2)) {
      pTab = 0;
      sqlite3ErrorMsg(&sParse, "cannot open view: %s", zTable);
    }

    if (pTab == 0 || ((iDb = sqlite3SchemaToIndex(db, pTab->pSchema)) == 1 && sqlite3OpenTempDatabase(&sParse))) {
      if (sParse.zErrMsg) {
        sqlite3DbFree(db, zErr);
        zErr = sParse.zErrMsg;
        sParse.zErrMsg = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      goto blob_open_out;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    iCol = sqlite3ColumnIndex(pTab, zColumn);
    if (iCol < 0) {
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", zColumn);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      goto blob_open_out;
    }

    if (wrFlag) {
      const char *zFault = 0;
      Index *pIdx;

      if (db->flags & 0x00004000) {
        FKey *pFKey;

        for (pFKey = pTab->u.tab.pFKey; pFKey; pFKey = pFKey->pNextFrom) {
          int j;
          for (j = 0; j < pFKey->nCol; j++) {
            if (pFKey->aCol[j].iFrom == iCol) {
              zFault = "foreign key";
            }
          }
        }
      }

      for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
        int j;
        for (j = 0; j < pIdx->nKeyCol; j++) {
          if (pIdx->aiColumn[j] == iCol || pIdx->aiColumn[j] == (-2)) {
            zFault = "indexed";
          }
        }
      }
      if (zFault) {
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        goto blob_open_out;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);

    if (pBlob->pStmt) {
      static const int iLn = 0;
      static const VdbeOpList openBlob[] = {
          {171, 0, 0, 0}, {114, 0, 0, 0}, {31, 0, 5, 1}, {96, 0, 0, 1}, {86, 1, 0, 0}, {72, 0, 0, 0},
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3VdbeAddOp4Int(v, 2, iDb, wrFlag, pTab->pSchema->schema_cookie, pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);

      aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(openBlob) / sizeof(openBlob[0]))), openBlob, iLn);

      sqlite3VdbeUsesBtree(v, iDb);

      if (db->mallocFailed == 0) {
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, 0);
      }
      if (db->mallocFailed == 0) {
        if (wrFlag)
          aOp[1].opcode = 116;
        aOp[1].p2 = pTab->tnum;
        aOp[1].p3 = iDb;

        aOp[1].p4type = (-3);
        aOp[1].p4.i = pTab->nCol + 1;
        aOp[3].p2 = pTab->nCol;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }

    pBlob->iCol = iCol;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    if (db->mallocFailed) {
      goto blob_open_out;
    }
    rc = blobSeekToRow(pBlob, iRow, &zErr);
    if ((++nAttempt) >= 50 || rc != SQLITE_SCHEMA)
      break;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_out:
  if (rc == SQLITE_OK && db->mallocFailed == 0) {
    *ppBlob = (sqlite3_blob *)pBlob;
  } else {
    if (pBlob && pBlob->pStmt)
      sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3ErrorWithMsg(db, rc, (zErr ? "%s" : (char *)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3VdbeSorterInit(sqlite3 *db, int nField, VdbeCursor *pCsr) {
  int pgsz;
  int i;
  VdbeSorter *pSorter;
  KeyInfo *pKeyInfo;
  int szKeyInfo;
  i64 sz;
  int rc = SQLITE_OK;

  int nWorker;

  if (sqlite3TempInMemory(db) || sqlite3Config.bCoreMutex == 0) {
    nWorker = 0;
  } else {
    nWorker = db->aLimit[SQLITE_LIMIT_WORKER_THREADS];
  }

  szKeyInfo = (offsetof(KeyInfo, aColl) + (pCsr->pKeyInfo->nAllField) * sizeof(CollSeq *));
  sz = (offsetof(VdbeSorter, aTask) + (nWorker + 1) * sizeof(SortSubtask));

  pSorter = (VdbeSorter *)sqlite3DbMallocZero(db, sz + szKeyInfo);
  pCsr->uc.pSorter = pSorter;
  if (pSorter == 0) {
    rc = 7;
  } else {
    Btree *pBt = db->aDb[0].pBt;
    pSorter->pKeyInfo = pKeyInfo = (KeyInfo *)((u8 *)pSorter + sz);
    memcpy(pKeyInfo, pCsr->pKeyInfo, szKeyInfo);
    pKeyInfo->db = 0;
    if (nField && nWorker == 0) {
      pKeyInfo->nKeyField = nField;
    }

    sqlite3BtreeEnter(pBt);
    pSorter->pgsz = pgsz = sqlite3BtreeGetPageSize(pBt);
    sqlite3BtreeLeave(pBt);
    pSorter->nTask = nWorker + 1;
    pSorter->iPrev = (u8)(nWorker - 1);
    pSorter->bUseThreads = (pSorter->nTask > 1);
    pSorter->db = db;
    for (i = 0; i < pSorter->nTask; i++) {
      SortSubtask *pTask = &pSorter->aTask[i];
      pTask->pSorter = pSorter;
    }

    if (!sqlite3TempInMemory(db)) {
      i64 mxCache;
      u32 szPma = sqlite3Config.szPma;
      pSorter->mnPmaSize = szPma * pgsz;

      mxCache = db->aDb[0].pSchema->cache_size;
      if (mxCache < 0) {
        mxCache = mxCache * -1024;
      } else {
        mxCache = mxCache * pgsz;
      }
      mxCache = ((mxCache) < ((1 << 29)) ? (mxCache) : ((1 << 29)));
      pSorter->mxPmaSize = ((pSorter->mnPmaSize) > ((int)mxCache) ? (pSorter->mnPmaSize) : ((int)mxCache));

      if (sqlite3Config.bSmallMalloc == 0) {
        pSorter->nMemory = pgsz;
        pSorter->list.aMemory = (u8 *)sqlite3Malloc(pgsz);
        if (!pSorter->list.aMemory)
          rc = 7;
      }
    }

    if (pKeyInfo->nAllField < 13 && (pKeyInfo->aColl[0] == 0 || pKeyInfo->aColl[0] == db->pDfltColl) &&
        (pKeyInfo->aSortFlags[0] & 0x02) == 0) {
      pSorter->typeMask = 0x01 | 0x02;
    }
  }

  return rc;
}

void vdbeSorterRecordFree(sqlite3 *db, SorterRecord *pRecord) {
  SorterRecord *p;
  SorterRecord *pNext;
  for (p = pRecord; p; p = pNext) {
    pNext = p->u.pNext;
    sqlite3DbFree(db, p);
  }
}

void vdbeSortSubtaskCleanup(sqlite3 *db, SortSubtask *pTask) {
  sqlite3DbFree(db, pTask->pUnpacked);

  if (pTask->list.aMemory) {
    sqlite3_free(pTask->list.aMemory);
  } else {
    vdbeSorterRecordFree(0, pTask->list.pList);
  }
  if (pTask->file.pFd) {
    sqlite3OsCloseFree(pTask->file.pFd);
  }
  if (pTask->file2.pFd) {
    sqlite3OsCloseFree(pTask->file2.pFd);
  }
  memset(pTask, 0, sizeof(SortSubtask));
}

void sqlite3VdbeSorterReset(sqlite3 *db, VdbeSorter *pSorter) {
  int i;
  (void)vdbeSorterJoinAll(pSorter, 0);

  if (pSorter->pReader) {
    vdbePmaReaderClear(pSorter->pReader);
    sqlite3DbFree(db, pSorter->pReader);
    pSorter->pReader = 0;
  }

  vdbeMergeEngineFree(pSorter->pMerger);
  pSorter->pMerger = 0;
  for (i = 0; i < pSorter->nTask; i++) {
    SortSubtask *pTask = &pSorter->aTask[i];
    vdbeSortSubtaskCleanup(db, pTask);
    pTask->pSorter = pSorter;
  }
  if (pSorter->list.aMemory == 0) {
    vdbeSorterRecordFree(0, pSorter->list.pList);
  }
  pSorter->list.pList = 0;
  pSorter->list.szPMA = 0;
  pSorter->bUsePMA = 0;
  pSorter->iMemory = 0;
  pSorter->mxKeysize = 0;
  sqlite3DbFree(db, pSorter->pUnpacked);
  pSorter->pUnpacked = 0;
}

void sqlite3VdbeSorterClose(sqlite3 *db, VdbeCursor *pCsr) {
  VdbeSorter *pSorter;

  pSorter = pCsr->uc.pSorter;
  if (pSorter) {
    int ii;
    for (ii = 0; ii < pSorter->nTask; ii++) {
      db->nSpill += pSorter->aTask[ii].nSpill;
    }
    sqlite3VdbeSorterReset(db, pSorter);
    sqlite3_free(pSorter->list.aMemory);
    sqlite3DbFree(db, pSorter);
    pCsr->uc.pSorter = 0;
  }
}

void vdbeSorterExtendFile(sqlite3 *db, sqlite3_file *pFd, i64 nByte) {
  if (nByte <= (i64)(db->nMaxSorterMmap) && pFd->pMethods->iVersion >= 3) {
    void *p = 0;
    int chunksize = 4 * 1024;
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_CHUNK_SIZE, &chunksize);
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_SIZE_HINT, &nByte);
    sqlite3OsFetch(pFd, 0, (int)nByte, &p);
    if (p)
      sqlite3OsUnfetch(pFd, 0, p);
  }
}

int vdbeSorterOpenTempFile(sqlite3 *db, i64 nExtend, sqlite3_file **ppFd) {
  int rc;
  if (sqlite3FaultSim(202))
    return (10 | (13 << 8));
  rc = sqlite3OsOpenMalloc(db->pVfs, 0, ppFd,
                           SQLITE_OPEN_TEMP_JOURNAL | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                               SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_DELETEONCLOSE,
                           &rc);
  if (rc == SQLITE_OK) {
    i64 max = 0x7fff0000;
    sqlite3OsFileControlHint(*ppFd, SQLITE_FCNTL_MMAP_SIZE, (void *)&max);
    if (nExtend > 0) {
      vdbeSorterExtendFile(db, *ppFd, nExtend);
    }
  }
  return rc;
}

int sqlite3VdbeSorterNext(sqlite3 *db, const VdbeCursor *pCsr) {
  VdbeSorter *pSorter;
  int rc;

  pSorter = pCsr->uc.pSorter;

  if (pSorter->bUsePMA) {
    if (pSorter->bUseThreads) {
      rc = vdbePmaReaderNext(pSorter->pReader);
      if (rc == SQLITE_OK && pSorter->pReader->pFd == 0)
        rc = SQLITE_DONE;
    } else {
      int res = 0;

      rc = vdbeMergeEngineStep(pSorter->pMerger, &res);
      if (rc == SQLITE_OK && res)
        rc = SQLITE_DONE;
    }
  } else {
    SorterRecord *pFree = pSorter->list.pList;
    pSorter->list.pList = pFree->u.pNext;
    pFree->u.pNext = 0;
    if (pSorter->list.aMemory == 0)
      vdbeSorterRecordFree(db, pFree);
    rc = pSorter->list.pList ? SQLITE_OK : SQLITE_DONE;
  }
  return rc;
}

int sqlite3MatchEName(const struct ExprList_item *pItem, const char *zCol, const char *zTab, const char *zDb,
                      int *pbRowid) {
  int n;
  const char *zSpan;
  int eEName = pItem->fg.eEName;
  if (eEName != 2 && (eEName != 3 || (pbRowid == 0))) {
    return 0;
  }

  zSpan = pItem->zEName;
  for (n = 0; (zSpan[n]) && zSpan[n] != '.'; n++) {
  }
  if (zDb && (sqlite3_strnicmp(zSpan, zDb, n) != 0 || zDb[n] != 0)) {
    return 0;
  }
  zSpan += n + 1;
  for (n = 0; (zSpan[n]) && zSpan[n] != '.'; n++) {
  }
  if (zTab && (sqlite3_strnicmp(zSpan, zTab, n) != 0 || zTab[n] != 0)) {
    return 0;
  }
  zSpan += n + 1;
  if (zCol) {
    if (eEName == 2 && sqlite3StrICmp(zSpan, zCol) != 0)
      return 0;
    if (eEName == 3 && sqlite3IsRowid(zCol) == 0)
      return 0;
  }
  if (eEName == 3)
    *pbRowid = 1;
  return 1;
}

int areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *pTopNC) {
  if (db->init.busy)
    return 1;
  if (pTopNC->ncFlags & 0x010000) {
    if (sqlite3WritableSchema(db) && (db->flags & 0x40000000) != 0) {
      return 1;
    }
    return (db->flags & 0x20000000) != 0;
  } else {
    return (db->flags & 0x40000000) != 0;
  }
}

Expr *sqlite3CreateColumnExpr(sqlite3 *db, SrcList *pSrc, int iSrc, int iCol) {
  Expr *p = sqlite3ExprAlloc(db, 168, 0, 0);
  if (p) {
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;

    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    if (p->y.pTab->iPKey == iCol) {
      p->iColumn = -1;
    } else {
      p->iColumn = (ynVar)iCol;
      if ((pTab->tabFlags & 0x00000060) != 0 && (pTab->aCol[iCol].colFlags & 0x0060) != 0) {
        pItem->colUsed = pTab->nCol >= 64 ? ((Bitmask)-1) : (((Bitmask)1) << (pTab->nCol)) - 1;
      } else {
        pItem->colUsed |=
            ((Bitmask)1) << (iCol >= ((int)(sizeof(Bitmask) * 8)) ? ((int)(sizeof(Bitmask) * 8)) - 1 : iCol);
      }
    }
  }
  return p;
}

Expr *sqlite3ExprAlloc(sqlite3 *db, int op, const Token *pToken, int dequote) {
  Expr *pNew;
  int nExtra = pToken ? pToken->n + 1 : 0;

  pNew = sqlite3DbMallocRawNN(db, sizeof(Expr) + nExtra);
  if (pNew) {
    memset(pNew, 0, sizeof(Expr));
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    if (nExtra) {
      pNew->u.zToken = (char *)&pNew[1];

      if (pToken->n)
        memcpy(pNew->u.zToken, pToken->z, pToken->n);
      pNew->u.zToken[pToken->n] = 0;
      if (dequote && (sqlite3CtypeMap[(unsigned char)(pNew->u.zToken[0])] & 0x80)) {
        sqlite3DequoteExpr(pNew);
      }
    }

    pNew->nHeight = 1;
  }
  return pNew;
}

Expr *sqlite3Expr(sqlite3 *db, int op, const char *zToken) {
  Token x;
  x.z = zToken;
  x.n = sqlite3Strlen30(zToken);
  return sqlite3ExprAlloc(db, op, &x, 0);
}

Expr *sqlite3ExprInt32(sqlite3 *db, int iVal) {
  Expr *pNew = sqlite3DbMallocRawNN(db, sizeof(Expr));
  if (pNew) {
    memset(pNew, 0, sizeof(Expr));
    pNew->op = 156;
    pNew->iAgg = -1;
    pNew->flags = 0x000800 | 0x800000 | (iVal ? 0x10000000 : 0x20000000);
    pNew->u.iValue = iVal;

    pNew->nHeight = 1;
  }
  return pNew;
}

void sqlite3ExprAttachSubtrees(sqlite3 *db, Expr *pRoot, Expr *pLeft, Expr *pRight) {
  if (pRoot == 0) {
    sqlite3ExprDelete(db, pLeft);
    sqlite3ExprDelete(db, pRight);
  } else {
    if (pRight) {
      pRoot->pRight = pRight;
      pRoot->flags |= (0x000200 | 0x400000 | 0x000008) & pRight->flags;

      pRoot->nHeight = pRight->nHeight + 1;
    } else {
      pRoot->nHeight = 1;
    }
    if (pLeft) {
      pRoot->pLeft = pLeft;
      pRoot->flags |= (0x000200 | 0x400000 | 0x000008) & pLeft->flags;

      if (pLeft->nHeight >= pRoot->nHeight) {
        pRoot->nHeight = pLeft->nHeight + 1;
      }
    }
  }
}

__attribute__((noinline)) void sqlite3ExprDeleteNN(sqlite3 *db, Expr *p) {
exprDeleteRestart:
  if (!(((p)->flags & (u32)((0x010000 | 0x800000))) != 0)) {
    if (p->pRight) {
      sqlite3ExprDeleteNN(db, p->pRight);
    } else if ((((p)->flags & 0x001000) != 0)) {
      sqlite3SelectDelete(db, p->x.pSelect);
    } else {
      sqlite3ExprListDelete(db, p->x.pList);

      if ((((p)->flags & (u32)(0x1000000)) != 0)) {
        sqlite3WindowDelete(db, p->y.pWin);
      }
    }
    if (p->pLeft && p->op != 178) {
      Expr *pLeft = p->pLeft;
      if (!(((p)->flags & (u32)(0x8000000)) != 0) && !(((pLeft)->flags & (u32)(0x8000000)) != 0)) {
        sqlite3DbNNFreeNN(db, p);
        p = pLeft;
        goto exprDeleteRestart;
      } else {
        sqlite3ExprDeleteNN(db, pLeft);
      }
    }
  }
  if (!(((p)->flags & (u32)(0x8000000)) != 0)) {
    sqlite3DbNNFreeNN(db, p);
  }
}

void sqlite3ExprDelete(sqlite3 *db, Expr *p) {
  if (p)
    sqlite3ExprDeleteNN(db, p);
}

void sqlite3ExprDeleteGeneric(sqlite3 *db, void *p) {
  if ((p))
    sqlite3ExprDeleteNN(db, (Expr *)p);
}

void sqlite3ClearOnOrUsing(sqlite3 *db, OnOrUsing *p) {
  if (p == 0) {
  } else if (p->pOn) {
    sqlite3ExprDeleteNN(db, p->pOn);
  } else if (p->pUsing) {
    sqlite3IdListDelete(db, p->pUsing);
  }
}

Expr *exprDup(sqlite3 *db, const Expr *p, int dupFlags, EdupBuf *pEdupBuf) {
  Expr *pNew;
  EdupBuf sEdupBuf;
  u32 staticFlag;
  int nToken = -1;

  if (pEdupBuf) {
    sEdupBuf.zAlloc = pEdupBuf->zAlloc;

    staticFlag = 0x8000000;

  } else {
    int nAlloc;
    if (dupFlags) {
      nAlloc = dupedExprSize(p);
    } else if (!(((p)->flags & (u32)(0x000800)) != 0) && p->u.zToken) {
      nToken = (strlen(p->u.zToken) & 0x3fffffff) + 1;
      nAlloc = (((sizeof(Expr) + nToken) + 7) & ~7);
    } else {
      nToken = 0;
      nAlloc = (((sizeof(Expr)) + 7) & ~7);
    }

    sEdupBuf.zAlloc = sqlite3DbMallocRawNN(db, nAlloc);

    staticFlag = 0;
  }
  pNew = (Expr *)sEdupBuf.zAlloc;

  if (pNew) {
    const unsigned nStructSize = dupedExprStructSize(p, dupFlags);
    int nNewSize = nStructSize & 0xfff;
    if (nToken < 0) {
      if (!(((p)->flags & (u32)(0x000800)) != 0) && p->u.zToken) {
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      } else {
        nToken = 0;
      }
    }
    if (dupFlags) {
      memcpy(sEdupBuf.zAlloc, p, nNewSize);
    } else {
      u32 nSize = (u32)exprStructSize(p);

      memcpy(sEdupBuf.zAlloc, p, nSize);
      if (nSize < sizeof(Expr)) {
        memset(&sEdupBuf.zAlloc[nSize], 0, sizeof(Expr) - nSize);
      }
      nNewSize = sizeof(Expr);
    }

    pNew->flags &= ~(0x004000 | 0x010000 | 0x8000000);
    pNew->flags |= nStructSize & (0x004000 | 0x010000);
    pNew->flags |= staticFlag;
    if (dupFlags) {
    }

    if (nToken > 0) {
      char *zToken = pNew->u.zToken = (char *)&sEdupBuf.zAlloc[nNewSize];
      memcpy(zToken, p->u.zToken, nToken);
      nNewSize += nToken;
    }
    sEdupBuf.zAlloc += (((nNewSize) + 7) & ~7);

    if (((p->flags | pNew->flags) & (0x010000 | 0x800000)) == 0) {
      if ((((p)->flags & 0x001000) != 0)) {
        pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, dupFlags);
      } else {
        pNew->x.pList = sqlite3ExprListDup(db, p->x.pList, p->op != 146 ? dupFlags : 0);
      }

      if ((((p)->flags & (u32)(0x1000000)) != 0)) {
        pNew->y.pWin = sqlite3WindowDup(db, pNew, p->y.pWin);
      }

      if (dupFlags) {
        if (p->op == 178) {
          pNew->pLeft = p->pLeft;

        } else {
          pNew->pLeft = p->pLeft ? exprDup(db, p->pLeft, 0x0001, &sEdupBuf) : 0;
        }
        pNew->pRight = p->pRight ? exprDup(db, p->pRight, 0x0001, &sEdupBuf) : 0;
      } else {
        if (p->op == 178) {
          pNew->pLeft = p->pLeft;

        } else {
          pNew->pLeft = sqlite3ExprDup(db, p->pLeft, 0);
        }
        pNew->pRight = sqlite3ExprDup(db, p->pRight, 0);
      }
    }
  }
  if (pEdupBuf)
    memcpy(pEdupBuf, &sEdupBuf, sizeof(sEdupBuf));

  return pNew;
}

With *sqlite3WithDup(sqlite3 *db, With *p) {
  With *pRet = 0;
  if (p) {
    sqlite3_int64 nByte = (offsetof(With, a) + (p->nCte) * sizeof(Cte));
    pRet = sqlite3DbMallocZero(db, nByte);
    if (pRet) {
      int i;
      pRet->nCte = p->nCte;
      for (i = 0; i < p->nCte; i++) {
        pRet->a[i].pSelect = sqlite3SelectDup(db, p->a[i].pSelect, 0);
        pRet->a[i].pCols = sqlite3ExprListDup(db, p->a[i].pCols, 0);
        pRet->a[i].zName = sqlite3DbStrDup(db, p->a[i].zName);
        pRet->a[i].eM10d = p->a[i].eM10d;
      }
    }
  }
  return pRet;
}

Expr *sqlite3ExprDup(sqlite3 *db, const Expr *p, int flags) {
  return p ? exprDup(db, p, flags, 0) : 0;
}

ExprList *sqlite3ExprListDup(sqlite3 *db, const ExprList *p, int flags) {
  ExprList *pNew;
  struct ExprList_item *pItem;
  const struct ExprList_item *pOldItem;
  int i;
  Expr *pPriorSelectColOld = 0;
  Expr *pPriorSelectColNew = 0;

  if (p == 0)
    return 0;
  pNew = sqlite3DbMallocRawNN(db, sqlite3DbMallocSize(db, p));
  if (pNew == 0)
    return 0;
  pNew->nExpr = p->nExpr;
  pNew->nAlloc = p->nAlloc;
  pItem = pNew->a;
  pOldItem = p->a;
  for (i = 0; i < p->nExpr; i++, pItem++, pOldItem++) {
    Expr *pOldExpr = pOldItem->pExpr;
    Expr *pNewExpr;
    pItem->pExpr = sqlite3ExprDup(db, pOldExpr, flags);
    if (pOldExpr && pOldExpr->op == 178 && (pNewExpr = pItem->pExpr) != 0) {
      if (pNewExpr->pRight) {
        pPriorSelectColOld = pOldExpr->pRight;
        pPriorSelectColNew = pNewExpr->pRight;
        pNewExpr->pLeft = pNewExpr->pRight;
      } else {
        if (pOldExpr->pLeft != pPriorSelectColOld) {
          pPriorSelectColOld = pOldExpr->pLeft;
          pPriorSelectColNew = sqlite3ExprDup(db, pPriorSelectColOld, flags);
          pNewExpr->pRight = pPriorSelectColNew;
        }
        pNewExpr->pLeft = pPriorSelectColNew;
      }
    }
    pItem->zEName = sqlite3DbStrDup(db, pOldItem->zEName);
    pItem->fg = pOldItem->fg;
    pItem->u = pOldItem->u;
  }
  return pNew;
}

SrcList *sqlite3SrcListDup(sqlite3 *db, const SrcList *p, int flags) {
  SrcList *pNew;
  int i;

  if (p == 0)
    return 0;
  pNew = sqlite3DbMallocRawNN(db, (offsetof(SrcList, a) + (p->nSrc) * sizeof(SrcItem)));
  if (pNew == 0)
    return 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  for (i = 0; i < p->nSrc; i++) {
    SrcItem *pNewItem = &pNew->a[i];
    const SrcItem *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->fg = pOldItem->fg;
    if (pOldItem->fg.isSubquery) {
      Subquery *pNewSubq = sqlite3DbMallocRaw(db, sizeof(Subquery));
      if (pNewSubq == 0) {
        pNewItem->fg.isSubquery = 0;
      } else {
        memcpy(pNewSubq, pOldItem->u4.pSubq, sizeof(*pNewSubq));
        pNewSubq->pSelect = sqlite3SelectDup(db, pNewSubq->pSelect, flags);
        if (pNewSubq->pSelect == 0) {
          sqlite3DbFree(db, pNewSubq);
          pNewSubq = 0;
          pNewItem->fg.isSubquery = 0;
        }
      }
      pNewItem->u4.pSubq = pNewSubq;
    } else if (pOldItem->fg.fixedSchema) {
      pNewItem->u4.pSchema = pOldItem->u4.pSchema;
    } else {
      pNewItem->u4.zDatabase = sqlite3DbStrDup(db, pOldItem->u4.zDatabase);
    }
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->iCursor = pOldItem->iCursor;
    if (pNewItem->fg.isIndexedBy) {
      pNewItem->u1.zIndexedBy = sqlite3DbStrDup(db, pOldItem->u1.zIndexedBy);
    } else if (pNewItem->fg.isTabFunc) {
      pNewItem->u1.pFuncArg = sqlite3ExprListDup(db, pOldItem->u1.pFuncArg, flags);
    } else {
      pNewItem->u1.nRow = pOldItem->u1.nRow;
    }
    pNewItem->u2 = pOldItem->u2;
    if (pNewItem->fg.isCte) {
      pNewItem->u2.pCteUse->nUse++;
    }
    pTab = pNewItem->pSTab = pOldItem->pSTab;
    if (pTab) {
      pTab->nTabRef++;
    }
    if (pOldItem->fg.isUsing) {
      pNewItem->u3.pUsing = sqlite3IdListDup(db, pOldItem->u3.pUsing);
    } else {
      pNewItem->u3.pOn = sqlite3ExprDup(db, pOldItem->u3.pOn, flags);
    }
    pNewItem->colUsed = pOldItem->colUsed;
  }
  return pNew;
}

IdList *sqlite3IdListDup(sqlite3 *db, const IdList *p) {
  IdList *pNew;
  int i;

  if (p == 0)
    return 0;
  pNew = sqlite3DbMallocRawNN(db, (offsetof(IdList, a) + (p->nId) * sizeof(struct IdList_item)));
  if (pNew == 0)
    return 0;
  pNew->nId = p->nId;
  for (i = 0; i < p->nId; i++) {
    struct IdList_item *pNewItem = &pNew->a[i];
    const struct IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
  }
  return pNew;
}

Select *sqlite3SelectDup(sqlite3 *db, const Select *pDup, int flags) {
  Select *pRet = 0;
  Select *pNext = 0;
  Select **pp = &pRet;
  const Select *p;

  for (p = pDup; p; p = p->pPrior) {
    Select *pNew = sqlite3DbMallocRawNN(db, sizeof(*p));
    if (pNew == 0)
      break;
    pNew->pEList = sqlite3ExprListDup(db, p->pEList, flags);
    pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
    pNew->pWhere = sqlite3ExprDup(db, p->pWhere, flags);
    pNew->pGroupBy = sqlite3ExprListDup(db, p->pGroupBy, flags);
    pNew->pHaving = sqlite3ExprDup(db, p->pHaving, flags);
    pNew->pOrderBy = sqlite3ExprListDup(db, p->pOrderBy, flags);
    pNew->op = p->op;
    pNew->pNext = pNext;
    pNew->pPrior = 0;
    pNew->pLimit = sqlite3ExprDup(db, p->pLimit, flags);
    pNew->iLimit = 0;
    pNew->iOffset = 0;
    pNew->selFlags = p->selFlags;
    pNew->nSelectRow = p->nSelectRow;
    pNew->pWith = sqlite3WithDup(db, p->pWith);

    pNew->pWin = 0;
    pNew->pWinDefn = sqlite3WindowListDup(db, p->pWinDefn);
    if (p->pWin && db->mallocFailed == 0)
      gatherSelectWindows(pNew);

    pNew->selId = p->selId;
    if (db->mallocFailed) {
      pNew->pNext = 0;
      sqlite3SelectDelete(db, pNew);
      break;
    }
    *pp = pNew;
    pp = &pNew->pPrior;
    pNext = pNew;
  }
  return pRet;
}

__attribute__((noinline)) ExprList *sqlite3ExprListAppendNew(sqlite3 *db, Expr *pExpr) {
  struct ExprList_item *pItem;
  ExprList *pList;

  pList = sqlite3DbMallocRawNN(db, (offsetof(ExprList, a) + (4) * sizeof(struct ExprList_item)));
  if (pList == 0) {
    sqlite3ExprDelete(db, pExpr);
    return 0;
  }
  pList->nAlloc = 4;
  pList->nExpr = 1;
  pItem = &pList->a[0];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  return pList;
}

__attribute__((noinline)) ExprList *sqlite3ExprListAppendGrow(sqlite3 *db, ExprList *pList, Expr *pExpr) {
  struct ExprList_item *pItem;
  ExprList *pNew;
  pList->nAlloc *= 2;
  pNew = sqlite3DbRealloc(db, pList, (offsetof(ExprList, a) + (pList->nAlloc) * sizeof(struct ExprList_item)));
  if (pNew == 0) {
    sqlite3ExprListDelete(db, pList);
    sqlite3ExprDelete(db, pExpr);
    return 0;
  } else {
    pList = pNew;
  }
  pItem = &pList->a[pList->nExpr++];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  return pList;
}

__attribute__((noinline)) void exprListDeleteNN(sqlite3 *db, ExprList *pList) {
  int i = pList->nExpr;
  struct ExprList_item *pItem = pList->a;

  do {
    sqlite3ExprDelete(db, pItem->pExpr);
    if (pItem->zEName)
      sqlite3DbNNFreeNN(db, pItem->zEName);
    pItem++;
  } while (--i > 0);
  sqlite3DbNNFreeNN(db, pList);
}

void sqlite3ExprListDelete(sqlite3 *db, ExprList *pList) {
  if (pList)
    exprListDeleteNN(db, pList);
}

void sqlite3ExprListDeleteGeneric(sqlite3 *db, void *pList) {
  if ((pList))
    exprListDeleteNN(db, (ExprList *)pList);
}

u32 sqlite3IsTrueOrFalse(const char *zIn) {
  if (sqlite3StrICmp(zIn, "true") == 0)
    return 0x10000000;
  if (sqlite3StrICmp(zIn, "false") == 0)
    return 0x20000000;
  return 0;
}

int sqlite3IsRowid(const char *z) {
  if (sqlite3StrICmp(z, "_ROWID_") == 0)
    return 1;
  if (sqlite3StrICmp(z, "ROWID") == 0)
    return 1;
  if (sqlite3StrICmp(z, "OID") == 0)
    return 1;
  return 0;
}

int sqlite3ExprIsIIF(sqlite3 *db, const Expr *pExpr) {
  ExprList *pList;
  if (pExpr->op == 172) {
    const char *z = pExpr->u.zToken;
    FuncDef *pDef;
    if ((z[0] != 'i' && z[0] != 'I'))
      return 0;
    if (pExpr->x.pList == 0)
      return 0;
    pDef = sqlite3FindFunction(db, z, pExpr->x.pList->nExpr, ((db)->enc), 0);

    if (pDef == 0)
      return 0;

    if ((pDef->funcFlags & 0x00400000) == 0)
      return 0;
    if (((int)(intptr_t)(pDef->pUserData)) != 5)
      return 0;
  } else if (pExpr->op == 158) {
    if (pExpr->pLeft != 0)
      return 0;
  } else {
    return 0;
  }
  pList = pExpr->x.pList;

  if (pList->nExpr == 2)
    return 1;
  if (pList->nExpr == 3 && sqlite3ExprIsNotTrue(pList->a[2].pExpr))
    return 1;
  return 0;
}

int addAggInfoColumn(sqlite3 *db, AggInfo *pInfo) {
  int i;
  pInfo->aCol = sqlite3ArrayAllocate(db, pInfo->aCol, sizeof(pInfo->aCol[0]), &pInfo->nColumn, &i);
  return i;
}

int addAggInfoFunc(sqlite3 *db, AggInfo *pInfo) {
  int i;
  pInfo->aFunc = sqlite3ArrayAllocate(db, pInfo->aFunc, sizeof(pInfo->aFunc[0]), &pInfo->nFunc, &i);
  return i;
}

void renameTokenFree(sqlite3 *db, RenameToken *pToken) {
  RenameToken *pNext;
  RenameToken *p;
  for (p = pToken; p; p = pNext) {
    pNext = p->pNext;
    sqlite3DbFree(db, p);
  }
}

int alterRtrimConstraint(sqlite3 *db, const char *pCons, int nCons) {
  u8 *zTmp = (u8 *)sqlite3MPrintf(db, "%.*s", nCons, pCons);
  int iOff = 0;
  int iEnd = 0;

  if (zTmp == 0)
    return 0;

  while (1) {
    int t = 0;
    int nToken = sqlite3GetToken(&zTmp[iOff], &t);
    if (t == 186)
      break;
    if (t != 184 && (t != 185 || zTmp[iOff] != '-')) {
      iEnd = iOff + nToken;
    }
    iOff += nToken;
  }

  sqlite3DbFree(db, zTmp);
  return iEnd;
}

static void sqlite3AlterFunctions(void) {
  static FuncDef aAlterTableFuncs[] = {
      {9, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, renameColumnFunc, 0, 0, 0, "sqlite_rename_column", {0}},
      {7, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, renameTableFunc, 0, 0, 0, "sqlite_rename_table", {0}},
      {7, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, renameTableTest, 0, 0, 0, "sqlite_rename_test", {0}},
      {3, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, dropColumnFunc, 0, 0, 0, "sqlite_drop_column", {0}},
      {2, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, renameQuotefixFunc, 0, 0, 0, "sqlite_rename_quotefix", {0}},
      {2, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, dropConstraintFunc, 0, 0, 0, "sqlite_drop_constraint", {0}},
      {2, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, failConstraintFunc, 0, 0, 0, "sqlite_fail", {0}},
      {3, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, addConstraintFunc, 0, 0, 0, "sqlite_add_constraint", {0}},
      {2, 0x00800000 | 0x00040000 | 1 | 0x0800, 0, 0, findConstraintFunc, 0, 0, 0, "sqlite_find_constraint", {0}},
  };
  sqlite3InsertBuiltinFuncs(aAlterTableFuncs, ((int)(sizeof(aAlterTableFuncs) / sizeof(aAlterTableFuncs[0]))));
}

void sqlite3DeleteIndexSamples(sqlite3 *db, Index *pIdx) {
  (void)(db);
  (void)(pIdx);
}

int sqlite3AnalysisLoad(sqlite3 *db, int iDb) {
  analysisInfo sInfo;
  HashElem *i;
  char *zSql;
  int rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  const Table *pStat1;

  for (i = ((&pSchema->tblHash)->first); i; i = ((i)->next)) {
    Table *pTab = ((i)->data);
    pTab->tabFlags &= ~0x00000010;
  }
  for (i = ((&pSchema->idxHash)->first); i; i = ((i)->next)) {
    Index *pIdx = ((i)->data);
    pIdx->hasStat1 = 0;
  }

  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  if ((pStat1 = sqlite3FindTable(db, "sqlite_stat1", sInfo.zDatabase)) && ((pStat1)->eTabType == 0)) {
    zSql = sqlite3MPrintf(db, "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    if (zSql == 0) {
      rc = 7;
    } else {
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  for (i = ((&pSchema->idxHash)->first); i; i = ((i)->next)) {
    Index *pIdx = ((i)->data);
    if (!pIdx->hasStat1)
      sqlite3DefaultRowEst(pIdx);
  }

  if (rc == 7) {
    sqlite3OomFault(db);
  }
  return rc;
}

int sqlite3DbIsNamed(sqlite3 *db, int iDb, const char *zName) {
  return (sqlite3StrICmp(db->aDb[iDb].zDbSName, zName) == 0 || (iDb == 0 && sqlite3StrICmp("main", zName) == 0));
}

int sqlite3_set_authorizer(sqlite3 *db,
                           int (*xAuth)(void *, int, const char *, const char *, const char *, const char *),
                           void *pArg) {
  sqlite3_mutex_enter(db->mutex);
  db->xAuth = (sqlite3_xauth)xAuth;
  db->pAuthArg = pArg;
  sqlite3ExpirePreparedStatements(db, 1);
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

Table *sqlite3FindTable(sqlite3 *db, const char *zName, const char *zDatabase) {
  Table *p = 0;
  int i;

  if (zDatabase) {
    for (i = 0; i < db->nDb; i++) {
      if (sqlite3StrICmp(zDatabase, db->aDb[i].zDbSName) == 0)
        break;
    }
    if (i >= db->nDb) {
      if (sqlite3StrICmp(zDatabase, "main") == 0) {
        i = 0;
      } else {
        return 0;
      }
    }
    p = sqlite3HashFind(&db->aDb[i].pSchema->tblHash, zName);
    if (p == 0 && sqlite3_strnicmp(zName, "sqlite_", 7) == 0) {
      if (i == 1) {
        if (sqlite3StrICmp(zName + 7, &"sqlite_temp_schema"[7]) == 0 ||
            sqlite3StrICmp(zName + 7, &"sqlite_schema"[7]) == 0 ||
            sqlite3StrICmp(zName + 7, &"sqlite_master"[7]) == 0) {
          p = sqlite3HashFind(&db->aDb[1].pSchema->tblHash, "sqlite_temp_master");
        }
      } else {
        if (sqlite3StrICmp(zName + 7, &"sqlite_schema"[7]) == 0) {
          p = sqlite3HashFind(&db->aDb[i].pSchema->tblHash, "sqlite_master");
        }
      }
    }
  } else {
    p = sqlite3HashFind(&db->aDb[1].pSchema->tblHash, zName);
    if (p)
      return p;

    p = sqlite3HashFind(&db->aDb[0].pSchema->tblHash, zName);
    if (p)
      return p;

    for (i = 2; i < db->nDb; i++) {
      p = sqlite3HashFind(&db->aDb[i].pSchema->tblHash, zName);
      if (p)
        break;
    }
    if (p == 0 && sqlite3_strnicmp(zName, "sqlite_", 7) == 0) {
      if (sqlite3StrICmp(zName + 7, &"sqlite_schema"[7]) == 0) {
        p = sqlite3HashFind(&db->aDb[0].pSchema->tblHash, "sqlite_master");
      } else if (sqlite3StrICmp(zName + 7, &"sqlite_temp_schema"[7]) == 0) {
        p = sqlite3HashFind(&db->aDb[1].pSchema->tblHash, "sqlite_temp_master");
      }
    }
  }
  return p;
}

const char *sqlite3PreferredTableName(const char *zName) {
  if (sqlite3_strnicmp(zName, "sqlite_", 7) == 0) {
    if (sqlite3StrICmp(zName + 7, &"sqlite_master"[7]) == 0) {
      return "sqlite_schema";
    }
    if (sqlite3StrICmp(zName + 7, &"sqlite_temp_master"[7]) == 0) {
      return "sqlite_temp_schema";
    }
  }
  return zName;
}

Index *sqlite3FindIndex(sqlite3 *db, const char *zName, const char *zDb) {
  Index *p = 0;
  int i;

  for (i = 0; i < db->nDb; i++) {
    int j = (i < 2) ? i ^ 1 : i;
    Schema *pSchema = db->aDb[j].pSchema;

    if (zDb && sqlite3DbIsNamed(db, j, zDb) == 0)
      continue;

    p = sqlite3HashFind(&pSchema->idxHash, zName);
    if (p)
      break;
  }
  return p;
}

void sqlite3FreeIndex(sqlite3 *db, Index *p) {
  sqlite3DeleteIndexSamples(db, p);

  sqlite3ExprDelete(db, p->pPartIdxWhere);
  sqlite3ExprListDelete(db, p->aColExpr);
  sqlite3DbFree(db, p->zColAff);
  if (p->isResized)
    sqlite3DbFree(db, (void *)p->azColl);

  sqlite3DbFree(db, p);
}

void sqlite3UnlinkAndDeleteIndex(sqlite3 *db, int iDb, const char *zIdxName) {
  Index *pIndex;
  Hash *pHash;

  pHash = &db->aDb[iDb].pSchema->idxHash;
  pIndex = sqlite3HashInsert(pHash, zIdxName, 0);
  if ((pIndex)) {
    if (pIndex->pTable->pIndex == pIndex) {
      pIndex->pTable->pIndex = pIndex->pNext;
    } else {
      Index *p;

      p = pIndex->pTable->pIndex;
      while ((p) && p->pNext != pIndex) {
        p = p->pNext;
      }
      if ((p && p->pNext == pIndex)) {
        p->pNext = pIndex->pNext;
      }
    }
    sqlite3FreeIndex(db, pIndex);
  }
  db->mDbFlags |= 0x0001;
}

void sqlite3CollapseDatabaseArray(sqlite3 *db) {
  int i, j;
  for (i = j = 2; i < db->nDb; i++) {
    struct Db *pDb = &db->aDb[i];
    if (pDb->pBt == 0) {
      sqlite3DbFree(db, pDb->zDbSName);
      pDb->zDbSName = 0;
      continue;
    }
    if (j < i) {
      db->aDb[j] = db->aDb[i];
    }
    j++;
  }
  db->nDb = j;
  if (db->nDb <= 2 && db->aDb != db->aDbStatic) {
    memcpy(db->aDbStatic, db->aDb, 2 * sizeof(db->aDb[0]));
    sqlite3DbFree(db, db->aDb);
    db->aDb = db->aDbStatic;
  }
}

void sqlite3ResetOneSchema(sqlite3 *db, int iDb) {
  int i;

  if (iDb >= 0) {
    (db)->aDb[iDb].pSchema->schemaFlags |= (0x0008);
    (db)->aDb[1].pSchema->schemaFlags |= (0x0008);
    db->mDbFlags &= ~0x0010;
  }

  if (db->nSchemaLock == 0) {
    for (i = 0; i < db->nDb; i++) {
      if ((((db)->aDb[i].pSchema->schemaFlags & (0x0008)) == (0x0008))) {
        sqlite3SchemaClear(db->aDb[i].pSchema);
      }
    }
  }
}

void sqlite3ResetAllSchemasOfConnection(sqlite3 *db) {
  int i;
  sqlite3BtreeEnterAll(db);
  for (i = 0; i < db->nDb; i++) {
    Db *pDb = &db->aDb[i];
    if (pDb->pSchema) {
      if (db->nSchemaLock == 0) {
        sqlite3SchemaClear(pDb->pSchema);
      } else {
        (db)->aDb[i].pSchema->schemaFlags |= (0x0008);
      }
    }
  }
  db->mDbFlags &= ~(0x0001 | 0x0010);
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
  if (db->nSchemaLock == 0) {
    sqlite3CollapseDatabaseArray(db);
  }
}

void sqlite3CommitInternalChanges(sqlite3 *db) {
  db->mDbFlags &= ~0x0001;
}

void sqlite3ColumnSetColl(sqlite3 *db, Column *pCol, const char *zColl) {
  i64 nColl;
  i64 n;
  char *zNew;

  n = sqlite3Strlen30(pCol->zCnName) + 1;
  if (pCol->colFlags & 0x0004) {
    n += sqlite3Strlen30(pCol->zCnName + n) + 1;
  }
  nColl = sqlite3Strlen30(zColl) + 1;
  zNew = sqlite3DbRealloc(db, pCol->zCnName, nColl + n);
  if (zNew) {
    pCol->zCnName = zNew;
    memcpy(pCol->zCnName + n, zColl, nColl);
    pCol->colFlags |= 0x0200;
  }
}

void sqlite3DeleteColumnNames(sqlite3 *db, Table *pTable) {
  int i;
  Column *pCol;

  if ((pCol = pTable->aCol) != 0) {
    for (i = 0; i < pTable->nCol; i++, pCol++) {
      sqlite3DbFree(db, pCol->zCnName);
    }
    sqlite3DbNNFreeNN(db, pTable->aCol);
    if ((pTable)->eTabType == 0) {
      sqlite3ExprListDelete(db, pTable->u.tab.pDfltList);
    }
    if (db->pnBytesFreed == 0) {
      pTable->aCol = 0;
      pTable->nCol = 0;
      if ((pTable)->eTabType == 0) {
        pTable->u.tab.pDfltList = 0;
      }
    }
  }
}

void __attribute__((noinline)) deleteTable(sqlite3 *db, Table *pTable) {
  Index *pIndex, *pNext;

  for (pIndex = pTable->pIndex; pIndex; pIndex = pNext) {
    pNext = pIndex->pNext;

    if (db->pnBytesFreed == 0 && !((pTable)->eTabType == 1)) {
      char *zName = pIndex->zName;
      sqlite3HashInsert(&pIndex->pSchema->idxHash, zName, 0);
    }
    sqlite3FreeIndex(db, pIndex);
  }

  if ((pTable)->eTabType == 0) {
    sqlite3FkDelete(db, pTable);
  }

  else if ((pTable)->eTabType == 1) {
    sqlite3VtabClear(db, pTable);
  }

  else {
    sqlite3SelectDelete(db, pTable->u.view.pSelect);
  }

  sqlite3DeleteColumnNames(db, pTable);
  sqlite3DbFree(db, pTable->zName);
  sqlite3DbFree(db, pTable->zColAff);
  sqlite3ExprListDelete(db, pTable->pCheck);
  sqlite3DbFree(db, pTable);
}

void sqlite3DeleteTable(sqlite3 *db, Table *pTable) {
  if (!pTable)
    return;
  if (db->pnBytesFreed == 0 && (--pTable->nTabRef) > 0)
    return;
  deleteTable(db, pTable);
}

void sqlite3DeleteTableGeneric(sqlite3 *db, void *pTable) {
  sqlite3DeleteTable(db, (Table *)pTable);
}

void sqlite3UnlinkAndDeleteTable(sqlite3 *db, int iDb, const char *zTabName) {
  Table *p;
  Db *pDb;

  pDb = &db->aDb[iDb];
  p = sqlite3HashInsert(&pDb->pSchema->tblHash, zTabName, 0);
  sqlite3DeleteTable(db, p);
  db->mDbFlags |= 0x0001;
}

char *sqlite3NameFromToken(sqlite3 *db, const Token *pName) {
  char *zName;
  if (pName) {
    zName = sqlite3DbStrNDup(db, (const char *)pName->z, pName->n);
    sqlite3Dequote(zName);
  } else {
    zName = 0;
  }
  return zName;
}

int sqlite3FindDbName(sqlite3 *db, const char *zName) {
  int i = -1;
  if (zName) {
    Db *pDb;
    for (i = (db->nDb - 1), pDb = &db->aDb[i]; i >= 0; i--, pDb--) {
      if (0 == sqlite3_stricmp(pDb->zDbSName, zName))
        break;

      if (i == 0 && 0 == sqlite3_stricmp("main", zName))
        break;
    }
  }
  return i;
}

int sqlite3FindDb(sqlite3 *db, Token *pName) {
  int i;
  char *zName;
  zName = sqlite3NameFromToken(db, pName);
  i = sqlite3FindDbName(db, zName);
  sqlite3DbFree(db, zName);
  return i;
}

int sqlite3WritableSchema(sqlite3 *db) {
  return (db->flags & (0x00000001 | 0x10000000)) == 0x00000001;
}

void sqlite3DeleteReturning(sqlite3 *db, void *pArg) {
  Returning *pRet = (Returning *)pArg;
  Hash *pHash;
  pHash = &(db->aDb[1].pSchema->trigHash);
  sqlite3HashInsert(pHash, pRet->zName, 0);
  sqlite3ExprListDelete(db, pRet->pReturnEL);
  sqlite3DbFree(db, pRet);
}

char sqlite3AffinityType(const char *zIn, Column *pCol) {
  u32 h = 0;
  char aff = 0x43;
  const char *zChar = 0;

  while (zIn[0]) {
    u8 x = *(u8 *)zIn;
    h = (h << 8) + sqlite3UpperToLower[x];
    zIn++;
    if (h == (('c' << 24) + ('h' << 16) + ('a' << 8) + 'r')) {
      aff = 0x42;
      zChar = zIn;
    } else if (h == (('c' << 24) + ('l' << 16) + ('o' << 8) + 'b')) {
      aff = 0x42;
    } else if (h == (('t' << 24) + ('e' << 16) + ('x' << 8) + 't')) {
      aff = 0x42;
    } else if (h == (('b' << 24) + ('l' << 16) + ('o' << 8) + 'b') && (aff == 0x43 || aff == 0x45)) {
      aff = 0x41;
      if (zIn[0] == '(')
        zChar = zIn;

    } else if (h == (('r' << 24) + ('e' << 16) + ('a' << 8) + 'l') && aff == 0x43) {
      aff = 0x45;
    } else if (h == (('f' << 24) + ('l' << 16) + ('o' << 8) + 'a') && aff == 0x43) {
      aff = 0x45;
    } else if (h == (('d' << 24) + ('o' << 16) + ('u' << 8) + 'b') && aff == 0x43) {
      aff = 0x45;

    } else if ((h & 0x00FFFFFF) == (('i' << 16) + ('n' << 8) + 't')) {
      aff = 0x44;
      break;
    }
  }

  if (pCol) {
    int v = 0;
    if (aff < 0x43) {
      if (zChar) {
        while (zChar[0]) {
          if ((sqlite3CtypeMap[(unsigned char)(zChar[0])] & 0x04)) {
            sqlite3GetInt32(zChar, &v);
            break;
          }
          zChar++;
        }
      } else {
        v = 16;
      }
    }

    v = v / 4 + 1;
    if (v > 255)
      v = 255;
    pCol->szEst = v;
  }
  return aff;
}

char *createTableStmt(sqlite3 *db, Table *p) {
  int i, k, len;
  i64 n;
  char *zStmt;
  char *zSep, *zSep2, *zEnd;
  Column *pCol;
  n = 0;
  for (pCol = p->aCol, i = 0; i < p->nCol; i++, pCol++) {
    n += identLength(pCol->zCnName) + 5;
  }
  n += identLength(p->zName);
  if (n < 50) {
    zSep = "";
    zSep2 = ",";
    zEnd = ")";
  } else {
    zSep = "\n  ";
    zSep2 = ",\n  ";
    zEnd = "\n)";
  }
  n += 35 + 6 * p->nCol;
  zStmt = sqlite3DbMallocRaw(0, n);
  if (zStmt == 0) {
    sqlite3OomFault(db);
    return 0;
  }

  memcpy(zStmt, "CREATE TABLE ", 13);
  k = 13;
  identPut(zStmt, &k, p->zName);
  zStmt[k++] = '(';
  for (pCol = p->aCol, i = 0; i < p->nCol; i++, pCol++) {
    static const char *const azType[] = {
        "", " TEXT", " NUM", " INT", " REAL", " NUM",
    };
    const char *zType;

    len = sqlite3Strlen30(zSep);

    memcpy(&zStmt[k], zSep, len);
    k += len;
    zSep = zSep2;
    identPut(zStmt, &k, pCol->zCnName);

    zType = azType[pCol->affinity - 0x41];
    len = sqlite3Strlen30(zType);

    memcpy(&zStmt[k], zType, len);
    k += len;
  }
  len = sqlite3Strlen30(zEnd);

  memcpy(&zStmt[k], zEnd, len + 1);
  return zStmt;
}

int sqlite3IsShadowTableOf(sqlite3 *db, Table *pTab, const char *zName) {
  int nName;
  Module *pMod;

  if (!((pTab)->eTabType == 1))
    return 0;
  nName = sqlite3Strlen30(pTab->zName);
  if (sqlite3_strnicmp(zName, pTab->zName, nName) != 0)
    return 0;
  if (zName[nName] != '_')
    return 0;
  pMod = (Module *)sqlite3HashFind(&db->aModule, pTab->u.vtab.azArg[0]);
  if (pMod == 0)
    return 0;
  if (pMod->pModule->iVersion < 3)
    return 0;
  if (pMod->pModule->xShadowName == 0)
    return 0;
  return pMod->pModule->xShadowName(zName + nName + 1);
}

void sqlite3MarkAllShadowTablesOf(sqlite3 *db, Table *pTab) {
  int nName;
  Module *pMod;
  HashElem *k;

  pMod = (Module *)sqlite3HashFind(&db->aModule, pTab->u.vtab.azArg[0]);
  if (pMod == 0)
    return;
  if (pMod->pModule == 0)
    return;
  if (pMod->pModule->iVersion < 3)
    return;
  if (pMod->pModule->xShadowName == 0)
    return;

  nName = sqlite3Strlen30(pTab->zName);
  for (k = ((&pTab->pSchema->tblHash)->first); k; k = ((k)->next)) {
    Table *pOther = ((k)->data);

    if (!((pOther)->eTabType == 0))
      continue;
    if (pOther->tabFlags & 0x00001000)
      continue;
    if (sqlite3_strnicmp(pOther->zName, pTab->zName, nName) == 0 && pOther->zName[nName] == '_' &&
        pMod->pModule->xShadowName(pOther->zName + nName + 1)) {
      pOther->tabFlags |= 0x00001000;
    }
  }
}

int sqlite3ShadowTableName(sqlite3 *db, const char *zName) {
  const char *zTail;
  Table *pTab;
  char *zCopy;
  zTail = _Generic(0 ? (zName) : (void *)1,
      const void *: (const char *)(strrchr(zName, '_')),
      default: strrchr(zName, '_'));
  if (zTail == 0)
    return 0;
  zCopy = sqlite3DbStrNDup(db, zName, (int)(zTail - zName));
  pTab = zCopy ? sqlite3FindTable(db, zCopy, 0) : 0;
  sqlite3DbFree(db, zCopy);
  if (pTab == 0)
    return 0;
  if (!((pTab)->eTabType == 1))
    return 0;
  return sqlite3IsShadowTableOf(db, pTab, zName);
}

void sqliteViewResetAll(sqlite3 *db, int idx) {
  HashElem *i;

  if (!(((db)->aDb[idx].pSchema->schemaFlags & (0x0002)) == (0x0002)))
    return;
  for (i = ((&db->aDb[idx].pSchema->tblHash)->first); i; i = ((i)->next)) {
    Table *pTab = ((i)->data);
    if ((pTab)->eTabType == 2) {
      sqlite3DeleteColumnNames(db, pTab);
    }
  }
  (db)->aDb[idx].pSchema->schemaFlags &= ~(0x0002);
}

void sqlite3RootPageMoved(sqlite3 *db, int iDb, Pgno iFrom, Pgno iTo) {
  HashElem *pElem;
  Hash *pHash;
  Db *pDb;

  pDb = &db->aDb[iDb];
  pHash = &pDb->pSchema->tblHash;
  for (pElem = ((pHash)->first); pElem; pElem = ((pElem)->next)) {
    Table *pTab = ((pElem)->data);
    if (pTab->tnum == iFrom) {
      pTab->tnum = iTo;
    }
  }
  pHash = &pDb->pSchema->idxHash;
  for (pElem = ((pHash)->first); pElem; pElem = ((pElem)->next)) {
    Index *pIdx = ((pElem)->data);
    if (pIdx->tnum == iFrom) {
      pIdx->tnum = iTo;
    }
  }
}

int sqlite3ReadOnlyShadowTables(sqlite3 *db) {
  if ((db->flags & 0x10000000) != 0 && db->pVtabCtx == 0 && db->nVdbeExec == 0 &&
      !((db)->nVTrans > 0 && (db)->aVTrans == 0)) {
    return 1;
  }

  return 0;
}

int tableMayNotBeDropped(sqlite3 *db, Table *pTab) {
  if (sqlite3_strnicmp(pTab->zName, "sqlite_", 7) == 0) {
    if (sqlite3_strnicmp(pTab->zName + 7, "stat", 4) == 0)
      return 0;
    if (sqlite3_strnicmp(pTab->zName + 7, "parameters", 10) == 0)
      return 0;
    return 1;
  }
  if ((pTab->tabFlags & 0x00001000) != 0 && sqlite3ReadOnlyShadowTables(db)) {
    return 1;
  }
  if (pTab->tabFlags & 0x00008000) {
    return 1;
  }
  return 0;
}

Index *sqlite3AllocateIndexObject(sqlite3 *db, int nCol, int nExtra, char **ppExtra) {
  Index *p;
  i64 nByte;

  nByte = (((sizeof(Index)) + 7) & ~7) + (((sizeof(char *) * nCol) + 7) & ~7) +
          (((sizeof(LogEst) * (nCol + 1) + sizeof(i16) * nCol + sizeof(u8) * nCol) + 7) & ~7);
  p = sqlite3DbMallocZero(db, nByte + nExtra);
  if (p) {
    char *pExtra = ((char *)p) + (((sizeof(Index)) + 7) & ~7);
    p->azColl = (const char **)pExtra;
    pExtra += (((sizeof(char *) * nCol) + 7) & ~7);
    p->aiRowLogEst = (LogEst *)pExtra;
    pExtra += sizeof(LogEst) * (nCol + 1);
    p->aiColumn = (i16 *)pExtra;
    pExtra += sizeof(i16) * nCol;
    p->aSortOrder = (u8 *)pExtra;

    p->nColumn = (u16)nCol;
    p->nKeyCol = (u16)(nCol - 1);
    *ppExtra = ((char *)p) + nByte;
  }
  return p;
}

void *sqlite3ArrayAllocate(sqlite3 *db, void *pArray, int szEntry, int *pnEntry, int *pIdx) {
  char *z;
  sqlite3_int64 n = *pIdx = *pnEntry;
  if ((n & (n - 1)) == 0) {
    sqlite3_int64 sz = (n == 0) ? 1 : 2 * n;
    void *pNew = sqlite3DbRealloc(db, pArray, sz * szEntry);
    if (pNew == 0) {
      *pIdx = -1;
      return pArray;
    }
    pArray = pNew;
  }
  z = (char *)pArray;
  memset(&z[n * szEntry], 0, szEntry);
  ++*pnEntry;
  return pArray;
}

void sqlite3IdListDelete(sqlite3 *db, IdList *pList) {
  int i;

  if (pList == 0)
    return;
  for (i = 0; i < pList->nId; i++) {
    sqlite3DbFree(db, pList->a[i].zName);
  }
  sqlite3DbNNFreeNN(db, pList);
}

void sqlite3SubqueryDelete(sqlite3 *db, Subquery *pSubq) {
  sqlite3SelectDelete(db, pSubq->pSelect);
  sqlite3DbFree(db, pSubq);
}

Select *sqlite3SubqueryDetach(sqlite3 *db, SrcItem *pItem) {
  Select *pSel;

  pSel = pItem->u4.pSubq->pSelect;
  sqlite3DbFree(db, pItem->u4.pSubq);
  pItem->u4.pSubq = 0;
  pItem->fg.isSubquery = 0;
  return pSel;
}

void sqlite3SrcListDelete(sqlite3 *db, SrcList *pList) {
  int i;
  SrcItem *pItem;

  if (pList == 0)
    return;
  for (pItem = pList->a, i = 0; i < pList->nSrc; i++, pItem++) {
    if (pItem->zName)
      sqlite3DbNNFreeNN(db, pItem->zName);
    if (pItem->zAlias)
      sqlite3DbNNFreeNN(db, pItem->zAlias);
    if (pItem->fg.isSubquery) {
      sqlite3SubqueryDelete(db, pItem->u4.pSubq);
    } else if (pItem->fg.fixedSchema == 0 && pItem->u4.zDatabase != 0) {
      sqlite3DbNNFreeNN(db, pItem->u4.zDatabase);
    }
    if (pItem->fg.isIndexedBy)
      sqlite3DbFree(db, pItem->u1.zIndexedBy);
    if (pItem->fg.isTabFunc)
      sqlite3ExprListDelete(db, pItem->u1.pFuncArg);
    sqlite3DeleteTable(db, pItem->pSTab);
    if (pItem->fg.isUsing) {
      sqlite3IdListDelete(db, pItem->u3.pUsing);
    } else if (pItem->u3.pOn) {
      sqlite3ExprDelete(db, pItem->u3.pOn);
    }
  }
  sqlite3DbNNFreeNN(db, pList);
}

void cteClear(sqlite3 *db, Cte *pCte) {
  sqlite3ExprListDelete(db, pCte->pCols);
  sqlite3SelectDelete(db, pCte->pSelect);
  sqlite3DbFree(db, pCte->zName);
}

void sqlite3CteDelete(sqlite3 *db, Cte *pCte) {
  cteClear(db, pCte);
  sqlite3DbFree(db, pCte);
}

void sqlite3WithDelete(sqlite3 *db, With *pWith) {
  if (pWith) {
    int i;
    for (i = 0; i < pWith->nCte; i++) {
      cteClear(db, &pWith->a[i]);
    }
    sqlite3DbFree(db, pWith);
  }
}

void sqlite3WithDeleteGeneric(sqlite3 *db, void *pWith) {
  sqlite3WithDelete(db, (With *)pWith);
}

void callCollNeeded(sqlite3 *db, int enc, const char *zName) {
  if (db->xCollNeeded) {
    char *zExternal = sqlite3DbStrDup(db, zName);
    if (!zExternal)
      return;
    db->xCollNeeded(db->pCollNeededArg, db, enc, zExternal);
    sqlite3DbFree(db, zExternal);
  }

  if (db->xCollNeeded16) {
    char const *zExternal;
    sqlite3_value *pTmp = sqlite3ValueNew(db);
    sqlite3ValueSetStr(pTmp, -1, zName, SQLITE_UTF8, ((sqlite3_destructor_type)0));
    zExternal = sqlite3ValueText(pTmp, 2);
    if (zExternal) {
      db->xCollNeeded16(db->pCollNeededArg, db, (int)((db)->enc), zExternal);
    }
    sqlite3ValueFree(pTmp);
  }
}

int synthCollSeq(sqlite3 *db, CollSeq *pColl) {
  CollSeq *pColl2;
  char *z = pColl->zName;
  int i;
  static const u8 aEnc[] = {SQLITE_UTF16BE, SQLITE_UTF16LE, SQLITE_UTF8};
  for (i = 0; i < 3; i++) {
    pColl2 = sqlite3FindCollSeq(db, aEnc[i], z, 0);
    if (pColl2->xCmp != 0) {
      memcpy(pColl, pColl2, sizeof(CollSeq));
      pColl->xDel = 0;
      return SQLITE_OK;
    }
  }
  return SQLITE_ERROR;
}

CollSeq *findCollSeqEntry(sqlite3 *db, const char *zName, int create) {
  CollSeq *pColl;
  pColl = sqlite3HashFind(&db->aCollSeq, zName);

  if (0 == pColl && create) {
    int nName = sqlite3Strlen30(zName) + 1;
    pColl = sqlite3DbMallocZero(db, 3 * sizeof(*pColl) + nName);
    if (pColl) {
      CollSeq *pDel = 0;
      pColl[0].zName = (char *)&pColl[3];
      pColl[0].enc = SQLITE_UTF8;
      pColl[1].zName = (char *)&pColl[3];
      pColl[1].enc = SQLITE_UTF16LE;
      pColl[2].zName = (char *)&pColl[3];
      pColl[2].enc = SQLITE_UTF16BE;
      memcpy(pColl[0].zName, zName, nName);
      pDel = sqlite3HashInsert(&db->aCollSeq, pColl[0].zName, pColl);

      if (pDel != 0) {
        sqlite3OomFault(db);
        sqlite3DbFree(db, pDel);
        pColl = 0;
      }
    }
  }
  return pColl;
}

CollSeq *sqlite3FindCollSeq(sqlite3 *db, u8 enc, const char *zName, int create) {
  CollSeq *pColl;

  if (zName) {
    pColl = findCollSeqEntry(db, zName, create);
    if (pColl)
      pColl += enc - 1;
  } else {
    pColl = db->pDfltColl;
  }
  return pColl;
}

void sqlite3SetTextEncoding(sqlite3 *db, u8 enc) {
  db->enc = enc;

  db->pDfltColl = sqlite3FindCollSeq(db, enc, sqlite3StrBINARY, 0);
  sqlite3ExpirePreparedStatements(db, 1);
}

FuncDef *sqlite3FunctionSearch(int h, const char *zFunc) {
  FuncDef *p;
  for (p = sqlite3BuiltinFunctions.a[h]; p; p = p->u.pHash) {
    if (sqlite3StrICmp(p->zName, zFunc) == 0) {
      return p;
    }
  }
  return 0;
}

FuncDef *sqlite3FindFunction(sqlite3 *db, const char *zName, int nArg, u8 enc, u8 createFlag) {
  FuncDef *p;
  FuncDef *pBest = 0;
  int bestScore = 0;
  int h;
  int nName;

  nName = sqlite3Strlen30(zName);

  p = (FuncDef *)sqlite3HashFind(&db->aFunc, zName);
  while (p) {
    int score = matchQuality(p, nArg, enc);
    if (score > bestScore) {
      pBest = p;
      bestScore = score;
    }
    p = p->pNext;
  }

  if (!createFlag && (pBest == 0 || (db->mDbFlags & 0x0002) != 0)) {
    bestScore = 0;
    h = (((sqlite3UpperToLower[(u8)zName[0]]) + (nName)) % 23);
    p = sqlite3FunctionSearch(h, zName);
    while (p) {
      int score = matchQuality(p, nArg, enc);
      if (score > bestScore) {
        pBest = p;
        bestScore = score;
      }
      p = p->pNext;
    }
  }

  if (createFlag && bestScore < 6 && (pBest = sqlite3DbMallocZero(db, sizeof(*pBest) + nName + 1)) != 0) {
    FuncDef *pOther;
    u8 *z;
    pBest->zName = (const char *)&pBest[1];
    pBest->nArg = (u16)nArg;
    pBest->funcFlags = enc;
    memcpy((char *)&pBest[1], zName, nName + 1);
    for (z = (u8 *)pBest->zName; *z; z++)
      *z = sqlite3UpperToLower[*z];
    pOther = (FuncDef *)sqlite3HashInsert(&db->aFunc, pBest->zName, pBest);
    if (pOther == pBest) {
      sqlite3DbFree(db, pBest);
      sqlite3OomFault(db);
      return 0;
    } else {
      pBest->pNext = pOther;
    }
  }

  if (pBest && (pBest->xSFunc || createFlag)) {
    return pBest;
  }
  return 0;
}

Schema *sqlite3SchemaGet(sqlite3 *db, Btree *pBt) {
  Schema *p;
  if (pBt) {
    p = (Schema *)sqlite3BtreeSchema(pBt, sizeof(Schema), sqlite3SchemaClear);
  } else {
    p = (Schema *)sqlite3DbMallocZero(0, sizeof(Schema));
  }
  if (!p) {
    sqlite3OomFault(db);
  } else if (0 == p->file_format) {
    sqlite3HashInit(&p->tblHash);
    sqlite3HashInit(&p->idxHash);
    sqlite3HashInit(&p->trigHash);
    sqlite3HashInit(&p->fkeyHash);
    p->enc = SQLITE_UTF8;
  }
  return p;
}

int sqlite3_strglob(const char *zGlobPattern, const char *zString) {
  if (zString == 0) {
    return zGlobPattern != 0;
  } else if (zGlobPattern == 0) {
    return 1;
  } else {
    return patternCompare((u8 *)zGlobPattern, (u8 *)zString, &globInfo, '[');
  }
}

int sqlite3_strlike(const char *zPattern, const char *zStr, unsigned int esc) {
  if (zStr == 0) {
    return zPattern != 0;
  } else if (zPattern == 0) {
    return 1;
  } else {
    return patternCompare((u8 *)zPattern, (u8 *)zStr, &likeInfoNorm, esc);
  }
}

void sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db) {
  int rc = sqlite3_overload_function(db, "MATCH", 2);

  if (rc == SQLITE_NOMEM) {
    sqlite3OomFault(db);
  }
}

void sqlite3RegisterLikeFunctions(sqlite3 *db, int caseSensitive) {
  FuncDef *pDef;
  struct compareInfo *pInfo;
  int flags;
  int nArg;
  if (caseSensitive) {
    pInfo = (struct compareInfo *)&likeInfoAlt;
    flags = 0x0004 | 0x0008;
  } else {
    pInfo = (struct compareInfo *)&likeInfoNorm;
    flags = 0x0004;
  }
  for (nArg = 2; nArg <= 3; nArg++) {
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);

    pDef->funcFlags |= flags;
    pDef->funcFlags &= ~0x00200000;
  }
}

int sqlite3IsLikeFunction(sqlite3 *db, Expr *pExpr, int *pIsNocase, char *aWc) {
  FuncDef *pDef;
  int nExpr;

  if (!pExpr->x.pList) {
    return 0;
  }
  nExpr = pExpr->x.pList->nExpr;

  pDef = sqlite3FindFunction(db, pExpr->u.zToken, nExpr, SQLITE_UTF8, 0);

  if ((pDef == 0) || (pDef->funcFlags & 0x0004) == 0) {
    return 0;
  }

  memcpy(aWc, pDef->pUserData, 3);

  if (nExpr < 3) {
    aWc[3] = 0;
  } else {
    Expr *pEscape = pExpr->x.pList->a[2].pExpr;
    char *zEscape;
    if (pEscape->op != 118)
      return 0;

    zEscape = pEscape->u.zToken;
    if (zEscape[0] == 0 || zEscape[1] != 0)
      return 0;
    if (zEscape[0] == aWc[0])
      return 0;
    if (zEscape[0] == aWc[1])
      return 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & 0x0008) == 0;
  return 1;
}

static void sqlite3RegisterBuiltinFunctions(void) {
  static FuncDef aBuiltinFunc[] = {

      {2,
       0x00800000 | 1 | 0x00040000 | 0x4000 | 0x00400000 | 0x0800 | (0),
       ((void *)(intptr_t)(1)),
       0,
       versionFunc,
       0,
       0,
       0,
       "implies_nonnull_row",
       {0}},
      {2,
       0x00800000 | 1 | 0x00040000 | 0x4000 | 0x00400000 | 0x0800 | (0),
       ((void *)(intptr_t)(3)),
       0,
       versionFunc,
       0,
       0,
       0,
       "expr_compare",
       {0}},
      {2,
       0x00800000 | 1 | 0x00040000 | 0x4000 | 0x00400000 | 0x0800 | (0),
       ((void *)(intptr_t)(2)),
       0,
       versionFunc,
       0,
       0,
       0,
       "expr_implies_expr",
       {0}},
      {1,
       0x00800000 | 1 | 0x00040000 | 0x4000 | 0x00400000 | 0x0800 | (0),
       ((void *)(intptr_t)(4)),
       0,
       versionFunc,
       0,
       0,
       0,
       "affinity",
       {0}},
      {1,
       0x00800000 | 1 | 0x000080000 | 0x00200000,
       ((void *)(intptr_t)(0)),
       0,
       loadExt,
       0,
       0,
       0,
       "load_extension",
       {0}},
      {2,
       0x00800000 | 1 | 0x000080000 | 0x00200000,
       ((void *)(intptr_t)(0)),
       0,
       loadExt,
       0,
       0,
       0,
       "load_extension",
       {0}},
      {1, 0x00800000 | 0x2000 | 1, 0, 0, compileoptionusedFunc, 0, 0, 0, "sqlite_compileoption_used", {0}},
      {1, 0x00800000 | 0x2000 | 1, 0, 0, compileoptiongetFunc, 0, 0, 0, "sqlite_compileoption_get", {0}},
      {1,
       0x00800000 | 1 | 0x00400000 | 0x0800 | (0x0400),
       ((void *)(intptr_t)(99)),
       0,
       versionFunc,
       0,
       0,
       0,
       "unlikely",
       {0}},
      {2,
       0x00800000 | 1 | 0x00400000 | 0x0800 | (0x0400),
       ((void *)(intptr_t)(99)),
       0,
       versionFunc,
       0,
       0,
       0,
       "likelihood",
       {0}},
      {1,
       0x00800000 | 1 | 0x00400000 | 0x0800 | (0x0400),
       ((void *)(intptr_t)(99)),
       0,
       versionFunc,
       0,
       0,
       0,
       "likely",
       {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(1)), 0, trimFunc, 0, 0, 0, "ltrim", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(1)), 0, trimFunc, 0, 0, 0, "ltrim", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(2)), 0, trimFunc, 0, 0, 0, "rtrim", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(2)), 0, trimFunc, 0, 0, 0, "rtrim", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(3)), 0, trimFunc, 0, 0, 0, "trim", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(3)), 0, trimFunc, 0, 0, 0, "trim", {0}},
      {-3, 0x00800000 | 0x0800 | 1 | (1 * 0x0020), ((void *)(intptr_t)(0)), 0, minmaxFunc, 0, 0, 0, "min", {0}},
      {1,
       0x00800000 | 1 | (1 * 0x0020) | 0x1000 | 0x08000000,
       ((void *)(intptr_t)(0)),
       0,
       minmaxStep,
       minMaxFinalize,
       minMaxValue,
       0,
       "min",
       {0}},
      {-3, 0x00800000 | 0x0800 | 1 | (1 * 0x0020), ((void *)(intptr_t)(1)), 0, minmaxFunc, 0, 0, 0, "max", {0}},
      {1,
       0x00800000 | 1 | (1 * 0x0020) | 0x1000 | 0x08000000,
       ((void *)(intptr_t)(1)),
       0,
       minmaxStep,
       minMaxFinalize,
       minMaxValue,
       0,
       "max",
       {0}},
      {1,
       0x00800000 | 0x0800 | 1 | (0 * 0x0020) | 0x0080,
       ((void *)(intptr_t)(0)),
       0,
       typeofFunc,
       0,
       0,
       0,
       "typeof",
       {0}},
      {1,
       0x00800000 | 0x0800 | 1 | (0 * 0x0020) | 0x0080 | 0x000100000,
       ((void *)(intptr_t)(0)),
       0,
       subtypeFunc,
       0,
       0,
       0,
       "subtype",
       {0}},
      {1,
       0x00800000 | 0x0800 | 1 | (0 * 0x0020) | 0x0040,
       ((void *)(intptr_t)(0)),
       0,
       lengthFunc,
       0,
       0,
       0,
       "length",
       {0}},
      {1,
       0x00800000 | 0x0800 | 1 | (0 * 0x0020) | 0x00c0,
       ((void *)(intptr_t)(0)),
       0,
       bytelengthFunc,
       0,
       0,
       0,
       "octet_length",
       {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, instrFunc, 0, 0, 0, "instr", {0}},
      {-1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, printfFunc, 0, 0, 0, "printf", {0}},
      {-1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, printfFunc, 0, 0, 0, "format", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, unicodeFunc, 0, 0, 0, "unicode", {0}},
      {-1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, charFunc, 0, 0, 0, "char", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, absFunc, 0, 0, 0, "abs", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, roundFunc, 0, 0, 0, "round", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, roundFunc, 0, 0, 0, "round", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, upperFunc, 0, 0, 0, "upper", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, lowerFunc, 0, 0, 0, "lower", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, hexFunc, 0, 0, 0, "hex", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, unhexFunc, 0, 0, 0, "unhex", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, unhexFunc, 0, 0, 0, "unhex", {0}},
      {-3, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, concatFunc, 0, 0, 0, "concat", {0}},
      {-4, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, concatwsFunc, 0, 0, 0, "concat_ws", {0}},
      {2, 0x00800000 | 1 | 0x00400000 | 0x0800 | (0), ((void *)(intptr_t)(0)), 0, versionFunc, 0, 0, 0, "ifnull", {0}},
      {0, 0x00800000 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, randomFunc, 0, 0, 0, "random", {0}},
      {1, 0x00800000 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, randomBlob, 0, 0, 0, "randomblob", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (1 * 0x0020), ((void *)(intptr_t)(0)), 0, nullifFunc, 0, 0, 0, "nullif", {0}},
      {0, 0x00800000 | 0x2000 | 1, 0, 0, versionFunc, 0, 0, 0, "sqlite_version", {0}},
      {0, 0x00800000 | 0x2000 | 1, 0, 0, sourceidFunc, 0, 0, 0, "sqlite_source_id", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, errlogFunc, 0, 0, 0, "sqlite_log", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, unistrFunc, 0, 0, 0, "unistr", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, quoteFunc, 0, 0, 0, "quote", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(1)), 0, quoteFunc, 0, 0, 0, "unistr_quote", {0}},
      {0,
       0x00800000 | 1 | (0 * 0x0020),
       ((void *)(intptr_t)(0)),
       0,
       last_insert_rowid,
       0,
       0,
       0,
       "last_insert_rowid",
       {0}},
      {0, 0x00800000 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, changes, 0, 0, 0, "changes", {0}},
      {0, 0x00800000 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, total_changes, 0, 0, 0, "total_changes", {0}},
      {3, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, replaceFunc, 0, 0, 0, "replace", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, zeroblobFunc, 0, 0, 0, "zeroblob", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, substrFunc, 0, 0, 0, "substr", {0}},
      {3, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, substrFunc, 0, 0, 0, "substr", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, substrFunc, 0, 0, 0, "substring", {0}},
      {3, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, substrFunc, 0, 0, 0, "substring", {0}},
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0,
       ((void *)(intptr_t)(0)),
       0,
       sumStep,
       sumFinalize,
       sumFinalize,
       sumInverse,
       "sum",
       {0}},
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0,
       ((void *)(intptr_t)(0)),
       0,
       sumStep,
       totalFinalize,
       totalFinalize,
       sumInverse,
       "total",
       {0}},
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0,
       ((void *)(intptr_t)(0)),
       0,
       sumStep,
       avgFinalize,
       avgFinalize,
       sumInverse,
       "avg",
       {0}},
      {0,
       0x00800000 | 1 | (0 * 0x0020) | 0x0100 | 0x08000000,
       ((void *)(intptr_t)(0)),
       0,
       countStep,
       countFinalize,
       countFinalize,
       countInverse,
       "count",
       {0}}

      ,
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0x08000000,
       ((void *)(intptr_t)(0)),
       0,
       countStep,
       countFinalize,
       countFinalize,
       countInverse,
       "count",
       {0}},
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0,
       ((void *)(intptr_t)(0)),
       0,
       groupConcatStep,
       groupConcatFinalize,
       groupConcatValue,
       groupConcatInverse,
       "group_concat",
       {0}},
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0,
       ((void *)(intptr_t)(0)),
       0,
       groupConcatStep,
       groupConcatFinalize,
       groupConcatValue,
       groupConcatInverse,
       "group_concat",
       {0}},
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0,
       ((void *)(intptr_t)(0)),
       0,
       groupConcatStep,
       groupConcatFinalize,
       groupConcatValue,
       groupConcatInverse,
       "string_agg",
       {0}},
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0x000200000 | 0x002000000,
       ((void *)(intptr_t)(0)),
       0,
       percentStep,
       percentFinal,
       percentValue,
       percentInverse,
       "median",
       {0}}

      ,
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0x000200000 | 0x002000000,
       ((void *)(intptr_t)(0x2)),
       0,
       percentStep,
       percentFinal,
       percentValue,
       percentInverse,
       "percentile",
       {0}}

      ,
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0x000200000 | 0x002000000,
       ((void *)(intptr_t)(0)),
       0,
       percentStep,
       percentFinal,
       percentValue,
       percentInverse,
       "percentile_cont",
       {0}}

      ,
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0x000200000 | 0x002000000,
       ((void *)(intptr_t)(0x1)),
       0,
       percentStep,
       percentFinal,
       percentValue,
       percentInverse,
       "percentile_disc",
       {0}}

      ,
      {2, 0x00800000 | 0x0800 | 1 | 0x0004 | 0x0008, (void *)&globInfo, 0, likeFunc, 0, 0, 0, "glob", {0}},
      {2, 0x00800000 | 0x0800 | 1 | 0x0004, (void *)&likeInfoNorm, 0, likeFunc, 0, 0, 0, "like", {0}},
      {3, 0x00800000 | 0x0800 | 1 | 0x0004, (void *)&likeInfoNorm, 0, likeFunc, 0, 0, 0, "like", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)xCeil, 0, ceilingFunc, 0, 0, 0, "ceil", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)xCeil, 0, ceilingFunc, 0, 0, 0, "ceiling", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)xFloor, 0, ceilingFunc, 0, 0, 0, "floor", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)trunc, 0, ceilingFunc, 0, 0, 0, "trunc", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, logFunc, 0, 0, 0, "ln", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(1)), 0, logFunc, 0, 0, 0, "log", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(1)), 0, logFunc, 0, 0, 0, "log10", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(2)), 0, logFunc, 0, 0, 0, "log2", {0}},
      {2, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, logFunc, 0, 0, 0, "log", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)exp, 0, math1Func, 0, 0, 0, "exp", {0}},
      {2, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)pow, 0, math2Func, 0, 0, 0, "pow", {0}},
      {2, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)pow, 0, math2Func, 0, 0, 0, "power", {0}},
      {2, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)fmod, 0, math2Func, 0, 0, 0, "mod", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)acos, 0, math1Func, 0, 0, 0, "acos", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)asin, 0, math1Func, 0, 0, 0, "asin", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)atan, 0, math1Func, 0, 0, 0, "atan", {0}},
      {2, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)atan2, 0, math2Func, 0, 0, 0, "atan2", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)cos, 0, math1Func, 0, 0, 0, "cos", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)sin, 0, math1Func, 0, 0, 0, "sin", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)tan, 0, math1Func, 0, 0, 0, "tan", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)cosh, 0, math1Func, 0, 0, 0, "cosh", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)sinh, 0, math1Func, 0, 0, 0, "sinh", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)tanh, 0, math1Func, 0, 0, 0, "tanh", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)acosh, 0, math1Func, 0, 0, 0, "acosh", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)asinh, 0, math1Func, 0, 0, 0, "asinh", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)atanh, 0, math1Func, 0, 0, 0, "atanh", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)sqrt, 0, math1Func, 0, 0, 0, "sqrt", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)degToRad, 0, math1Func, 0, 0, 0, "radians", {0}},
      {1, 0x00800000 | 0x0800 | 1, (void *)(intptr_t)radToDeg, 0, math1Func, 0, 0, 0, "degrees", {0}},
      {0, 0x00800000 | 0x0800 | 1, 0, 0, piFunc, 0, 0, 0, "pi", {0}},
      {1, 0x00800000 | 0x0800 | 1 | (0 * 0x0020), ((void *)(intptr_t)(0)), 0, signFunc, 0, 0, 0, "sign", {0}},
      {-4,
       0x00800000 | 1 | 0x00400000 | 0x0800 | (0),
       ((void *)(intptr_t)(0)),
       0,
       versionFunc,
       0,
       0,
       0,
       "coalesce",
       {0}},
      {-4, 0x00800000 | 1 | 0x00400000 | 0x0800 | (0), ((void *)(intptr_t)(5)), 0, versionFunc, 0, 0, 0, "iif", {0}},
      {-4, 0x00800000 | 1 | 0x00400000 | 0x0800 | (0), ((void *)(intptr_t)(5)), 0, versionFunc, 0, 0, 0, "if", {0}},
  };

  sqlite3AlterFunctions();

  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3InsertBuiltinFuncs(aBuiltinFunc, ((int)(sizeof(aBuiltinFunc) / sizeof(aBuiltinFunc[0]))));
}

Expr *exprTableColumn(sqlite3 *db, Table *pTab, int iCursor, i16 iCol) {
  Expr *pExpr = sqlite3Expr(db, 168, 0);
  if (pExpr) {
    pExpr->y.pTab = pTab;
    pExpr->iTable = iCursor;
    pExpr->iColumn = iCol;
  }
  return pExpr;
}

void fkTriggerDelete(sqlite3 *dbMem, Trigger *p) {
  if (p) {
    TriggerStep *pStep = p->step_list;
    sqlite3SrcListDelete(dbMem, pStep->pSrc);
    sqlite3ExprDelete(dbMem, pStep->pWhere);
    sqlite3ExprListDelete(dbMem, pStep->pExprList);
    sqlite3SelectDelete(dbMem, pStep->pSelect);
    sqlite3ExprDelete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

void sqlite3FkClearTriggerCache(sqlite3 *db, int iDb) {
  HashElem *k;
  Hash *pHash = &db->aDb[iDb].pSchema->tblHash;
  for (k = ((pHash)->first); k; k = ((k)->next)) {
    Table *pTab = ((k)->data);
    FKey *pFKey;
    if (!((pTab)->eTabType == 0))
      continue;
    for (pFKey = pTab->u.tab.pFKey; pFKey; pFKey = pFKey->pNextFrom) {
      fkTriggerDelete(db, pFKey->apTrigger[0]);
      pFKey->apTrigger[0] = 0;
      fkTriggerDelete(db, pFKey->apTrigger[1]);
      pFKey->apTrigger[1] = 0;
    }
  }
}

void sqlite3FkDelete(sqlite3 *db, Table *pTab) {
  FKey *pFKey;
  FKey *pNext;

  for (pFKey = pTab->u.tab.pFKey; pFKey; pFKey = pNext) {
    if (db->pnBytesFreed == 0) {
      if (pFKey->pPrevTo) {
        pFKey->pPrevTo->pNextTo = pFKey->pNextTo;
      } else {
        const char *z = (pFKey->pNextTo ? pFKey->pNextTo->zTo : pFKey->zTo);
        sqlite3HashInsert(&pTab->pSchema->fkeyHash, z, pFKey->pNextTo);
      }
      if (pFKey->pNextTo) {
        pFKey->pNextTo->pPrevTo = pFKey->pPrevTo;
      }
    }

    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);

    pNext = pFKey->pNextFrom;
    sqlite3DbFree(db, pFKey);
  }
}

__attribute__((noinline)) const char *computeIndexAffStr(sqlite3 *db, Index *pIdx) {
  int n;
  Table *pTab = pIdx->pTable;
  pIdx->zColAff = (char *)sqlite3DbMallocRaw(0, pIdx->nColumn + 1);
  if (!pIdx->zColAff) {
    sqlite3OomFault(db);
    return 0;
  }
  for (n = 0; n < pIdx->nColumn; n++) {
    i16 x = pIdx->aiColumn[n];
    char aff;
    if (x >= 0) {
      aff = pTab->aCol[x].affinity;
    } else if (x == (-1)) {
      aff = 0x44;
    } else {
      aff = sqlite3ExprAffinity(pIdx->aColExpr->a[n].pExpr);
    }
    if (aff < 0x41)
      aff = 0x41;
    if (aff > 0x43)
      aff = 0x43;
    pIdx->zColAff[n] = aff;
  }
  pIdx->zColAff[n] = 0;
  return pIdx->zColAff;
}

const char *sqlite3IndexAffinityStr(sqlite3 *db, Index *pIdx) {
  if (!pIdx->zColAff)
    return computeIndexAffStr(db, pIdx);
  return pIdx->zColAff;
}

char *sqlite3TableAffinityStr(sqlite3 *db, const Table *pTab) {
  char *zColAff;
  zColAff = (char *)sqlite3DbMallocRaw(db, pTab->nCol + 1);
  if (zColAff) {
    int i, j;
    for (i = j = 0; i < pTab->nCol; i++) {
      if ((pTab->aCol[i].colFlags & 0x0020) == 0) {
        zColAff[j++] = pTab->aCol[i].affinity;
      }
    }
    do {
      zColAff[j--] = 0;
    } while (j >= 0 && zColAff[j] <= 0x41);
  }
  return zColAff;
}

int sqlite3_exec(sqlite3 *db, const char *zSql, sqlite3_callback xCallback, void *pArg, char **pzErrMsg) {
  int rc = SQLITE_OK;
  const char *zLeftover;
  sqlite3_stmt *pStmt = 0;
  char **azCols = 0;
  int callbackIsInit;

  if (!sqlite3SafetyCheckOk(db))
    return sqlite3MisuseError(142342);
  if (zSql == 0)
    zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  while (rc == SQLITE_OK && zSql[0]) {
    int nCol = 0;
    char **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);

    if (rc != SQLITE_OK) {
      continue;
    }
    if (!pStmt) {
      zSql = zLeftover;
      continue;
    }
    callbackIsInit = 0;

    while (1) {
      int i;
      rc = sqlite3_step(pStmt);

      if (xCallback && (SQLITE_ROW == rc || (SQLITE_DONE == rc && !callbackIsInit && db->flags & 0x00000100))) {
        if (!callbackIsInit) {
          nCol = sqlite3_column_count(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2 * nCol + 1) * sizeof(const char *));
          if (azCols == 0) {
            goto exec_out;
          }
          for (i = 0; i < nCol; i++) {
            azCols[i] = (char *)sqlite3_column_name(pStmt, i);
          }
          callbackIsInit = 1;
        }
        if (rc == SQLITE_ROW) {
          azVals = &azCols[nCol];
          for (i = 0; i < nCol; i++) {
            azVals[i] = (char *)sqlite3_column_text(pStmt, i);
            if (!azVals[i] && sqlite3_column_type(pStmt, i) != SQLITE_NULL) {
              sqlite3OomFault(db);
              goto exec_out;
            }
          }
          azVals[i] = 0;
        }
        if (xCallback(pArg, nCol, azVals, azCols)) {
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          goto exec_out;
        }
      }

      if (rc != SQLITE_ROW) {
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        while ((sqlite3CtypeMap[(unsigned char)(zSql[0])] & 0x01))
          zSql++;
        break;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  if (pStmt)
    sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  if (rc != SQLITE_OK && pzErrMsg) {
    *pzErrMsg = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    if (*pzErrMsg == 0) {
      rc = 7;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  } else if (pzErrMsg) {
    *pzErrMsg = 0;
  }

  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3LoadExtension(sqlite3 *db, const char *zFile, const char *zProc, char **pzErrMsg) {
  sqlite3_vfs *pVfs = db->pVfs;
  void *handle;
  sqlite3_loadext_entry xInit;
  char *zErrmsg = 0;
  const char *zEntry;
  char *zAltEntry = 0;
  void **aHandle;
  u64 nMsg = strlen(zFile);
  int ii;
  int rc;

  static const char *azEndings[] = {

      "so"};

  if (pzErrMsg)
    *pzErrMsg = 0;

  if ((db->flags & 0x00010000) == 0) {
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("not authorized");
    }
    return SQLITE_ERROR;
  }

  zEntry = zProc ? zProc : "sqlite3_extension_init";

  if (nMsg > 4096)
    goto extension_not_found;

  if (nMsg == 0)
    goto extension_not_found;

  handle = sqlite3OsDlOpen(pVfs, zFile);

  for (ii = 0; ii < ((int)(sizeof(azEndings) / sizeof(azEndings[0]))) && handle == 0; ii++) {
    char *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    if (zAltFile == 0)
      return 7;
    if (nMsg + strlen(azEndings[ii]) + 1 <= 4096) {
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }

  if (handle == 0)
    goto extension_not_found;
  xInit = (sqlite3_loadext_entry)sqlite3OsDlSym(pVfs, handle, zEntry);

  if (xInit == 0 && zProc == 0) {
    int iFile, iEntry, c;
    int ncFile = sqlite3Strlen30(zFile);
    int cnt = 0;
    zAltEntry = sqlite3_malloc64(ncFile + 30);
    if (zAltEntry == 0) {
      sqlite3OsDlClose(pVfs, handle);
      return 7;
    }
    do {
      memcpy(zAltEntry, "sqlite3_", 8);
      for (iFile = ncFile - 1; iFile >= 0 && !((zFile[iFile]) == '/'); iFile--) {
      }
      iFile++;
      if (sqlite3_strnicmp(zFile + iFile, "lib", 3) == 0)
        iFile += 3;
      for (iEntry = 8; (c = zFile[iFile]) != 0 && c != '.'; iFile++) {
        if ((sqlite3CtypeMap[(unsigned char)(c)] & 0x02) || (cnt && (sqlite3CtypeMap[(unsigned char)(c)] & 0x04))) {
          zAltEntry[iEntry++] = (char)sqlite3UpperToLower[(unsigned)c];
        }
      }
      memcpy(zAltEntry + iEntry, "_init", 6);
      zEntry = zAltEntry;
      xInit = (sqlite3_loadext_entry)sqlite3OsDlSym(pVfs, handle, zEntry);
    } while (xInit == 0 && (++cnt) < 2);
  }
  if (xInit == 0) {
    if (pzErrMsg) {
      nMsg += strlen(zEntry) + 300;
      *pzErrMsg = zErrmsg = sqlite3_malloc64(nMsg);
      if (zErrmsg) {
        sqlite3_snprintf((int)nMsg, zErrmsg, "no entry point [%s] in shared library [%s]", zEntry, zFile);
        sqlite3OsDlError(pVfs, nMsg - 1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(zAltEntry);
    return SQLITE_ERROR;
  }
  sqlite3_free(zAltEntry);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  if (rc) {
    if (rc == (0 | (1 << 8)))
      return SQLITE_OK;
    if (pzErrMsg) {
      *pzErrMsg = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    return SQLITE_ERROR;
  }

  aHandle = sqlite3DbMallocZero(db, sizeof(handle) * (db->nExtension + 1));
  if (aHandle == 0) {
    return 7;
  }
  if (db->nExtension > 0) {
    memcpy(aHandle, db->aExtension, sizeof(handle) * db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  return SQLITE_OK;

extension_not_found:
  if (pzErrMsg) {
    nMsg += 300;
    *pzErrMsg = zErrmsg = sqlite3_malloc64(nMsg);
    if (zErrmsg) {
      sqlite3_snprintf((int)nMsg, zErrmsg, "unable to open shared library [%.*s]", 4096, zFile);
      sqlite3OsDlError(pVfs, nMsg - 1, zErrmsg);
    }
  }
  return SQLITE_ERROR;
}

int sqlite3_load_extension(sqlite3 *db, const char *zFile, const char *zProc, char **pzErrMsg) {
  int rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3LoadExtension(db, zFile, zProc, pzErrMsg);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

void sqlite3CloseExtensions(sqlite3 *db) {
  int i;

  for (i = 0; i < db->nExtension; i++) {
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

int sqlite3_enable_load_extension(sqlite3 *db, int onoff) {
  sqlite3_mutex_enter(db->mutex);
  if (onoff) {
    db->flags |= 0x00010000 | 0x00020000;
  } else {
    db->flags &= ~(u64)(0x00010000 | 0x00020000);
  }
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_auto_extension(void (*xInit)(void)) {
  int rc = SQLITE_OK;

  rc = sqlite3_initialize();
  if (rc) {
    return rc;
  } else {
    u32 i;

    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);

    sqlite3_mutex_enter(mutex);
    for (i = 0; i < sqlite3Autoext.nExt; i++) {
      if (sqlite3Autoext.aExt[i] == xInit)
        break;
    }
    if (i == sqlite3Autoext.nExt) {
      u64 nByte = (sqlite3Autoext.nExt + 1) * sizeof(sqlite3Autoext.aExt[0]);
      void (**aNew)(void);
      aNew = sqlite3_realloc64(sqlite3Autoext.aExt, nByte);
      if (aNew == 0) {
        rc = 7;
      } else {
        sqlite3Autoext.aExt = aNew;
        sqlite3Autoext.aExt[sqlite3Autoext.nExt] = xInit;
        sqlite3Autoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);

    return rc;
  }
}

int sqlite3_cancel_auto_extension(void (*xInit)(void)) {
  sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);

  int i;
  int n = 0;

  sqlite3_mutex_enter(mutex);
  for (i = (int)sqlite3Autoext.nExt - 1; i >= 0; i--) {
    if (sqlite3Autoext.aExt[i] == xInit) {
      sqlite3Autoext.nExt--;
      sqlite3Autoext.aExt[i] = sqlite3Autoext.aExt[sqlite3Autoext.nExt];
      n++;
      break;
    }
  }
  sqlite3_mutex_leave(mutex);
  return n;
}

void sqlite3_reset_auto_extension(void) {
  if (sqlite3_initialize() == SQLITE_OK) {
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);

    sqlite3_mutex_enter(mutex);
    sqlite3_free(sqlite3Autoext.aExt);
    sqlite3Autoext.aExt = 0;
    sqlite3Autoext.nExt = 0;
    sqlite3_mutex_leave(mutex);
  }
}

void sqlite3AutoLoadExtensions(sqlite3 *db) {
  u32 i;
  int go = 1;
  int rc;
  sqlite3_loadext_entry xInit;

  if (sqlite3Autoext.nExt == 0) {
    return;
  }
  for (i = 0; go; i++) {
    char *zErrmsg;

    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);

    const sqlite3_api_routines *pThunk = &sqlite3Apis;

    sqlite3_mutex_enter(mutex);
    if (i >= sqlite3Autoext.nExt) {
      xInit = 0;
      go = 0;
    } else {
      xInit = (sqlite3_loadext_entry)sqlite3Autoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    if (xInit && (rc = xInit(db, &zErrmsg, pThunk)) != 0) {
      sqlite3ErrorWithMsg(db, rc, "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}

u8 sqlite3GetBoolean(const char *z, u8 dflt) {
  return getSafetyLevel(z, 1, dflt) != 0;
}

void setAllPagerFlags(sqlite3 *db) {
  if (db->autoCommit) {
    Db *pDb = db->aDb;
    int n = db->nDb;

    while ((n--) > 0) {
      if (pDb->pBt) {
        sqlite3BtreeSetPagerFlags(pDb->pBt, pDb->safety_level | (db->flags & 0x38));
      }
      pDb++;
    }
  }
}

const char *sqlite3JournalModename(int eMode) {
  static char *const azModeName[] = {"delete", "persist", "off", "truncate", "memory", "wal"};

  if (eMode == ((int)(sizeof(azModeName) / sizeof(azModeName[0]))))
    return 0;
  return azModeName[eMode];
}

int pragmaVtabConnect(sqlite3 *db, void *pAux, int argc, const char *const *argv, sqlite3_vtab **ppVtab, char **pzErr) {
  const PragmaName *pPragma = (const PragmaName *)pAux;
  PragmaVtab *pTab = 0;
  int rc;
  int i, j;
  char cSep = '(';
  StrAccum acc;
  char zBuf[200];

  (void)(argc);
  (void)(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, sizeof(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  for (i = 0, j = pPragma->iPragCName; i < pPragma->nPragCName; i++, j++) {
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  if (i == 0) {
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  if (pPragma->mPragFlg & 0x20) {
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  if (pPragma->mPragFlg & (0x40 | 0x80)) {
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);

  rc = sqlite3_declare_vtab(db, zBuf);
  if (rc == SQLITE_OK) {
    pTab = (PragmaVtab *)sqlite3_malloc(sizeof(PragmaVtab));
    if (pTab == 0) {
      rc = SQLITE_NOMEM;
    } else {
      memset(pTab, 0, sizeof(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  } else {
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab *)pTab;
  return rc;
}

Module *sqlite3PragmaVtabRegister(sqlite3 *db, const char *zName) {
  const PragmaName *pName;

  pName = pragmaLocate(zName + 7);
  if (pName == 0)
    return 0;
  if ((pName->mPragFlg & (0x10 | 0x20)) == 0)
    return 0;

  return sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (void *)pName, 0);
}

int sqlite3InitCallback(void *pInit, int argc, char **argv, char **NotUsed) {
  InitData *pData = (InitData *)pInit;
  sqlite3 *db = pData->db;
  int iDb = pData->iDb;

  (void)(NotUsed), (void)(argc);

  db->mDbFlags |= 0x0040;
  if (argv == 0)
    return 0;
  pData->nInitRow++;
  if (db->mallocFailed) {
    corruptSchema(pData, argv, 0);
    return 1;
  }

  if (argv[3] == 0) {
    corruptSchema(pData, argv, 0);
  } else if (argv[4] && 'c' == sqlite3UpperToLower[(unsigned char)argv[4][0]] &&
             'r' == sqlite3UpperToLower[(unsigned char)argv[4][1]]) {
    int rc;
    u8 saved_iDb = db->init.iDb;
    sqlite3_stmt *pStmt;

    db->init.iDb = iDb;
    if (sqlite3GetUInt32(argv[3], &db->init.newTnum) == 0 || (db->init.newTnum > pData->mxPage && pData->mxPage > 0)) {
      if (sqlite3Config.bExtraSchemaChecks) {
        corruptSchema(pData, argv, "invalid rootpage");
      }
    }
    db->init.orphanTrigger = 0;
    db->init.azInit = (const char **)argv;
    pStmt = 0;
    sqlite3Prepare(db, argv[4], -1, 0, 0, &pStmt, 0);
    rc = db->errCode;

    db->init.iDb = saved_iDb;

    if (SQLITE_OK != rc) {
      if (db->init.orphanTrigger) {
      } else {
        if (rc > pData->rc)
          pData->rc = rc;
        if (rc == SQLITE_NOMEM) {
          sqlite3OomFault(db);
        } else if (rc != SQLITE_INTERRUPT && (rc & 0xFF) != SQLITE_LOCKED) {
          corruptSchema(pData, argv, sqlite3_errmsg(db));
        }
      }
    }
    db->init.azInit = sqlite3StdType;
    sqlite3_finalize(pStmt);
  } else if (argv[1] == 0 || (argv[4] != 0 && argv[4][0] != 0)) {
    corruptSchema(pData, argv, 0);
  } else {
    Index *pIndex;
    pIndex = sqlite3FindIndex(db, argv[1], db->aDb[iDb].zDbSName);
    if (pIndex == 0) {
      corruptSchema(pData, argv, "orphan index");
    } else if (sqlite3GetUInt32(argv[3], &pIndex->tnum) == 0 || pIndex->tnum < 2 || pIndex->tnum > pData->mxPage ||
               sqlite3IndexHasDuplicateRootPage(pIndex)) {
      if (sqlite3Config.bExtraSchemaChecks) {
        corruptSchema(pData, argv, "invalid rootpage");
      }
    }
  }
  return 0;
}

int sqlite3InitOne(sqlite3 *db, int iDb, char **pzErrMsg, u32 mFlags) {
  int rc;
  int i;

  int size;

  Db *pDb;
  char const *azArg[6];
  int meta[5];
  InitData initData;
  const char *zSchemaTabName;
  int openedTransaction = 0;
  int mask = ((db->mDbFlags & 0x0040) | ~0x0040);

  db->init.busy = 1;

  azArg[0] = "table";
  azArg[1] = zSchemaTabName = ((!0) && (iDb == 1) ? "sqlite_temp_master" : "sqlite_master");
  azArg[2] = azArg[1];
  azArg[3] = "1";
  azArg[4] =
      "CREATE TABLE x(type text,name text,tbl_name text,"
      "rootpage int,sql text)";
  azArg[5] = 0;
  initData.db = db;
  initData.iDb = iDb;
  initData.rc = SQLITE_OK;
  initData.pzErrMsg = pzErrMsg;
  initData.mInitFlags = mFlags;
  initData.nInitRow = 0;
  initData.mxPage = 0;
  sqlite3InitCallback(&initData, 5, (char **)azArg, 0);
  db->mDbFlags &= mask;
  if (initData.rc) {
    rc = initData.rc;
    goto error_out;
  }

  pDb = &db->aDb[iDb];
  if (pDb->pBt == 0) {
    (db)->aDb[1].pSchema->schemaFlags |= (0x0001);
    rc = SQLITE_OK;
    goto error_out;
  }

  sqlite3BtreeEnter(pDb->pBt);
  if (sqlite3BtreeTxnState(pDb->pBt) == SQLITE_TXN_NONE) {
    rc = sqlite3BtreeBeginTrans(pDb->pBt, 0, 0);
    if (rc != SQLITE_OK) {
      sqlite3SetString(pzErrMsg, db, sqlite3ErrStr(rc));
      goto initone_error_out;
    }
    openedTransaction = 1;
  }

  for (i = 0; i < ((int)(sizeof(meta) / sizeof(meta[0]))); i++) {
    sqlite3BtreeGetMeta(pDb->pBt, i + 1, (u32 *)&meta[i]);
  }
  if ((db->flags & 0x02000000) != 0) {
    memset(meta, 0, sizeof(meta));
  }
  pDb->pSchema->schema_cookie = meta[1 - 1];

  if (meta[5 - 1]) {
    if (iDb == 0 && (db->mDbFlags & 0x0040) == 0) {
      u8 encoding;

      encoding = (u8)meta[5 - 1] & 3;
      if (encoding == 0)
        encoding = SQLITE_UTF8;

      sqlite3SetTextEncoding(db, encoding);
    } else {
      if ((meta[5 - 1] & 3) != ((db)->enc)) {
        sqlite3SetString(pzErrMsg, db,
                         "attached databases must use the same"
                         " text encoding as main database");
        rc = SQLITE_ERROR;
        goto initone_error_out;
      }
    }
  }
  pDb->pSchema->enc = ((db)->enc);

  if (pDb->pSchema->cache_size == 0) {
    size = sqlite3AbsInt32(meta[3 - 1]);
    if (size == 0) {
      size = -2000;
    }
    pDb->pSchema->cache_size = size;

    sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
  }

  pDb->pSchema->file_format = (u8)meta[2 - 1];
  if (pDb->pSchema->file_format == 0) {
    pDb->pSchema->file_format = 1;
  }
  if (pDb->pSchema->file_format > 4) {
    sqlite3SetString(pzErrMsg, db, "unsupported file format");
    rc = SQLITE_ERROR;
    goto initone_error_out;
  }

  if (iDb == 0 && meta[2 - 1] >= 4) {
    db->flags &= ~(u64)0x00000002;
  }

  initData.mxPage = sqlite3BtreeLastPage(pDb->pBt);
  {
    char *zSql;
    zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\".%s ORDER BY rowid", db->aDb[iDb].zDbSName, zSchemaTabName);

    {
      sqlite3_xauth xAuth;
      xAuth = db->xAuth;
      db->xAuth = 0;

      rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &initData, 0);

      db->xAuth = xAuth;
    }

    if (rc == SQLITE_OK)
      rc = initData.rc;
    sqlite3DbFree(db, zSql);

    if (rc == SQLITE_OK) {
      sqlite3AnalysisLoad(db, iDb);
    }
  }

  if (db->mallocFailed) {
    rc = 7;
    sqlite3ResetAllSchemasOfConnection(db);
    pDb = &db->aDb[iDb];
  } else if (rc == SQLITE_OK || ((db->flags & 0x08000000) && rc != SQLITE_NOMEM)) {
    (db)->aDb[iDb].pSchema->schemaFlags |= (0x0001);
    rc = SQLITE_OK;
  }

initone_error_out:
  if (openedTransaction) {
    sqlite3BtreeCommit(pDb->pBt);
  }
  sqlite3BtreeLeave(pDb->pBt);

error_out:
  if (rc) {
    if (rc == SQLITE_NOMEM || rc == (10 | (12 << 8))) {
      sqlite3OomFault(db);
    }
    sqlite3ResetOneSchema(db, iDb);
  }
  db->init.busy = 0;
  return rc;
}

int sqlite3Init(sqlite3 *db, char **pzErrMsg) {
  int i, rc;
  int commit_internal = !(db->mDbFlags & 0x0001);

  ((db)->enc) = ((db)->aDb[0].pSchema->enc);

  if (!(((db)->aDb[0].pSchema->schemaFlags & (0x0001)) == (0x0001))) {
    rc = sqlite3InitOne(db, 0, pzErrMsg, 0);
    if (rc)
      return rc;
  }

  for (i = db->nDb - 1; i > 0; i--) {
    if (!(((db)->aDb[i].pSchema->schemaFlags & (0x0001)) == (0x0001))) {
      rc = sqlite3InitOne(db, i, pzErrMsg, 0);
      if (rc)
        return rc;
    }
  }
  if (commit_internal) {
    sqlite3CommitInternalChanges(db);
  }
  return SQLITE_OK;
}

int sqlite3SchemaToIndex(sqlite3 *db, Schema *pSchema) {
  int i = -32768;

  if (pSchema) {
    for (i = 0; 1; i++) {
      if (db->aDb[i].pSchema == pSchema) {
        break;
      }
    }
  }
  return i;
}

int sqlite3Prepare(sqlite3 *db, const char *zSql, int nBytes, u32 prepFlags, Vdbe *pReprepare, sqlite3_stmt **ppStmt,
                   const char **pzTail) {
  int rc = SQLITE_OK;
  int i;
  Parse sParse;

  memset((((char *)(&sParse)) + offsetof(Parse, zErrMsg)), 0, (offsetof(Parse, aTempReg) - offsetof(Parse, zErrMsg)));
  memset((((char *)(&sParse)) + offsetof(Parse, sLastToken)), 0, (sizeof(Parse) - offsetof(Parse, sLastToken)));
  sParse.pOuterParse = db->pParse;
  db->pParse = &sParse;
  sParse.db = db;
  if (pReprepare) {
    sParse.pReprepare = pReprepare;
    sParse.explain = sqlite3_stmt_isexplain((sqlite3_stmt *)pReprepare);
  } else {
  }

  if (db->mallocFailed) {
    sqlite3ErrorMsg(&sParse, "out of memory");
    db->errCode = rc = SQLITE_NOMEM;
    goto end_prepare;
  }

  if (prepFlags & SQLITE_PREPARE_PERSISTENT) {
    sParse.disableLookaside++;
    db->lookaside.bDisable++;
    db->lookaside.sz = 0;
  }
  sParse.prepFlags = prepFlags & 0xff;

  if (!db->noSharedCache) {
    for (i = 0; i < db->nDb; i++) {
      Btree *pBt = db->aDb[i].pBt;
      if (pBt) {
        rc = sqlite3BtreeSchemaLocked(pBt);
        if (rc) {
          const char *zDb = db->aDb[i].zDbSName;
          sqlite3ErrorWithMsg(db, rc, "database schema is locked: %s", zDb);
          goto end_prepare;
        }
      }
    }
  }

  if (db->pDisconnect)
    sqlite3VtabUnlockList(db);

  if (nBytes >= 0 && (nBytes == 0 || zSql[nBytes - 1] != 0)) {
    char *zSqlCopy;
    int mxLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
    if (nBytes > mxLen) {
      sqlite3ErrorWithMsg(db, SQLITE_TOOBIG, "statement too long");
      rc = sqlite3ApiExit(db, SQLITE_TOOBIG);
      goto end_prepare;
    }
    zSqlCopy = sqlite3DbStrNDup(db, zSql, nBytes);
    if (zSqlCopy) {
      sqlite3RunParser(&sParse, zSqlCopy);
      sParse.zTail = &zSql[sParse.zTail - zSqlCopy];
      sqlite3DbFree(db, zSqlCopy);
    } else {
      sParse.zTail = &zSql[nBytes];
    }
  } else {
    sqlite3RunParser(&sParse, zSql);
  }

  if (pzTail) {
    *pzTail = sParse.zTail;
  }

  if (db->init.busy == 0) {
    sqlite3VdbeSetSql(sParse.pVdbe, zSql, (int)(sParse.zTail - zSql), prepFlags);
  }
  if (db->mallocFailed) {
    sParse.rc = 7;
    sParse.checkSchema = 0;
  }
  if (sParse.rc != SQLITE_OK && sParse.rc != SQLITE_DONE) {
    if (sParse.checkSchema && db->init.busy == 0) {
      schemaIsValid(&sParse);
    }
    if (sParse.pVdbe) {
      sqlite3VdbeFinalize(sParse.pVdbe);
    }

    rc = sParse.rc;
    if (sParse.zErrMsg) {
      sqlite3ErrorWithMsg(db, rc, "%s", sParse.zErrMsg);
      sqlite3DbFree(db, sParse.zErrMsg);
    } else {
      sqlite3Error(db, rc);
    }
  } else {
    *ppStmt = (sqlite3_stmt *)sParse.pVdbe;
    rc = SQLITE_OK;
    sqlite3ErrorClear(db);
  }

  while (sParse.pTriggerPrg) {
    TriggerPrg *pT = sParse.pTriggerPrg;
    sParse.pTriggerPrg = pT->pNext;
    sqlite3DbFree(db, pT);
  }

end_prepare:
  sqlite3ParseObjectReset(&sParse);
  return rc;
}

int sqlite3LockAndPrepare(sqlite3 *db, const char *zSql, int nBytes, u32 prepFlags, Vdbe *pOld, sqlite3_stmt **ppStmt,
                          const char **pzTail) {
  int rc;
  int cnt = 0;

  *ppStmt = 0;
  if (!sqlite3SafetyCheckOk(db) || zSql == 0) {
    return sqlite3MisuseError(148759);
  }
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  do {
    rc = sqlite3Prepare(db, zSql, nBytes, prepFlags, pOld, ppStmt, pzTail);

    if (rc == SQLITE_OK || db->mallocFailed)
      break;
    cnt++;
  } while ((rc == (1 | (2 << 8)) && (cnt <= 25)) || (rc == SQLITE_SCHEMA && (sqlite3ResetOneSchema(db, -1), cnt) == 1));
  sqlite3BtreeLeaveAll(db);

  rc = sqlite3ApiExit(db, rc);

  db->busyHandler.nBusy = 0;
  sqlite3_mutex_leave(db->mutex);

  return rc;
}

int sqlite3_prepare(sqlite3 *db, const char *zSql, int nBytes, sqlite3_stmt **ppStmt, const char **pzTail) {
  int rc;
  rc = sqlite3LockAndPrepare(db, zSql, nBytes, 0, 0, ppStmt, pzTail);

  return rc;
}

int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nBytes, sqlite3_stmt **ppStmt, const char **pzTail) {
  int rc;

  rc = sqlite3LockAndPrepare(db, zSql, nBytes, 0x80, 0, ppStmt, pzTail);

  return rc;
}

int sqlite3_prepare_v3(sqlite3 *db, const char *zSql, int nBytes, unsigned int prepFlags, sqlite3_stmt **ppStmt,
                       const char **pzTail) {
  int rc;

  rc = sqlite3LockAndPrepare(db, zSql, nBytes, 0x80 | (prepFlags & 0x3f), 0, ppStmt, pzTail);

  return rc;
}

int sqlite3Prepare16(sqlite3 *db, const void *zSql, int nBytes, u32 prepFlags, sqlite3_stmt **ppStmt,
                     const void **pzTail) {
  char *zSql8;
  const char *zTail8 = 0;
  int rc = SQLITE_OK;

  *ppStmt = 0;
  if (!sqlite3SafetyCheckOk(db) || zSql == 0) {
    return sqlite3MisuseError(148910);
  }

  if (nBytes >= 0) {
    int sz;
    const char *z = (const char *)zSql;
    for (sz = 0; sz < nBytes && (z[sz] != 0 || z[sz + 1] != 0); sz += 2) {
    }
    nBytes = sz;
  } else {
    int sz;
    const char *z = (const char *)zSql;
    for (sz = 0; z[sz] != 0 || z[sz + 1] != 0; sz += 2) {
    }
    nBytes = sz;
  }

  sqlite3_mutex_enter(db->mutex);
  zSql8 = sqlite3Utf16to8(db, zSql, nBytes, 2);
  if (zSql8) {
    rc = sqlite3LockAndPrepare(db, zSql8, -1, prepFlags, 0, ppStmt, &zTail8);
  }

  if (zTail8 && pzTail) {
    int chars_parsed = sqlite3Utf8CharLen(zSql8, (int)(zTail8 - zSql8));
    *pzTail = (u8 *)zSql + sqlite3Utf16ByteLen(zSql, nBytes, chars_parsed);
  }
  sqlite3DbFree(db, zSql8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_prepare16(sqlite3 *db, const void *zSql, int nBytes, sqlite3_stmt **ppStmt, const void **pzTail) {
  int rc;
  rc = sqlite3Prepare16(db, zSql, nBytes, 0, ppStmt, pzTail);

  return rc;
}

int sqlite3_prepare16_v2(sqlite3 *db, const void *zSql, int nBytes, sqlite3_stmt **ppStmt, const void **pzTail) {
  int rc;
  rc = sqlite3Prepare16(db, zSql, nBytes, 0x80, ppStmt, pzTail);

  return rc;
}

int sqlite3_prepare16_v3(sqlite3 *db, const void *zSql, int nBytes, unsigned int prepFlags, sqlite3_stmt **ppStmt,
                         const void **pzTail) {
  int rc;
  rc = sqlite3Prepare16(db, zSql, nBytes, 0x80 | (prepFlags & 0x3f), ppStmt, pzTail);

  return rc;
}

void clearSelect(sqlite3 *db, Select *p, int bFree) {
  while (p) {
    Select *pPrior = p->pPrior;
    sqlite3ExprListDelete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3ExprDelete(db, p->pWhere);
    sqlite3ExprListDelete(db, p->pGroupBy);
    sqlite3ExprDelete(db, p->pHaving);
    sqlite3ExprListDelete(db, p->pOrderBy);
    sqlite3ExprDelete(db, p->pLimit);
    if ((p->pWith))
      sqlite3WithDelete(db, p->pWith);

    if ((p->pWinDefn)) {
      sqlite3WindowListDelete(db, p->pWinDefn);
    }
    while (p->pWin) {
      sqlite3WindowUnlinkFromSelect(p->pWin);
    }

    if (bFree)
      sqlite3DbNNFreeNN(db, p);
    p = pPrior;
    bFree = 1;
  }
}

void sqlite3SelectDelete(sqlite3 *db, Select *p) {
  if ((p))
    clearSelect(db, p, 1);
}

void sqlite3SelectDeleteGeneric(sqlite3 *db, void *p) {
  if ((p))
    clearSelect(db, (Select *)p, 1);
}

KeyInfo *sqlite3KeyInfoAlloc(sqlite3 *db, int N, int X) {
  int nExtra = (N + X) * (sizeof(CollSeq *) + 1);
  KeyInfo *p;

  if ((N + X > 0xffff))
    return (KeyInfo *)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, (offsetof(KeyInfo, aColl) + (0) * sizeof(CollSeq *)) + nExtra);
  if (p) {
    p->aSortFlags = (u8 *)&p->aColl[N + X];
    p->nKeyField = (u16)N;
    p->nAllField = (u16)(N + X);
    p->enc = ((db)->enc);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  } else {
    return (KeyInfo *)sqlite3OomFault(db);
  }
  return p;
}

u8 minMaxQuery(sqlite3 *db, Expr *pFunc, ExprList **ppMinMax) {
  int eRet = 0x0000;
  ExprList *pEList;
  const char *zFunc;
  ExprList *pOrderBy;
  u8 sortFlags = 0;

  pEList = pFunc->x.pList;
  if (pEList == 0 || pEList->nExpr != 1 || (((pFunc)->flags & (u32)(0x1000000)) != 0) ||
      (((db)->dbOptFlags & (0x00010000)) != 0)) {
    return eRet;
  }

  zFunc = pFunc->u.zToken;
  if (sqlite3StrICmp(zFunc, "min") == 0) {
    eRet = 0x0001;
    if (sqlite3ExprCanBeNull(pEList->a[0].pExpr)) {
      sortFlags = 0x02;
    }
  } else if (sqlite3StrICmp(zFunc, "max") == 0) {
    eRet = 0x0002;
    sortFlags = 0x01;
  } else {
    return eRet;
  }
  *ppMinMax = pOrderBy = sqlite3ExprListDup(db, pEList, 0);

  if (pOrderBy)
    pOrderBy->a[0].fg.sortFlags = sortFlags;
  return eRet;
}

void agginfoFree(sqlite3 *db, void *pArg) {
  AggInfo *p = (AggInfo *)pArg;
  sqlite3DbFree(db, p->aCol);
  sqlite3DbFree(db, p->aFunc);
  sqlite3DbFreeNN(db, p);
}

int sqlite3_get_table_cb(void *pArg, int nCol, char **argv, char **colv) {
  TabResult *p = (TabResult *)pArg;
  int need;
  int i;
  char *z;

  if (p->nRow == 0 && argv != 0) {
    need = nCol * 2;
  } else {
    need = nCol;
  }
  if (p->nData + need > p->nAlloc) {
    char **azNew;
    p->nAlloc = p->nAlloc * 2 + need;
    azNew = sqlite3Realloc(p->azResult, sizeof(char *) * p->nAlloc);
    if (azNew == 0)
      goto malloc_failed;
    p->azResult = azNew;
  }

  if (p->nRow == 0) {
    p->nColumn = nCol;
    for (i = 0; i < nCol; i++) {
      z = sqlite3_mprintf("%s", colv[i]);
      if (z == 0)
        goto malloc_failed;
      p->azResult[p->nData++] = z;
    }
  } else if ((int)p->nColumn != nCol) {
    sqlite3_free(p->zErrMsg);
    p->zErrMsg = sqlite3_mprintf("sqlite3_get_table() called with two or more incompatible queries");
    p->rc = SQLITE_ERROR;
    return 1;
  }

  if (argv != 0) {
    for (i = 0; i < nCol; i++) {
      if (argv[i] == 0) {
        z = 0;
      } else {
        int n = sqlite3Strlen30(argv[i]) + 1;
        z = sqlite3_malloc64(n);
        if (z == 0)
          goto malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  return 0;

malloc_failed:
  p->rc = 7;
  return 1;
}

int sqlite3_get_table(sqlite3 *db, const char *zSql, char ***pazResult, int *pnRow, int *pnColumn, char **pzErrMsg) {
  int rc;
  TabResult res;

  *pazResult = 0;
  if (pnColumn)
    *pnColumn = 0;
  if (pnRow)
    *pnRow = 0;
  if (pzErrMsg)
    *pzErrMsg = 0;
  res.zErrMsg = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(sizeof(char *) * res.nAlloc);
  if (res.azResult == 0) {
    db->errCode = SQLITE_NOMEM;
    return 7;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pzErrMsg);

  res.azResult[0] = ((void *)(intptr_t)(res.nData));
  if ((rc & 0xff) == SQLITE_ABORT) {
    sqlite3_free_table(&res.azResult[1]);
    if (res.zErrMsg) {
      if (pzErrMsg) {
        sqlite3_free(*pzErrMsg);
        *pzErrMsg = sqlite3_mprintf("%s", res.zErrMsg);
      }
      sqlite3_free(res.zErrMsg);
    }
    db->errCode = res.rc;
    return res.rc;
  }
  sqlite3_free(res.zErrMsg);
  if (rc != SQLITE_OK) {
    sqlite3_free_table(&res.azResult[1]);
    return rc;
  }
  if (res.nAlloc > res.nData) {
    char **azNew;
    azNew = sqlite3Realloc(res.azResult, sizeof(char *) * res.nData);
    if (azNew == 0) {
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      return 7;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  if (pnColumn)
    *pnColumn = res.nColumn;
  if (pnRow)
    *pnRow = res.nRow;
  return rc;
}

void sqlite3DeleteTriggerStep(sqlite3 *db, TriggerStep *pTriggerStep) {
  while (pTriggerStep) {
    TriggerStep *pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    sqlite3ExprDelete(db, pTmp->pWhere);
    sqlite3ExprListDelete(db, pTmp->pExprList);
    sqlite3SelectDelete(db, pTmp->pSelect);
    sqlite3IdListDelete(db, pTmp->pIdList);
    sqlite3UpsertDelete(db, pTmp->pUpsert);
    sqlite3SrcListDelete(db, pTmp->pSrc);
    sqlite3DbFree(db, pTmp->zSpan);

    sqlite3DbFree(db, pTmp);
  }
}

char *triggerSpanDup(sqlite3 *db, const char *zStart, const char *zEnd) {
  char *z = sqlite3DbSpanDup(db, zStart, zEnd);
  int i;
  if (z)
    for (i = 0; z[i]; i++)
      if ((sqlite3CtypeMap[(unsigned char)(z[i])] & 0x01))
        z[i] = ' ';
  return z;
}

TriggerStep *sqlite3TriggerSelectStep(sqlite3 *db, Select *pSelect, const char *zStart, const char *zEnd) {
  TriggerStep *pTriggerStep = sqlite3DbMallocZero(db, sizeof(TriggerStep));
  if (pTriggerStep == 0) {
    sqlite3SelectDelete(db, pSelect);
    return 0;
  }
  pTriggerStep->op = 139;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = 11;
  pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
  return pTriggerStep;
}

void sqlite3DeleteTrigger(sqlite3 *db, Trigger *pTrigger) {
  if (pTrigger == 0 || pTrigger->bReturning)
    return;
  sqlite3DeleteTriggerStep(db, pTrigger->step_list);
  sqlite3DbFree(db, pTrigger->zName);
  sqlite3DbFree(db, pTrigger->table);
  sqlite3ExprDelete(db, pTrigger->pWhen);
  sqlite3IdListDelete(db, pTrigger->pColumns);
  sqlite3DbFree(db, pTrigger);
}

void sqlite3UnlinkAndDeleteTrigger(sqlite3 *db, int iDb, const char *zName) {
  Trigger *pTrigger;
  Hash *pHash;

  pHash = &(db->aDb[iDb].pSchema->trigHash);
  pTrigger = sqlite3HashInsert(pHash, zName, 0);
  if ((pTrigger)) {
    if (pTrigger->pSchema == pTrigger->pTabSchema) {
      Table *pTab = tableOfTrigger(pTrigger);
      if (pTab) {
        Trigger **pp;
        for (pp = &pTab->pTrigger; *pp; pp = &((*pp)->pNext)) {
          if (*pp == pTrigger) {
            *pp = (*pp)->pNext;
            break;
          }
        }
      }
    }
    sqlite3DeleteTrigger(db, pTrigger);
    db->mDbFlags |= 0x0001;
  }
}

int tempTriggersExist(sqlite3 *db) {
  if (db->aDb[1].pSchema == 0)
    return 0;
  if (((&db->aDb[1].pSchema->trigHash)->first) == 0)
    return 0;
  return 1;
}

void __attribute__((noinline)) upsertDelete(sqlite3 *db, Upsert *p) {
  do {
    Upsert *pNext = p->pNextUpsert;
    sqlite3ExprListDelete(db, p->pUpsertTarget);
    sqlite3ExprDelete(db, p->pUpsertTargetWhere);
    sqlite3ExprListDelete(db, p->pUpsertSet);
    sqlite3ExprDelete(db, p->pUpsertWhere);
    sqlite3DbFree(db, p->pToFree);
    sqlite3DbFree(db, p);
    p = pNext;
  } while (p);
}

void sqlite3UpsertDelete(sqlite3 *db, Upsert *p) {
  if (p)
    upsertDelete(db, p);
}

Upsert *sqlite3UpsertDup(sqlite3 *db, Upsert *p) {
  if (p == 0)
    return 0;
  return sqlite3UpsertNew(db, sqlite3ExprListDup(db, p->pUpsertTarget, 0), sqlite3ExprDup(db, p->pUpsertTargetWhere, 0),
                          sqlite3ExprListDup(db, p->pUpsertSet, 0), sqlite3ExprDup(db, p->pUpsertWhere, 0),
                          sqlite3UpsertDup(db, p->pNextUpsert));
}

Upsert *sqlite3UpsertNew(sqlite3 *db, ExprList *pTarget, Expr *pTargetWhere, ExprList *pSet, Expr *pWhere,
                         Upsert *pNext) {
  Upsert *pNew;
  pNew = sqlite3DbMallocZero(db, sizeof(Upsert));
  if (pNew == 0) {
    sqlite3ExprListDelete(db, pTarget);
    sqlite3ExprDelete(db, pTargetWhere);
    sqlite3ExprListDelete(db, pSet);
    sqlite3ExprDelete(db, pWhere);
    sqlite3UpsertDelete(db, pNext);
    return 0;
  } else {
    pNew->pUpsertTarget = pTarget;
    pNew->pUpsertTargetWhere = pTargetWhere;
    pNew->pUpsertSet = pSet;
    pNew->pUpsertWhere = pWhere;
    pNew->isDoUpdate = pSet != 0;
    pNew->pNextUpsert = pNext;
  }
  return pNew;
}

int execSql(sqlite3 *db, char **pzErrMsg, const char *zSql) {
  sqlite3_stmt *pStmt;
  int rc;

  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if (rc != SQLITE_OK)
    return rc;
  while (SQLITE_ROW == (rc = sqlite3_step(pStmt))) {
    const char *zSubSql = (const char *)sqlite3_column_text(pStmt, 0);

    if (zSubSql && (strncmp(zSubSql, "CRE", 3) == 0 || strncmp(zSubSql, "INS", 3) == 0)) {
      rc = execSql(db, pzErrMsg, zSubSql);
      if (rc != SQLITE_OK)
        break;
    }
  }

  if (rc == SQLITE_DONE)
    rc = SQLITE_OK;
  if (rc) {
    sqlite3SetString(pzErrMsg, db, sqlite3_errmsg(db));
  }
  (void)sqlite3_finalize(pStmt);
  return rc;
}

int execSqlF(sqlite3 *db, char **pzErrMsg, const char *zSql, ...) {
  char *z;
  va_list ap;
  int rc;

  va_start(ap, zSql);
  z = sqlite3VMPrintf(db, zSql, ap);

  va_end(ap);
  if (z == 0)
    return SQLITE_NOMEM;
  rc = execSql(db, pzErrMsg, z);
  sqlite3DbFree(db, z);
  return rc;
}

__attribute__((noinline)) int sqlite3RunVacuum(char **pzErrMsg, sqlite3 *db, int iDb, sqlite3_value *pOut) {
  int rc = SQLITE_OK;
  Btree *pMain;
  Btree *pTemp;
  u32 saved_mDbFlags;
  u64 saved_flags;
  i64 saved_nChange;
  i64 saved_nTotalChange;
  u32 saved_openFlags;
  u8 saved_mTrace;
  Db *pDb = 0;
  int isMemDb;
  int nRes;
  int nDb;
  const char *zDbMain;
  const char *zOut;
  u32 pgflags = 0x01;
  u64 iRandom;
  char zDbVacuum[42];

  if (!db->autoCommit) {
    sqlite3SetString(pzErrMsg, db, "cannot VACUUM from within a transaction");
    return SQLITE_ERROR;
  }
  if (db->nVdbeActive > 1) {
    sqlite3SetString(pzErrMsg, db, "cannot VACUUM - SQL statements in progress");
    return SQLITE_ERROR;
  }
  saved_openFlags = db->openFlags;
  if (pOut) {
    if (sqlite3_value_type(pOut) != 3) {
      sqlite3SetString(pzErrMsg, db, "non-text filename");
      return SQLITE_ERROR;
    }
    zOut = (const char *)sqlite3_value_text(pOut);
    db->openFlags &= ~SQLITE_OPEN_READONLY;
    db->openFlags |= SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
  } else {
    zOut = "";
  }

  saved_flags = db->flags;
  saved_mDbFlags = db->mDbFlags;
  saved_nChange = db->nChange;
  saved_nTotalChange = db->nTotalChange;
  saved_mTrace = db->mTrace;
  db->flags |= 0x00000001 | 0x00000200 | ((u64)(0x00040) << 32) | ((u64)(0x00010) << 32) | ((u64)(0x00020) << 32);
  db->mDbFlags |= 0x0002 | 0x0004;
  db->flags &= ~(u64)(0x00004000 | 0x00001000 | 0x10000000 | ((u64)(0x00001) << 32));
  db->mTrace = 0;

  zDbMain = db->aDb[iDb].zDbSName;
  pMain = db->aDb[iDb].pBt;
  isMemDb = sqlite3PagerIsMemdb(sqlite3BtreePager(pMain));

  sqlite3_randomness(sizeof(iRandom), &iRandom);
  sqlite3_snprintf(sizeof(zDbVacuum), zDbVacuum, "vacuum_%016llx", iRandom);
  nDb = db->nDb;
  rc = execSqlF(db, pzErrMsg, "ATTACH %Q AS %s", zOut, zDbVacuum);
  db->openFlags = saved_openFlags;
  if (rc != SQLITE_OK)
    goto end_of_vacuum;

  pDb = &db->aDb[nDb];

  pTemp = pDb->pBt;
  nRes = sqlite3BtreeGetRequestedReserve(pMain);
  if (pOut) {
    sqlite3_file *id = sqlite3PagerFile(sqlite3BtreePager(pTemp));
    i64 sz = 0;
    const char *zFilename;
    if (id->pMethods != 0 && (sqlite3OsFileSize(id, &sz) != SQLITE_OK || sz > 0)) {
      rc = SQLITE_ERROR;
      sqlite3SetString(pzErrMsg, db, "output file already exists");
      goto end_of_vacuum;
    }
    db->mDbFlags |= 0x0008;

    pgflags = db->aDb[iDb].safety_level | (db->flags & 0x38);

    zFilename = sqlite3BtreeGetFilename(pTemp);
    if ((zFilename)) {
      int nNew = (int)sqlite3_uri_int64(zFilename, "reserve", nRes);
      if (nNew >= 0 && nNew <= 255)
        nRes = nNew;
    }
  }

  sqlite3BtreeSetCacheSize(pTemp, db->aDb[iDb].pSchema->cache_size);
  sqlite3BtreeSetSpillSize(pTemp, sqlite3BtreeSetSpillSize(pMain, 0));
  sqlite3BtreeSetPagerFlags(pTemp, pgflags | 0x20);

  rc = execSql(db, pzErrMsg, "BEGIN");
  if (rc != SQLITE_OK)
    goto end_of_vacuum;
  rc = sqlite3BtreeBeginTrans(pMain, pOut == 0 ? 2 : 0, 0);
  if (rc != SQLITE_OK)
    goto end_of_vacuum;

  if (sqlite3PagerGetJournalMode(sqlite3BtreePager(pMain)) == 5 && pOut == 0) {
    db->nextPagesize = 0;
  }

  if (sqlite3BtreeSetPageSize(pTemp, sqlite3BtreeGetPageSize(pMain), nRes, 0) ||
      (!isMemDb && sqlite3BtreeSetPageSize(pTemp, db->nextPagesize, nRes, 0)) || (db->mallocFailed)) {
    rc = 7;
    goto end_of_vacuum;
  }

  sqlite3BtreeSetAutoVacuum(pTemp, db->nextAutovac >= 0 ? db->nextAutovac : sqlite3BtreeGetAutoVacuum(pMain));

  db->init.iDb = nDb;
  rc = execSqlF(db, pzErrMsg,
                "SELECT sql FROM \"%w\".sqlite_schema"
                " WHERE type='table'AND name<>'sqlite_sequence'"
                " AND coalesce(rootpage,1)>0",
                zDbMain);
  if (rc != SQLITE_OK)
    goto end_of_vacuum;
  rc = execSqlF(db, pzErrMsg,
                "SELECT sql FROM \"%w\".sqlite_schema"
                " WHERE type='index'",
                zDbMain);
  if (rc != SQLITE_OK)
    goto end_of_vacuum;
  db->init.iDb = 0;

  rc = execSqlF(db, pzErrMsg,
                "SELECT'INSERT INTO %s.'||quote(name)"
                "||' SELECT*FROM\"%w\".'||quote(name)"
                "FROM %s.sqlite_schema "
                "WHERE type='table'AND coalesce(rootpage,1)>0",
                zDbVacuum, zDbMain, zDbVacuum);

  db->mDbFlags &= ~0x0004;
  if (rc != SQLITE_OK)
    goto end_of_vacuum;

  rc = execSqlF(db, pzErrMsg,
                "INSERT INTO %s.sqlite_schema"
                " SELECT*FROM \"%w\".sqlite_schema"
                " WHERE type IN('view','trigger')"
                " OR(type='table'AND rootpage=0)",
                zDbVacuum, zDbMain);
  if (rc)
    goto end_of_vacuum;

  {
    u32 meta;
    int i;

    static const unsigned char aCopy[] = {
        1, 1, 3, 0, 5, 0, 6, 0, 8, 0,
    };

    for (i = 0; i < ((int)(sizeof(aCopy) / sizeof(aCopy[0]))); i += 2) {
      sqlite3BtreeGetMeta(pMain, aCopy[i], &meta);
      rc = sqlite3BtreeUpdateMeta(pTemp, aCopy[i], meta + aCopy[i + 1]);
      if ((rc != SQLITE_OK))
        goto end_of_vacuum;
    }

    if (pOut == 0) {
      rc = sqlite3BtreeCopyFile(pMain, pTemp);
    }
    if (rc != SQLITE_OK)
      goto end_of_vacuum;
    rc = sqlite3BtreeCommit(pTemp);
    if (rc != SQLITE_OK)
      goto end_of_vacuum;

    if (pOut == 0) {
      sqlite3BtreeSetAutoVacuum(pMain, sqlite3BtreeGetAutoVacuum(pTemp));
    }
  }

  if (pOut == 0) {
    nRes = sqlite3BtreeGetRequestedReserve(pTemp);
    rc = sqlite3BtreeSetPageSize(pMain, sqlite3BtreeGetPageSize(pTemp), nRes, 1);
  }

end_of_vacuum:
  db->init.iDb = 0;
  db->mDbFlags = saved_mDbFlags;
  db->flags = saved_flags;
  db->nChange = saved_nChange;
  db->nTotalChange = saved_nTotalChange;
  db->mTrace = saved_mTrace;
  sqlite3BtreeSetPageSize(pMain, -1, 0, 1);

  db->autoCommit = 1;

  if (pDb) {
    sqlite3BtreeClose(pDb->pBt);
    pDb->pBt = 0;
    pDb->pSchema = 0;
  }

  sqlite3ResetAllSchemasOfConnection(db);

  return rc;
}

Module *sqlite3VtabCreateModule(sqlite3 *db, const char *zName, const sqlite3_module *pModule, void *pAux,
                                void (*xDestroy)(void *)) {
  Module *pMod;
  Module *pDel;
  char *zCopy;
  if (pModule == 0) {
    zCopy = (char *)zName;
    pMod = 0;
  } else {
    int nName = sqlite3Strlen30(zName);
    pMod = (Module *)sqlite3Malloc(sizeof(Module) + nName + 1);
    if (pMod == 0) {
      sqlite3OomFault(db);
      return 0;
    }
    zCopy = (char *)(&pMod[1]);
    memcpy(zCopy, zName, nName + 1);
    pMod->zName = zCopy;
    pMod->pModule = pModule;
    pMod->pAux = pAux;
    pMod->xDestroy = xDestroy;
    pMod->pEpoTab = 0;
    pMod->nRefModule = 1;
  }
  pDel = (Module *)sqlite3HashInsert(&db->aModule, zCopy, (void *)pMod);
  if (pDel) {
    if (pDel == pMod) {
      sqlite3OomFault(db);
      sqlite3DbFree(db, pDel);
      pMod = 0;
    } else {
      sqlite3VtabEponymousTableClear(db, pDel);
      sqlite3VtabModuleUnref(db, pDel);
    }
  }
  return pMod;
}

int createModule(sqlite3 *db, const char *zName, const sqlite3_module *pModule, void *pAux, void (*xDestroy)(void *)) {
  int rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  (void)sqlite3VtabCreateModule(db, zName, pModule, pAux, xDestroy);
  rc = sqlite3ApiExit(db, rc);
  if (rc != SQLITE_OK && xDestroy)
    xDestroy(pAux);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_create_module(sqlite3 *db, const char *zName, const sqlite3_module *pModule, void *pAux) {
  return createModule(db, zName, pModule, pAux, 0);
}

int sqlite3_create_module_v2(sqlite3 *db, const char *zName, const sqlite3_module *pModule, void *pAux,
                             void (*xDestroy)(void *)) {
  return createModule(db, zName, pModule, pAux, xDestroy);
}

int sqlite3_drop_modules(sqlite3 *db, const char **azNames) {
  HashElem *pThis, *pNext;

  sqlite3_mutex_enter(db->mutex);
  for (pThis = ((&db->aModule)->first); pThis; pThis = pNext) {
    Module *pMod = (Module *)((pThis)->data);
    pNext = ((pThis)->next);
    if (azNames) {
      int ii;
      for (ii = 0; azNames[ii] != 0 && strcmp(azNames[ii], pMod->zName) != 0; ii++) {
      }
      if (azNames[ii] != 0)
        continue;
    }
    createModule(db, pMod->zName, 0, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

void sqlite3VtabModuleUnref(sqlite3 *db, Module *pMod) {
  pMod->nRefModule--;
  if (pMod->nRefModule == 0) {
    if (pMod->xDestroy) {
      pMod->xDestroy(pMod->pAux);
    }

    sqlite3DbFree(db, pMod);
  }
}

VTable *sqlite3GetVTable(sqlite3 *db, Table *pTab) {
  VTable *pVtab;

  for (pVtab = pTab->u.vtab.p; pVtab && pVtab->db != db; pVtab = pVtab->pNext)
    ;
  return pVtab;
}

VTable *vtabDisconnectAll(sqlite3 *db, Table *p) {
  VTable *pRet = 0;
  VTable *pVTable;

  pVTable = p->u.vtab.p;
  p->u.vtab.p = 0;

  while (pVTable) {
    sqlite3 *db2 = pVTable->db;
    VTable *pNext = pVTable->pNext;

    if (db2 == db) {
      pRet = pVTable;
      p->u.vtab.p = pRet;
      pRet->pNext = 0;
    } else {
      pVTable->pNext = db2->pDisconnect;
      db2->pDisconnect = pVTable;
    }
    pVTable = pNext;
  }

  return pRet;
}

void sqlite3VtabDisconnect(sqlite3 *db, Table *p) {
  VTable **ppVTab;

  for (ppVTab = &p->u.vtab.p; *ppVTab; ppVTab = &(*ppVTab)->pNext) {
    if ((*ppVTab)->db == db) {
      VTable *pVTab = *ppVTab;
      *ppVTab = pVTab->pNext;
      sqlite3VtabUnlock(pVTab);
      break;
    }
  }
}

void sqlite3VtabUnlockList(sqlite3 *db) {
  VTable *p = db->pDisconnect;

  if (p) {
    db->pDisconnect = 0;
    do {
      VTable *pNext = p->pNext;
      sqlite3VtabUnlock(p);
      p = pNext;
    } while (p);
  }
}

void sqlite3VtabClear(sqlite3 *db, Table *p) {
  if (db->pnBytesFreed == 0)
    vtabDisconnectAll(0, p);
  if (p->u.vtab.azArg) {
    int i;
    for (i = 0; i < p->u.vtab.nArg; i++) {
      if (i != 1)
        sqlite3DbFree(db, p->u.vtab.azArg[i]);
    }
    sqlite3DbFree(db, p->u.vtab.azArg);
  }
}

int vtabCallConstructor(sqlite3 *db, Table *pTab, Module *pMod,
                        int (*xConstruct)(sqlite3 *, void *, int, const char *const *, sqlite3_vtab **, char **),
                        char **pzErr) {
  VtabCtx sCtx;
  VTable *pVTable;
  int rc;
  const char *const *azArg;
  int nArg = pTab->u.vtab.nArg;
  char *zErr = 0;
  char *zModuleName;
  int iDb;
  VtabCtx *pCtx;

  azArg = (const char *const *)pTab->u.vtab.azArg;

  for (pCtx = db->pVtabCtx; pCtx; pCtx = pCtx->pPrior) {
    if (pCtx->pTab == pTab) {
      *pzErr = sqlite3MPrintf(db, "vtable constructor called recursively: %s", pTab->zName);
      return SQLITE_LOCKED;
    }
  }

  zModuleName = sqlite3DbStrDup(db, pTab->zName);
  if (!zModuleName) {
    return 7;
  }

  pVTable = sqlite3MallocZero(sizeof(VTable));
  if (!pVTable) {
    sqlite3OomFault(db);
    sqlite3DbFree(db, zModuleName);
    return 7;
  }
  pVTable->db = db;
  pVTable->pMod = pMod;
  pVTable->eVtabRisk = 1;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  pTab->u.vtab.azArg[1] = db->aDb[iDb].zDbSName;

  sCtx.pTab = pTab;
  sCtx.pVTable = pVTable;
  sCtx.pPrior = db->pVtabCtx;
  sCtx.bDeclared = 0;
  db->pVtabCtx = &sCtx;
  pTab->nTabRef++;
  rc = xConstruct(db, pMod->pAux, nArg, azArg, &pVTable->pVtab, &zErr);

  sqlite3DeleteTable(db, pTab);
  db->pVtabCtx = sCtx.pPrior;
  if (rc == SQLITE_NOMEM)
    sqlite3OomFault(db);

  if (SQLITE_OK != rc) {
    if (zErr == 0) {
      *pzErr = sqlite3MPrintf(db, "vtable constructor failed: %s", zModuleName);
    } else {
      *pzErr = sqlite3MPrintf(db, "%s", zErr);
      sqlite3_free(zErr);
    }
    sqlite3DbFree(db, pVTable);
  } else if ((pVTable->pVtab)) {
    memset(pVTable->pVtab, 0, sizeof(pVTable->pVtab[0]));
    pVTable->pVtab->pModule = pMod->pModule;
    pMod->nRefModule++;
    pVTable->nRef = 1;
    if (sCtx.bDeclared == 0) {
      const char *zFormat = "vtable constructor did not declare schema: %s";
      *pzErr = sqlite3MPrintf(db, zFormat, zModuleName);
      sqlite3VtabUnlock(pVTable);
      rc = SQLITE_ERROR;
    } else {
      int iCol;
      u16 oooHidden = 0;

      pVTable->pNext = pTab->u.vtab.p;
      pTab->u.vtab.p = pVTable;

      for (iCol = 0; iCol < pTab->nCol; iCol++) {
        char *zType = sqlite3ColumnType(&pTab->aCol[iCol], "");
        int nType;
        int i = 0;
        nType = sqlite3Strlen30(zType);
        for (i = 0; i < nType; i++) {
          if (0 == sqlite3_strnicmp("hidden", &zType[i], 6) && (i == 0 || zType[i - 1] == ' ') &&
              (zType[i + 6] == '\0' || zType[i + 6] == ' ')) {
            break;
          }
        }
        if (i < nType) {
          int j;
          int nDel = 6 + (zType[i + 6] ? 1 : 0);
          for (j = i; (j + nDel) <= nType; j++) {
            zType[j] = zType[j + nDel];
          }
          if (zType[i] == '\0' && i > 0) {
            zType[i - 1] = '\0';
          }
          pTab->aCol[iCol].colFlags |= 0x0002;
          pTab->tabFlags |= 0x00000002;
          oooHidden = 0x00000400;
        } else {
          pTab->tabFlags |= oooHidden;
        }
      }
    }
  }

  sqlite3DbFree(db, zModuleName);
  return rc;
}

int growVTrans(sqlite3 *db) {
  const int ARRAY_INCR = 5;

  if ((db->nVTrans % ARRAY_INCR) == 0) {
    VTable **aVTrans;
    sqlite3_int64 nBytes = sizeof(sqlite3_vtab *) * ((sqlite3_int64)db->nVTrans + ARRAY_INCR);
    aVTrans = sqlite3DbRealloc(db, (void *)db->aVTrans, nBytes);
    if (!aVTrans) {
      return 7;
    }
    memset(&aVTrans[db->nVTrans], 0, sizeof(sqlite3_vtab *) * ARRAY_INCR);
    db->aVTrans = aVTrans;
  }

  return SQLITE_OK;
}

void addToVTrans(sqlite3 *db, VTable *pVTab) {
  db->aVTrans[db->nVTrans++] = pVTab;
  sqlite3VtabLock(pVTab);
}

int sqlite3VtabCallCreate(sqlite3 *db, int iDb, const char *zTab, char **pzErr) {
  int rc = SQLITE_OK;
  Table *pTab;
  Module *pMod;
  const char *zMod;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);

  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module *)sqlite3HashFind(&db->aModule, zMod);

  if (pMod == 0 || pMod->pModule->xCreate == 0 || pMod->pModule->xDestroy == 0) {
    *pzErr = sqlite3MPrintf(db, "no such module: %s", zMod);
    rc = SQLITE_ERROR;
  } else {
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xCreate, pzErr);
  }

  if (rc == SQLITE_OK && (sqlite3GetVTable(db, pTab))) {
    rc = growVTrans(db);
    if (rc == SQLITE_OK) {
      addToVTrans(db, sqlite3GetVTable(db, pTab));
    }
  }

  return rc;
}

int sqlite3_declare_vtab(sqlite3 *db, const char *zCreateTable) {
  VtabCtx *pCtx;
  int rc = SQLITE_OK;
  Table *pTab;
  Parse sParse;
  int initBusy;
  int i;
  const unsigned char *z;
  static const u8 aKeyword[] = {17, 16, 0};

  z = (const unsigned char *)zCreateTable;
  for (i = 0; aKeyword[i]; i++) {
    int tokenType = 0;
    do {
      z += sqlite3GetToken(z, &tokenType);
    } while (tokenType == 184 || tokenType == 185);
    if (tokenType != aKeyword[i]) {
      sqlite3ErrorWithMsg(db, SQLITE_ERROR, "syntax error");
      return SQLITE_ERROR;
    }
  }

  sqlite3_mutex_enter(db->mutex);
  pCtx = db->pVtabCtx;
  if (!pCtx || pCtx->bDeclared) {
    sqlite3Error(db, sqlite3MisuseError(162738));
    sqlite3_mutex_leave(db->mutex);
    return sqlite3MisuseError(162740);
  }

  pTab = pCtx->pTab;

  sqlite3ParseObjectInit(&sParse, db);
  sParse.eParseMode = 1;
  sParse.disableTriggers = 1;

  initBusy = db->init.busy;
  db->init.busy = 0;
  sParse.nQueryLoop = 1;
  if (SQLITE_OK == sqlite3RunParser(&sParse, zCreateTable)) {
    if (!pTab->aCol) {
      Table *pNew = sParse.pNewTable;
      Index *pIdx;
      pTab->aCol = pNew->aCol;

      sqlite3ExprListDelete(db, pNew->u.tab.pDfltList);
      pTab->nNVCol = pTab->nCol = pNew->nCol;
      pTab->tabFlags |= pNew->tabFlags & (0x00000080 | 0x00000200);
      pNew->nCol = 0;
      pNew->aCol = 0;

      if (!(((pNew)->tabFlags & 0x00000080) == 0) && pCtx->pVTable->pMod->pModule->xUpdate != 0 &&
          sqlite3PrimaryKeyIndex(pNew)->nKeyCol != 1) {
        rc = SQLITE_ERROR;
      }
      pIdx = pNew->pIndex;
      if (pIdx) {
        pTab->pIndex = pIdx;
        pNew->pIndex = 0;
        pIdx->pTable = pTab;
      }
    }
    pCtx->bDeclared = 1;
  } else {
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, (sParse.zErrMsg ? "%s" : 0), sParse.zErrMsg);
    sqlite3DbFree(db, sParse.zErrMsg);
    rc = SQLITE_ERROR;
  }
  sParse.eParseMode = 0;

  if (sParse.pVdbe) {
    sqlite3VdbeFinalize(sParse.pVdbe);
  }
  sqlite3DeleteTable(db, sParse.pNewTable);
  sqlite3ParseObjectReset(&sParse);
  db->init.busy = initBusy;

  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3VtabCallDestroy(sqlite3 *db, int iDb, const char *zTab) {
  int rc = SQLITE_OK;
  Table *pTab;

  pTab = sqlite3FindTable(db, zTab, db->aDb[iDb].zDbSName);
  if ((pTab != 0) && (((pTab)->eTabType == 1)) && (pTab->u.vtab.p != 0)) {
    VTable *p;
    int (*xDestroy)(sqlite3_vtab *);
    for (p = pTab->u.vtab.p; p; p = p->pNext) {
      if (p->pVtab->nRef > 0) {
        return SQLITE_LOCKED;
      }
    }
    p = vtabDisconnectAll(db, pTab);
    xDestroy = p->pMod->pModule->xDestroy;
    if (xDestroy == 0)
      xDestroy = p->pMod->pModule->xDisconnect;

    pTab->nTabRef++;
    rc = xDestroy(p->pVtab);

    if (rc == SQLITE_OK) {
      p->pVtab = 0;
      pTab->u.vtab.p = 0;
      sqlite3VtabUnlock(p);
    }
    sqlite3DeleteTable(db, pTab);
  }

  return rc;
}

void callFinaliser(sqlite3 *db, int offset) {
  int i;
  if (db->aVTrans) {
    VTable **aVTrans = db->aVTrans;
    db->aVTrans = 0;
    for (i = 0; i < db->nVTrans; i++) {
      VTable *pVTab = aVTrans[i];
      sqlite3_vtab *p = pVTab->pVtab;
      if (p) {
        int (*x)(sqlite3_vtab *);
        x = *(int (**)(sqlite3_vtab *))((char *)p->pModule + offset);
        if (x)
          x(p);
      }
      pVTab->iSavepoint = 0;
      sqlite3VtabUnlock(pVTab);
    }
    sqlite3DbFree(db, aVTrans);
    db->nVTrans = 0;
  }
}

int sqlite3VtabSync(sqlite3 *db, Vdbe *p) {
  int i;
  int rc = SQLITE_OK;
  VTable **aVTrans = db->aVTrans;

  db->aVTrans = 0;
  for (i = 0; rc == SQLITE_OK && i < db->nVTrans; i++) {
    int (*x)(sqlite3_vtab *);
    sqlite3_vtab *pVtab = aVTrans[i]->pVtab;
    if (pVtab && (x = pVtab->pModule->xSync) != 0) {
      rc = x(pVtab);
      sqlite3VtabImportErrmsg(p, pVtab);
    }
  }
  db->aVTrans = aVTrans;
  return rc;
}

int sqlite3VtabRollback(sqlite3 *db) {
  callFinaliser(db, offsetof(sqlite3_module, xRollback));
  return SQLITE_OK;
}

int sqlite3VtabCommit(sqlite3 *db) {
  callFinaliser(db, offsetof(sqlite3_module, xCommit));
  return SQLITE_OK;
}

int sqlite3VtabBegin(sqlite3 *db, VTable *pVTab) {
  int rc = SQLITE_OK;
  const sqlite3_module *pModule;

  if (((db)->nVTrans > 0 && (db)->aVTrans == 0)) {
    return SQLITE_LOCKED;
  }
  if (!pVTab) {
    return SQLITE_OK;
  }
  pModule = pVTab->pVtab->pModule;

  if (pModule->xBegin) {
    int i;

    for (i = 0; i < db->nVTrans; i++) {
      if (db->aVTrans[i] == pVTab) {
        return SQLITE_OK;
      }
    }

    rc = growVTrans(db);
    if (rc == SQLITE_OK) {
      rc = pModule->xBegin(pVTab->pVtab);
      if (rc == SQLITE_OK) {
        int iSvpt = db->nStatement + db->nSavepoint;
        addToVTrans(db, pVTab);
        if (iSvpt && pModule->xSavepoint) {
          pVTab->iSavepoint = iSvpt;
          rc = pModule->xSavepoint(pVTab->pVtab, iSvpt - 1);
        }
      }
    }
  }
  return rc;
}

int sqlite3VtabSavepoint(sqlite3 *db, int op, int iSavepoint) {
  int rc = 0;

  if (db->aVTrans) {
    int i;
    for (i = 0; rc == SQLITE_OK && i < db->nVTrans; i++) {
      VTable *pVTab = db->aVTrans[i];
      const sqlite3_module *pMod = pVTab->pMod->pModule;
      if (pVTab->pVtab && pMod->iVersion >= 2) {
        int (*xMethod)(sqlite3_vtab *, int);
        sqlite3VtabLock(pVTab);
        switch (op) {
          case 0:
            xMethod = pMod->xSavepoint;
            pVTab->iSavepoint = iSavepoint + 1;
            break;
          case 2:
            xMethod = pMod->xRollbackTo;
            break;
          default:
            xMethod = pMod->xRelease;
            break;
        }
        if (xMethod && pVTab->iSavepoint > iSavepoint) {
          u64 savedFlags = (db->flags & 0x10000000);
          db->flags &= ~(u64)0x10000000;
          rc = xMethod(pVTab->pVtab, iSavepoint);
          db->flags |= savedFlags;
        }
        sqlite3VtabUnlock(pVTab);
      }
    }
  }
  return rc;
}

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *db, FuncDef *pDef, int nArg, Expr *pExpr) {
  Table *pTab;
  sqlite3_vtab *pVtab;
  sqlite3_module *pMod;
  void (*xSFunc)(sqlite3_context *, int, sqlite3_value **) = 0;
  void *pArg = 0;
  FuncDef *pNew;
  int rc = 0;

  if (pExpr == 0)
    return pDef;
  if (pExpr->op != 168)
    return pDef;

  pTab = pExpr->y.pTab;
  if (pTab == 0)
    return pDef;
  if (!((pTab)->eTabType == 1))
    return pDef;
  pVtab = sqlite3GetVTable(db, pTab)->pVtab;

  pMod = (sqlite3_module *)pVtab->pModule;
  if (pMod->xFindFunction == 0)
    return pDef;

  rc = pMod->xFindFunction(pVtab, nArg, pDef->zName, &xSFunc, &pArg);
  if (rc == 0) {
    return pDef;
  }

  pNew = sqlite3DbMallocZero(db, sizeof(*pNew) + sqlite3Strlen30(pDef->zName) + 1);
  if (pNew == 0) {
    return pDef;
  }
  *pNew = *pDef;
  pNew->zName = (const char *)&pNew[1];
  memcpy((char *)&pNew[1], pDef->zName, sqlite3Strlen30(pDef->zName) + 1);
  pNew->xSFunc = xSFunc;
  pNew->pUserData = pArg;
  pNew->funcFlags |= 0x0010;
  return pNew;
}

void sqlite3VtabEponymousTableClear(sqlite3 *db, Module *pMod) {
  Table *pTab = pMod->pEpoTab;
  if (pTab != 0) {
    pTab->tabFlags |= 0x00004000;
    sqlite3DeleteTable(db, pTab);
    pMod->pEpoTab = 0;
  }
}

int sqlite3_vtab_on_conflict(sqlite3 *db) {
  static const unsigned char aMap[] = {SQLITE_ROLLBACK, SQLITE_ABORT, SQLITE_FAIL, SQLITE_IGNORE, SQLITE_REPLACE};

  return (int)aMap[db->vtabOnConflict - 1];
}

int sqlite3_vtab_config(sqlite3 *db, int op, ...) {
  va_list ap;
  int rc = SQLITE_OK;
  VtabCtx *p;

  sqlite3_mutex_enter(db->mutex);
  p = db->pVtabCtx;
  if (!p) {
    rc = sqlite3MisuseError(163238);
  } else {
    va_start(ap, op);
    switch (op) {
      case SQLITE_VTAB_CONSTRAINT_SUPPORT: {
        p->pVTable->bConstraint = (u8)va_arg(ap, int);
        break;
      }
      case SQLITE_VTAB_INNOCUOUS: {
        p->pVTable->eVtabRisk = 0;
        break;
      }
      case SQLITE_VTAB_DIRECTONLY: {
        p->pVTable->eVtabRisk = 2;
        break;
      }
      case SQLITE_VTAB_USES_ALL_SCHEMAS: {
        p->pVTable->bAllSchemas = 1;
        break;
      }
      default: {
        rc = sqlite3MisuseError(163260);
        break;
      }
    }

    va_end(ap);
  }

  if (rc != SQLITE_OK)
    sqlite3Error(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

void whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p) {
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

void whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p) {
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

int isAuxiliaryVtabOperator(sqlite3 *db, Expr *pExpr, unsigned char *peOp2, Expr **ppLeft, Expr **ppRight) {
  if (pExpr->op == 172) {
    ExprList *pList;
    Expr *pCol;
    int i;

    pList = pExpr->x.pList;
    if (pList == 0 || pList->nExpr != 2) {
      return 0;
    }

    pCol = pList->a[1].pExpr;

    if (((pCol)->op == 168 && (pCol)->y.pTab->eTabType == 1) && (i = sqlite3ExprIsLikeOperator(pExpr)) != 0) {
      *peOp2 = i;
      *ppRight = pList->a[0].pExpr;
      *ppLeft = pCol;
      return 1;
    }

    pCol = pList->a[0].pExpr;

    if (((pCol)->op == 168 && (pCol)->y.pTab->eTabType == 1)) {
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      void (*xNotUsed)(sqlite3_context *, int, sqlite3_value **);
      void *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;

      pMod = (sqlite3_module *)pVtab->pModule;
      if (pMod->xFindFunction != 0) {
        i = pMod->xFindFunction(pVtab, 2, pExpr->u.zToken, &xNotUsed, &pNotUsed);
        if (i >= SQLITE_INDEX_CONSTRAINT_FUNCTION) {
          *peOp2 = i;
          *ppRight = pList->a[1].pExpr;
          *ppLeft = pCol;
          return 1;
        }
      }
    }
  } else if (pExpr->op >= 54) {
    return 0;
  } else if (pExpr->op == 53 || pExpr->op == 46 || pExpr->op == 52) {
    int res = 0;
    Expr *pLeft = pExpr->pLeft;
    Expr *pRight = pExpr->pRight;

    if (((pLeft)->op == 168 && (pLeft)->y.pTab->eTabType == 1)) {
      res++;
    }

    if (pRight && ((pRight)->op == 168 && (pRight)->y.pTab->eTabType == 1)) {
      res++;
      {
        Expr *t = pLeft;
        pLeft = pRight;
        pRight = t;
      };
    }
    *ppLeft = pLeft;
    *ppRight = pRight;
    if (pExpr->op == 53)
      *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    if (pExpr->op == 46)
      *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    if (pExpr->op == 52)
      *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    return res;
  }
  return 0;
}

void freeIndexInfo(sqlite3 *db, sqlite3_index_info *pIdxInfo) {
  HiddenIndexInfo *pHidden;
  int i;

  pHidden = (HiddenIndexInfo *)&pIdxInfo[1];

  for (i = 0; i < pIdxInfo->nConstraint; i++) {
    sqlite3ValueFree(pHidden->aRhs[i]);
    pHidden->aRhs[i] = 0;
  }
  freeIdxStr(pIdxInfo);
  sqlite3DbFree(db, pIdxInfo);
}

void whereLoopClearUnion(sqlite3 *db, WhereLoop *p) {
  if (p->wsFlags & (0x00000400 | 0x00004000)) {
    if ((p->wsFlags & 0x00000400) != 0 && p->u.vtab.needFree) {
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    } else if ((p->wsFlags & 0x00004000) != 0 && p->u.btree.pIndex != 0) {
      sqlite3DbFree(db, p->u.btree.pIndex->zColAff);
      sqlite3DbFreeNN(db, p->u.btree.pIndex);
      p->u.btree.pIndex = 0;
    }
  }
}

void whereLoopClear(sqlite3 *db, WhereLoop *p) {
  if (p->aLTerm != p->aLTermSpace) {
    sqlite3DbFreeNN(db, p->aLTerm);
    p->aLTerm = p->aLTermSpace;
    p->nLSlot = ((int)(sizeof(p->aLTermSpace) / sizeof(p->aLTermSpace[0])));
  }
  whereLoopClearUnion(db, p);
  p->nLTerm = 0;
  p->wsFlags = 0;
}

int whereLoopResize(sqlite3 *db, WhereLoop *p, int n) {
  WhereTerm **paNew;
  if (p->nLSlot >= n)
    return SQLITE_OK;
  n = (n + 7) & ~7;
  paNew = sqlite3DbMallocRawNN(db, sizeof(p->aLTerm[0]) * n);
  if (paNew == 0)
    return 7;
  memcpy(paNew, p->aLTerm, sizeof(p->aLTerm[0]) * p->nLSlot);
  if (p->aLTerm != p->aLTermSpace)
    sqlite3DbFreeNN(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  return SQLITE_OK;
}

int whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom) {
  whereLoopClearUnion(db, pTo);
  if (pFrom->nLTerm > pTo->nLSlot && whereLoopResize(db, pTo, pFrom->nLTerm)) {
    memset(pTo, 0, offsetof(WhereLoop, nLSlot));
    return 7;
  }
  memcpy(pTo, pFrom, offsetof(WhereLoop, nLSlot));
  memcpy(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm * sizeof(pTo->aLTerm[0]));
  if (pFrom->wsFlags & 0x00000400) {
    pFrom->u.vtab.needFree = 0;
  } else if ((pFrom->wsFlags & 0x00004000) != 0) {
    pFrom->u.btree.pIndex = 0;
  }
  return SQLITE_OK;
}

void whereLoopDelete(sqlite3 *db, WhereLoop *p) {
  whereLoopClear(db, p);
  sqlite3DbNNFreeNN(db, p);
}

void whereInfoFree(sqlite3 *db, WhereInfo *pWInfo) {
  sqlite3WhereClauseClear(&pWInfo->sWC);
  while (pWInfo->pLoops) {
    WhereLoop *p = pWInfo->pLoops;
    pWInfo->pLoops = p->pNextLoop;
    whereLoopDelete(db, p);
  }
  while (pWInfo->pMemToFree) {
    WhereMemBlock *pNext = pWInfo->pMemToFree->pNext;
    sqlite3DbNNFreeNN(db, pWInfo->pMemToFree);
    pWInfo->pMemToFree = pNext;
  }
  sqlite3DbNNFreeNN(db, pWInfo);
}

void whereIndexedExprCleanup(sqlite3 *db, void *pObject) {
  IndexedExpr **pp = (IndexedExpr **)pObject;
  while (*pp != 0) {
    IndexedExpr *p = *pp;
    *pp = p->pIENext;
    sqlite3ExprDelete(db, p->pExpr);
    sqlite3DbFreeNN(db, p);
  }
}

void sqlite3WindowDelete(sqlite3 *db, Window *p) {
  if (p) {
    sqlite3WindowUnlinkFromSelect(p);
    sqlite3ExprDelete(db, p->pFilter);
    sqlite3ExprListDelete(db, p->pPartition);
    sqlite3ExprListDelete(db, p->pOrderBy);
    sqlite3ExprDelete(db, p->pEnd);
    sqlite3ExprDelete(db, p->pStart);
    sqlite3DbFree(db, p->zName);
    sqlite3DbFree(db, p->zBase);
    sqlite3DbFree(db, p);
  }
}

void sqlite3WindowListDelete(sqlite3 *db, Window *p) {
  while (p) {
    Window *pNext = p->pNextWin;
    sqlite3WindowDelete(db, p);
    p = pNext;
  }
}

Window *sqlite3WindowDup(sqlite3 *db, Expr *pOwner, Window *p) {
  Window *pNew = 0;
  if ((p)) {
    pNew = sqlite3DbMallocZero(db, sizeof(Window));
    if (pNew) {
      pNew->zName = sqlite3DbStrDup(db, p->zName);
      pNew->zBase = sqlite3DbStrDup(db, p->zBase);
      pNew->pFilter = sqlite3ExprDup(db, p->pFilter, 0);
      pNew->pWFunc = p->pWFunc;
      pNew->pPartition = sqlite3ExprListDup(db, p->pPartition, 0);
      pNew->pOrderBy = sqlite3ExprListDup(db, p->pOrderBy, 0);
      pNew->eFrmType = p->eFrmType;
      pNew->eEnd = p->eEnd;
      pNew->eStart = p->eStart;
      pNew->eExclude = p->eExclude;
      pNew->regResult = p->regResult;
      pNew->regAccum = p->regAccum;
      pNew->iArgCol = p->iArgCol;
      pNew->iEphCsr = p->iEphCsr;
      pNew->bExprArgs = p->bExprArgs;
      pNew->pStart = sqlite3ExprDup(db, p->pStart, 0);
      pNew->pEnd = sqlite3ExprDup(db, p->pEnd, 0);
      pNew->pOwner = pOwner;
      pNew->bImplicitFrame = p->bImplicitFrame;
    }
  }
  return pNew;
}

Window *sqlite3WindowListDup(sqlite3 *db, Window *p) {
  Window *pWin;
  Window *pRet = 0;
  Window **pp = &pRet;

  for (pWin = p; pWin; pWin = pWin->pNextWin) {
    *pp = sqlite3WindowDup(db, 0, pWin);
    if (*pp == 0)
      break;
    pp = &((*pp)->pNextWin);
  }

  return pRet;
}

void sqlite3ParserInit(void *yypRawParser, Parse *pParse) {
  yyParser *yypParser = (yyParser *)yypRawParser;
  yypParser->pParse = pParse;

  yypParser->yystack = yypParser->yystk0;
  yypParser->yystackEnd = &yypParser->yystack[50 - 1];

  yypParser->yytos = yypParser->yystack;
  yypParser->yystack[0].stateno = 0;
  yypParser->yystack[0].major = 0;
}

void sqlite3ParserFinalize(void *p) {
  yyParser *pParser = (yyParser *)p;

  yyStackEntry *yytos = pParser->yytos;
  while (yytos > pParser->yystack) {
    if (yytos->major >= 206) {
      yy_destructor(pParser, yytos->major, &yytos->minor);
    }
    yytos--;
  }

  if (pParser->yystack != pParser->yystk0) {
    parserStackFree(pParser->yystack, ((pParser)->pParse));
  }
}

void sqlite3Parser(void *yyp, int yymajor, Token yyminor) {
  YYMINORTYPE yyminorunion;
  unsigned short int yyact;

  yyParser *yypParser = (yyParser *)yyp;
  Parse *pParse = yypParser->pParse;

  yyact = yypParser->yytos->stateno;

  while (1) {
    yyact = yy_find_shift_action((unsigned short int)yymajor, yyact);
    if (yyact >= 1282) {
      unsigned int yyruleno = yyact - 1282;

      if (yyRuleInfoNRhs[yyruleno] == 0) {
        if (yypParser->yytos >= yypParser->yystackEnd) {
          if (yyGrowStack(yypParser)) {
            yyStackOverflow(yypParser);
            break;
          }
        }
      }
      yyact = yy_reduce(yypParser, yyruleno, yymajor, yyminor, pParse);
    } else if (yyact <= 1278) {
      yy_shift(yypParser, yyact, (unsigned short int)yymajor, yyminor);

      break;
    } else if (yyact == 1280) {
      yypParser->yytos--;
      yy_accept(yypParser);
      return;
    } else {
      yyminorunion.yy0 = yyminor;

      yy_syntax_error(yypParser, yymajor, yyminor);
      yy_destructor(yypParser, (unsigned short int)yymajor, &yyminorunion);
      break;
    }
  }

  return;
}

int sqlite3ParserFallback(int iToken) {
  return yyFallback[iToken];
}

static int sqlite3KeywordCode(const unsigned char *z, int n) {
  int id = 60;
  if (n >= 2)
    keywordCode((char *)z, n, &id);
  return id;
}

int sqlite3_keyword_name(int i, const char **pzName, int *pnName) {
  if (i < 0 || i >= 147)
    return SQLITE_ERROR;
  i++;
  *pzName = zKWText + aKWOffset[i];
  *pnName = aKWLen[i];
  return SQLITE_OK;
}

int sqlite3_keyword_count(void) {
  return 147;
}

int sqlite3_keyword_check(const char *zName, int nName) {
  return 60 != sqlite3KeywordCode((const u8 *)zName, nName);
}

int sqlite3IsIdChar(u8 c) {
  return ((sqlite3CtypeMap[(unsigned char)c] & 0x46) != 0);
}

i64 sqlite3GetToken(const unsigned char *z, int *tokenType) {
  i64 i;
  int c;
  switch (aiClass[*z]) {
    case 7: {
      for (i = 1; (sqlite3CtypeMap[(unsigned char)(z[i])] & 0x01); i++) {
      }
      *tokenType = 184;
      return i;
    }
    case 11: {
      if (z[1] == '-') {
        for (i = 2; (c = z[i]) != 0 && c != '\n'; i++) {
        }
        *tokenType = 185;
        return i;
      } else if (z[1] == '>') {
        *tokenType = 113;
        return 2 + (z[2] == '>');
      }
      *tokenType = 108;
      return 1;
    }
    case 17: {
      *tokenType = 22;
      return 1;
    }
    case 18: {
      *tokenType = 23;
      return 1;
    }
    case 19: {
      *tokenType = 1;
      return 1;
    }
    case 20: {
      *tokenType = 107;
      return 1;
    }
    case 21: {
      *tokenType = 109;
      return 1;
    }
    case 16: {
      if (z[1] != '*' || z[2] == 0) {
        *tokenType = 110;
        return 1;
      }
      for (i = 3, c = z[2]; (c != '*' || z[i] != '/') && (c = z[i]) != 0; i++) {
      }
      if (c)
        i++;
      *tokenType = 185;
      return i;
    }
    case 22: {
      *tokenType = 111;
      return 1;
    }
    case 14: {
      *tokenType = 54;
      return 1 + (z[1] == '=');
    }
    case 12: {
      if ((c = z[1]) == '=') {
        *tokenType = 56;
        return 2;
      } else if (c == '>') {
        *tokenType = 53;
        return 2;
      } else if (c == '<') {
        *tokenType = 105;
        return 2;
      } else {
        *tokenType = 57;
        return 1;
      }
    }
    case 13: {
      if ((c = z[1]) == '=') {
        *tokenType = 58;
        return 2;
      } else if (c == '>') {
        *tokenType = 106;
        return 2;
      } else {
        *tokenType = 55;
        return 1;
      }
    }
    case 15: {
      if (z[1] != '=') {
        *tokenType = 186;
        return 1;
      } else {
        *tokenType = 53;
        return 2;
      }
    }
    case 10: {
      if (z[1] != '|') {
        *tokenType = 104;
        return 1;
      } else {
        *tokenType = 112;
        return 2;
      }
    }
    case 23: {
      *tokenType = 25;
      return 1;
    }
    case 24: {
      *tokenType = 103;
      return 1;
    }
    case 25: {
      *tokenType = 115;
      return 1;
    }
    case 8: {
      int delim = z[0];
      for (i = 1; (c = z[i]) != 0; i++) {
        if (c == delim) {
          if (z[i + 1] == delim) {
            i++;
          } else {
            break;
          }
        }
      }
      if (c == '\'') {
        *tokenType = 118;
        return i + 1;
      } else if (c != 0) {
        *tokenType = 60;
        return i + 1;
      } else {
        *tokenType = 186;
        return i;
      }
    }
    case 26: {
      if (!(sqlite3CtypeMap[(unsigned char)(z[1])] & 0x04)) {
        *tokenType = 142;
        return 1;
      }

      __attribute__((fallthrough));
    }
    case 3: {
      *tokenType = 156;

      if (z[0] == '0' && (z[1] == 'x' || z[1] == 'X') && (sqlite3CtypeMap[(unsigned char)(z[2])] & 0x08)) {
        for (i = 3; 1; i++) {
          if ((sqlite3CtypeMap[(unsigned char)(z[i])] & 0x08) == 0) {
            if (z[i] == '_') {
              *tokenType = 183;
            } else {
              break;
            }
          }
        }
      } else {
        for (i = 0; 1; i++) {
          if ((sqlite3CtypeMap[(unsigned char)(z[i])] & 0x04) == 0) {
            if (z[i] == '_') {
              *tokenType = 183;
            } else {
              break;
            }
          }
        }

        if (z[i] == '.') {
          if (*tokenType == 156)
            *tokenType = 154;
          for (i++; 1; i++) {
            if ((sqlite3CtypeMap[(unsigned char)(z[i])] & 0x04) == 0) {
              if (z[i] == '_') {
                *tokenType = 183;
              } else {
                break;
              }
            }
          }
        }
        if ((z[i] == 'e' || z[i] == 'E') &&
            ((sqlite3CtypeMap[(unsigned char)(z[i + 1])] & 0x04) ||
             ((z[i + 1] == '+' || z[i + 1] == '-') && (sqlite3CtypeMap[(unsigned char)(z[i + 2])] & 0x04)))) {
          if (*tokenType == 156)
            *tokenType = 154;
          for (i += 2; 1; i++) {
            if ((sqlite3CtypeMap[(unsigned char)(z[i])] & 0x04) == 0) {
              if (z[i] == '_') {
                *tokenType = 183;
              } else {
                break;
              }
            }
          }
        }
      }
      while (((sqlite3CtypeMap[(unsigned char)z[i]] & 0x46) != 0)) {
        *tokenType = 186;
        i++;
      }
      return i;
    }
    case 9: {
      for (i = 1, c = z[0]; c != ']' && (c = z[i]) != 0; i++) {
      }
      *tokenType = c == ']' ? 60 : 186;
      return i;
    }
    case 6: {
      *tokenType = 157;
      for (i = 1; (sqlite3CtypeMap[(unsigned char)(z[i])] & 0x04); i++) {
      }
      return i;
    }
    case 4:
    case 5: {
      i64 n = 0;
      *tokenType = 157;
      for (i = 1; (c = z[i]) != 0; i++) {
        if (((sqlite3CtypeMap[(unsigned char)c] & 0x46) != 0)) {
          n++;

        } else if (c == '(' && n > 0) {
          do {
            i++;
          } while ((c = z[i]) != 0 && !(sqlite3CtypeMap[(unsigned char)(c)] & 0x01) && c != ')');
          if (c == ')') {
            i++;
          } else {
            *tokenType = 186;
          }
          break;
        } else if (c == ':' && z[i + 1] == ':') {
          i++;

        } else {
          break;
        }
      }
      if (n == 0)
        *tokenType = 186;
      return i;
    }
    case 1: {
      if (aiClass[z[1]] > 2) {
        i = 1;
        break;
      }
      for (i = 2; aiClass[z[i]] <= 2; i++) {
      }
      if (((sqlite3CtypeMap[(unsigned char)z[i]] & 0x46) != 0)) {
        i++;
        break;
      }
      *tokenType = 60;
      return keywordCode((char *)z, i, tokenType);
    }
    case 0: {
      if (z[1] == '\'') {
        *tokenType = 155;
        for (i = 2; (sqlite3CtypeMap[(unsigned char)(z[i])] & 0x08); i++) {
        }
        if (z[i] != '\'' || i % 2) {
          *tokenType = 186;
          while (z[i] && z[i] != '\'') {
            i++;
          }
        }
        if (z[i])
          i++;
        return i;
      }

      __attribute__((fallthrough));
    }
    case 2:
    case 27: {
      i = 1;
      break;
    }
    case 30: {
      if (z[1] == 0xbb && z[2] == 0xbf) {
        *tokenType = 184;
        return 3;
      }
      i = 1;
      break;
    }
    case 29: {
      *tokenType = 186;
      return 0;
    }
    default: {
      *tokenType = 186;
      return 1;
    }
  }
  while (((sqlite3CtypeMap[(unsigned char)z[i]] & 0x46) != 0)) {
    i++;
  }
  *tokenType = 60;
  return i;
}

int sqlite3TestExtInit(sqlite3 *db) {
  (void)db;
  return sqlite3FaultSim(500);
}

int (*const sqlite3BuiltinExtensions[])(sqlite3 *) = {

    sqlite3TestExtInit,
};

const char *sqlite3_libversion(void) {
  return sqlite3_version;
}

int sqlite3_libversion_number(void) {
  return 3053004;
}

char *sqlite3_temp_directory = 0;

char *sqlite3_data_directory = 0;

int sqlite3_initialize(void) {
  sqlite3_mutex *pMainMtx;
  int rc;

  if (sqlite3Config.isInit) {
    sqlite3MemoryBarrier();
    return SQLITE_OK;
  }

  rc = sqlite3MutexInit();
  if (rc)
    return rc;

  pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
  sqlite3_mutex_enter(pMainMtx);
  sqlite3Config.isMutexInit = 1;
  if (!sqlite3Config.isMallocInit) {
    rc = sqlite3MallocInit();
  }
  if (rc == SQLITE_OK) {
    sqlite3Config.isMallocInit = 1;
    if (!sqlite3Config.pInitMutex) {
      sqlite3Config.pInitMutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      if (sqlite3Config.bCoreMutex && !sqlite3Config.pInitMutex) {
        rc = 7;
      }
    }
  }
  if (rc == SQLITE_OK) {
    sqlite3Config.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMainMtx);

  if (rc != SQLITE_OK) {
    return rc;
  }

  sqlite3_mutex_enter(sqlite3Config.pInitMutex);
  if (sqlite3Config.isInit == 0 && sqlite3Config.inProgress == 0) {
    sqlite3Config.inProgress = 1;

    memset(&sqlite3BuiltinFunctions, 0, sizeof(sqlite3BuiltinFunctions));
    sqlite3RegisterBuiltinFunctions();
    if (sqlite3Config.isPCacheInit == 0) {
      rc = sqlite3PcacheInitialize();
    }
    if (rc == SQLITE_OK) {
      sqlite3Config.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }

    if (rc == SQLITE_OK) {
      rc = sqlite3MemdbInit();
    }

    if (rc == SQLITE_OK) {
      sqlite3PCacheBufferSetup(sqlite3Config.pPage, sqlite3Config.szPage, sqlite3Config.nPage);
    }
    if (rc == SQLITE_OK) {
      sqlite3MemoryBarrier();
      sqlite3Config.isInit = 1;
    }
    sqlite3Config.inProgress = 0;
  }
  sqlite3_mutex_leave(sqlite3Config.pInitMutex);

  sqlite3_mutex_enter(pMainMtx);
  sqlite3Config.nRefInitMutex--;
  if (sqlite3Config.nRefInitMutex <= 0) {
    sqlite3_mutex_free(sqlite3Config.pInitMutex);
    sqlite3Config.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMainMtx);

  return rc;
}

int sqlite3_shutdown(void) {
  if (sqlite3Config.isInit) {
    sqlite3_os_end();
    sqlite3_reset_auto_extension();
    sqlite3Config.isInit = 0;
  }
  if (sqlite3Config.isPCacheInit) {
    sqlite3PcacheShutdown();
    sqlite3Config.isPCacheInit = 0;
  }
  if (sqlite3Config.isMallocInit) {
    sqlite3MallocEnd();
    sqlite3Config.isMallocInit = 0;

    sqlite3_data_directory = 0;
    sqlite3_temp_directory = 0;
  }
  if (sqlite3Config.isMutexInit) {
    sqlite3MutexEnd();
    sqlite3Config.isMutexInit = 0;
  }

  return SQLITE_OK;
}

int sqlite3_config(int op, ...) {
  va_list ap;
  int rc = SQLITE_OK;

  if (sqlite3Config.isInit) {
    static const u64 mAnytimeConfigOption = 0 | (((u64)1) << (16)) | (((u64)1) << (24));
    if (op < 0 || op > 63 || ((((u64)1) << (op)) & mAnytimeConfigOption) == 0) {
      return sqlite3MisuseError(187811);
    };
  }

  va_start(ap, op);
  switch (op) {
    case SQLITE_CONFIG_SINGLETHREAD: {
      sqlite3Config.bCoreMutex = 0;
      sqlite3Config.bFullMutex = 0;
      break;
    }

    case SQLITE_CONFIG_MULTITHREAD: {
      sqlite3Config.bCoreMutex = 1;
      sqlite3Config.bFullMutex = 0;
      break;
    }

    case SQLITE_CONFIG_SERIALIZED: {
      sqlite3Config.bCoreMutex = 1;
      sqlite3Config.bFullMutex = 1;
      break;
    }

    case SQLITE_CONFIG_MUTEX: {
      sqlite3Config.mutex = *va_arg(ap, sqlite3_mutex_methods *);
      break;
    }

    case SQLITE_CONFIG_GETMUTEX: {
      *va_arg(ap, sqlite3_mutex_methods *) = sqlite3Config.mutex;
      break;
    }

    case SQLITE_CONFIG_MALLOC: {
      sqlite3Config.m = *va_arg(ap, sqlite3_mem_methods *);
      break;
    }
    case SQLITE_CONFIG_GETMALLOC: {
      if (sqlite3Config.m.xMalloc == 0)
        sqlite3MemSetDefault();
      *va_arg(ap, sqlite3_mem_methods *) = sqlite3Config.m;
      break;
    }
    case SQLITE_CONFIG_MEMSTATUS: {
      sqlite3Config.bMemstat = va_arg(ap, int);
      break;
    }
    case SQLITE_CONFIG_SMALL_MALLOC: {
      sqlite3Config.bSmallMalloc = va_arg(ap, int);
      break;
    }
    case SQLITE_CONFIG_PAGECACHE: {
      sqlite3Config.pPage = va_arg(ap, void *);
      sqlite3Config.szPage = va_arg(ap, int);
      sqlite3Config.nPage = va_arg(ap, int);
      break;
    }
    case SQLITE_CONFIG_PCACHE_HDRSZ: {
      *va_arg(ap, int *) = sqlite3HeaderSizeBtree() + sqlite3HeaderSizePcache() + sqlite3HeaderSizePcache1();
      break;
    }

    case SQLITE_CONFIG_PCACHE: {
      break;
    }
    case SQLITE_CONFIG_GETPCACHE: {
      rc = SQLITE_ERROR;
      break;
    }

    case SQLITE_CONFIG_PCACHE2: {
      sqlite3Config.pcache2 = *va_arg(ap, sqlite3_pcache_methods2 *);
      break;
    }
    case SQLITE_CONFIG_GETPCACHE2: {
      if (sqlite3Config.pcache2.xInit == 0) {
        sqlite3PCacheSetDefault();
      }
      *va_arg(ap, sqlite3_pcache_methods2 *) = sqlite3Config.pcache2;
      break;
    }

    case 13: {
      sqlite3Config.szLookaside = va_arg(ap, int);
      sqlite3Config.nLookaside = va_arg(ap, int);
      break;
    }

    case SQLITE_CONFIG_LOG: {
      typedef void (*LOGFUNC_t)(void *, int, const char *);
      LOGFUNC_t xLog = va_arg(ap, LOGFUNC_t);
      void *pLogArg = va_arg(ap, void *);
      __atomic_store_n((&sqlite3Config.xLog), (xLog), 0);
      __atomic_store_n((&sqlite3Config.pLogArg), (pLogArg), 0);
      break;
    }

    case SQLITE_CONFIG_URI: {
      int bOpenUri = va_arg(ap, int);
      __atomic_store_n((&sqlite3Config.bOpenUri), (bOpenUri), 0);
      break;
    }

    case SQLITE_CONFIG_COVERING_INDEX_SCAN: {
      sqlite3Config.bUseCis = va_arg(ap, int);
      break;
    }

    case 22: {
      sqlite3_int64 szMmap = va_arg(ap, sqlite3_int64);
      sqlite3_int64 mxMmap = va_arg(ap, sqlite3_int64);

      if (mxMmap < 0 || mxMmap > 0x7fff0000) {
        mxMmap = 0x7fff0000;
      }
      if (szMmap < 0)
        szMmap = 0;
      if (szMmap > mxMmap)
        szMmap = mxMmap;
      sqlite3Config.mxMmap = mxMmap;
      sqlite3Config.szMmap = szMmap;
      break;
    }

    case 25: {
      sqlite3Config.szPma = va_arg(ap, unsigned int);
      break;
    }

    case SQLITE_CONFIG_STMTJRNL_SPILL: {
      sqlite3Config.nStmtSpill = va_arg(ap, int);
      break;
    }

    case 29: {
      sqlite3Config.mxMemdbSize = va_arg(ap, sqlite3_int64);
      break;
    }

    case SQLITE_CONFIG_ROWID_IN_VIEW: {
      int *pVal = va_arg(ap, int *);

      *pVal = 0;

      break;
    }

    default: {
      rc = SQLITE_ERROR;
      break;
    }
  }

  va_end(ap);
  return rc;
}

int setupLookaside(sqlite3 *db, void *pBuf, int sz, int cnt) {
  void *pStart;
  sqlite3_int64 szAlloc;
  int nBig;
  int nSm;

  if (sqlite3LookasideUsed(db, 0) > 0) {
    return SQLITE_BUSY;
  }

  if (db->lookaside.bMalloced) {
    sqlite3_free(db->lookaside.pStart);
  }

  sz = ((sz) & ~7);
  if (sz <= (int)sizeof(LookasideSlot *))
    sz = 0;
  if (sz > 65528)
    sz = 65528;

  if (cnt < 1)
    cnt = 0;
  if (sz > 0 && cnt > (0x7fff0000 / sz))
    cnt = 0x7fff0000 / sz;
  szAlloc = (i64)sz * (i64)cnt;
  if (szAlloc == 0) {
    sz = 0;
    pStart = 0;
  } else if (pBuf == 0) {
    sqlite3BeginBenignMalloc();
    pStart = sqlite3Malloc(szAlloc);
    sqlite3EndBenignMalloc();
    if (pStart)
      szAlloc = sqlite3MallocSize(pStart);
  } else {
    pStart = pBuf;
  }

  if (sz >= 128 * 3) {
    nBig = szAlloc / (3 * 128 + sz);
    nSm = (szAlloc - (i64)sz * (i64)nBig) / 128;
  } else if (sz >= 128 * 2) {
    nBig = szAlloc / (128 + sz);
    nSm = (szAlloc - (i64)sz * (i64)nBig) / 128;
  } else if (sz > 0) {
    nBig = szAlloc / sz;
    nSm = 0;
  } else {
    nBig = nSm = 0;
  }
  db->lookaside.pStart = pStart;
  db->lookaside.pInit = 0;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  db->lookaside.szTrue = (u16)sz;
  if (pStart) {
    int i;
    LookasideSlot *p;

    p = (LookasideSlot *)pStart;
    for (i = 0; i < nBig; i++) {
      p->pNext = db->lookaside.pInit;
      db->lookaside.pInit = p;
      p = (LookasideSlot *)&((u8 *)p)[sz];
    }

    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.pMiddle = p;
    for (i = 0; i < nSm; i++) {
      p->pNext = db->lookaside.pSmallInit;
      db->lookaside.pSmallInit = p;
      p = (LookasideSlot *)&((u8 *)p)[128];
    }

    db->lookaside.pEnd = p;
    db->lookaside.bDisable = 0;
    db->lookaside.bMalloced = pBuf == 0 ? 1 : 0;
    db->lookaside.nSlot = nBig + nSm;
  } else {
    db->lookaside.pStart = 0;

    db->lookaside.pSmallInit = 0;
    db->lookaside.pSmallFree = 0;
    db->lookaside.pMiddle = 0;

    db->lookaside.pEnd = 0;
    db->lookaside.bDisable = 1;
    db->lookaside.sz = 0;
    db->lookaside.bMalloced = 0;
    db->lookaside.nSlot = 0;
  }
  db->lookaside.pTrueEnd = db->lookaside.pEnd;

  return 0;
}

sqlite3_mutex *sqlite3_db_mutex(sqlite3 *db) {
  return db->mutex;
}

int sqlite3_db_release_memory(sqlite3 *db) {
  int i;

  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  for (i = 0; i < db->nDb; i++) {
    Btree *pBt = db->aDb[i].pBt;
    if (pBt) {
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_db_cacheflush(sqlite3 *db) {
  int i;
  int rc = SQLITE_OK;
  int bSeenBusy = 0;

  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  for (i = 0; rc == SQLITE_OK && i < db->nDb; i++) {
    Btree *pBt = db->aDb[i].pBt;
    if (pBt && sqlite3BtreeTxnState(pBt) == SQLITE_TXN_WRITE) {
      Pager *pPager = sqlite3BtreePager(pBt);
      rc = sqlite3PagerFlush(pPager);
      if (rc == SQLITE_BUSY) {
        bSeenBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  return ((rc == SQLITE_OK && bSeenBusy) ? SQLITE_BUSY : rc);
}

int sqlite3_db_config(sqlite3 *db, int op, ...) {
  va_list ap;
  int rc;

  sqlite3_mutex_enter(db->mutex);

  va_start(ap, op);
  switch (op) {
    case SQLITE_DBCONFIG_MAINDBNAME: {
      db->aDb[0].zDbSName = va_arg(ap, char *);
      rc = SQLITE_OK;
      break;
    }
    case SQLITE_DBCONFIG_LOOKASIDE: {
      void *pBuf = va_arg(ap, void *);
      int sz = va_arg(ap, int);
      int cnt = va_arg(ap, int);
      rc = setupLookaside(db, pBuf, sz, cnt);
      break;
    }
    case SQLITE_DBCONFIG_FP_DIGITS: {
      int nIn = va_arg(ap, int);
      int *pOut = va_arg(ap, int *);
      if (nIn > 3 && nIn < 24)
        db->nFpDigit = (u8)nIn;
      if (pOut)
        *pOut = db->nFpDigit;
      rc = SQLITE_OK;
      break;
    }
    default: {
      static const struct {
        int op;
        u64 mask;
      } aFlagOp[] = {
          {SQLITE_DBCONFIG_ENABLE_FKEY, 0x00004000},
          {SQLITE_DBCONFIG_ENABLE_TRIGGER, 0x00040000},
          {SQLITE_DBCONFIG_ENABLE_VIEW, 0x80000000},
          {SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER, 0x00400000},
          {SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 0x00010000},
          {SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE, 0x00000800},
          {SQLITE_DBCONFIG_ENABLE_QPSG, 0x00800000},
          {SQLITE_DBCONFIG_TRIGGER_EQP, 0x01000000},
          {SQLITE_DBCONFIG_RESET_DATABASE, 0x02000000},
          {SQLITE_DBCONFIG_DEFENSIVE, 0x10000000},
          {SQLITE_DBCONFIG_WRITABLE_SCHEMA, 0x00000001 | 0x08000000},
          {SQLITE_DBCONFIG_LEGACY_ALTER_TABLE, 0x04000000},
          {SQLITE_DBCONFIG_DQS_DDL, 0x20000000},
          {SQLITE_DBCONFIG_DQS_DML, 0x40000000},
          {SQLITE_DBCONFIG_LEGACY_FILE_FORMAT, 0x00000002},
          {SQLITE_DBCONFIG_TRUSTED_SCHEMA, 0x00000080},
          {SQLITE_DBCONFIG_STMT_SCANSTATUS, 0x00000400},
          {SQLITE_DBCONFIG_REVERSE_SCANORDER, 0x00001000},
          {SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE, ((u64)(0x00010) << 32)},
          {SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE, ((u64)(0x00020) << 32)},
          {SQLITE_DBCONFIG_ENABLE_COMMENTS, ((u64)(0x00040) << 32)},
      };
      unsigned int i;
      rc = SQLITE_ERROR;
      for (i = 0; i < ((int)(sizeof(aFlagOp) / sizeof(aFlagOp[0]))); i++) {
        if (aFlagOp[i].op == op) {
          int onoff = va_arg(ap, int);
          int *pRes = va_arg(ap, int *);
          u64 oldFlags = db->flags;
          if (onoff > 0) {
            db->flags |= aFlagOp[i].mask;
          } else if (onoff == 0) {
            db->flags &= ~(u64)aFlagOp[i].mask;
          }
          if (oldFlags != db->flags) {
            sqlite3ExpirePreparedStatements(db, 0);
          }
          if (pRes) {
            *pRes = (db->flags & aFlagOp[i].mask) != 0;
          }
          rc = SQLITE_OK;
          break;
        }
      }
      break;
    }
  }

  va_end(ap);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

sqlite_int64 sqlite3_last_insert_rowid(sqlite3 *db) {
  i64 iRet;

  sqlite3_mutex_enter(db->mutex);
  iRet = db->lastRowid;
  sqlite3_mutex_leave(db->mutex);
  return iRet;
}

void sqlite3_set_last_insert_rowid(sqlite3 *db, sqlite3_int64 iRowid) {
  sqlite3_mutex_enter(db->mutex);
  db->lastRowid = iRowid;
  sqlite3_mutex_leave(db->mutex);
}

sqlite3_int64 sqlite3_changes64(sqlite3 *db) {
  i64 iRet;

  sqlite3_mutex_enter(db->mutex);
  iRet = db->nChange;
  sqlite3_mutex_leave(db->mutex);
  return iRet;
}

int sqlite3_changes(sqlite3 *db) {
  return (int)sqlite3_changes64(db);
}

sqlite3_int64 sqlite3_total_changes64(sqlite3 *db) {
  i64 iRet;

  sqlite3_mutex_enter(db->mutex);
  iRet = db->nTotalChange;
  sqlite3_mutex_leave(db->mutex);
  return iRet;
}

int sqlite3_total_changes(sqlite3 *db) {
  return (int)sqlite3_total_changes64(db);
}

void sqlite3CloseSavepoints(sqlite3 *db) {
  while (db->pSavepoint) {
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->pNext;
    sqlite3DbFree(db, pTmp);
  }
  db->nSavepoint = 0;
  db->nStatement = 0;
  db->isTransactionSavepoint = 0;
}

void functionDestroy(sqlite3 *db, FuncDef *p) {
  FuncDestructor *pDestructor;

  pDestructor = p->u.pDestructor;
  if (pDestructor) {
    pDestructor->nRef--;
    if (pDestructor->nRef == 0) {
      pDestructor->xDestroy(pDestructor->pUserData);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

void disconnectAllVtab(sqlite3 *db) {
  int i;
  HashElem *p;
  sqlite3BtreeEnterAll(db);
  for (i = 0; i < db->nDb; i++) {
    Schema *pSchema = db->aDb[i].pSchema;
    if (pSchema) {
      for (p = ((&pSchema->tblHash)->first); p; p = ((p)->next)) {
        Table *pTab = (Table *)((p)->data);
        if ((pTab)->eTabType == 1)
          sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  for (p = ((&db->aModule)->first); p; p = ((p)->next)) {
    Module *pMod = (Module *)((p)->data);
    if (pMod->pEpoTab) {
      sqlite3VtabDisconnect(db, pMod->pEpoTab);
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
}

int connectionIsBusy(sqlite3 *db) {
  int j;

  if (db->pVdbe)
    return 1;
  for (j = 0; j < db->nDb; j++) {
    Btree *pBt = db->aDb[j].pBt;
    if (pBt && sqlite3BtreeIsInBackup(pBt))
      return 1;
  }
  return 0;
}

int sqlite3Close(sqlite3 *db, int forceZombie) {
  if (!db) {
    return SQLITE_OK;
  }
  if (!sqlite3SafetyCheckSickOrOk(db)) {
    return sqlite3MisuseError(188644);
  }
  sqlite3_mutex_enter(db->mutex);
  if (db->mTrace & SQLITE_TRACE_CLOSE) {
    db->trace.xV2(SQLITE_TRACE_CLOSE, db->pTraceArg, db, 0);
  }

  disconnectAllVtab(db);

  sqlite3VtabRollback(db);

  if (!forceZombie && connectionIsBusy(db)) {
    sqlite3ErrorWithMsg(db, SQLITE_BUSY,
                        "unable to close due to unfinalized "
                        "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    return SQLITE_BUSY;
  }

  while (db->pDbData) {
    DbClientData *p = db->pDbData;
    db->pDbData = p->pNext;

    if (p->xDestructor)
      p->xDestructor(p->pData);
    sqlite3_free(p);
  }

  db->eOpenState = 0xa7;
  sqlite3LeaveMutexAndCloseZombie(db);
  return SQLITE_OK;
}

int sqlite3_txn_state(sqlite3 *db, const char *zSchema) {
  int iDb, nDb;
  int iTxn = -1;

  sqlite3_mutex_enter(db->mutex);
  if (zSchema) {
    nDb = iDb = sqlite3FindDbName(db, zSchema);
    if (iDb < 0)
      nDb--;
  } else {
    iDb = 0;
    nDb = db->nDb - 1;
  }
  for (; iDb <= nDb; iDb++) {
    Btree *pBt = db->aDb[iDb].pBt;
    int x = pBt != 0 ? sqlite3BtreeTxnState(pBt) : SQLITE_TXN_NONE;
    if (x > iTxn)
      iTxn = x;
  }
  sqlite3_mutex_leave(db->mutex);
  return iTxn;
}

int sqlite3_close(sqlite3 *db) {
  return sqlite3Close(db, 0);
}

int sqlite3_close_v2(sqlite3 *db) {
  return sqlite3Close(db, 1);
}

void sqlite3LeaveMutexAndCloseZombie(sqlite3 *db) {
  HashElem *i;
  int j;

  if (db->eOpenState != 0xa7 || connectionIsBusy(db)) {
    sqlite3_mutex_leave(db->mutex);
    return;
  }

  sqlite3RollbackAll(db, SQLITE_OK);

  sqlite3CloseSavepoints(db);

  for (j = 0; j < db->nDb; j++) {
    struct Db *pDb = &db->aDb[j];
    if (pDb->pBt) {
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      if (j != 1) {
        pDb->pSchema = 0;
      }
    }
  }

  if (db->aDb[1].pSchema) {
    sqlite3SchemaClear(db->aDb[1].pSchema);
  }
  sqlite3VtabUnlockList(db);

  sqlite3CollapseDatabaseArray(db);

  for (i = ((&db->aFunc)->first); i; i = ((i)->next)) {
    FuncDef *pNext, *p;
    p = ((i)->data);
    do {
      functionDestroy(db, p);
      pNext = p->pNext;
      sqlite3DbFree(db, p);
      p = pNext;
    } while (p);
  }
  sqlite3HashClear(&db->aFunc);
  for (i = ((&db->aCollSeq)->first); i; i = ((i)->next)) {
    CollSeq *pColl = (CollSeq *)((i)->data);

    for (j = 0; j < 3; j++) {
      if (pColl[j].xDel) {
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);

  for (i = ((&db->aModule)->first); i; i = ((i)->next)) {
    Module *pMod = (Module *)((i)->data);
    sqlite3VtabEponymousTableClear(db, pMod);
    sqlite3VtabModuleUnref(db, pMod);
  }
  sqlite3HashClear(&db->aModule);

  sqlite3Error(db, SQLITE_OK);
  sqlite3ValueFree(db->pErr);
  sqlite3CloseExtensions(db);

  db->eOpenState = 0xd5;

  sqlite3DbFree(db, db->aDb[1].pSchema);
  if (db->xAutovacDestr) {
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  sqlite3_mutex_leave(db->mutex);
  db->eOpenState = 0xce;
  sqlite3_mutex_free(db->mutex);

  if (db->lookaside.bMalloced) {
    sqlite3_free(db->lookaside.pStart);
  }
  sqlite3_free(db);
}

void sqlite3RollbackAll(sqlite3 *db, int tripCode) {
  int i;
  int inTrans = 0;
  int schemaChange;

  sqlite3BeginBenignMalloc();

  sqlite3BtreeEnterAll(db);
  schemaChange = (db->mDbFlags & 0x0001) != 0 && db->init.busy == 0;

  for (i = 0; i < db->nDb; i++) {
    Btree *p = db->aDb[i].pBt;
    if (p) {
      if (sqlite3BtreeTxnState(p) == SQLITE_TXN_WRITE) {
        inTrans = 1;
      }
      sqlite3BtreeRollback(p, tripCode, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3EndBenignMalloc();

  if (schemaChange) {
    sqlite3ExpirePreparedStatements(db, 0);
    sqlite3ResetAllSchemasOfConnection(db);
  }
  sqlite3BtreeLeaveAll(db);

  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~(u64)(0x00080000 | ((u64)(0x00002) << 32));

  if (db->xRollbackCallback && (inTrans || !db->autoCommit)) {
    db->xRollbackCallback(db->pRollbackArg);
  }
}

const char *sqlite3ErrStr(int rc) {
  static const char *const aMsg[] = {
      "not an error",
      "SQL logic error",
      0,
      "access permission denied",
      "query aborted",
      "database is locked",
      "database table is locked",
      "out of memory",
      "attempt to write a readonly database",
      "interrupted",
      "disk I/O error",
      "database disk image is malformed",
      "unknown operation",
      "database or disk is full",
      "unable to open database file",
      "locking protocol",
      0,
      "database schema has changed",
      "string or blob too big",
      "constraint failed",
      "datatype mismatch",
      "bad parameter or other API misuse",
      0,
      "authorization denied",
      0,
      "column index out of range",
      "file is not a database",
      "notification message",
      "warning message",
  };
  const char *zErr = "unknown error";
  switch (rc) {
    case (4 | (2 << 8)): {
      zErr = "abort due to ROLLBACK";
      break;
    }
    case SQLITE_ROW: {
      zErr = "another row available";
      break;
    }
    case SQLITE_DONE: {
      zErr = "no more rows available";
      break;
    }
    default: {
      rc &= 0xff;
      if ((rc >= 0) && rc < ((int)(sizeof(aMsg) / sizeof(aMsg[0]))) && aMsg[rc] != 0) {
        zErr = aMsg[rc];
      }
      break;
    }
  }
  return zErr;
}

int sqlite3_busy_handler(sqlite3 *db, int (*xBusy)(void *, int), void *pArg) {
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xBusyHandler = xBusy;
  db->busyHandler.pBusyArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;

  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

void sqlite3_progress_handler(sqlite3 *db, int nOps, int (*xProgress)(void *), void *pArg) {
  sqlite3_mutex_enter(db->mutex);
  if (nOps > 0) {
    db->xProgress = xProgress;
    db->nProgressOps = (unsigned)nOps;
    db->pProgressArg = pArg;
  } else {
    db->xProgress = 0;
    db->nProgressOps = 0;
    db->pProgressArg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}

int sqlite3_busy_timeout(sqlite3 *db, int ms) {
  sqlite3_mutex_enter(db->mutex);
  if (ms > 0) {
    sqlite3_busy_handler(db, (int (*)(void *, int))sqliteDefaultBusyCallback, (void *)db);
    db->busyTimeout = ms;

  } else {
    sqlite3_busy_handler(db, 0, 0);
  }
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_setlk_timeout(sqlite3 *db, int ms, int flags) {
  if (ms < -1)
    return SQLITE_RANGE;

  (void)(db);
  (void)(flags);

  return SQLITE_OK;
}

void sqlite3_interrupt(sqlite3 *db) {
  __atomic_store_n((&db->u1.isInterrupted), (1), 0);
}

int sqlite3_is_interrupted(sqlite3 *db) {
  return __atomic_load_n((&db->u1.isInterrupted), 0) != 0;
}

int sqlite3CreateFunc(sqlite3 *db, const char *zFunctionName, int nArg, int enc, void *pUserData,
                      void (*xSFunc)(sqlite3_context *, int, sqlite3_value **),
                      void (*xStep)(sqlite3_context *, int, sqlite3_value **), void (*xFinal)(sqlite3_context *),
                      void (*xValue)(sqlite3_context *), void (*xInverse)(sqlite3_context *, int, sqlite3_value **),
                      FuncDestructor *pDestructor) {
  FuncDef *p;
  int extraFlags;

  if (zFunctionName == 0 || (xSFunc != 0 && xFinal != 0) || ((xFinal == 0) != (xStep == 0)) ||
      ((xValue == 0) != (xInverse == 0)) || (nArg < -1 || nArg > 1000) || (255 < sqlite3Strlen30(zFunctionName))) {
    return sqlite3MisuseError(189341);
  }

  extraFlags = enc & (SQLITE_DETERMINISTIC | SQLITE_DIRECTONLY | SQLITE_SUBTYPE | SQLITE_INNOCUOUS |
                      SQLITE_RESULT_SUBTYPE | SQLITE_SELFORDER1);
  enc &= (0x0003 | 5);

  extraFlags ^= 0x00200000;

  switch (enc) {
    case SQLITE_UTF16:
      enc = 2;
      break;
    case SQLITE_ANY: {
      int rc;
      rc = sqlite3CreateFunc(db, zFunctionName, nArg, (SQLITE_UTF8 | extraFlags) ^ 0x00200000, pUserData, xSFunc, xStep,
                             xFinal, xValue, xInverse, pDestructor);
      if (rc == SQLITE_OK) {
        rc = sqlite3CreateFunc(db, zFunctionName, nArg, (SQLITE_UTF16LE | extraFlags) ^ 0x00200000, pUserData, xSFunc,
                               xStep, xFinal, xValue, xInverse, pDestructor);
      }
      if (rc != SQLITE_OK) {
        return rc;
      }
      enc = SQLITE_UTF16BE;
      break;
    }
    case SQLITE_UTF8:
    case SQLITE_UTF16LE:
    case SQLITE_UTF16BE:
      break;
    default:
      enc = SQLITE_UTF8;
      break;
  }

  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 0);
  if (p && (p->funcFlags & 0x0003) == (u32)enc && p->nArg == nArg) {
    if (db->nVdbeActive) {
      sqlite3ErrorWithMsg(db, SQLITE_BUSY, "unable to delete/modify user-function due to active statements");

      return SQLITE_BUSY;
    } else {
      sqlite3ExpirePreparedStatements(db, 0);
    }
  } else if (xSFunc == 0 && xFinal == 0) {
    return SQLITE_OK;
  }

  p = sqlite3FindFunction(db, zFunctionName, nArg, (u8)enc, 1);

  if (!p) {
    return 7;
  }

  functionDestroy(db, p);

  if (pDestructor) {
    pDestructor->nRef++;
  }
  p->u.pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & 0x0003) | extraFlags;
  p->xSFunc = xSFunc ? xSFunc : xStep;
  p->xFinalize = xFinal;
  p->xValue = xValue;
  p->xInverse = xInverse;
  p->pUserData = pUserData;
  p->nArg = (u16)nArg;
  return SQLITE_OK;
}

int createFunctionApi(sqlite3 *db, const char *zFunc, int nArg, int enc, void *p,
                      void (*xSFunc)(sqlite3_context *, int, sqlite3_value **),
                      void (*xStep)(sqlite3_context *, int, sqlite3_value **), void (*xFinal)(sqlite3_context *),
                      void (*xValue)(sqlite3_context *), void (*xInverse)(sqlite3_context *, int, sqlite3_value **),
                      void (*xDestroy)(void *)) {
  int rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

  sqlite3_mutex_enter(db->mutex);
  if (xDestroy) {
    pArg = (FuncDestructor *)sqlite3Malloc(sizeof(FuncDestructor));
    if (!pArg) {
      sqlite3OomFault(db);
      xDestroy(p);
      goto out;
    }
    pArg->nRef = 0;
    pArg->xDestroy = xDestroy;
    pArg->pUserData = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p, xSFunc, xStep, xFinal, xValue, xInverse, pArg);
  if (pArg && pArg->nRef == 0) {
    xDestroy(p);
    sqlite3_free(pArg);
  }

out:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_create_function(sqlite3 *db, const char *zFunc, int nArg, int enc, void *p,
                            void (*xSFunc)(sqlite3_context *, int, sqlite3_value **),
                            void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                            void (*xFinal)(sqlite3_context *)) {
  return createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep, xFinal, 0, 0, 0);
}

int sqlite3_create_function_v2(sqlite3 *db, const char *zFunc, int nArg, int enc, void *p,
                               void (*xSFunc)(sqlite3_context *, int, sqlite3_value **),
                               void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                               void (*xFinal)(sqlite3_context *), void (*xDestroy)(void *)) {
  return createFunctionApi(db, zFunc, nArg, enc, p, xSFunc, xStep, xFinal, 0, 0, xDestroy);
}

int sqlite3_create_window_function(sqlite3 *db, const char *zFunc, int nArg, int enc, void *p,
                                   void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                                   void (*xFinal)(sqlite3_context *), void (*xValue)(sqlite3_context *),
                                   void (*xInverse)(sqlite3_context *, int, sqlite3_value **),
                                   void (*xDestroy)(void *)) {
  return createFunctionApi(db, zFunc, nArg, enc, p, 0, xStep, xFinal, xValue, xInverse, xDestroy);
}

int sqlite3_create_function16(sqlite3 *db, const void *zFunctionName, int nArg, int eTextRep, void *p,
                              void (*xSFunc)(sqlite3_context *, int, sqlite3_value **),
                              void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                              void (*xFinal)(sqlite3_context *)) {
  int rc;
  char *zFunc8;

  sqlite3_mutex_enter(db->mutex);

  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, 2);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, eTextRep, p, xSFunc, xStep, xFinal, 0, 0, 0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_overload_function(sqlite3 *db, const char *zName, int nArg) {
  int rc;
  char *zCopy;

  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3FindFunction(db, zName, nArg, SQLITE_UTF8, 0) != 0;
  sqlite3_mutex_leave(db->mutex);
  if (rc)
    return SQLITE_OK;
  zCopy = sqlite3_mprintf("%s", zName);
  if (zCopy == 0)
    return SQLITE_NOMEM;
  return sqlite3_create_function_v2(db, zName, nArg, SQLITE_UTF8, zCopy, sqlite3InvalidFunction, 0, 0, sqlite3_free);
}

void *sqlite3_trace(sqlite3 *db, void (*xTrace)(void *, const char *), void *pArg) {
  void *pOld;

  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->mTrace = xTrace ? 0x40 : 0;
  db->trace.xLegacy = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pOld;
}

int sqlite3_trace_v2(sqlite3 *db, unsigned mTrace, int (*xTrace)(unsigned, void *, void *, void *), void *pArg) {
  sqlite3_mutex_enter(db->mutex);
  if (mTrace == 0)
    xTrace = 0;
  if (xTrace == 0)
    mTrace = 0;
  db->mTrace = mTrace;
  db->trace.xV2 = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

void *sqlite3_profile(sqlite3 *db, void (*xProfile)(void *, const char *, sqlite_uint64), void *pArg) {
  void *pOld;

  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  db->mTrace &= 0x0f;
  if (db->xProfile)
    db->mTrace |= 0x80;
  sqlite3_mutex_leave(db->mutex);
  return pOld;
}

void *sqlite3_commit_hook(sqlite3 *db, int (*xCallback)(void *), void *pArg) {
  void *pOld;

  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pOld;
}

void *sqlite3_update_hook(sqlite3 *db, void (*xCallback)(void *, int, char const *, char const *, sqlite_int64),
                          void *pArg) {
  void *pRet;

  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pRet;
}

void *sqlite3_rollback_hook(sqlite3 *db, void (*xCallback)(void *), void *pArg) {
  void *pRet;

  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pRet;
}

int sqlite3_autovacuum_pages(sqlite3 *db, unsigned int (*xCallback)(void *, const char *, u32, u32, u32), void *pArg,
                             void (*xDestructor)(void *)) {
  sqlite3_mutex_enter(db->mutex);
  if (db->xAutovacDestr) {
    db->xAutovacDestr(db->pAutovacPagesArg);
  }
  db->xAutovacPages = xCallback;
  db->pAutovacPagesArg = pArg;
  db->xAutovacDestr = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_wal_autocheckpoint(sqlite3 *db, int nFrame) {
  if (nFrame > 0) {
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, ((void *)(intptr_t)(nFrame)));
  } else {
    sqlite3_wal_hook(db, 0, 0);
  }

  return SQLITE_OK;
}

void *sqlite3_wal_hook(sqlite3 *db, int (*xCallback)(void *, sqlite3 *, const char *, int), void *pArg) {
  void *pRet;

  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pRet;
}

int sqlite3_wal_checkpoint_v2(sqlite3 *db, const char *zDb, int eMode, int *pnLog, int *pnCkpt) {
  int rc;
  int iDb;

  if (pnLog)
    *pnLog = -1;
  if (pnCkpt)
    *pnCkpt = -1;

  if (eMode < -1 || eMode > SQLITE_CHECKPOINT_TRUNCATE) {
    return sqlite3MisuseError(189966);
  }

  sqlite3_mutex_enter(db->mutex);
  if (zDb && zDb[0]) {
    iDb = sqlite3FindDbName(db, zDb);
  } else {
    iDb = (10 + 2);
  }
  if (iDb < 0) {
    rc = SQLITE_ERROR;
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, "unknown database: %s", zDb);
  } else {
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, eMode, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);

  if (db->nVdbeActive == 0) {
    __atomic_store_n((&db->u1.isInterrupted), (0), 0);
  }

  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb) {
  return sqlite3_wal_checkpoint_v2(db, zDb, SQLITE_CHECKPOINT_PASSIVE, 0, 0);
}

int sqlite3Checkpoint(sqlite3 *db, int iDb, int eMode, int *pnLog, int *pnCkpt) {
  int rc = SQLITE_OK;
  int i;
  int bBusy = 0;

  for (i = 0; i < db->nDb && rc == SQLITE_OK; i++) {
    if (i == iDb || iDb == (10 + 2)) {
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, eMode, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      if (rc == SQLITE_BUSY) {
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  return (rc == SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}

int sqlite3TempInMemory(const sqlite3 *db) {
  return (db->temp_store == 2);
}

const char *sqlite3_errmsg(sqlite3 *db) {
  const char *z;
  if (!db) {
    return sqlite3ErrStr(7);
  }
  if (!sqlite3SafetyCheckSickOrOk(db)) {
    return sqlite3ErrStr(sqlite3MisuseError(190102));
  }
  sqlite3_mutex_enter(db->mutex);
  if (db->mallocFailed) {
    z = sqlite3ErrStr(7);
  } else {
    z = db->errCode ? (char *)sqlite3_value_text(db->pErr) : 0;

    if (z == 0) {
      z = sqlite3ErrStr(db->errCode);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  return z;
}

int sqlite3_set_errmsg(sqlite3 *db, int errcode, const char *zMsg) {
  int rc = SQLITE_OK;
  if (!sqlite3SafetyCheckOk(db)) {
    return sqlite3MisuseError(190129);
  }
  sqlite3_mutex_enter(db->mutex);
  if (zMsg) {
    sqlite3ErrorWithMsg(db, errcode, "%s", zMsg);
  } else {
    sqlite3Error(db, errcode);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_error_offset(sqlite3 *db) {
  int iOffset = -1;
  if (db && sqlite3SafetyCheckSickOrOk(db)) {
    sqlite3_mutex_enter(db->mutex);
    if (db->errCode) {
      iOffset = db->errByteOffset;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  return iOffset;
}

const void *sqlite3_errmsg16(sqlite3 *db) {
  static const u16 outOfMem[] = {'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0};
  static const u16 misuse[] = {'b', 'a', 'd', ' ', 'p', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', ' ', 'o', 'r', ' ',
                               'o', 't', 'h', 'e', 'r', ' ', 'A', 'P', 'I', ' ', 'm', 'i', 's', 'u', 's', 'e', 0};

  const void *z;
  if (!db) {
    return (void *)outOfMem;
  }
  if (!sqlite3SafetyCheckSickOrOk(db)) {
    return (void *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  if (db->mallocFailed) {
    z = (void *)outOfMem;
  } else {
    z = sqlite3_value_text16(db->pErr);
    if (z == 0) {
      sqlite3ErrorWithMsg(db, db->errCode, sqlite3ErrStr(db->errCode));
      z = sqlite3_value_text16(db->pErr);
    }

    sqlite3OomClear(db);
  }
  sqlite3_mutex_leave(db->mutex);
  return z;
}

int sqlite3_errcode(sqlite3 *db) {
  int iRet;
  if (!db)
    return 7;
  if (!sqlite3SafetyCheckSickOrOk(db)) {
    return sqlite3MisuseError(190208);
  }
  sqlite3_mutex_enter(db->mutex);
  if (db->mallocFailed) {
    iRet = 7;
  } else {
    iRet = db->errCode & db->errMask;
  }
  sqlite3_mutex_leave(db->mutex);
  return iRet;
}

int sqlite3_extended_errcode(sqlite3 *db) {
  int iRet;
  if (!db)
    return 7;
  if (!sqlite3SafetyCheckSickOrOk(db)) {
    return sqlite3MisuseError(190223);
  }
  sqlite3_mutex_enter(db->mutex);
  if (db->mallocFailed) {
    iRet = 7;
  } else {
    iRet = db->errCode;
  }
  sqlite3_mutex_leave(db->mutex);
  return iRet;
}

int sqlite3_system_errno(sqlite3 *db) {
  int iRet = 0;
  if (db) {
    sqlite3_mutex_enter(db->mutex);
    iRet = db->iSysErrno;
    sqlite3_mutex_leave(db->mutex);
  }
  return iRet;
}

const char *sqlite3_errstr(int rc) {
  return sqlite3ErrStr(rc);
}

int createCollation(sqlite3 *db, const char *zName, u8 enc, void *pCtx,
                    int (*xCompare)(void *, int, const void *, int, const void *), void (*xDel)(void *)) {
  CollSeq *pColl;
  int enc2;

  enc2 = enc;
  if (enc2 == SQLITE_UTF16 || enc2 == SQLITE_UTF16_ALIGNED) {
    enc2 = 2;
  }
  if (enc2 < SQLITE_UTF8 || enc2 > SQLITE_UTF16BE) {
    return sqlite3MisuseError(190281);
  }

  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 0);
  if (pColl && pColl->xCmp) {
    if (db->nVdbeActive) {
      sqlite3ErrorWithMsg(db, SQLITE_BUSY, "unable to delete/modify collation sequence due to active statements");
      return SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db, 0);

    if ((pColl->enc & ~SQLITE_UTF16_ALIGNED) == enc2) {
      CollSeq *aColl = sqlite3HashFind(&db->aCollSeq, zName);
      int j;
      for (j = 0; j < 3; j++) {
        CollSeq *p = &aColl[j];
        if (p->enc == pColl->enc) {
          if (p->xDel) {
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 1);
  if (pColl == 0)
    return 7;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  return SQLITE_OK;
}

int sqlite3_limit(sqlite3 *db, int limitId, int newLimit) {
  int oldLimit;

  if (limitId < 0 || limitId >= (12 + 1)) {
    return -1;
  }
  sqlite3_mutex_enter(db->mutex);
  oldLimit = db->aLimit[limitId];
  if (newLimit >= 0) {
    if (newLimit > aHardLimit[limitId]) {
      newLimit = aHardLimit[limitId];
    } else if (newLimit < 30 && limitId == SQLITE_LIMIT_LENGTH) {
      newLimit = 30;
    }
    db->aLimit[limitId] = newLimit;
  }
  sqlite3_mutex_leave(db->mutex);
  return oldLimit;
}

int sqlite3_open(const char *zFilename, sqlite3 **ppDb) {
  return openDatabase(zFilename, ppDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}

int sqlite3_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs) {
  return openDatabase(filename, ppDb, (unsigned int)flags, zVfs);
}

int sqlite3_open16(const void *zFilename, sqlite3 **ppDb) {
  char const *zFilename8;
  sqlite3_value *pVal;
  int rc;

  *ppDb = 0;

  rc = sqlite3_initialize();
  if (rc)
    return rc;

  if (zFilename == 0)
    zFilename = "\000\000";
  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zFilename, 2, ((sqlite3_destructor_type)0));
  zFilename8 = sqlite3ValueText(pVal, SQLITE_UTF8);
  if (zFilename8) {
    rc = openDatabase(zFilename8, ppDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);

    if (rc == SQLITE_OK && !(((*ppDb)->aDb[0].pSchema->schemaFlags & (0x0001)) == (0x0001))) {
      ((*ppDb)->aDb[0].pSchema->enc) = ((*ppDb)->enc) = 2;
    }
  } else {
    rc = 7;
  }
  sqlite3ValueFree(pVal);

  return rc & 0xff;
}

int sqlite3_create_collation(sqlite3 *db, const char *zName, int enc, void *pCtx,
                             int (*xCompare)(void *, int, const void *, int, const void *)) {
  return sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

int sqlite3_create_collation_v2(sqlite3 *db, const char *zName, int enc, void *pCtx,
                                int (*xCompare)(void *, int, const void *, int, const void *), void (*xDel)(void *)) {
  int rc;

  sqlite3_mutex_enter(db->mutex);

  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_create_collation16(sqlite3 *db, const void *zName, int enc, void *pCtx,
                               int (*xCompare)(void *, int, const void *, int, const void *)) {
  int rc = SQLITE_OK;
  char *zName8;

  sqlite3_mutex_enter(db->mutex);

  zName8 = sqlite3Utf16to8(db, zName, -1, 2);
  if (zName8) {
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_collation_needed(sqlite3 *db, void *pCollNeededArg,
                             void (*xCollNeeded)(void *, sqlite3 *, int eTextRep, const char *)) {
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_collation_needed16(sqlite3 *db, void *pCollNeededArg,
                               void (*xCollNeeded16)(void *, sqlite3 *, int eTextRep, const void *)) {
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

void *sqlite3_get_clientdata(sqlite3 *db, const char *zName) {
  DbClientData *p;

  sqlite3_mutex_enter(db->mutex);
  for (p = db->pDbData; p; p = p->pNext) {
    if (strcmp(p->zName, zName) == 0) {
      void *pResult = p->pData;
      sqlite3_mutex_leave(db->mutex);
      return pResult;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  return 0;
}

int sqlite3_set_clientdata(sqlite3 *db, const char *zName, void *pData, void (*xDestructor)(void *)) {
  DbClientData *p, **pp;
  sqlite3_mutex_enter(db->mutex);
  pp = &db->pDbData;
  for (p = db->pDbData; p && strcmp(p->zName, zName); p = p->pNext) {
    pp = &p->pNext;
  }
  if (p) {
    if (p->xDestructor)
      p->xDestructor(p->pData);
    if (pData == 0) {
      *pp = p->pNext;
      sqlite3_free(p);
      sqlite3_mutex_leave(db->mutex);
      return SQLITE_OK;
    }
  } else if (pData == 0) {
    sqlite3_mutex_leave(db->mutex);
    return SQLITE_OK;
  } else {
    size_t n = strlen(zName);
    p = sqlite3_malloc64((offsetof(DbClientData, zName) + (n + 1)));
    if (p == 0) {
      if (xDestructor)
        xDestructor(pData);
      sqlite3_mutex_leave(db->mutex);
      return SQLITE_NOMEM;
    }
    memcpy(p->zName, zName, n + 1);
    p->pNext = db->pDbData;
    db->pDbData = p;
  }
  p->pData = pData;
  p->xDestructor = xDestructor;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_global_recover(void) {
  return SQLITE_OK;
}

int sqlite3_get_autocommit(sqlite3 *db) {
  int iRet;

  sqlite3_mutex_enter(db->mutex);
  iRet = db->autoCommit;
  sqlite3_mutex_leave(db->mutex);
  return iRet;
}

int sqlite3ReportError(int iErr, int lineno, const char *zType) {
  sqlite3_log(iErr, "%s at line %d of [%.10s]", zType, lineno, 20 + sqlite3_sourceid());
  return iErr;
}

int sqlite3CorruptError(int lineno) {
  return sqlite3ReportError(SQLITE_CORRUPT, lineno, "database corruption");
}

int sqlite3MisuseError(int lineno) {
  return sqlite3ReportError(SQLITE_MISUSE, lineno, "misuse");
}

int sqlite3CantopenError(int lineno) {
  return sqlite3ReportError(SQLITE_CANTOPEN, lineno, "cannot open file");
}

void sqlite3_thread_cleanup(void) {}

int sqlite3_table_column_metadata(sqlite3 *db, const char *zDbName, const char *zTableName, const char *zColumnName,
                                  char const **pzDataType, char const **pzCollSeq, int *pNotNull, int *pPrimaryKey,
                                  int *pAutoinc) {
  int rc;
  char *zErrMsg = 0;
  Table *pTab = 0;
  Column *pCol = 0;
  int iCol = 0;
  char const *zDataType = 0;
  char const *zCollSeq = 0;
  int notnull = 0;
  int primarykey = 0;
  int autoinc = 0;

  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &zErrMsg);
  if (SQLITE_OK != rc) {
    goto error_out;
  }

  pTab = sqlite3FindTable(db, zTableName, zDbName);
  if (!pTab || ((pTab)->eTabType == 2)) {
    pTab = 0;
    goto error_out;
  }

  if (zColumnName == 0) {
  } else {
    iCol = sqlite3ColumnIndex(pTab, zColumnName);
    if (iCol >= 0) {
      pCol = &pTab->aCol[iCol];
    } else {
      if ((((pTab)->tabFlags & 0x00000080) == 0) && sqlite3IsRowid(zColumnName)) {
        iCol = pTab->iPKey;
        pCol = iCol >= 0 ? &pTab->aCol[iCol] : 0;
      } else {
        pTab = 0;
        goto error_out;
      }
    }
  }

  if (pCol) {
    zDataType = sqlite3ColumnType(pCol, 0);
    zCollSeq = sqlite3ColumnColl(pCol);
    notnull = pCol->notNull != 0;
    primarykey = (pCol->colFlags & 0x0001) != 0;
    autoinc = pTab->iPKey == iCol && (pTab->tabFlags & 0x00000008) != 0;
  } else {
    zDataType = "INTEGER";
    primarykey = 1;
  }
  if (!zCollSeq) {
    zCollSeq = sqlite3StrBINARY;
  }

error_out:
  sqlite3BtreeLeaveAll(db);

  if (pzDataType)
    *pzDataType = zDataType;
  if (pzCollSeq)
    *pzCollSeq = zCollSeq;
  if (pNotNull)
    *pNotNull = notnull;
  if (pPrimaryKey)
    *pPrimaryKey = primarykey;
  if (pAutoinc)
    *pAutoinc = autoinc;

  if (SQLITE_OK == rc && !pTab) {
    sqlite3DbFree(db, zErrMsg);
    zErrMsg = sqlite3MPrintf(db, "no such table column: %s.%s", zTableName, zColumnName);
    rc = SQLITE_ERROR;
  }
  sqlite3ErrorWithMsg(db, rc, (zErrMsg ? "%s" : 0), zErrMsg);
  sqlite3DbFree(db, zErrMsg);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_sleep(int ms) {
  sqlite3_vfs *pVfs;
  int rc;
  pVfs = sqlite3_vfs_find(0);
  if (pVfs == 0)
    return 0;

  rc = (sqlite3OsSleep(pVfs, ms < 0 ? 0 : 1000 * ms) / 1000);
  return rc;
}

int sqlite3_extended_result_codes(sqlite3 *db, int onoff) {
  sqlite3_mutex_enter(db->mutex);
  db->errMask = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

int sqlite3_file_control(sqlite3 *db, const char *zDbName, int op, void *pArg) {
  int rc = SQLITE_ERROR;
  Btree *pBtree;

  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  if (pBtree) {
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);

    fd = sqlite3PagerFile(pPager);

    if (op == SQLITE_FCNTL_FILE_POINTER) {
      *(sqlite3_file **)pArg = fd;
      rc = SQLITE_OK;
    } else if (op == SQLITE_FCNTL_VFS_POINTER) {
      *(sqlite3_vfs **)pArg = sqlite3PagerVfs(pPager);
      rc = SQLITE_OK;
    } else if (op == SQLITE_FCNTL_JOURNAL_POINTER) {
      *(sqlite3_file **)pArg = sqlite3PagerJrnlFile(pPager);
      rc = SQLITE_OK;
    } else if (op == SQLITE_FCNTL_DATA_VERSION) {
      *(unsigned int *)pArg = sqlite3PagerDataVersion(pPager);
      rc = SQLITE_OK;
    } else if (op == SQLITE_FCNTL_RESERVE_BYTES) {
      int iNew = *(int *)pArg;
      *(int *)pArg = sqlite3BtreeGetRequestedReserve(pBtree);
      if (iNew >= 0 && iNew <= 255) {
        sqlite3BtreeSetPageSize(pBtree, 0, iNew, 0);
      }
      rc = SQLITE_OK;
    } else if (op == SQLITE_FCNTL_RESET_CACHE) {
      sqlite3BtreeClearCache(pBtree);
      rc = SQLITE_OK;
    } else {
      int nSave = db->busyHandler.nBusy;
      rc = sqlite3OsFileControl(fd, op, pArg);
      db->busyHandler.nBusy = nSave;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_test_control(int op, ...) {
  int rc = 0;

  va_list ap;

  va_start(ap, op);
  switch (op) {
    case SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      break;
    }

    case SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      break;
    }

    case SQLITE_TESTCTRL_PRNG_SEED: {
      int x = va_arg(ap, int);
      int y;
      sqlite3 *db = va_arg(ap, sqlite3 *);

      if (db && (y = db->aDb[0].pSchema->schema_cookie) != 0) {
        x = y;
      }
      sqlite3Config.iPrngSeed = x;
      sqlite3_randomness(0, 0);
      break;
    }

    case SQLITE_TESTCTRL_FK_NO_ACTION: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      int b = va_arg(ap, int);
      if (b) {
        db->flags |= ((u64)(0x00008) << 32);
      } else {
        db->flags &= ~((u64)(0x00008) << 32);
      }
      break;
    }

    case SQLITE_TESTCTRL_BITVEC_TEST: {
      int sz = va_arg(ap, int);
      int *aProg = va_arg(ap, int *);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      break;
    }

    case SQLITE_TESTCTRL_FAULT_INSTALL: {
      typedef int (*sqlite3FaultFuncType)(int);
      sqlite3Config.xTestCallback = va_arg(ap, sqlite3FaultFuncType);
      rc = sqlite3FaultSim(0);
      break;
    }

    case SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      typedef void (*void_function)(void);
      void_function xBenignBegin;
      void_function xBenignEnd;
      xBenignBegin = va_arg(ap, void_function);
      xBenignEnd = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, xBenignEnd);
      break;
    }

    case SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = sqlite3PendingByte;

      {
        unsigned int newVal = va_arg(ap, unsigned int);
        if (newVal)
          sqlite3PendingByte = newVal;
      }

      break;
    }

    case SQLITE_TESTCTRL_ASSERT: {
      volatile int x = 0;

      rc = x;

      break;
    }

    case SQLITE_TESTCTRL_ALWAYS: {
      int x = va_arg(ap, int);
      rc = x ? (x) : 0;
      break;
    }

    case SQLITE_TESTCTRL_BYTEORDER: {
      rc = 1234 * 100 + 1 * 10 + 0;
      break;
    }

    case SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      db->dbOptFlags = va_arg(ap, u32);
      break;
    }

    case SQLITE_TESTCTRL_GETOPT: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      int *pN = va_arg(ap, int *);
      *pN = db->dbOptFlags;
      break;
    }

    case SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3Config.bLocaltimeFault = va_arg(ap, int);
      if (sqlite3Config.bLocaltimeFault == 2) {
        typedef int (*sqlite3LocaltimeType)(const void *, void *);
        sqlite3Config.xAltLocaltime = va_arg(ap, sqlite3LocaltimeType);
      } else {
        sqlite3Config.xAltLocaltime = 0;
      }
      break;
    }

    case SQLITE_TESTCTRL_INTERNAL_FUNCTIONS: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      db->mDbFlags ^= 0x0020;
      break;
    }

    case SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3Config.neverCorrupt = va_arg(ap, int);
      break;
    }

    case SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS: {
      sqlite3Config.bExtraSchemaChecks = va_arg(ap, int);
      break;
    }

    case SQLITE_TESTCTRL_ONCE_RESET_THRESHOLD: {
      sqlite3Config.iOnceResetThreshold = va_arg(ap, int);
      break;
    }

    case SQLITE_TESTCTRL_VDBE_COVERAGE: {
      break;
    }

    case SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      db->nMaxSorterMmap = va_arg(ap, int);
      break;
    }

    case SQLITE_TESTCTRL_ISINIT: {
      if (sqlite3Config.isInit == 0)
        rc = SQLITE_ERROR;
      break;
    }

    case SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      int iDb;
      sqlite3_mutex_enter(db->mutex);
      iDb = sqlite3FindDbName(db, va_arg(ap, const char *));
      if (iDb >= 0) {
        db->init.iDb = iDb;
        db->init.busy = db->init.imposterTable = va_arg(ap, int);
        db->init.newTnum = va_arg(ap, int);
        if (db->init.busy == 0 && db->init.newTnum > 0) {
          sqlite3ResetAllSchemasOfConnection(db);
        }
      }
      sqlite3_mutex_leave(db->mutex);
      break;
    }

    case 27: {
      sqlite3_context *pCtx = va_arg(ap, sqlite3_context *);
      sqlite3ResultIntReal(pCtx);
      break;
    }

    case SQLITE_TESTCTRL_SEEK_COUNT: {
      sqlite3 *db = va_arg(ap, sqlite3 *);
      u64 *pn = va_arg(ap, sqlite3_uint64 *);
      *pn = 0;
      (void)db;
      break;
    }

    case SQLITE_TESTCTRL_TRACEFLAGS: {
      int opTrace = va_arg(ap, int);
      u32 *ptr = va_arg(ap, u32 *);
      switch (opTrace) {
        case 0:
          *ptr = sqlite3TreeTrace;
          break;
        case 1:
          sqlite3TreeTrace = *ptr;
          break;
        case 2:
          *ptr = sqlite3WhereTrace;
          break;
        case 3:
          sqlite3WhereTrace = *ptr;
          break;
      }
      break;
    }

    case SQLITE_TESTCTRL_LOGEST: {
      double rIn = va_arg(ap, double);
      LogEst rLogEst = sqlite3LogEstFromDouble(rIn);
      int *pI1 = va_arg(ap, int *);
      u64 *pU64 = va_arg(ap, u64 *);
      int *pI2 = va_arg(ap, int *);
      *pI1 = rLogEst;
      *pU64 = sqlite3LogEstToInt(rLogEst);
      *pI2 = sqlite3LogEst(*pU64);
      break;
    }

    case SQLITE_TESTCTRL_ATOF: {
      const char *z = va_arg(ap, const char *);
      double *pR = va_arg(ap, double *);
      rc = sqlite3AtoF(z, pR);
      break;
    }

    case 14: {
      break;
    }
  }

  va_end(ap);

  return rc;
}

const char *sqlite3_create_filename(const char *zDatabase, const char *zJournal, const char *zWal, int nParam,
                                    const char **azParam) {
  sqlite3_int64 nByte;
  int i;
  char *pResult, *p;
  nByte = strlen(zDatabase) + strlen(zJournal) + strlen(zWal) + 10;
  for (i = 0; i < nParam * 2; i++) {
    nByte += strlen(azParam[i]) + 1;
  }
  pResult = p = sqlite3_malloc64(nByte);
  if (p == 0)
    return 0;
  memset(p, 0, 4);
  p += 4;
  p = appendText(p, zDatabase);
  for (i = 0; i < nParam * 2; i++) {
    p = appendText(p, azParam[i]);
  }
  *(p++) = 0;
  p = appendText(p, zJournal);
  p = appendText(p, zWal);
  *(p++) = 0;
  *(p++) = 0;

  return pResult + 4;
}

void sqlite3_free_filename(const char *p) {
  if (p == 0)
    return;
  p = databaseName(p);
  sqlite3_free((char *)p - 4);
}

const char *sqlite3_uri_parameter(const char *zFilename, const char *zParam) {
  if (zFilename == 0 || zParam == 0)
    return 0;
  zFilename = databaseName(zFilename);
  return uriParameter(zFilename, zParam);
}

const char *sqlite3_uri_key(const char *zFilename, int N) {
  if (zFilename == 0 || N < 0)
    return 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  while ((zFilename) && zFilename[0] && (N--) > 0) {
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  return zFilename[0] ? zFilename : 0;
}

int sqlite3_uri_boolean(const char *zFilename, const char *zParam, int bDflt) {
  const char *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt != 0;
  return z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

sqlite3_int64 sqlite3_uri_int64(const char *zFilename, const char *zParam, sqlite3_int64 bDflt) {
  const char *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  if (z && sqlite3DecOrHexToI64(z, &v) == 0) {
    bDflt = v;
  }
  return bDflt;
}

const char *sqlite3_filename_database(const char *zFilename) {
  if (zFilename == 0)
    return 0;
  return databaseName(zFilename);
}

const char *sqlite3_filename_journal(const char *zFilename) {
  if (zFilename == 0)
    return 0;
  zFilename = databaseName(zFilename);
  zFilename += sqlite3Strlen30(zFilename) + 1;
  while ((zFilename) && zFilename[0]) {
    zFilename += sqlite3Strlen30(zFilename) + 1;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  return zFilename + 1;
}

const char *sqlite3_filename_wal(const char *zFilename) {
  zFilename = sqlite3_filename_journal(zFilename);
  if (zFilename)
    zFilename += sqlite3Strlen30(zFilename) + 1;
  return zFilename;
}

Btree *sqlite3DbNameToBtree(sqlite3 *db, const char *zDbName) {
  int iDb = zDbName ? sqlite3FindDbName(db, zDbName) : 0;
  return iDb < 0 ? 0 : db->aDb[iDb].pBt;
}

const char *sqlite3_db_name(sqlite3 *db, int N) {
  const char *zRet = 0;

  sqlite3_mutex_enter(db->mutex);
  if (N >= 0 && N < db->nDb) {
    zRet = db->aDb[N].zDbSName;
  }
  sqlite3_mutex_leave(db->mutex);
  return zRet;
}

const char *sqlite3_db_filename(sqlite3 *db, const char *zDbName) {
  Btree *pBt;

  pBt = sqlite3DbNameToBtree(db, zDbName);
  return pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

int sqlite3_db_readonly(sqlite3 *db, const char *zDbName) {
  Btree *pBt;

  pBt = sqlite3DbNameToBtree(db, zDbName);
  return pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

int jsonEachConnect(sqlite3 *db, void *pAux, int argc, const char *const *argv, sqlite3_vtab **ppVtab, char **pzErr) {
  JsonEachConnection *pNew;
  int rc;

  (void)(pzErr);
  (void)(argv);
  (void)(argc);
  (void)(pAux);
  rc = sqlite3_declare_vtab(db,
                            "CREATE TABLE x(key,value,type,atom,id,parent,fullkey,path,"
                            "json HIDDEN,root HIDDEN)");
  if (rc == SQLITE_OK) {
    pNew = (JsonEachConnection *)sqlite3DbMallocZero(db, sizeof(*pNew));
    *ppVtab = (sqlite3_vtab *)pNew;
    if (pNew == 0)
      return SQLITE_NOMEM;
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    pNew->db = db;
    pNew->eMode = argv[0][4] == 'b' ? 2 : 1;
    pNew->bRecursive = argv[0][4 + pNew->eMode] == 't';
  }
  return rc;
}

static void sqlite3RegisterJsonFunctions(void) {
  static FuncDef aJsonFunc[] = {

      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonRemoveFunc,
       0,
       0,
       0,
       "json",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonRemoveFunc,
       0,
       0,
       0,
       "jsonb",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((0) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonArrayFunc,
       0,
       0,
       0,
       "json_array",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((0) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonArrayFunc,
       0,
       0,
       0,
       "jsonb_array",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0x08 | ((0) * 0x10))),
       0,
       jsonSetFunc,
       0,
       0,
       0,
       "json_array_insert",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0x08 | ((1) * 0x10))),
       0,
       jsonSetFunc,
       0,
       0,
       0,
       "jsonb_array_insert",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonArrayLengthFunc,
       0,
       0,
       0,
       "json_array_length",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonArrayLengthFunc,
       0,
       0,
       0,
       "json_array_length",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonErrorFunc,
       0,
       0,
       0,
       "json_error_position",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonExtractFunc,
       0,
       0,
       0,
       "json_extract",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonExtractFunc,
       0,
       0,
       0,
       "jsonb_extract",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0x01 | ((0) * 0x10))),
       0,
       jsonExtractFunc,
       0,
       0,
       0,
       "->",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0x02 | ((0) * 0x10))),
       0,
       jsonExtractFunc,
       0,
       0,
       0,
       "->>",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonSetFunc,
       0,
       0,
       0,
       "json_insert",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonSetFunc,
       0,
       0,
       0,
       "jsonb_insert",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((0) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonObjectFunc,
       0,
       0,
       0,
       "json_object",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((0) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonObjectFunc,
       0,
       0,
       0,
       "jsonb_object",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonPatchFunc,
       0,
       0,
       0,
       "json_patch",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonPatchFunc,
       0,
       0,
       0,
       "jsonb_patch",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonPrettyFunc,
       0,
       0,
       0,
       "json_pretty",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonPrettyFunc,
       0,
       0,
       0,
       "json_pretty",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((0) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonQuoteFunc,
       0,
       0,
       0,
       "json_quote",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonRemoveFunc,
       0,
       0,
       0,
       "json_remove",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonRemoveFunc,
       0,
       0,
       0,
       "jsonb_remove",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonReplaceFunc,
       0,
       0,
       0,
       "json_replace",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((1) * 0x10))),
       0,
       jsonReplaceFunc,
       0,
       0,
       0,
       "jsonb_replace",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((1) * 0x001000000),
       ((void *)(intptr_t)(0x04 | ((0) * 0x10))),
       0,
       jsonSetFunc,
       0,
       0,
       0,
       "json_set",
       {0}},
      {-1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((1) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0x04 | ((1) * 0x10))),
       0,
       jsonSetFunc,
       0,
       0,
       0,
       "jsonb_set",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonTypeFunc,
       0,
       0,
       0,
       "json_type",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonTypeFunc,
       0,
       0,
       0,
       "json_type",
       {0}},
      {1,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonValidFunc,
       0,
       0,
       0,
       "json_valid",
       {0}},
      {2,
       0x00800000 | 0x000000800 | 0x0800 | 1 | ((1) * 0x8000) | ((0) * 0x000100000) | ((0) * 0x001000000),
       ((void *)(intptr_t)(0 | ((0) * 0x10))),
       0,
       jsonValidFunc,
       0,
       0,
       0,
       "json_valid",
       {0}},
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0x000100000 | 0x001000000 | 1 | 0x000000800,
       ((void *)(intptr_t)(0)),
       0,
       jsonArrayStep,
       jsonArrayFinal,
       jsonArrayValue,
       jsonGroupInverse,
       "json_group_array",
       {0}}

      ,
      {1,
       0x00800000 | 1 | (0 * 0x0020) | 0x000100000 | 0x001000000 | 1 | 0x000000800,
       ((void *)(intptr_t)(0x10)),
       0,
       jsonArrayStep,
       jsonArrayFinal,
       jsonArrayValue,
       jsonGroupInverse,
       "jsonb_group_array",
       {0}}

      ,
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0x000100000 | 0x001000000 | 1 | 0x000000800,
       ((void *)(intptr_t)(0)),
       0,
       jsonObjectStep,
       jsonObjectFinal,
       jsonObjectValue,
       jsonGroupInverse,
       "json_group_object",
       {0}}

      ,
      {2,
       0x00800000 | 1 | (0 * 0x0020) | 0x000100000 | 0x001000000 | 1 | 0x000000800,
       ((void *)(intptr_t)(0x10)),
       0,
       jsonObjectStep,
       jsonObjectFinal,
       jsonObjectValue,
       jsonGroupInverse,
       "jsonb_group_object",
       {0}}

  };
  sqlite3InsertBuiltinFuncs(aJsonFunc, ((int)(sizeof(aJsonFunc) / sizeof(aJsonFunc[0]))));
}

Module *sqlite3JsonVtabRegister(sqlite3 *db, const char *zName) {
  unsigned int i;
  static const char *azModule[] = {"json_each", "json_tree", "jsonb_each", "jsonb_tree"};

  for (i = 0; i < sizeof(azModule) / sizeof(azModule[0]); i++) {
    if (sqlite3StrICmp(azModule[i], zName) == 0) {
      return sqlite3VtabCreateModule(db, azModule[i], &jsonEachModule, 0, 0);
    }
  }
  return 0;
}

const char *sqlite3_sourceid(void) {
  return "2026-07-24 19:02:57 bf7c7f30031888f4e796e429ab3978879485813aaca6f641c7b33e4e09459bcc";
}
