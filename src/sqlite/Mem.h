#pragma once

#include "sqlite/i64.h"
#include "sqlite/CollSeq.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
typedef struct FuncDef FuncDef;
typedef struct VdbeOp Op;
typedef struct sqlite3 sqlite3;

typedef struct sqlite3_value Mem;

int sqlite3MemCompare(const Mem *, const Mem *, const CollSeq *);
int sqlite3BlobCompare(const Mem *, const Mem *);
i64 sqlite3VdbeIntValue(const Mem *);

void sqlite3MemSetDefault(void);
int sqlite3VdbeChangeEncoding(Mem *, int);
int sqlite3VdbeMemTooBig(Mem *);
int sqlite3VdbeMemCopy(Mem *, const Mem *);
void sqlite3VdbeMemShallowCopy(Mem *, const Mem *, int);
void sqlite3VdbeMemMove(Mem *, Mem *);
int sqlite3VdbeMemNulTerminate(Mem *);
int sqlite3VdbeMemSetStr(Mem *, const char *, i64, u8, void (*)(void *));
int sqlite3VdbeMemSetText(Mem *, const char *, i64, void (*)(void *));
void sqlite3VdbeMemSetInt64(Mem *, i64);
void sqlite3VdbeMemSetDouble(Mem *, double);
void sqlite3VdbeMemSetPointer(Mem *, void *, const char *, void (*)(void *));
void sqlite3VdbeMemInit(Mem *, sqlite3 *, u16);
void sqlite3VdbeMemSetNull(Mem *);
void sqlite3VdbeMemSetZeroBlob(Mem *, int);
int sqlite3VdbeMemSetRowSet(Mem *);
int sqlite3VdbeMemZeroTerminateIfAble(Mem *);
int sqlite3VdbeMemMakeWriteable(Mem *);
int sqlite3VdbeMemStringify(Mem *, u8, u8);
int sqlite3VdbeMemIntegerify(Mem *);
double sqlite3VdbeRealValue(Mem *);
int sqlite3MemRealValueRC(Mem *, double *);
int sqlite3VdbeBooleanValue(Mem *, int ifNull);
void sqlite3VdbeIntegerAffinity(Mem *);
int sqlite3VdbeMemRealify(Mem *);
int sqlite3VdbeMemNumerify(Mem *);
int sqlite3VdbeMemCast(Mem *, u8, u8);
void sqlite3VdbeMemRelease(Mem *p);
void sqlite3VdbeMemReleaseMalloc(Mem *p);
int sqlite3VdbeMemFinalize(Mem *, FuncDef *);
int sqlite3VdbeMemAggValue(Mem *, Mem *, FuncDef *);
int sqlite3VdbeMemGrow(Mem *pMem, int n, int preserve);
int sqlite3VdbeMemClearAndResize(Mem *pMem, int n);
int sqlite3VdbeMemTranslate(Mem *, u8);
int sqlite3VdbeMemHandleBom(Mem *pMem);
int sqlite3VdbeMemExpandBlob(Mem *);
void *sqlite3MemMalloc(int nByte);
void *sqlite3MemRealloc(void *pPrior, int nByte);
__attribute__((noinline)) int vdbeMemAddTerminator(Mem *pMem);
__attribute__((noinline)) void vdbeMemClearExternAndSetNull(Mem *p);
__attribute__((noinline)) void vdbeMemClear(Mem *p);
__attribute__((noinline)) i64 memIntValue(const Mem *pMem);
__attribute__((noinline)) int sqlite3MemRealValueRCSlowPath(Mem *pMem, double *pValue);
__attribute__((noinline)) double sqlite3MemRealValueNoRC(Mem *pMem);
__attribute__((noinline)) void vdbeReleaseAndSetInt64(Mem *pMem, i64 val);
__attribute__((noinline)) void vdbeClrCopy(Mem *pTo, const Mem *pFrom, int eType);
void initMemArray(Mem *p, int N, sqlite3 *db, u16 flags);
void releaseMemArray(Mem *p, int N);
__attribute__((noinline)) int vdbeCompareMemStringWithEncodingChange(const Mem *pMem1, const Mem *pMem2,
                                                                     const CollSeq *pColl, u8 *prcErr);
int vdbeCompareMemString(const Mem *pMem1, const Mem *pMem2, const CollSeq *pColl, u8 *prcErr);
int alsoAnInt(Mem *pRec, double rValue, i64 *piValue);
void applyNumericAffinity(Mem *pRec, int bTryForInt);
void applyAffinity(Mem *pRec, char affinity, u8 enc);
u16 __attribute__((noinline)) computeNumericType(Mem *pMem);
u16 numericType(Mem *pMem);
__attribute__((noinline)) Mem *out2PrereleaseWithClear(Mem *pOut);
u64 filterHash(const Mem *aMem, const Op *pOp);
const char *vdbeMemTypeName(Mem *pMem);

int isAllZero(const char *z, int n);


