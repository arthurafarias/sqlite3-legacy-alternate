
#pragma once

#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/AutoincInfo.h"
#include "sqlite/Token.h"
#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/Pgno.h"
#include "sqlite/Returning.h"
#include "sqlite/TriggerPrg.h"
#include "sqlite/VList.h"
#include "sqlite/bft.h"
#include "sqlite/i16.h"
#include "sqlite/i8.h"
#include "sqlite/u16.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
#include "sqlite/PragmaName.h"
  struct AuthContext;
  struct CollSeq;
  struct Column;
  struct DateTime;
  struct DistinctCtx;
  struct FKey;
  struct FuncDef;
  struct Index;
  struct KeyInfo;
  struct Module;
  struct NameContext;
  struct OnOrUsing;
  struct ParseCleanup;
  struct RenameCtx;
  struct RenameToken;
  struct RowLoadInfo;
  struct SelectDest;
  struct SortCtx;
  struct SubrtnSig;
  struct Walker;
  struct WhereClause;
  struct WhereInfo;
  struct WhereLevel;
  struct WhereLoop;
  struct WhereLoopBuilder;
  struct WhereTerm;
  struct sqlite3_index_info;
  struct sqlite3_vfs;

  struct sqlite3;
  struct Vdbe;

  struct IndexedExpr;
  struct TableLock;

  struct Parse {
    sqlite3 *db;
    char *zErrMsg;
    Vdbe *pVdbe;
    int rc;
    LogEst nQueryLoop;
    u8 nested;
    u8 nTempReg;
    u8 isMultiWrite;
    u8 disableLookaside;
    u8 prepFlags;
    u8 withinRJSubrtn;
    u8 mSubrtnSig;
    u8 eTriggerOp;
    u8 eOrconf;
    bft disableTriggers : 1;
    bft mayAbort : 1;
    bft hasCompound : 1;
    bft bReturning : 1;
    bft bHasExists : 1;
    bft colNamesSet : 1;
    bft bHasWith : 1;
    bft okConstFactor : 1;
    bft checkSchema : 1;
    int nRangeReg;
    int iRangeReg;
    int nErr;
    int nTab;
    int nMem;
    int szOpAlloc;
    int iSelfTab;

    int nNestSel;
    int nLabel;
    int nLabelAlloc;
    int *aLabel;
    ExprList *pConstExpr;
    IndexedExpr *pIdxEpr;
    IndexedExpr *pIdxPartExpr;
    yDbMask writeMask;
    yDbMask cookieMask;
    int nMaxArg;
    int nSelect;

    u32 nProgressSteps;

    int nTableLock;
    TableLock *aTableLock;

    AutoincInfo *pAinc;
    Parse *pToplevel;
    Table *pTriggerTab;
    TriggerPrg *pTriggerPrg;
    ParseCleanup *pCleanup;
    int aTempReg[8];
    Parse *pOuterParse;
    Token sNameToken;
    u32 oldmask;
    u32 newmask;
    union {
      struct {
        int addrCrTab;
        int regRowid;
        int regRoot;
        Token constraintName;
      } cr;
      struct {
        Returning *pReturning;
      } d;
    } u1;
    Token sLastToken;
    ynVar nVar;
    u8 iPkSortOrder;
    u8 explain;
    u8 eParseMode;

    int nVtabLock;

    int nHeight;
    int addrExplain;
    VList *pVList;
    Vdbe *pReprepare;
    const char *zTail;
    Table *pNewTable;
    Index *pNewIndex;

    Trigger *pNewTrigger;
    const char *zAuthContext;

    Token sArg;
    Table **apVtabLock;

    With *pWith;

    RenameToken *pRename;
  };
  Vdbe *sqlite3VdbeCreate(Parse *);
  int sqlite3VdbeAddFunctionCall(Parse *, int, int, int, int, const FuncDef *, int);
  int sqlite3VdbeExplain(Parse *, u8, const char *, ...);
  void sqlite3VdbeExplainPop(Parse *);
  int sqlite3VdbeExplainParent(Parse *);
  void sqlite3VdbeSetP4KeyInfo(Parse *, Index *);
  int sqlite3VdbeMakeLabel(Parse *);
  Window *sqlite3WindowAlloc(Parse *, int, int, Expr *, int, Expr *, u8);
  void sqlite3WindowAttach(Parse *, Expr *, Window *);
  void sqlite3WindowCodeInit(Parse *, Select *);
  void sqlite3WindowCodeStep(Parse *, Select *, WhereInfo *, int, int);
  int sqlite3WindowRewrite(Parse *, Select *);
  void sqlite3WindowUpdate(Parse *, Window *, Window *, FuncDef *);
  void sqlite3WindowChain(Parse *, Window *, Window *);
  Window *sqlite3WindowAssemble(Parse *, Window *, ExprList *, ExprList *, Token *);
  int sqlite3RunParser(Parse *, const char *);
  void sqlite3FinishCoding(Parse *);
  int sqlite3GetTempReg(Parse *);
  void sqlite3ReleaseTempReg(Parse *, int);
  int sqlite3GetTempRange(Parse *, int);
  void sqlite3ReleaseTempRange(Parse *, int, int);
  void sqlite3ClearTempRegCache(Parse *);
  void sqlite3TouchRegister(Parse *, int);
  Expr *sqlite3PExpr(Parse *, int, Expr *, Expr *);
  void sqlite3PExprAddSelect(Parse *, Expr *, Select *);
  Expr *sqlite3ExprAnd(Parse *, Expr *, Expr *);
  Expr *sqlite3ExprFunction(Parse *, ExprList *, const Token *, int);
  void sqlite3ExprAddFunctionOrderBy(Parse *, Expr *, ExprList *);
  void sqlite3ExprOrderByAggregateError(Parse *, Expr *);
  void sqlite3ExprFunctionUsable(Parse *, const Expr *, const FuncDef *);
  void sqlite3ExprAssignVarNumber(Parse *, Expr *, u32);
  int sqlite3ExprDeferredDelete(Parse *, Expr *);
  void sqlite3ExprUnmapAndDelete(Parse *, Expr *);
  ExprList *sqlite3ExprListAppend(Parse *, ExprList *, Expr *);
  ExprList *sqlite3ExprListAppendVector(Parse *, ExprList *, IdList *, Expr *);
  Select *sqlite3ExprListToValues(Parse *, int, ExprList *);
  void sqlite3ExprListSetName(Parse *, ExprList *, const Token *, int);
  void sqlite3ExprListSetSpan(Parse *, ExprList *, const char *, const char *);

  Select *sqlite3MultiValues(Parse * pParse, Select * pLeft, ExprList * pRow);
  void sqlite3MultiValuesEnd(Parse * pParse, Select * pVal);
  void sqlite3ProgressCheck(Parse *);
  void sqlite3ErrorMsg(Parse *, const char *, ...);
  void sqlite3DequoteNumber(Parse *, Expr *);
  void sqlite3Pragma(Parse *, Token *, Token *, Token *, int);
  void sqlite3ColumnSetExpr(Parse *, Table *, Column *, Expr *);
  void sqlite3GenerateColumnNames(Parse * pParse, Select * pSelect);
  int sqlite3ColumnsFromExprList(Parse *, ExprList *, i16 *, Column **);
  void sqlite3SubqueryColumnTypes(Parse *, Table *, Select *, char);
  Table *sqlite3ResultSetOfSelect(Parse *, Select *, char);
  void sqlite3OpenSchemaTable(Parse *, int);
  void sqlite3StartTable(Parse *, Token *, Token *, int, int, int, int);
  void sqlite3AddColumn(Parse *, Token, Token);
  void sqlite3AddNotNull(Parse *, int);
  void sqlite3AddPrimaryKey(Parse *, ExprList *, int, int, int);
  void sqlite3AddCheckConstraint(Parse *, Expr *, const char *, const char *);
  void sqlite3AddDefaultValue(Parse *, Expr *, const char *, const char *);
  void sqlite3AddCollateType(Parse *, Token *);
  void sqlite3AddGenerated(Parse *, Expr *, Token *);
  void sqlite3EndTable(Parse *, Token *, Token *, u32, Select *);
  void sqlite3AddReturning(Parse *, ExprList *);
  int sqlite3ParseUri(const char *, const char *, unsigned int *, sqlite3_vfs **, char **, char **);
  void sqlite3CreateView(Parse *, Token *, Token *, Token *, ExprList *, Select *, int, int);
  int sqlite3ViewGetColumnNames(Parse *, Table *);
  void sqlite3DropTable(Parse *, SrcList *, int, int);
  void sqlite3CodeDropTable(Parse *, Table *, int, int);
  void sqlite3AutoincrementBegin(Parse * pParse);
  void sqlite3AutoincrementEnd(Parse * pParse);
  void sqlite3Insert(Parse *, SrcList *, Select *, IdList *, int, Upsert *);
  void sqlite3ComputeGeneratedColumns(Parse *, int, Table *);
  IdList *sqlite3IdListAppend(Parse *, IdList *, Token *);
  SrcList *sqlite3SrcListEnlarge(Parse *, SrcList *, int, int);
  SrcList *sqlite3SrcListAppendList(Parse * pParse, SrcList * p1, SrcList * p2);
  SrcList *sqlite3SrcListAppend(Parse *, SrcList *, Token *, Token *);
  int sqlite3SrcItemAttachSubquery(Parse *, SrcItem *, Select *, int);
  SrcList *sqlite3SrcListAppendFromTerm(Parse *, SrcList *, Token *, Token *, Token *, Select *, OnOrUsing *);
  void sqlite3SrcListIndexedBy(Parse *, SrcList *, Token *);
  void sqlite3SrcListFuncArgs(Parse *, SrcList *, ExprList *);
  int sqlite3IndexedByLookup(Parse *, SrcItem *);
  void sqlite3SrcListShiftJoinType(Parse *, SrcList *);
  void sqlite3SrcListAssignCursors(Parse *, SrcList *);
  void sqlite3CreateIndex(Parse *, Token *, Token *, SrcList *, ExprList *, int, Token *, Expr *, int, int, u8);
  void sqlite3DropIndex(Parse *, SrcList *, int);
  int sqlite3Select(Parse *, Select *, SelectDest *);
  Select *sqlite3SelectNew(Parse *, ExprList *, SrcList *, Expr *, ExprList *, Expr *, ExprList *, u32, Expr *);
  void sqlite3SelectCheckOnClauses(Parse * pParse, Select * pSelect);
  Table *sqlite3SrcListLookup(Parse *, SrcList *);
  int sqlite3IsReadOnly(Parse *, Table *, Trigger *);
  void sqlite3OpenTable(Parse *, int iCur, int iDb, Table *, int);
  void sqlite3DeleteFrom(Parse *, SrcList *, Expr *, ExprList *, Expr *);
  void sqlite3Update(Parse *, SrcList *, ExprList *, Expr *, int, ExprList *, Expr *, Upsert *);
  WhereInfo *sqlite3WhereBegin(Parse *, SrcList *, Expr *, ExprList *, ExprList *, Select *, u16, int);
  void sqlite3ExprCodeLoadIndexColumn(Parse *, Index *, int, int, int);
  int sqlite3ExprCodeGetColumn(Parse *, Table *, int, int, int, u8);
  void sqlite3ExprCodeMove(Parse *, int, int, int);
  void sqlite3ExprCode(Parse *, Expr *, int);
  void sqlite3ExprCodeGeneratedColumn(Parse *, Table *, Column *, int);
  void sqlite3ExprCodeCopy(Parse *, Expr *, int);
  void sqlite3ExprCodeFactorable(Parse *, Expr *, int);
  int sqlite3ExprCodeRunJustOnce(Parse *, Expr *, int);
  void sqlite3ExprNullRegisterRange(Parse *, int, int);
  int sqlite3ExprCodeTemp(Parse *, Expr *, int *);
  int sqlite3ExprCodeTarget(Parse *, Expr *, int);
  int sqlite3ExprCodeExprList(Parse *, ExprList *, int, int, u8);
  void sqlite3ExprIfTrue(Parse *, Expr *, int, int);
  void sqlite3ExprIfFalse(Parse *, Expr *, int, int);
  void sqlite3ExprIfFalseDup(Parse *, Expr *, int, int);
  Table *sqlite3LocateTable(Parse *, u32 flags, const char *, const char *);
  Table *sqlite3LocateTableItem(Parse *, u32 flags, SrcItem *);
  void sqlite3Vacuum(Parse *, Token *, Expr *);
  int sqlite3ExprCompare(const Parse *, const Expr *, const Expr *, int);
  int sqlite3ExprImpliesExpr(const Parse *, const Expr *, const Expr *, int);
  int sqlite3ReferencesSrcList(Parse *, Expr *, SrcList *);
  Vdbe *sqlite3GetVdbe(Parse *);
  void sqlite3CodeVerifySchema(Parse *, int);
  void sqlite3CodeVerifyNamedSchema(Parse *, const char *zDb);
  void sqlite3BeginTransaction(Parse *, int);
  void sqlite3EndTransaction(Parse *, int);
  void sqlite3Savepoint(Parse *, int, Token *);
  int sqlite3ExprIsConstant(Parse *, Expr *);
  int sqlite3ExprIsConstantOrGroupBy(Parse *, Expr *, ExprList *);
  void sqlite3GenerateRowDelete(Parse *, Table *, Trigger *, int, int, int, i16, u8, u8, u8, int);
  void sqlite3GenerateRowIndexDelete(Parse *, Table *, int, int, int *, int);
  int sqlite3GenerateIndexKey(Parse *, Index *, int, int, int, int *, Index *, int);
  void sqlite3ResolvePartIdxLabel(Parse *, int);
  void sqlite3GenerateConstraintChecks(Parse *, Table *, int *, int, int, int, int, u8, u8, int, int *, int *,
                                       Upsert *);
  void sqlite3CompleteInsertion(Parse *, Table *, int, int, int, int *, int, int, int);
  int sqlite3OpenTableAndIndices(Parse *, Table *, int, u8, int, u8 *, int *, int *);
  void sqlite3BeginWriteOperation(Parse *, int, int);
  void sqlite3MultiWrite(Parse *);
  void sqlite3MayAbort(Parse *);
  void sqlite3HaltConstraint(Parse *, int, int, char *, i8, u8);
  void sqlite3UniqueConstraint(Parse *, int, Index *);
  void sqlite3RowidConstraint(Parse *, int, Table *);
  void sqlite3ChangeCookie(Parse *, int);
  void sqlite3MaterializeView(Parse *, Table *, Expr *, ExprList *, Expr *, int);
  void sqlite3BeginTrigger(Parse *, Token *, Token *, int, int, IdList *, SrcList *, Expr *, int, int);
  void sqlite3FinishTrigger(Parse *, TriggerStep *, Token *);
  void sqlite3DropTrigger(Parse *, SrcList *, int);
  void sqlite3DropTriggerPtr(Parse *, Trigger *);
  Trigger *sqlite3TriggersExist(Parse *, Table *, int, ExprList *, int *pMask);
  Trigger *sqlite3TriggerList(Parse *, Table *);
  void sqlite3CodeRowTrigger(Parse *, Trigger *, int, ExprList *, int, Table *, int, int, int);
  void sqlite3CodeRowTriggerDirect(Parse *, Trigger *, Table *, int, int, int);
  TriggerStep *sqlite3TriggerInsertStep(Parse *, SrcList *, IdList *, Select *, u8, Upsert *, const char *,
                                        const char *);
  TriggerStep *sqlite3TriggerUpdateStep(Parse *, SrcList *, SrcList *, ExprList *, Expr *, u8, const char *,
                                        const char *);
  TriggerStep *sqlite3TriggerDeleteStep(Parse *, SrcList *, Expr *, const char *, const char *);
  u32 sqlite3TriggerColmask(Parse *, Trigger *, ExprList *, int, int, Table *, int);
  int sqlite3JoinType(Parse *, Token *, Token *, Token *);
  void sqlite3CreateForeignKey(Parse *, ExprList *, Token *, ExprList *, int);
  void sqlite3DeferForeignKey(Parse *, int);
  void sqlite3AuthRead(Parse *, Expr *, Schema *, SrcList *);
  int sqlite3AuthCheck(Parse *, int, const char *, const char *, const char *);
  void sqlite3AuthContextPush(Parse *, AuthContext *, const char *);
  int sqlite3AuthReadCol(Parse *, const char *, const char *, int);
  void sqlite3Attach(Parse *, Expr *, Expr *, Expr *);
  void sqlite3Detach(Parse *, Expr *);
  int sqlite3TwoPartName(Parse *, Token *, Token *, Token **);
  int sqlite3ReadSchema(Parse * pParse);
  CollSeq *sqlite3LocateCollSeq(Parse * pParse, const char *zName);
  CollSeq *sqlite3ExprCollSeq(Parse * pParse, const Expr *pExpr);
  CollSeq *sqlite3ExprNNCollSeq(Parse * pParse, const Expr *pExpr);
  int sqlite3ExprCollSeqMatch(Parse *, const Expr *, const Expr *);
  Expr *sqlite3ExprAddCollateToken(const Parse *pParse, Expr *, const Token *, int);
  Expr *sqlite3ExprAddCollateString(const Parse *, Expr *, const char *);
  int sqlite3CheckCollSeq(Parse *, CollSeq *);
  int sqlite3CheckObjectName(Parse *, const char *, const char *, const char *);
  void sqlite3Reindex(Parse *, Token *, Token *);
  void sqlite3AlterRenameTable(Parse *, SrcList *, Token *);
  void sqlite3AlterRenameColumn(Parse *, SrcList *, Token *, Token *);
  void sqlite3AlterDropConstraint(Parse *, SrcList *, Token *, Token *);
  void sqlite3AlterAddConstraint(Parse * pParse, SrcList * pSrc, Token * pFirst, Token * pName, const char *zExpr,
                                 int nExpr, Expr *pExpr);
  void sqlite3AlterSetNotNull(Parse *, SrcList *, Token *, Token *);
  void sqlite3NestedParse(Parse *, const char *, ...);
  void sqlite3CodeRhsOfIN(Parse *, Expr *, int, int);
  int sqlite3CodeSubselect(Parse *, Expr *);
  void sqlite3SelectPrep(Parse *, Select *, NameContext *);
  int sqlite3ExpandSubquery(Parse *, SrcItem *);
  void sqlite3SelectWrongNumTermsError(Parse * pParse, Select * p);
  void sqlite3ResolveSelectNames(Parse *, Select *, NameContext *);
  int sqlite3ResolveSelfReference(Parse *, Table *, int, Expr *, ExprList *);
  int sqlite3ResolveOrderGroupBy(Parse *, Select *, ExprList *, const char *);
  void sqlite3AlterFinishAddColumn(Parse *, Token *);
  void sqlite3AlterBeginAddColumn(Parse *, SrcList *);
  void sqlite3AlterDropColumn(Parse *, SrcList *, const Token *);
  const void *sqlite3RenameTokenMap(Parse *, const void *, const Token *);
  void sqlite3RenameTokenRemap(Parse *, const void *pTo, const void *pFrom);
  void sqlite3RenameExprUnmap(Parse *, Expr *);
  void sqlite3RenameExprlistUnmap(Parse *, ExprList *);
  CollSeq *sqlite3GetCollSeq(Parse *, u8, CollSeq *, const char *);
  void sqlite3Analyze(Parse *, Token *, Token *);
  KeyInfo *sqlite3KeyInfoOfIndex(Parse *, Index *);
  KeyInfo *sqlite3KeyInfoFromExprList(Parse *, ExprList *, int, int);
  int sqlite3HasExplicitNulls(Parse *, ExprList *);
  int sqlite3OpenTempDatabase(Parse *);
  int sqlite3ExprCheckIN(Parse *, Expr *);
  void sqlite3TableLock(Parse *, int, Pgno, u8, const char *);
  int sqlite3VtabEponymousTableInit(Parse *, Module *);
  void sqlite3VtabMakeWritable(Parse *, Table *);
  void sqlite3VtabBeginParse(Parse *, Token *, Token *, Token *, int);
  void sqlite3VtabFinishParse(Parse *, Token *);
  void sqlite3VtabArgInit(Parse *);
  void sqlite3VtabArgExtend(Parse *, Token *);
  int sqlite3VtabCallConnect(Parse *, Table *);
  void sqlite3VtabUsesAllSchemas(Parse *);
  void sqlite3ParseObjectInit(Parse *, sqlite3 *);
  void sqlite3ParseObjectReset(Parse *);
  void *sqlite3ParserAddCleanup(Parse *, void (*)(sqlite3 *, void *), void *);
  void sqlite3ExprListCheckLength(Parse *, ExprList *, const char *);
  CollSeq *sqlite3ExprCompareCollSeq(Parse *, const Expr *);
  CollSeq *sqlite3BinaryCompareCollSeq(Parse *, const Expr *, const Expr *);
  Cte *sqlite3CteNew(Parse *, Token *, ExprList *, Select *, u8);
  With *sqlite3WithAdd(Parse *, With *, Cte *);
  With *sqlite3WithPush(Parse *, With *, u8);
  int sqlite3UpsertAnalyzeTarget(Parse *, SrcList *, Upsert *, Upsert *);
  void sqlite3UpsertDoUpdate(Parse *, Upsert *, Table *, Index *, int);
  void sqlite3FkCheck(Parse *, Table *, int, int, int *, int);
  void sqlite3FkDropTable(Parse *, SrcList *, Table *);
  void sqlite3FkActions(Parse *, Table *, ExprList *, int, int *, int);
  int sqlite3FkRequired(Parse *, Table *, int *, int);
  u32 sqlite3FkOldmask(Parse *, Table *);
  int sqlite3FkLocateIndex(Parse *, Table *, FKey *, Index **, int **);
  int sqlite3FindInIndex(Parse *, Expr *, u32, int *, int *, int *);
  void sqlite3ExprSetHeightAndFlags(Parse * pParse, Expr * p);
  int sqlite3ExprCheckHeight(Parse *, int);
  Expr *sqlite3ExprForVectorField(Parse *, Expr *, int, int);
  void sqlite3VectorErrorMsg(Parse *, Expr *);
  int parseHhMmSs(const char *zDate, DateTime *p);
  int parseYyyyMmDd(const char *zDate, DateTime *p);
  __attribute__((noinline)) void resizeResolveLabel(Parse * p, Vdbe * v, int j);
  void resolveAlias(Parse * pParse, ExprList * pEList, int iCol, Expr *pExpr, int nSubquery);
  void extendFJMatch(Parse * pParse, ExprList * *ppList, SrcItem * pMatch, i16 iColumn);
  int lookupName(Parse * pParse, const char *zDb, const char *zTab, const Expr *pRight, NameContext *pNC, Expr *pExpr);
  void notValidImpl(Parse * pParse, NameContext * pNC, const char *zMsg, Expr *pExpr, Expr *pError);
  int resolveAsName(Parse * pParse, ExprList * pEList, Expr * pE);
  int resolveOrderByTermToExprList(Parse * pParse, Select * pSelect, Expr * pE);
  void resolveOutOfRangeError(Parse * pParse, const char *zType, int i, int mx, Expr *pError);
  int resolveCompoundOrderBy(Parse * pParse, Select * pSelect);
  void exprCodeBetween(Parse *, Expr *, int, void (*)(Parse *, Expr *, int, int), int);
  int exprCodeVector(Parse * pParse, Expr * p, int *piToFree);
  int codeCompare(Parse * pParse, Expr * pLeft, Expr * pRight, int opcode, int in1, int in2, int dest, int jumpIfNull,
                  int isCommuted);
  int exprCodeSubselect(Parse * pParse, Expr * pExpr);
  int exprVectorRegister(Parse * pParse, Expr * pVector, int iField, int regSelect, Expr **ppExpr, int *pRegFree);
  void codeVectorCompare(Parse * pParse, Expr * pExpr, int dest, u8 op, u8 p5);
  int exprComputeOperands(Parse * pParse, Expr * pExpr, int *pR1, int *pR2, int *pFree1, int *pFree2);
  int exprIsConst(Parse * pParse, Expr * p, int initFlag);
  int sqlite3ExprIsConstantNotJoin(Parse * pParse, Expr * p);
  int sqlite3InRhsIsConstant(Parse * pParse, Expr * pIn);
  char *exprINAffinity(Parse * pParse, const Expr *pExpr);
  void sqlite3SubselectError(Parse * pParse, int nActual, int nExpect);
  int findCompatibleInRhsSubrtn(Parse * pParse, Expr * pExpr, SubrtnSig * pNewSig);
  void sqlite3ExprCodeIN(Parse * pParse, Expr * pExpr, int destIfFalse, int destIfNull);
  void codeInteger(Parse * pParse, Expr * pExpr, int negFlag, int iMem);
  int exprCodeInlineFunction(Parse * pParse, ExprList * pFarg, int iFuncId, int target);
  int sqlite3ExprCanReturnSubtype(Parse * pParse, Expr * pExpr);
  __attribute__((noinline)) int sqlite3IndexedExprLookup(Parse * pParse, Expr * pExpr, int target);
  int exprPartidxExprLookup(Parse * pParse, Expr * pExpr, int iTarget);
  __attribute__((noinline)) int exprCodeTargetAndOr(Parse * pParse, Expr * pExpr, int target, int *pTmpReg);
  __attribute__((noinline)) int exprCompareVariable(const Parse *pParse, const Expr *pVar, const Expr *pExpr);
  int exprImpliesNotNull(const Parse *pParse, const Expr *p, const Expr *pNN, int iTab, int seenNot);
  void findOrCreateAggInfoColumn(Parse * pParse, AggInfo * pAggInfo, Expr * pExpr);
  int isAlterableTable(Parse * pParse, Table * pTab);
  void renameTestSchema(Parse * pParse, const char *zDb, int bTemp, const char *zWhen, int bNoDQS);
  void renameFixQuotes(Parse * pParse, const char *zDb, int bTemp);
  void renameReloadSchema(Parse * pParse, int iDb, u16 p5);
  void sqlite3ErrorIfNotEmpty(Parse * pParse, const char *zDb, const char *zTab, const char *zErr);
  int isRealTable(Parse * pParse, Table * pTab, int iOp);
  void unmapColumnIdlistNames(Parse * pParse, const IdList *pIdList);
  RenameToken *renameTokenFind(Parse * pParse, struct RenameCtx * pCtx, const void *pPtr);
  void renameColumnElistNames(Parse * pParse, RenameCtx * pCtx, const ExprList *pEList, const char *zOld);
  void renameColumnIdlistNames(Parse * pParse, RenameCtx * pCtx, const IdList *pIdList, const char *zOld);
  int renameParseSql(Parse * p, const char *zDb, sqlite3 *db, const char *zSql, int bTemp);
  int renameResolveTrigger(Parse * pParse);
  void renameParseCleanup(Parse * pParse);
  int alterFindCol(Parse * pParse, Table * pTab, Token * pCol, int *piCol);
  Table *alterFindTable(Parse * pParse, SrcList * pSrc, int *piDb, const char **pzDb, int bAuth);
  void openStatTable(Parse * pParse, int iDb, int iStatCur, const char *zWhere, const char *zWhereType);
  void callStatGet(Parse * pParse, int regStat, int iParam, int regOut);
  void analyzeOneTable(Parse * pParse, Table * pTab, Index * pOnlyIdx, int iStatCur, int iMem, int iTab);
  void loadAnalysis(Parse * pParse, int iDb);
  void analyzeDatabase(Parse * pParse, int iDb);
  void analyzeTable(Parse * pParse, Table * pTab, Index * pOnlyIdx);
  void codeAttach(Parse * pParse, int type, FuncDef const *pFunc, Expr *pAuthArg, Expr *pFilename, Expr *pDbname,
                  Expr *pKey);
  void sqliteAuthBadReturnCode(Parse * pParse);
  __attribute__((noinline)) void lockTable(Parse * pParse, int iDb, Pgno iTab, u8 isWriteLock, const char *zName);
  void codeTableLocks(Parse * pParse);
  void sqlite3ForceNotReadOnly(Parse * pParse);
  void makeColumnPartOfPrimaryKey(Parse * pParse, Column * pCol);
  int resizeIndexObject(Parse * pParse, Index * pIdx, int N);
  void convertToWithoutRowidTable(Parse * pParse, Table * pTab);
  __attribute__((noinline)) int viewGetColumnNames(Parse * pParse, Table * pTable);
  void destroyRootPage(Parse * pParse, int iTable, int iDb);
  void destroyTable(Parse * pParse, Table * pTab);
  void sqlite3ClearStatTables(Parse * pParse, int iDb, const char *zType, const char *zName);
  void sqlite3RefillIndex(Parse * pParse, Index * pIndex, int memRootPage);
  void sqlite3CodeVerifySchemaAtToplevel(Parse * pToplevel, int iDb);
  int vtabIsReadOnly(Parse * pParse, Table * pTab);
  int tabIsReadOnly(Parse * pParse, Table * pTab);
  void fkLookupParent(Parse * pParse, int iDb, Table *pTab, Index *pIdx, FKey *pFKey, int *aiCol, int regData,
                      int nIncr, int isIgnore);
  Expr *exprTableRegister(Parse * pParse, Table * pTab, int regBase, i16 iCol);
  void fkScanChildren(Parse * pParse, SrcList * pSrc, Table * pTab, Index * pIdx, FKey * pFKey, int *aiCol, int regData,
                      int nIncr);
  int isSetNullAction(Parse * pParse, FKey * pFKey);
  Trigger *fkActionTrigger(Parse * pParse, Table * pTab, FKey * pFKey, ExprList * pChanges);
  int readsTable(Parse * p, int iDb, Table *pTab);
  int autoIncBegin(Parse * pParse, int iDb, Table *pTab);
  void autoIncStep(Parse * pParse, int memId, int regRowid);
  __attribute__((noinline)) void autoIncrementEnd(Parse * pParse);
  int exprListIsConstant(Parse * pParse, ExprList * pRow);
  int exprListIsNoAffinity(Parse * pParse, ExprList * pRow);
  int xferOptimization(Parse * pParse, Table * pDest, Select * pSelect, int onError, int iDbDest);
  int invalidateTempStorage(Parse * pParse);
  int changeTempStorage(Parse * pParse, const char *zStorageType);
  void schemaIsValid(Parse * pParse);
  int sqlite3ProcessJoin(Parse * pParse, Select * p);
  void innerLoopLoadRow(Parse * pParse, Select * pSelect, RowLoadInfo * pInfo);
  int makeSorterRecord(Parse * pParse, SortCtx * pSort, Select * pSelect, int regBase, int nBase);
  void pushOntoSorter(Parse * pParse, SortCtx * pSort, Select * pSelect, int regData, int regOrigData, int nData,
                      int nPrefixReg);
  int codeDistinct(Parse * pParse, int eTnctType, int iTab, int addrRepeat, ExprList *pEList, int regElem);
  void fixDistinctOpenEph(Parse * pParse, int eTnctType, int iVal, int iOpenEphAddr);
  void selectInnerLoop(Parse * pParse, Select * p, int srcTab, SortCtx *pSort, DistinctCtx *pDistinct,
                       SelectDest *pDest, int iContinue, int iBreak);
  void explainTempTable(Parse * pParse, const char *zUsage);
  void generateSortTail(Parse * pParse, Select * p, SortCtx * pSort, int nColumn, SelectDest *pDest);
  void generateColumnTypes(Parse * pParse, SrcList * pTabList, ExprList * pEList);
  void computeLimitRegisters(Parse * pParse, Select * p, int iBreak);
  CollSeq *multiSelectCollSeq(Parse * pParse, Select * p, int iCol);
  KeyInfo *multiSelectByMergeKeyInfo(Parse * pParse, Select * p, int nExtra);
  void generateWithRecursiveQuery(Parse * pParse, Select * p, SelectDest * pDest);
  int multiSelectByMerge(Parse * pParse, Select * p, SelectDest * pDest);
  int multiSelectValues(Parse * pParse, Select * p, SelectDest * pDest);
  int multiSelect(Parse * pParse, Select * p, SelectDest * pDest);
  int generateOutputSubroutine(Parse * pParse, Select * p, SelectDest * pIn, SelectDest * pDest, int regReturn,
                               int regPrev, KeyInfo *pKeyInfo, int iBreak);
  void srclistRenumberCursors(Parse * pParse, int *aCsrMap, SrcList *pSrc, int iExcept);
  void renumberCursors(Parse * pParse, Select * p, int iExcept, int *aCsrMap);
  int flattenSubquery(Parse * pParse, Select * p, int iFrom, int isAgg);
  int propagateConstants(Parse * pParse, Select * p);
  int pushDownWindowCheck(Parse * pParse, Select * pSubq, Expr * pExpr);
  int pushDownWhereTerms(Parse * pParse, Select * pSubq, Expr * pWhere, SrcList * pSrcList, int iSrc);
  int cannotBeFunction(Parse * pParse, SrcItem * pFrom);
  int resolveFromTermToCte(Parse * pParse, Walker * pWalker, SrcItem * pFrom);
  void sqlite3SelectExpand(Parse * pParse, Select * pSelect);
  void sqlite3SelectAddTypeInfo(Parse * pParse, Select * pSelect);
  void optimizeAggregateUseOfIndexedExpr(Parse * pParse, Select * pSelect, AggInfo * pAggInfo, NameContext * pNC);
  void assignAggregateRegisters(Parse * pParse, AggInfo * pAggInfo);
  void resetAccumulator(Parse * pParse, AggInfo * pAggInfo);
  void finalizeAggFunctions(Parse * pParse, AggInfo * pAggInfo);
  void updateAccumulator(Parse * pParse, int regAcc, AggInfo *pAggInfo, int eDistinctType);
  void explainSimpleCount(Parse * pParse, Table * pTab, Index * pIdx);
  void havingToWhere(Parse * pParse, Select * p);
  int countOfViewOptimization(Parse * pParse, Select * p);
  int fromClauseTermCanBeCoroutine(Parse * pParse, SrcList * pTabList, int i, int selFlags);
  __attribute__((noinline)) void existsToJoin(Parse * pParse, Select * p, Expr * pWhere);
  TriggerStep *triggerStepAllocate(Parse * pParse, u8 op, SrcList * pTabList, const char *zStart, const char *zEnd);
  __attribute__((noinline)) Trigger *triggersReallyExist(Parse * pParse, Table * pTab, int op, ExprList *pChanges,
                                                         int *pMask);
  int isAsteriskTerm(Parse * pParse, Expr * pTerm);
  ExprList *sqlite3ExpandReturning(Parse * pParse, ExprList * pList, Table * pTab);
  void codeReturningTrigger(Parse * pParse, Trigger * pTrigger, Table * pTab, int regIn);
  int codeTriggerProgram(Parse * pParse, TriggerStep * pStepList, int orconf);
  void transferParseError(Parse * pTo, Parse * pFrom);
  TriggerPrg *codeRowTrigger(Parse * pParse, Trigger * pTrigger, Table * pTab, int orconf);
  TriggerPrg *getRowTrigger(Parse * pParse, Trigger * pTrigger, Table * pTab, int orconf);
  void updateVirtualTable(Parse * pParse, SrcList * pSrc, Table * pTab, ExprList * pChanges, Expr * pRowidExpr,
                          int *aXRef, Expr *pWhere, int onError);
  Expr *exprRowColumn(Parse * pParse, int iCol);
  void updateFromSelect(Parse * pParse, int iEph, Index *pPk, ExprList *pChanges, SrcList *pTabList, Expr *pWhere,
                        ExprList *pOrderBy, Expr *pLimit);
  void addModuleArgument(Parse * pParse, Table * pTable, char *zArg);
  void addArgumentToVtab(Parse * pParse);
  int sqlite3WhereExplainOneScan(Parse * pParse, SrcList * pTabList, WhereLevel * pLevel, u16 wctrlFlags);
  int sqlite3WhereExplainBloomFilter(const Parse *pParse, const WhereInfo *pWInfo, const WhereLevel *pLevel);
  void sqlite3WhereAddExplainText(Parse * pParse, int addr, SrcList *pTabList, WhereLevel *pLevel, u16 wctrlFlags);
  Bitmask sqlite3WhereCodeOneLoopStart(Parse * pParse, Vdbe * v, WhereInfo * pWInfo, int iLevel, WhereLevel *pLevel,
                                       Bitmask notReady);
  void sqlite3WhereTabFuncArgs(Parse *, SrcItem *, WhereClause *);
  void codeApplyAffinity(Parse * pParse, int base, int n, char *zAff);
  Expr *removeUnindexableInClauseTerms(Parse * pParse, int iEq, WhereLoop *pLoop, Expr *pX);
  __attribute__((noinline)) void codeINTerm(Parse * pParse, WhereTerm * pTerm, WhereLevel * pLevel, int iEq, int bRev,
                                            int iTarget);
  int codeEqualityTerm(Parse * pParse, WhereTerm * pTerm, WhereLevel * pLevel, int iEq, int bRev, int iTarget);
  int codeAllEqualityTerms(Parse * pParse, WhereLevel * pLevel, int bRev, int nExtraReg, char **pzAff);
  void codeExprOrVector(Parse * pParse, Expr * p, int iReg, int nReg);
  __attribute__((noinline)) void filterPullDown(Parse * pParse, WhereInfo * pWInfo, int iLevel, int addrNxt,
                                                Bitmask notReady);
  u16 exprCommute(Parse * pParse, Expr * pExpr);
  int isLikeOrGlob(Parse * pParse, Expr * pExpr, Expr * *ppPrefix, int *pisComplete, int *pnoCase);
  int termIsEquivalence(Parse * pParse, Expr * pExpr, SrcList * pSrc);
  __attribute__((noinline)) const char *indexInAffinityOk(Parse * pParse, WhereTerm * pTerm, u8 idxaff);
  int findIndexCol(Parse * pParse, ExprList * pList, int iBase, Index *pIdx, int iCol);
  int isDistinctRedundant(Parse * pParse, SrcList * pTabList, WhereClause * pWC, ExprList * pDistinct);
  void translateColumnToCopy(Parse * pParse, int iStart, int iTabCur, int iRegister, int iAutoidxCur);
  __attribute__((noinline)) void constructAutomaticIndex(Parse * pParse, WhereClause * pWC, const Bitmask notReady,
                                                         WhereLevel *pLevel);
  int vtabBestIndex(Parse * pParse, Table * pTab, sqlite3_index_info * p);
  int whereRangeScanEst(Parse * pParse, WhereLoopBuilder * pBuilder, WhereTerm * pLower, WhereTerm * pUpper,
                        WhereLoop * pLoop);
  int whereRangeVectorLen(Parse * pParse, int iCur, Index *pIdx, int nEq, WhereTerm *pTerm);
  void wherePartIdxExpr(Parse * pParse, Index * pIdx, Expr * pPart, Bitmask * pMask, int iIdxCur, SrcItem *pItem);
  __attribute__((noinline)) void whereAddIndexedExpr(Parse * pParse, Index * pIdx, int iIdxCur, SrcItem *pTabItem);
  Window *windowFind(Parse * pParse, Window * pList, const char *zName);
  void selectWindowRewriteEList(Parse * pParse, Window * pWin, SrcList * pSrc, ExprList * pEList, Table * pTab,
                                ExprList * *ppSub);
  ExprList *exprListAppendList(Parse * pParse, ExprList * pList, ExprList * pAppend, int bIntToNull);
  Expr *sqlite3WindowOffsetExpr(Parse * pParse, Expr * pExpr);
  void windowCheckValue(Parse * pParse, int reg, int eCond);
  int windowInitAccum(Parse * pParse, Window * pMWin);
  void windowIfNewPeer(Parse * pParse, ExprList * pOrderBy, int regNew, int regOld, int addr);
  int windowExprGtZero(Parse * pParse, Expr * pExpr);
  void parserSyntaxError(Parse * pParse, Token * p);
  void disableLookaside(Parse * pParse);
  void parserDoubleLinkSelect(Parse * pParse, Select * p);
  Select *attachWithToSelect(Parse * pParse, Select * pSelect, With * pWith);
  int parserStackSizeLimit(Parse * pParse);
  Expr *tokenExpr(Parse * pParse, int op, Token t);
  Expr *sqlite3PExprIsNull(Parse * pParse, int op, Expr *pLeft);
  Expr *sqlite3PExprIs(Parse * pParse, int op, Expr *pLeft, Expr *pRight);
  ExprList *parserAddExprIdListTerm(Parse * pParse, ExprList * pPrior, Token * pIdToken, int hasCollate, int sortOrder);

  extern const struct ExprList_item zeroItem;
  u8 getSafetyLevel(const char *z, int omitFull, u8 dflt);
  const PragmaName *pragmaLocate(const char *zName);
  void sqlite3ParserInit(void *yypRawParser, Parse *pParse);


