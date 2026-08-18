
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Bool.h"
#include "sqlite/Pgno.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct BtCursor BtCursor;
  typedef struct Btree Btree;
  typedef struct KeyInfo KeyInfo;
  typedef struct sqlite3_value Mem;
  typedef struct VdbeSorter VdbeSorter;
  typedef struct VdbeTxtBlbCache VdbeTxtBlbCache;
  typedef struct sqlite3_vtab_cursor sqlite3_vtab_cursor;

  typedef struct VdbeCursor VdbeCursor;
  struct VdbeCursor {
    u8 eCurType;
    i8 iDb;
    u8 nullRow;
    u8 deferredMoveto;
    u8 isTable;

    Bool isEphemeral : 1;
    Bool useRandomRowid : 1;
    Bool isOrdered : 1;
    Bool noReuse : 1;
    Bool colCache : 1;
    u16 seekHit;
    union {
      Btree *pBtx;
      u32 *aAltMap;
    } ub;
    i64 seqCount;

    u32 cacheStatus;
    int seekResult;
    VdbeCursor *pAltCursor;
    union {
      BtCursor *pCursor;
      sqlite3_vtab_cursor *pVCur;
      VdbeSorter *pSorter;
    } uc;
    KeyInfo *pKeyInfo;
    u32 iHdrOffset;
    Pgno pgnoRoot;
    i16 nField;
    u16 nHdrParsed;
    i64 movetoTarget;
    u32 *aOffset;
    const u8 *aRow;
    u32 payloadSize;
    u32 szRow;

    VdbeTxtBlbCache *pCache;

    u32 aType[];
  };

  int __attribute__((noinline)) sqlite3VdbeHandleMovedCursor(VdbeCursor * p);
  int __attribute__((noinline)) sqlite3VdbeFinishMoveto(VdbeCursor *);
  int sqlite3VdbeCursorRestore(VdbeCursor *);
  int sqlite3VdbeSorterRowkey(const VdbeCursor *, Mem *);
  int sqlite3VdbeSorterRewind(const VdbeCursor *, int *);
  int sqlite3VdbeSorterWrite(const VdbeCursor *, Mem *);
  int sqlite3VdbeSorterCompare(const VdbeCursor *, Mem *, int, int *);
  __attribute__((noinline)) int vdbeColumnFromOverflow(VdbeCursor * pC, int iCol, u32 t, i64 iOffset, u32 cacheStatus,
                                                       u32 colCacheCtr, Mem *pDest);

#ifdef __cplusplus
}
#endif
