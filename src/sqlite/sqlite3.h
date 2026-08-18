#pragma once

#include "sqlite/Db.h"
#include "sqlite/Hash.h"
#include "sqlite/Lookaside.h"
#include "sqlite/Pgno.h"
#include "sqlite/VList.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3InitInfo.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite3_xauth.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/BusyHandler.h"
#include "sqlite/LogEst.h"
#include "sqlite/Token.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3DigitPairs_t.h"
#include "sqlite/sqlite3PrngType.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_uint64.h"
#include <stdarg.h>
#include "sqlite/SqliteVersion.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteExtendedResultCode.h"
#include "sqlite/SqliteOpenFlags.h"
#include "sqlite/SqliteIoCap.h"
#include "sqlite/SqliteLockLevel.h"
#include "sqlite/SqliteSyncFlags.h"
#include "sqlite/SqliteFileControlOpcode.h"
#include "sqlite/SqliteAccessFlags.h"
#include "sqlite/SqliteShmFlags.h"
#include "sqlite/SqliteConfigOption.h"
#include "sqlite/SqliteDbConfigOption.h"
#include "sqlite/SqliteSetlkTimeoutFlags.h"
#include "sqlite/SqliteAuthorizerReturnCode.h"
#include "sqlite/SqliteAuthorizerActionCode.h"
#include "sqlite/SqliteTraceEventCode.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqlitePrepareFlags.h"
#include "sqlite/SqliteFundamentalDatatype.h"
#include "sqlite/SqliteTextEncoding.h"
#include "sqlite/SqliteFunctionFlags.h"
#include "sqlite/SqliteDestructorType.h"
#include "sqlite/SqliteWin32DirectoryType.h"
#include "sqlite/SqliteTxnState.h"
#include "sqlite/SqliteIndexScanFlags.h"
#include "sqlite/SqliteIndexConstraintOp.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteTestCtrlOpcode.h"
#include "sqlite/SqliteStatusParameter.h"
#include "sqlite/SqliteDbStatusParameter.h"
#include "sqlite/SqliteStmtStatusParameter.h"
#include "sqlite/SqliteCheckpointMode.h"
#include "sqlite/SqliteVtabConfigOption.h"
#include "sqlite/SqliteConflictResolution.h"
#include "sqlite/SqliteScanStatusOpcode.h"
#include "sqlite/SqliteScanStatusFlags.h"
#include "sqlite/SqliteSerializeFlags.h"
#include "sqlite/SqliteDeserializeFlags.h"
#include "sqlite/sqlite3_compileoption_used.h"
#include "sqlite/sqlite3_compileoption_get.h"
#include "sqlite/sqlite3_threadsafe.h"
#include "sqlite/sqlite3_complete.h"
#include "sqlite/sqlite3_complete16.h"
#include "sqlite/sqlite3_free_table.h"
#include "sqlite/sqlite3_mprintf.h"
#include "sqlite/sqlite3_vmprintf.h"
#include "sqlite/sqlite3_snprintf.h"
#include "sqlite/sqlite3_vsnprintf.h"
#include "sqlite/sqlite3_malloc.h"
#include "sqlite/sqlite3_malloc64.h"
#include "sqlite/sqlite3_realloc.h"
#include "sqlite/sqlite3_realloc64.h"
#include "sqlite/sqlite3_free.h"
#include "sqlite/sqlite3_msize.h"
#include "sqlite/sqlite3_memory_used.h"
#include "sqlite/sqlite3_memory_highwater.h"
#include "sqlite/sqlite3_randomness.h"

struct AggInfo;
struct AuxData;
struct BtCursor;
struct CollSeq;
struct Column;
struct Cte;
struct DbClientData;
struct EdupBuf;
struct Expr;
struct ExprList;
struct ExprList_item;
struct IdList;
struct Index;
struct KeyInfo;
typedef struct sqlite3_value Mem;

struct MemFile;
struct Module;
struct NameContext;
struct OnOrUsing;
typedef struct VdbeOp Op;
struct PCache;
struct Parse;
struct PgHdr;
struct RenameToken;
struct RowSet;
struct Savepoint;
struct Select;
struct SortSubtask;
struct SorterRecord;
struct SrcItem;
struct SrcList;
struct Subquery;
struct Table;
struct Trigger;
struct TriggerStep;
struct UnpackedRecord;
struct Upsert;
struct VTable;
struct ValueNewStat4Ctx;
struct Vdbe;
struct VdbeCursor;
struct VdbeSorter;
struct VtabCtx;
struct WhereAndInfo;
struct WhereInfo;
struct WhereLoop;
struct WhereOrInfo;
struct Window;
struct With;
struct sqlite3_backup;
struct sqlite3_blob;
struct sqlite3_index_info;
struct sqlite3_module;
struct sqlite3_mutex;
struct sqlite3_mutex_methods;
struct sqlite3_stmt;
struct sqlite3_str;
struct sqlite3_vfs;
struct sqlite3_vtab;


