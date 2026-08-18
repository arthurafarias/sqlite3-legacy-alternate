
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct LookasideSlot LookasideSlot;
  typedef struct Lookaside Lookaside;
  struct Lookaside {
    u32 bDisable;         /* Only operate the lookaside when zero */
    u16 sz;               /* Size of each buffer in bytes */
    u16 szTrue;           /* True value of sz, even if disabled */
    u8 bMalloced;         /* True if pStart obtained from sqlite3_malloc() */
    u32 nSlot;            /* Number of lookaside slots allocated */
    u32 anStat[3];        /* 0: hits.  1: size misses.  2: full misses */
    LookasideSlot *pInit; /* List of buffers not previously used */
    LookasideSlot *pFree; /* List of available buffers */
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
    LookasideSlot *pSmallInit; /* List of small buffers not previously used */
    LookasideSlot *pSmallFree; /* List of available small buffers */
    void *pMiddle;             /* First byte past end of full-size buffers and
                               ** the first byte of LOOKASIDE_SMALL buffers */
#endif                         /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
    void *pStart;              /* First byte of available memory space */
    void *pEnd;                /* First byte past end of available space */
    void *pTrueEnd;            /* True value of pEnd, when db->pnBytesFreed!=0 */
  };

#ifdef __cplusplus
}
#endif
