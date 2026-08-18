
#pragma once

#include "sqlite/Mem.h"
#include "sqlite/compareInfo.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct CollSeq CollSeq;
  typedef struct DateTime DateTime;
  typedef struct FuncDef FuncDef;
  typedef struct JsonParse JsonParse;
  typedef struct Parse Parse;
  typedef struct RenameCtx RenameCtx;
  typedef struct sqlite3_str StrAccum;
  typedef struct Vdbe Vdbe;

  typedef struct sqlite3_context sqlite3_context;
  typedef struct sqlite3 sqlite3;

  struct sqlite3_context {
    Mem *pOut;
    FuncDef *pFunc;
    Mem *pMem;
    Vdbe *pVdbe;
    int iOp;
    int isError;
    u8 enc;
    u8 skipFlag;
    u16 argc;
    sqlite3_value *argv[1];
  };

  int sqlite3_aggregate_count(sqlite3_context *);
  int sqlite3_result_zeroblob64(sqlite3_context *, sqlite3_uint64 n);
  sqlite3 *sqlite3_context_db_handle(sqlite3_context *);
  void *sqlite3_aggregate_context(sqlite3_context *, int nBytes);
  void *sqlite3_get_auxdata(sqlite3_context *, int N);
  void *sqlite3_user_data(sqlite3_context *);
  void sqlite3_result_blob(sqlite3_context *, const void *, int, void (*)(void *));
  void sqlite3_result_blob64(sqlite3_context *, const void *, sqlite3_uint64, void (*)(void *));
  void sqlite3_result_double(sqlite3_context *, double);
  void sqlite3_result_error_code(sqlite3_context *, int);
  void sqlite3_result_error_nomem(sqlite3_context *);
  void sqlite3_result_error_toobig(sqlite3_context *);
  void sqlite3_result_error(sqlite3_context *, const char *, int);
  void sqlite3_result_error16(sqlite3_context *, const void *, int);
  void sqlite3_result_int(sqlite3_context *, int);
  void sqlite3_result_int64(sqlite3_context *, sqlite3_int64);
  void sqlite3_result_null(sqlite3_context *);
  void sqlite3_result_pointer(sqlite3_context *, void *, const char *, void (*)(void *));
  void sqlite3_result_subtype(sqlite3_context *, unsigned int);
  void sqlite3_result_text(sqlite3_context *, const char *, int, void (*)(void *));
  void sqlite3_result_text16(sqlite3_context *, const void *, int, void (*)(void *));
  void sqlite3_result_text16be(sqlite3_context *, const void *, int, void (*)(void *));
  void sqlite3_result_text16le(sqlite3_context *, const void *, int, void (*)(void *));
  void sqlite3_result_text64(sqlite3_context *, const char *z, sqlite3_uint64 n, void (*)(void *),
                             unsigned char encoding);
  void sqlite3_result_value(sqlite3_context *, sqlite3_value *);
  void sqlite3_result_zeroblob(sqlite3_context *, int n);
  void sqlite3_set_auxdata(sqlite3_context *, int N, void *, void (*)(void *));
  int sqlite3_vtab_nochange(sqlite3_context *);
  int sqlite3NotPureFunc(sqlite3_context *);

  const char *sqlite3VdbeFuncName(const sqlite3_context *);
  void sqlite3ResultIntReal(sqlite3_context *);
  void sqlite3ResultStrAccum(sqlite3_context *, StrAccum *);
  sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context *);
  int setDateTimeToCurrent(sqlite3_context * context, DateTime * p);
  int parseDateOrTime(sqlite3_context * context, const char *zDate, DateTime *p);
  int parseModifier(sqlite3_context * pCtx, const char *z, int n, DateTime *p, int idx);
  int isDate(sqlite3_context * context, int argc, sqlite3_value **argv, DateTime *p);
  void juliandayFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void unixepochFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void datetimeFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void timeFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void dateFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void strftimeFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void ctimeFunc(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void cdateFunc(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void timediffFunc(sqlite3_context * context, int NotUsed1, sqlite3_value **argv);
  void ctimestampFunc(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void setResultStrOrError(sqlite3_context * pCtx, const char *z, int n, u8 enc, void (*xDel)(void *));
  __attribute__((noinline)) void *createAggContext(sqlite3_context * p, int nByte);
  void errorMPrintf(sqlite3_context * pCtx, const char *zFmt, ...);
  void renameColumnParseError(sqlite3_context * pCtx, const char *zWhen, sqlite3_value *pType, sqlite3_value *pObject,
                              Parse *pParse);
  int renameEditSql(sqlite3_context * pCtx, RenameCtx * pRename, const char *zSql, const char *zNew, int bQuote);
  void renameColumnFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void renameTableFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void renameQuotefixFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void renameTableTest(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void dropColumnFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  int quotedCompare(sqlite3_context * ctx, int t, const u8 *zQuote, int nQuote, const u8 *zCmp, int *pRes);
  int skipCreateTable(sqlite3_context * ctx, const u8 *zSql, int *piOff);
  void dropConstraintFunc(sqlite3_context * ctx, int NotUsed, sqlite3_value **argv);
  void addConstraintFunc(sqlite3_context * ctx, int NotUsed, sqlite3_value **argv);
  void failConstraintFunc(sqlite3_context * ctx, int NotUsed, sqlite3_value **argv);
  void findConstraintFunc(sqlite3_context * ctx, int NotUsed, sqlite3_value **argv);
  void statInit(sqlite3_context * context, int argc, sqlite3_value **argv);
  void statPush(sqlite3_context * context, int argc, sqlite3_value **argv);
  void statGet(sqlite3_context * context, int argc, sqlite3_value **argv);
  void attachFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void detachFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  CollSeq *sqlite3GetFuncCollSeq(sqlite3_context * context);
  void sqlite3SkipAccumulatorLoad(sqlite3_context * context);
  void minmaxFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void typeofFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void subtypeFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void lengthFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void bytelengthFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void absFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void instrFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void printfFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void substrFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void roundFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void *contextMalloc(sqlite3_context * context, i64 nByte);
  void upperFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void lowerFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void randomFunc(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void randomBlob(sqlite3_context * context, int argc, sqlite3_value **argv);
  void last_insert_rowid(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void changes(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void total_changes(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void likeFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void nullifFunc(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void versionFunc(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void sourceidFunc(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void errlogFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void compileoptionusedFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void compileoptiongetFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void unistrFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void quoteFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void unicodeFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void charFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void hexFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void unhexFunc(sqlite3_context * pCtx, int argc, sqlite3_value **argv);
  void zeroblobFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void replaceFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void trimFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void concatFuncCore(sqlite3_context * context, int argc, sqlite3_value **argv, int nSep, const char *zSep);
  void concatFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void concatwsFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void loadExt(sqlite3_context * context, int argc, sqlite3_value **argv);
  void sumStep(sqlite3_context * context, int argc, sqlite3_value **argv);
  void sumInverse(sqlite3_context * context, int argc, sqlite3_value **argv);
  void sumFinalize(sqlite3_context * context);
  void avgFinalize(sqlite3_context * context);
  void totalFinalize(sqlite3_context * context);
  void countStep(sqlite3_context * context, int argc, sqlite3_value **argv);
  void countFinalize(sqlite3_context * context);
  void countInverse(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void minmaxStep(sqlite3_context * context, int NotUsed, sqlite3_value **argv);
  void minMaxValueFinalize(sqlite3_context * context, int bValue);
  void minMaxValue(sqlite3_context * context);
  void minMaxFinalize(sqlite3_context * context);
  void groupConcatStep(sqlite3_context * context, int argc, sqlite3_value **argv);
  void groupConcatInverse(sqlite3_context * context, int argc, sqlite3_value **argv);
  void groupConcatFinalize(sqlite3_context * context);
  void groupConcatValue(sqlite3_context * context);
  void ceilingFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void logFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void math1Func(sqlite3_context * context, int argc, sqlite3_value **argv);
  void math2Func(sqlite3_context * context, int argc, sqlite3_value **argv);
  void piFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void signFunc(sqlite3_context * context, int argc, sqlite3_value **argv);
  void percentError(sqlite3_context * pCtx, const char *zFormat, ...);
  void percentStep(sqlite3_context * pCtx, int argc, sqlite3_value **argv);
  void percentInverse(sqlite3_context * pCtx, int argc, sqlite3_value **argv);
  void percentCompute(sqlite3_context * pCtx, int bIsFinal);
  void percentFinal(sqlite3_context * pCtx);
  void percentValue(sqlite3_context * pCtx);
  void row_numberStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void row_numberValueFunc(sqlite3_context * pCtx);
  void dense_rankStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void dense_rankValueFunc(sqlite3_context * pCtx);
  void nth_valueStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void nth_valueFinalizeFunc(sqlite3_context * pCtx);
  void first_valueStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void first_valueFinalizeFunc(sqlite3_context * pCtx);
  void rankStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void rankValueFunc(sqlite3_context * pCtx);
  void percent_rankStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void percent_rankInvFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void percent_rankValueFunc(sqlite3_context * pCtx);
  void cume_distStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void cume_distInvFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void cume_distValueFunc(sqlite3_context * pCtx);
  void ntileStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void ntileInvFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void ntileValueFunc(sqlite3_context * pCtx);
  void last_valueStepFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void last_valueInvFunc(sqlite3_context * pCtx, int nArg, sqlite3_value **apArg);
  void last_valueValueFunc(sqlite3_context * pCtx);
  void last_valueFinalizeFunc(sqlite3_context * pCtx);
  void noopStepFunc(sqlite3_context * p, int n, sqlite3_value **a);
  void noopValueFunc(sqlite3_context * p);
  void sqlite3InvalidFunction(sqlite3_context * context, int NotUsed, sqlite3_value **NotUsed2);
  void jsonReturnParse(sqlite3_context *, JsonParse *);
  JsonParse *jsonParseFuncArg(sqlite3_context *, sqlite3_value *, u32);
  int jsonCacheInsert(sqlite3_context * ctx, JsonParse * pParse);
  JsonParse *jsonCacheSearch(sqlite3_context * ctx, sqlite3_value * pArg);
  void jsonWrongNumArgs(sqlite3_context * pCtx, const char *zFuncName);
  void jsonReturnTextJsonFromBlob(sqlite3_context * ctx, const u8 *aBlob, u32 nBlob);
  int jsonFunctionArgToBlob(sqlite3_context * ctx, sqlite3_value * pArg, JsonParse * pParse);
  char *jsonBadPathError(sqlite3_context * ctx, const char *zPath, int rc);
  void jsonInsertIntoBlob(sqlite3_context * ctx, int argc, sqlite3_value **argv, int eEdit);
  void jsonQuoteFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonArrayFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonArrayLengthFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonExtractFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonPatchFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonObjectFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonRemoveFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonReplaceFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonSetFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonTypeFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonPrettyFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonValidFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonErrorFunc(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonArrayStep(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonArrayCompute(sqlite3_context * ctx, int isFinal);
  void jsonArrayValue(sqlite3_context * ctx);
  void jsonArrayFinal(sqlite3_context * ctx);
  void jsonGroupInverse(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonObjectStep(sqlite3_context * ctx, int argc, sqlite3_value **argv);
  void jsonObjectCompute(sqlite3_context * ctx, int isFinal);
  void jsonObjectValue(sqlite3_context * ctx);
  void jsonObjectFinal(sqlite3_context * ctx);

  int patternCompare(const u8 *zPattern, const u8 *zString, const struct compareInfo *pInfo, u32 matchOther);