struct sqlite3 {
  sqlite3_vfs *pVfs;
  struct Vdbe *pVdbe;
  CollSeq *pDfltColl;
  sqlite3_mutex *mutex;
  Db *aDb;
  int nDb;
  u32 mDbFlags;
  u64 flags;
  i64 lastRowid;
  i64 szMmap;
  u32 nSchemaLock;
  unsigned int openFlags;
  int errCode;
  int errByteOffset;
  int errMask;
  int iSysErrno;
  u32 dbOptFlags;
  u8 enc;
  u8 autoCommit;
  u8 temp_store;
  u8 mallocFailed;
  u8 bBenignMalloc;
  u8 dfltLockMode;
  signed char nextAutovac;
  u8 suppressErr;
  u8 vtabOnConflict;
  u8 isTransactionSavepoint;
  u8 mTrace;
  u8 noSharedCache;
  u8 nSqlExec;
  u8 eOpenState;
  u8 nFpDigit;
  int nextPagesize;
  i64 nChange;
  i64 nTotalChange;
  int aLimit[(12 + 1)];
  int nMaxSorterMmap;
  sqlite3InitInfo init;
  int nVdbeActive;
  int nVdbeRead;
  int nVdbeWrite;
  int nVdbeExec;
  int nVDestroy;
  int nExtension;
  void **aExtension;
  union {
    void (*xLegacy)(void *, const char *);
    int (*xV2)(u32, void *, void *, void *);
  } trace;
  void *pTraceArg;

  void (*xProfile)(void *, const char *, u64);
  void *pProfileArg;

  void *pCommitArg;
  int (*xCommitCallback)(void *);
  void *pRollbackArg;
  void (*xRollbackCallback)(void *);
  void *pUpdateArg;
  void (*xUpdateCallback)(void *, int, const char *, const char *, sqlite_int64);
  void *pAutovacPagesArg;
  void (*xAutovacDestr)(void *);
  unsigned int (*xAutovacPages)(void *, const char *, u32, u32, u32);
  Parse *pParse;
  int (*xWalCallback)(void *, sqlite3 *, const char *, int);
  void *pWalArg;

  void (*xCollNeeded)(void *, sqlite3 *, int eTextRep, const char *);
  void (*xCollNeeded16)(void *, sqlite3 *, int eTextRep, const void *);
  void *pCollNeededArg;
  sqlite3_value *pErr;
  union {
    volatile int isInterrupted;
    double notUsed1;
  } u1;
  Lookaside lookaside;

  sqlite3_xauth xAuth;
  void *pAuthArg;

  int (*xProgress)(void *);
  void *pProgressArg;
  unsigned nProgressOps;

  int nVTrans;
  Hash aModule;
  VtabCtx *pVtabCtx;
  VTable **aVTrans;
  VTable *pDisconnect;

  Hash aFunc;
  Hash aCollSeq;

  BusyHandler busyHandler;
  Db aDbStatic[2];
  Savepoint *pSavepoint;
  int nAnalysisLimit;
  int busyTimeout;

  int nSavepoint;
  int nStatement;
  i64 nDeferredCons;
  i64 nDeferredImmCons;
  int *pnBytesFreed;
  DbClientData *pDbData;
  u64 nSpill;
};

int sqlite3_initialize(void);
int sqlite3_shutdown(void);
int sqlite3_os_init(void);
int sqlite3_os_end(void);
int sqlite3_config(int, ...);

int sqlite3_close(sqlite3 *);
int sqlite3_close_v2(sqlite3 *);
int sqlite3_exec(sqlite3 *, const char *sql, int (*callback)(void *, int, char **, char **), void *, char **errmsg);
int sqlite3_db_config(sqlite3 *, int op, ...);
int sqlite3_extended_result_codes(sqlite3 *, int onoff);
sqlite3_int64 sqlite3_last_insert_rowid(sqlite3 *);
void sqlite3_set_last_insert_rowid(sqlite3 *, sqlite3_int64);
int sqlite3_changes(sqlite3 *);
sqlite3_int64 sqlite3_changes64(sqlite3 *);
int sqlite3_total_changes(sqlite3 *);
sqlite3_int64 sqlite3_total_changes64(sqlite3 *);
void sqlite3_interrupt(sqlite3 *);
int sqlite3_is_interrupted(sqlite3 *);
int sqlite3_busy_handler(sqlite3 *, int (*)(void *, int), void *);
int sqlite3_busy_timeout(sqlite3 *, int ms);
int sqlite3_setlk_timeout(sqlite3 *, int ms, int flags);
int sqlite3_get_table(sqlite3 *db, const char *zSql, char ***pazResult, int *pnRow, int *pnColumn, char **pzErrmsg);
int sqlite3_set_authorizer(sqlite3 *, int (*xAuth)(void *, int, const char *, const char *, const char *, const char *),
                           void *pUserData);
void *sqlite3_trace(sqlite3 *, void (*xTrace)(void *, const char *), void *);
void *sqlite3_profile(sqlite3 *, void (*xProfile)(void *, const char *, sqlite3_uint64), void *);
int sqlite3_trace_v2(sqlite3 *, unsigned uMask, int (*xCallback)(unsigned, void *, void *, void *), void *pCtx);
void sqlite3_progress_handler(sqlite3 *, int, int (*)(void *), void *);
int sqlite3_errcode(sqlite3 *db);
int sqlite3_extended_errcode(sqlite3 *db);
const char *sqlite3_errmsg(sqlite3 *);
const void *sqlite3_errmsg16(sqlite3 *);
int sqlite3_error_offset(sqlite3 *db);
int sqlite3_set_errmsg(sqlite3 *db, int errcode, const char *zErrMsg);
int sqlite3_limit(sqlite3 *, int id, int newVal);
int sqlite3_prepare(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
int sqlite3_prepare_v3(sqlite3 *db, const char *zSql, int nByte, unsigned int prepFlags, sqlite3_stmt **ppStmt,
                       const char **pzTail);
int sqlite3_prepare16(sqlite3 *db, const void *zSql, int nByte, sqlite3_stmt **ppStmt, const void **pzTail);
int sqlite3_prepare16_v2(sqlite3 *db, const void *zSql, int nByte, sqlite3_stmt **ppStmt, const void **pzTail);
int sqlite3_prepare16_v3(sqlite3 *db, const void *zSql, int nByte, unsigned int prepFlags, sqlite3_stmt **ppStmt,
                         const void **pzTail);
int sqlite3_create_function(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp,
                            void (*xFunc)(sqlite3_context *, int, sqlite3_value **),
                            void (*xStep)(sqlite3_context *, int, sqlite3_value **), void (*xFinal)(sqlite3_context *));
int sqlite3_create_function16(sqlite3 *db, const void *zFunctionName, int nArg, int eTextRep, void *pApp,
                              void (*xFunc)(sqlite3_context *, int, sqlite3_value **),
                              void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                              void (*xFinal)(sqlite3_context *));
int sqlite3_create_function_v2(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp,
                               void (*xFunc)(sqlite3_context *, int, sqlite3_value **),
                               void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                               void (*xFinal)(sqlite3_context *), void (*xDestroy)(void *));
int sqlite3_create_window_function(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp,
                                   void (*xStep)(sqlite3_context *, int, sqlite3_value **),
                                   void (*xFinal)(sqlite3_context *), void (*xValue)(sqlite3_context *),
                                   void (*xInverse)(sqlite3_context *, int, sqlite3_value **),
                                   void (*xDestroy)(void *));
void *sqlite3_get_clientdata(sqlite3 *, const char *);
int sqlite3_set_clientdata(sqlite3 *, const char *, void *, void (*)(void *));
int sqlite3_create_collation(sqlite3 *, const char *zName, int eTextRep, void *pArg,
                             int (*xCompare)(void *, int, const void *, int, const void *));
int sqlite3_create_collation_v2(sqlite3 *, const char *zName, int eTextRep, void *pArg,
                                int (*xCompare)(void *, int, const void *, int, const void *),
                                void (*xDestroy)(void *));
int sqlite3_create_collation16(sqlite3 *, const void *zName, int eTextRep, void *pArg,
                               int (*xCompare)(void *, int, const void *, int, const void *));
int sqlite3_collation_needed(sqlite3 *, void *, void (*)(void *, sqlite3 *, int eTextRep, const char *));
int sqlite3_collation_needed16(sqlite3 *, void *, void (*)(void *, sqlite3 *, int eTextRep, const void *));
int sqlite3_get_autocommit(sqlite3 *);
const char *sqlite3_db_name(sqlite3 *db, int N);
sqlite3_filename sqlite3_db_filename(sqlite3 *db, const char *zDbName);
int sqlite3_db_readonly(sqlite3 *db, const char *zDbName);
int sqlite3_txn_state(sqlite3 *, const char *zSchema);
sqlite3_stmt *sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt);
void *sqlite3_commit_hook(sqlite3 *, int (*)(void *), void *);
void *sqlite3_rollback_hook(sqlite3 *, void (*)(void *), void *);
int sqlite3_autovacuum_pages(sqlite3 *db,
                             unsigned int (*)(void *, const char *, unsigned int, unsigned int, unsigned int), void *,
                             void (*)(void *));
void *sqlite3_update_hook(sqlite3 *, void (*)(void *, int, char const *, char const *, sqlite3_int64), void *);
int sqlite3_db_release_memory(sqlite3 *);
int sqlite3_table_column_metadata(sqlite3 *db, const char *zDbName, const char *zTableName, const char *zColumnName,
                                  char const **pzDataType, char const **pzCollSeq, int *pNotNull, int *pPrimaryKey,
                                  int *pAutoinc);
int sqlite3_load_extension(sqlite3 *db, const char *zFile, const char *zProc, char **pzErrMsg);
int sqlite3_enable_load_extension(sqlite3 *db, int onoff);
int sqlite3_create_module(sqlite3 *db, const char *zName, const sqlite3_module *p, void *pClientData);
int sqlite3_create_module_v2(sqlite3 *db, const char *zName, const sqlite3_module *p, void *pClientData,
                             void (*xDestroy)(void *));
int sqlite3_drop_modules(sqlite3 *db, const char **azKeep);
int sqlite3_declare_vtab(sqlite3 *, const char *zSQL);
int sqlite3_overload_function(sqlite3 *, const char *zFuncName, int nArg);
int sqlite3_blob_open(sqlite3 *, const char *zDb, const char *zTable, const char *zColumn, sqlite3_int64 iRow,
                      int flags, sqlite3_blob **ppBlob);
sqlite3_mutex *sqlite3_db_mutex(sqlite3 *);
int sqlite3_file_control(sqlite3 *, const char *zDbName, int op, void *);
sqlite3_str *sqlite3_str_new(sqlite3 *);
int sqlite3_db_status(sqlite3 *, int op, int *pCur, int *pHiwtr, int resetFlg);
int sqlite3_db_status64(sqlite3 *, int, sqlite3_int64 *, sqlite3_int64 *, int);
sqlite3_backup *sqlite3_backup_init(sqlite3 *pDest, const char *zDestName, sqlite3 *pSource, const char *zSourceName);
void *sqlite3_wal_hook(sqlite3 *, int (*)(void *, sqlite3 *, const char *, int), void *);
int sqlite3_wal_autocheckpoint(sqlite3 *db, int N);
int sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb);
int sqlite3_wal_checkpoint_v2(sqlite3 *db, const char *zDb, int eMode, int *pnLog, int *pnCkpt);
int sqlite3_vtab_config(sqlite3 *, int op, ...);
int sqlite3_vtab_on_conflict(sqlite3 *);
int sqlite3_db_cacheflush(sqlite3 *);
int sqlite3_system_errno(sqlite3 *);
unsigned char *sqlite3_serialize(sqlite3 *db, const char *zSchema, sqlite3_int64 *piSize, unsigned int mFlags);
int sqlite3_deserialize(sqlite3 *db, const char *zSchema, unsigned char *pData, sqlite3_int64 szDb, sqlite3_int64 szBuf,
                        unsigned mFlags);
int sqlite3BtreeIntegrityCheck(sqlite3 *db, Btree *p, Pgno *aRoot, sqlite3_value *aCnt, int nRoot, int mxErr,
                               int *pnErr, char **pzOut);
int sqlite3BtreeCount(sqlite3 *, BtCursor *, i64 *);
void sqlite3BtreeEnterAll(sqlite3 *);
void sqlite3BtreeLeaveAll(sqlite3 *);
void sqlite3WindowDelete(sqlite3 *, Window *);
void sqlite3WindowListDelete(sqlite3 *db, Window *p);
Window *sqlite3WindowDup(sqlite3 *db, Expr *pOwner, Window *p);
Window *sqlite3WindowListDup(sqlite3 *db, Window *p);
void *sqlite3DbMallocZero(sqlite3 *, u64);
void *sqlite3DbMallocRaw(sqlite3 *, u64);
void *sqlite3DbMallocRawNN(sqlite3 *, u64);
char *sqlite3DbStrDup(sqlite3 *, const char *);
char *sqlite3DbStrNDup(sqlite3 *, const char *, u64);
char *sqlite3DbSpanDup(sqlite3 *, const char *, const char *);
void *sqlite3DbReallocOrFree(sqlite3 *, void *, u64);
void *sqlite3DbRealloc(sqlite3 *, void *, u64);
void sqlite3DbFree(sqlite3 *, void *);
void sqlite3DbFreeNN(sqlite3 *, void *);
void sqlite3DbNNFreeNN(sqlite3 *, void *);
int sqlite3DbMallocSize(sqlite3 *, const void *);
int sqlite3LookasideUsed(sqlite3 *, int *);
char *sqlite3MPrintf(sqlite3 *, const char *, ...);
char *sqlite3VMPrintf(sqlite3 *, const char *, va_list);
int sqlite3ErrorToParser(sqlite3 *, int);
Expr *sqlite3ExprAlloc(sqlite3 *, int, const Token *, int);
Expr *sqlite3Expr(sqlite3 *, int, const char *);
Expr *sqlite3ExprInt32(sqlite3 *, int);
void sqlite3ExprAttachSubtrees(sqlite3 *, Expr *, Expr *, Expr *);
void sqlite3ExprDelete(sqlite3 *, Expr *);
void sqlite3ExprDeleteGeneric(sqlite3 *, void *);
void sqlite3ExprListDelete(sqlite3 *, ExprList *);
void sqlite3ExprListDeleteGeneric(sqlite3 *, void *);
int sqlite3Init(sqlite3 *, char **);
int sqlite3InitOne(sqlite3 *, int, char **, u32);
Module *sqlite3PragmaVtabRegister(sqlite3 *, const char *zName);
void sqlite3ResetAllSchemasOfConnection(sqlite3 *);
void sqlite3ResetOneSchema(sqlite3 *, int);
void sqlite3CollapseDatabaseArray(sqlite3 *);
void sqlite3CommitInternalChanges(sqlite3 *);
void sqlite3ColumnSetColl(sqlite3 *, Column *, const char *zColl);
void sqlite3DeleteColumnNames(sqlite3 *, Table *);
Btree *sqlite3DbNameToBtree(sqlite3 *, const char *);
RowSet *sqlite3RowSetInit(sqlite3 *);
void sqlite3DeleteTable(sqlite3 *, Table *);
void sqlite3DeleteTableGeneric(sqlite3 *, void *);
void sqlite3FreeIndex(sqlite3 *, Index *);
void *sqlite3ArrayAllocate(sqlite3 *, void *, int, int *, int *);
void sqlite3SubqueryDelete(sqlite3 *, Subquery *);
Select *sqlite3SubqueryDetach(sqlite3 *, SrcItem *);
void sqlite3IdListDelete(sqlite3 *, IdList *);
void sqlite3ClearOnOrUsing(sqlite3 *, OnOrUsing *);
void sqlite3SrcListDelete(sqlite3 *, SrcList *);
Index *sqlite3AllocateIndexObject(sqlite3 *, int, int, char **);
void sqlite3SelectDelete(sqlite3 *, Select *);
void sqlite3SelectDeleteGeneric(sqlite3 *, void *);
Table *sqlite3FindTable(sqlite3 *, const char *, const char *);
Index *sqlite3FindIndex(sqlite3 *, const char *, const char *);
void sqlite3UnlinkAndDeleteTable(sqlite3 *, int, const char *);
void sqlite3UnlinkAndDeleteIndex(sqlite3 *, int, const char *);
char *sqlite3NameFromToken(sqlite3 *, const Token *);
void sqlite3RollbackAll(sqlite3 *, int);
void sqlite3CloseSavepoints(sqlite3 *);
void sqlite3LeaveMutexAndCloseZombie(sqlite3 *);
Expr *sqlite3ExprDup(sqlite3 *, const Expr *, int);
ExprList *sqlite3ExprListDup(sqlite3 *, const ExprList *, int);
SrcList *sqlite3SrcListDup(sqlite3 *, const SrcList *, int);
IdList *sqlite3IdListDup(sqlite3 *, const IdList *);
Select *sqlite3SelectDup(sqlite3 *, const Select *, int);
FuncDef *sqlite3FindFunction(sqlite3 *, const char *, int, u8, u8);
void sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *);
Module *sqlite3JsonVtabRegister(sqlite3 *, const char *);
int sqlite3SafetyCheckOk(sqlite3 *);
int sqlite3SafetyCheckSickOrOk(sqlite3 *);
With *sqlite3WithDup(sqlite3 *db, With *p);
void sqlite3DeleteTriggerStep(sqlite3 *, TriggerStep *);
TriggerStep *sqlite3TriggerSelectStep(sqlite3 *, Select *, const char *, const char *);
void sqlite3DeleteTrigger(sqlite3 *, Trigger *);
void sqlite3UnlinkAndDeleteTrigger(sqlite3 *, int, const char *);
int sqlite3DbIsNamed(sqlite3 *db, int iDb, const char *zName);
VList *sqlite3VListAdd(sqlite3 *, VList *, const char *, int, int);
const char *sqlite3IndexAffinityStr(sqlite3 *, Index *);
char *sqlite3TableAffinityStr(sqlite3 *, const Table *);
void sqlite3ErrorWithMsg(sqlite3 *, int, const char *, ...);
void sqlite3Error(sqlite3 *, int);
void sqlite3ErrorClear(sqlite3 *);
void sqlite3SystemError(sqlite3 *, int);
void *sqlite3HexToBlob(sqlite3 *, const char *z, int n);
CollSeq *sqlite3FindCollSeq(sqlite3 *, u8 enc, const char *, int);
void sqlite3SetTextEncoding(sqlite3 *db, u8);
int sqlite3WritableSchema(sqlite3 *);
void sqlite3VdbeSetChanges(sqlite3 *, i64);
sqlite3_value *sqlite3ValueNew(sqlite3 *);
char *sqlite3Utf16to8(sqlite3 *, const void *, int, u8);
int sqlite3ValueFromExpr(sqlite3 *, const Expr *, u8, u8, sqlite3_value **);
void sqlite3RootPageMoved(sqlite3 *, int, Pgno, Pgno);
void sqlite3ExpirePreparedStatements(sqlite3 *, int);
int sqlite3FindDb(sqlite3 *, Token *);
int sqlite3FindDbName(sqlite3 *, const char *);
int sqlite3AnalysisLoad(sqlite3 *, int iDB);
void sqlite3DeleteIndexSamples(sqlite3 *, Index *);
void sqlite3RegisterLikeFunctions(sqlite3 *, int);
int sqlite3IsLikeFunction(sqlite3 *, Expr *, int *, char *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
int sqlite3SchemaToIndex(sqlite3 *db, Schema *);
KeyInfo *sqlite3KeyInfoAlloc(sqlite3 *, int, int);
int sqlite3CreateFunc(sqlite3 *, const char *, int, int, void *, void (*)(sqlite3_context *, int, sqlite3_value **),
                      void (*)(sqlite3_context *, int, sqlite3_value **), void (*)(sqlite3_context *),
                      void (*)(sqlite3_context *), void (*)(sqlite3_context *, int, sqlite3_value **),
                      FuncDestructor *pDestructor);
void *sqlite3OomFault(sqlite3 *);
void sqlite3OomClear(sqlite3 *);
int sqlite3ApiExit(sqlite3 *db, int);
Expr *sqlite3CreateColumnExpr(sqlite3 *, SrcList *, int, int);
void sqlite3RecordErrorByteOffset(sqlite3 *, const char *);
void sqlite3RecordErrorOffsetOfExpr(sqlite3 *, const Expr *);
void sqlite3AutoLoadExtensions(sqlite3 *);
void sqlite3CloseExtensions(sqlite3 *);
void sqlite3VtabClear(sqlite3 *db, Table *);
void sqlite3VtabDisconnect(sqlite3 *db, Table *p);
int sqlite3VtabSync(sqlite3 *db, Vdbe *);
int sqlite3VtabRollback(sqlite3 *db);
int sqlite3VtabCommit(sqlite3 *db);
void sqlite3VtabModuleUnref(sqlite3 *, Module *);
void sqlite3VtabUnlockList(sqlite3 *);
int sqlite3VtabSavepoint(sqlite3 *, int, int);
VTable *sqlite3GetVTable(sqlite3 *, Table *);
Module *sqlite3VtabCreateModule(sqlite3 *, const char *, const sqlite3_module *, void *, void (*)(void *));
int sqlite3ReadOnlyShadowTables(sqlite3 *db);
int sqlite3ShadowTableName(sqlite3 *db, const char *zName);
int sqlite3IsShadowTableOf(sqlite3 *, Table *, const char *);
void sqlite3MarkAllShadowTablesOf(sqlite3 *, Table *);
void sqlite3VtabEponymousTableClear(sqlite3 *, Module *);
int sqlite3VtabCallCreate(sqlite3 *, int, const char *, char **);
int sqlite3VtabCallDestroy(sqlite3 *, int, const char *);
int sqlite3VtabBegin(sqlite3 *, VTable *);
FuncDef *sqlite3VtabOverloadFunction(sqlite3 *, FuncDef *, int nArg, Expr *);
int sqlite3Checkpoint(sqlite3 *, int, int, int *, int *);
void sqlite3CteDelete(sqlite3 *, Cte *);
void sqlite3WithDelete(sqlite3 *, With *);
void sqlite3WithDeleteGeneric(sqlite3 *, void *);
Upsert *sqlite3UpsertNew(sqlite3 *, ExprList *, Expr *, ExprList *, Expr *, Upsert *);
void sqlite3UpsertDelete(sqlite3 *, Upsert *);
Upsert *sqlite3UpsertDup(sqlite3 *, Upsert *);
void sqlite3FkClearTriggerCache(sqlite3 *, int);
void sqlite3FkDelete(sqlite3 *, Table *);
void sqlite3VdbeDeleteAuxData(sqlite3 *, AuxData **, int, int);
int sqlite3VdbeIdxKeyCompare(sqlite3 *, VdbeCursor *, UnpackedRecord *, int *);
int sqlite3VdbeIdxRowid(sqlite3 *, BtCursor *, i64 *);
char *sqlite3VdbeDisplayP4(sqlite3 *, Op *);
int sqlite3VdbeSorterInit(sqlite3 *, int, VdbeCursor *);
void sqlite3VdbeSorterReset(sqlite3 *, VdbeSorter *);
void sqlite3VdbeSorterClose(sqlite3 *, VdbeCursor *);
int sqlite3VdbeSorterNext(sqlite3 *, const VdbeCursor *);
int isLookaside(sqlite3 *db, const void *p);
int lookasideMallocSize(sqlite3 *db, const void *p);
__attribute__((noinline)) void measureAllocationSize(sqlite3 *db, void *p);
__attribute__((noinline)) void *dbMallocRawFinish(sqlite3 *db, u64 n);
__attribute__((noinline)) void *dbReallocFinish(sqlite3 *db, void *p, u64 n);
__attribute__((noinline)) int apiHandleError(sqlite3 *db, int rc);
__attribute__((noinline)) void sqlite3ErrorFinish(sqlite3 *db, int err_code);
MemFile *memdbFromDbSchema(sqlite3 *db, const char *zSchema);
void __attribute__((noinline)) btreeEnterAll(sqlite3 *db);
void __attribute__((noinline)) btreeLeaveAll(sqlite3 *db);
Btree *findBtree(sqlite3 *pErrorDb, sqlite3 *pDb, const char *zDb);
int checkReadTransaction(sqlite3 *db, Btree *p);
sqlite3_value *valueNew(sqlite3 *db, struct ValueNewStat4Ctx *p);
int valueFromExpr(sqlite3 *db, const Expr *pExpr, u8 enc, u8 affinity, sqlite3_value **ppVal,
                  struct ValueNewStat4Ctx *pCtx);
void freeEphemeralFunction(sqlite3 *db, FuncDef *pDef);
void vdbeFreeOpArray(sqlite3 *, Op *, int);
__attribute__((noinline)) void freeP4Mem(sqlite3 *db, Mem *p);
__attribute__((noinline)) void freeP4FuncCtx(sqlite3 *db, sqlite3_context *p);
void freeP4(sqlite3 *db, int p4type, void *p4);
int vdbeCommit(sqlite3 *db, Vdbe *p);
void sqlite3VdbeClearObject(sqlite3 *db, Vdbe *p);
__attribute__((noinline)) void invokeProfileCallback(sqlite3 *db, Vdbe *p);
int doWalCallbacks(sqlite3 *db);
void vdbeSorterRecordFree(sqlite3 *db, SorterRecord *pRecord);
void vdbeSortSubtaskCleanup(sqlite3 *db, SortSubtask *pTask);
void vdbeSorterExtendFile(sqlite3 *db, sqlite3_file *pFd, i64 nByte);
int vdbeSorterOpenTempFile(sqlite3 *db, i64 nExtend, sqlite3_file **ppFd);
int areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *pTopNC);
__attribute__((noinline)) void sqlite3ExprDeleteNN(sqlite3 *db, Expr *p);
Expr *exprDup(sqlite3 *db, const Expr *p, int dupFlags, EdupBuf *pEdupBuf);
__attribute__((noinline)) ExprList *sqlite3ExprListAppendNew(sqlite3 *db, Expr *pExpr);
__attribute__((noinline)) ExprList *sqlite3ExprListAppendGrow(sqlite3 *db, ExprList *pList, Expr *pExpr);
__attribute__((noinline)) void exprListDeleteNN(sqlite3 *db, ExprList *pList);
int sqlite3ExprIsIIF(sqlite3 *db, const Expr *pExpr);
int addAggInfoColumn(sqlite3 *db, AggInfo *pInfo);
int addAggInfoFunc(sqlite3 *db, AggInfo *pInfo);
void renameTokenFree(sqlite3 *db, RenameToken *pToken);
int alterRtrimConstraint(sqlite3 *db, const char *pCons, int nCons);
void __attribute__((noinline)) deleteTable(sqlite3 *db, Table *pTable);
void sqlite3DeleteReturning(sqlite3 *db, void *pArg);
char *createTableStmt(sqlite3 *db, Table *p);
void sqliteViewResetAll(sqlite3 *db, int idx);
int tableMayNotBeDropped(sqlite3 *db, Table *pTab);
void cteClear(sqlite3 *db, Cte *pCte);
void callCollNeeded(sqlite3 *db, int enc, const char *zName);
int synthCollSeq(sqlite3 *db, CollSeq *pColl);
CollSeq *findCollSeqEntry(sqlite3 *db, const char *zName, int create);
Expr *exprTableColumn(sqlite3 *db, Table *pTab, int iCursor, i16 iCol);
void fkTriggerDelete(sqlite3 *dbMem, Trigger *p);
__attribute__((noinline)) const char *computeIndexAffStr(sqlite3 *db, Index *pIdx);
int sqlite3LoadExtension(sqlite3 *db, const char *zFile, const char *zProc, char **pzErrMsg);
void setAllPagerFlags(sqlite3 *db);
int pragmaVtabConnect(sqlite3 *db, void *pAux, int argc, const char *const *argv, sqlite3_vtab **ppVtab, char **pzErr);
int sqlite3Prepare(sqlite3 *db, const char *zSql, int nBytes, u32 prepFlags, Vdbe *pReprepare, sqlite3_stmt **ppStmt,
                   const char **pzTail);
int sqlite3LockAndPrepare(sqlite3 *db, const char *zSql, int nBytes, u32 prepFlags, Vdbe *pOld, sqlite3_stmt **ppStmt,
                          const char **pzTail);
int sqlite3Prepare16(sqlite3 *db, const void *zSql, int nBytes, u32 prepFlags, sqlite3_stmt **ppStmt,
                     const void **pzTail);
void clearSelect(sqlite3 *db, Select *p, int bFree);
u8 minMaxQuery(sqlite3 *db, Expr *pFunc, ExprList **ppMinMax);
void agginfoFree(sqlite3 *db, void *pArg);
char *triggerSpanDup(sqlite3 *db, const char *zStart, const char *zEnd);
int tempTriggersExist(sqlite3 *db);
void __attribute__((noinline)) upsertDelete(sqlite3 *db, Upsert *p);
int execSql(sqlite3 *db, char **pzErrMsg, const char *zSql);
int execSqlF(sqlite3 *db, char **pzErrMsg, const char *zSql, ...);
int createModule(sqlite3 *db, const char *zName, const sqlite3_module *pModule, void *pAux, void (*xDestroy)(void *));
VTable *vtabDisconnectAll(sqlite3 *db, Table *p);
int vtabCallConstructor(sqlite3 *db, Table *pTab, Module *pMod,
                        int (*xConstruct)(sqlite3 *, void *, int, const char *const *, sqlite3_vtab **, char **),
                        char **pzErr);
int growVTrans(sqlite3 *db);
void addToVTrans(sqlite3 *db, VTable *pVTab);
void callFinaliser(sqlite3 *db, int offset);
void whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p);
void whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p);
int isAuxiliaryVtabOperator(sqlite3 *db, Expr *pExpr, unsigned char *peOp2, Expr **ppLeft, Expr **ppRight);
int whereLoopResize(sqlite3 *, WhereLoop *, int);
void freeIndexInfo(sqlite3 *db, sqlite3_index_info *pIdxInfo);
void whereLoopClearUnion(sqlite3 *db, WhereLoop *p);
void whereLoopClear(sqlite3 *db, WhereLoop *p);
int whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom);
void whereLoopDelete(sqlite3 *db, WhereLoop *p);
void whereInfoFree(sqlite3 *db, WhereInfo *pWInfo);
void whereIndexedExprCleanup(sqlite3 *db, void *pObject);
int sqlite3TestExtInit(sqlite3 *db);
extern int (*const sqlite3BuiltinExtensions[1])(sqlite3 *);
int setupLookaside(sqlite3 *db, void *pBuf, int sz, int cnt);
void functionDestroy(sqlite3 *db, FuncDef *p);
void disconnectAllVtab(sqlite3 *db);
int connectionIsBusy(sqlite3 *db);
int sqlite3Close(sqlite3 *db, int forceZombie);
int createFunctionApi(sqlite3 *db, const char *zFunc, int nArg, int enc, void *p,
                      void (*xSFunc)(sqlite3_context *, int, sqlite3_value **),
                      void (*xStep)(sqlite3_context *, int, sqlite3_value **), void (*xFinal)(sqlite3_context *),
                      void (*xValue)(sqlite3_context *), void (*xInverse)(sqlite3_context *, int, sqlite3_value **),
                      void (*xDestroy)(void *));
int createCollation(sqlite3 *db, const char *zName, u8 enc, void *pCtx,
                    int (*xCompare)(void *, int, const void *, int, const void *), void (*xDel)(void *));
int jsonEachConnect(sqlite3 *db, void *pAux, int argc, const char *const *argv, sqlite3_vtab **ppVtab, char **pzErr);
extern int (*const sqlite3BuiltinExtensions[1])(sqlite3 *);
int sqlite3_open(const char *filename, sqlite3 **ppDb);
int sqlite3_open16(const void *filename, sqlite3 **ppDb);
int sqlite3_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
int sqlite3TempInMemory(const sqlite3 *);
const char *sqlite3_errstr(int);
int sqlite3_global_recover(void);
void sqlite3_thread_cleanup(void);
int sqlite3_memory_alarm(void (*)(void *, sqlite3_int64, int), void *, sqlite3_int64);
int sqlite3_sleep(int);
extern char *sqlite3_temp_directory;
extern char *sqlite3_data_directory;
int sqlite3_enable_shared_cache(int);
int sqlite3_release_memory(int);
int sqlite3_auto_extension(void (*xEntryPoint)(void));
int sqlite3_cancel_auto_extension(void (*xEntryPoint)(void));
void sqlite3_reset_auto_extension(void);
int sqlite3_test_control(int op, ...);
int sqlite3_keyword_count(void);
int sqlite3_keyword_name(int, const char **, int *);
int sqlite3_keyword_check(const char *, int);
int sqlite3_status(int op, int *pCurrent, int *pHighwater, int resetFlag);
int sqlite3_status64(int op, sqlite3_int64 *pCurrent, sqlite3_int64 *pHighwater, int resetFlag);
int sqlite3_stricmp(const char *, const char *);
int sqlite3_strnicmp(const char *, const char *, int);
int sqlite3_strglob(const char *zGlob, const char *zStr);
int sqlite3_strlike(const char *zGlob, const char *zStr, unsigned int cEsc);
void sqlite3_log(int iErrCode, const char *zFormat, ...);
extern u32 sqlite3WhereTrace;
int sqlite3PcacheOpen(int szPage, int szExtra, int bPurgeable, int (*xStress)(void *, PgHdr *), void *pStress,
                      PCache *pToInit);
int sqlite3PcacheSize(void);
int sqlite3ReportError(int iErr, int lineno, const char *zType);
int sqlite3CorruptError(int);
int sqlite3MisuseError(int);
int sqlite3CantopenError(int);
int sqlite3IsIdChar(u8);
int sqlite3StrICmp(const char *, const char *);
int sqlite3Strlen30(const char *);
void *sqlite3Malloc(u64);
void *sqlite3MallocZero(u64);
void *sqlite3Realloc(void *, u64);
int sqlite3MallocSize(const void *);
void *sqlite3PageMalloc(int);
void sqlite3PageFree(void *);
int sqlite3HeapNearlyFull(void);
sqlite3_mutex_methods const *sqlite3DefaultMutex(void);
sqlite3_mutex_methods const *sqlite3NoopMutex(void);
sqlite3_mutex *sqlite3MutexAlloc(int);
int sqlite3MutexInit(void);
void sqlite3MemoryBarrier(void);
void sqlite3StatusUp(int, int);
void sqlite3StatusDown(int, int);
void sqlite3StatusHighwater(int, int);
sqlite3_mutex *sqlite3Pcache1Mutex(void);
sqlite3_mutex *sqlite3MallocMutex(void);
int sqlite3IsNaN(double);
int sqlite3IsOverflow(double);
void sqlite3Dequote(char *);
int sqlite3InitCallback(void *, int, char **, char **);
int sqlite3FaultSim(int);
const char *sqlite3PreferredTableName(const char *);
int sqlite3RunVacuum(char **, sqlite3 *, int, sqlite3_value *);
u32 sqlite3IsTrueOrFalse(const char *);
int sqlite3IsRowid(const char *);
FuncDef *sqlite3FunctionSearch(int, const char *);
int sqlite3AppendOneUtf8Character(char *, u32);
int sqlite3RealSameAsInt(double, sqlite3_int64);
i64 sqlite3RealToI64(double);
int sqlite3Int64ToText(i64, char *);
int sqlite3AtoF(const char *z, double *);
int sqlite3GetInt32(const char *, int *);
int sqlite3Atoi(const char *);
int sqlite3Utf8CharLen(const char *pData, int nByte);
u32 sqlite3Utf8Read(const u8 **);
int sqlite3Utf8ReadLimited(const u8 *, int, u32 *);

int sqlite3PutVarint(unsigned char *, u64);
u8 sqlite3GetVarint(const unsigned char *, u64 *);
u8 sqlite3GetVarint32(const unsigned char *, u32 *);
int sqlite3VarintLen(u64 v);
int sqlite3Atoi64(const char *, i64 *, int, u8);
int sqlite3DecOrHexToI64(const char *, i64 *);
u8 sqlite3HexToInt(int h);
const char *sqlite3ErrStr(int);
int sqlite3AddInt64(i64 *, i64);
int sqlite3SubInt64(i64 *, i64);
int sqlite3MulInt64(i64 *, i64);
int sqlite3AbsInt32(int);
u8 sqlite3GetBoolean(const char *z, u8);
i64 sqlite3GetToken(const unsigned char *, int *);
u8 sqlite3StrIHash(const char *);

void sqlite3NoopDestructor(void *);

int sqlite3ParserFallback(int);
const char *sqlite3JournalModename(int);
void sqlite3BeginBenignMalloc(void);
void sqlite3EndBenignMalloc(void);
u32 sqlite3Get4byte(const u8 *);
void sqlite3Put4byte(u8 *, u32);
const char **sqlite3CompileOptions(int *pnOpt);
extern const unsigned char sqlite3UpperToLower[274];
extern const unsigned char sqlite3CtypeMap[256];
extern int sqlite3PendingByte;
extern u32 sqlite3TreeTrace;
extern const unsigned char sqlite3OpcodeProperty[192];
extern const char sqlite3StrBINARY[7];
extern const unsigned char sqlite3StdTypeLen[6];
extern const char sqlite3StdTypeAffinity[6];
extern const char *sqlite3StdType[6];
int sqlite3IntFloatCompare(i64, double);
const char *sqlite3OpcodeName(int);
extern struct sqlite3PrngType sqlite3Prng;
extern struct sqlite3PrngType sqlite3SavedPrng;
extern const unsigned char sqlite3Utf8Trans1[64];
void sqlite3Fp2Convert10(u64 m, int e, int n, u64 *pD, int *pP);
double sqlite3Fp10Convert2(u64 d, int p);

extern const sqlite3DigitPairs_t sqlite3DigitPairs;

extern const u8 sqlite3SmallTypeSizes[128];

int sqlite3_get_table_cb(void *pArg, int nCol, char **argv, char **colv);

void sqlite3ParserFinalize(void *p);
