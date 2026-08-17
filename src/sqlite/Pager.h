
#pragma once
#include "sqlite/PagerSavepoint.h"
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/DbPage.h"
#include "sqlite/Pgno.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  typedef struct PCache PCache;
  typedef struct PgHdr PgHdr;
  typedef struct Wal Wal;

  typedef struct sqlite3_backup sqlite3_backup;
  typedef struct sqlite3 sqlite3;
  typedef struct Pager Pager;
  typedef struct sqlite3_vfs sqlite3_vfs;
  typedef struct sqlite3_file sqlite3_file;
  typedef struct Bitvec Bitvec;

  struct Pager {
    sqlite3_vfs *pVfs;
    u8 exclusiveMode;
    u8 journalMode;
    u8 useJournal;
    u8 noSync;
    u8 fullSync;
    u8 extraSync;
    u8 syncFlags;
    u8 walSyncFlags;
    u8 tempFile;
    u8 noLock;
    u8 readOnly;
    u8 memDb;
    u8 memVfs;
    u8 eState;
    u8 eLock;
    u8 changeCountDone;
    u8 setSuper;
    u8 doNotSpill;
    u8 subjInMemory;
    u8 bUseFetch;
    u8 hasHeldSharedLock;
    Pgno dbSize;
    Pgno dbOrigSize;
    Pgno dbFileSize;
    Pgno dbHintSize;
    int errCode;
    int nRec;
    u32 cksumInit;
    u32 nSubRec;
    Bitvec *pInJournal;
    sqlite3_file *fd;
    sqlite3_file *jfd;
    sqlite3_file *sjfd;
    i64 journalOff;
    i64 journalHdr;
    sqlite3_backup *pBackup;
    PagerSavepoint *aSavepoint;
    int nSavepoint;
    u32 iDataVersion;
    char dbFileVers[16];

    int nMmapOut;
    sqlite3_int64 szMmap;
    PgHdr *pMmapFreelist;

    u16 nExtra;
    i16 nReserve;
    u32 vfsFlags;
    u32 sectorSize;
    Pgno mxPgno;
    Pgno lckPgno;
    i64 pageSize;
    i64 journalSizeLimit;
    char *zFilename;
    char *zJournal;
    int (*xBusyHandler)(void *);
    void *pBusyHandlerArg;
    u32 aStat[4];

    void (*xReiniter)(DbPage *);
    int (*xGet)(Pager *, Pgno, DbPage **, int);
    char *pTmpSpace;
    PCache *pPCache;

    Wal *pWal;
    char *zWal;
  };

  int sqlite3PagerClose(Pager * pPager, sqlite3 *);
  int sqlite3PagerReadFileheader(Pager *, int, unsigned char *);
  void sqlite3PagerSetBusyHandler(Pager *, int (*)(void *), void *);
  int sqlite3PagerSetPagesize(Pager *, u32 *, int);
  Pgno sqlite3PagerMaxPageCount(Pager *, Pgno);
  void sqlite3PagerSetCachesize(Pager *, int);
  int sqlite3PagerSetSpillsize(Pager *, int);
  void sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
  void sqlite3PagerShrink(Pager *);
  void sqlite3PagerSetFlags(Pager *, unsigned);
  int sqlite3PagerLockingMode(Pager *, int);
  int sqlite3PagerSetJournalMode(Pager *, int);
  int sqlite3PagerGetJournalMode(Pager *);
  int sqlite3PagerOkToChangeJournalMode(Pager *);
  i64 sqlite3PagerJournalSizeLimit(Pager *, i64);
  sqlite3_backup **sqlite3PagerBackupPtr(Pager *);
  int sqlite3PagerFlush(Pager *);
  int sqlite3PagerGet(Pager * pPager, Pgno pgno, DbPage * *ppPage, int clrFlag);
  DbPage *sqlite3PagerLookup(Pager * pPager, Pgno pgno);
  int sqlite3PagerMovepage(Pager *, DbPage *, Pgno, int);
  void sqlite3PagerPagecount(Pager *, int *);
  int sqlite3PagerBegin(Pager *, int exFlag, int);
  int sqlite3PagerCommitPhaseOne(Pager *, const char *zSuper, int);
  int sqlite3PagerExclusiveLock(Pager *);
  int sqlite3PagerSync(Pager * pPager, const char *zSuper);
  int sqlite3PagerCommitPhaseTwo(Pager *);
  int sqlite3PagerRollback(Pager *);
  int sqlite3PagerOpenSavepoint(Pager * pPager, int n);
  int sqlite3PagerSavepoint(Pager * pPager, int op, int iSavepoint);
  int sqlite3PagerSharedLock(Pager * pPager);
  int sqlite3PagerCheckpoint(Pager * pPager, sqlite3 *, int, int *, int *);
  int sqlite3PagerWalSupported(Pager * pPager);
  int sqlite3PagerWalCallback(Pager * pPager);
  int sqlite3PagerOpenWal(Pager * pPager, int *pisOpen);
  int sqlite3PagerCloseWal(Pager * pPager, sqlite3 *);
  int sqlite3PagerDirectReadOk(Pager * pPager, Pgno pgno);
  u8 sqlite3PagerIsreadonly(Pager *);
  u32 sqlite3PagerDataVersion(Pager *);
  int sqlite3PagerMemUsed(Pager *);
  sqlite3_vfs *sqlite3PagerVfs(Pager *);
  sqlite3_file *sqlite3PagerFile(Pager *);
  sqlite3_file *sqlite3PagerJrnlFile(Pager *);
  const char *sqlite3PagerJournalname(Pager *);
  void *sqlite3PagerTempSpace(Pager *);
  int sqlite3PagerIsMemdb(Pager *);
  void sqlite3PagerCacheStat(Pager *, int, int, u64 *);
  void sqlite3PagerClearCache(Pager *);
  const char *sqlite3PagerFilename(const Pager *, int);
  void sqlite3PagerTruncateImage(Pager *, Pgno);

  int getPageNormal(Pager *, Pgno, DbPage **, int);
  int getPageError(Pager *, Pgno, DbPage **, int);
  int getPageMMap(Pager *, Pgno, DbPage **, int);
  void setGetterMethod(Pager * pPager);
  int pagerUnlockDb(Pager * pPager, int eLock);
  int pagerLockDb(Pager * pPager, int eLock);
  int jrnlBufferSize(Pager * pPager);
  i64 journalHdrOffset(Pager * pPager);
  int zeroJournalHdr(Pager * pPager, int doTruncate);
  int writeJournalHdr(Pager * pPager);
  int readJournalHdr(Pager * pPager, int isHot, i64 journalSize, u32 *pNRec, u32 *pDbSize);
  int writeSuperJournal(Pager * pPager, const char *zSuper);
  void pager_reset(Pager * pPager);
  void releaseAllSavepoints(Pager * pPager);
  int addToSavepointBitvecs(Pager * pPager, Pgno pgno);
  void pager_unlock(Pager * pPager);
  int pager_error(Pager * pPager, int rc);
  int pager_truncate(Pager * pPager, Pgno nPage);
  int pagerFlushOnCommit(Pager * pPager, int bCommit);
  int pager_end_transaction(Pager * pPager, int hasSuper, int bCommit);
  int pager_playback(Pager * pPager, int isHot);
  void pagerUnlockAndRollback(Pager * pPager);
  u32 pager_cksum(Pager * pPager, const u8 *aData);
  int pager_playback_one_page(Pager * pPager, i64 * pOffset, Bitvec * pDone, int isMainJrnl, int isSavepnt);
  int pager_delsuper(Pager * pPager, const char *zSuper);
  void setSectorSize(Pager * pPager);
  int pagerRollbackWal(Pager * pPager);
  int pagerWalFrames(Pager * pPager, PgHdr * pList, Pgno nTruncate, int isCommit);
  int pagerBeginReadTransaction(Pager * pPager);
  int pagerPagecount(Pager * pPager, Pgno * pnPage);
  int pagerOpenWalIfPresent(Pager * pPager);
  int pagerPlaybackSavepoint(Pager * pPager, PagerSavepoint * pSavepoint);
  void pagerFixMaplimit(Pager * pPager);
  int pagerOpentemp(Pager * pPager, sqlite3_file * pFile, int vfsFlags);
  int pager_wait_on_lock(Pager * pPager, int locktype);
  int pagerSyncHotJournal(Pager * pPager);
  int pagerAcquireMapPage(Pager * pPager, Pgno pgno, void *pData, PgHdr **ppPage);
  void pagerFreeMapHdrs(Pager * pPager);
  int databaseIsUnmoved(Pager * pPager);
  int syncJournal(Pager * pPager, int newHdr);
  int pager_write_pagelist(Pager * pPager, PgHdr * pList);
  int openSubJournal(Pager * pPager);
  int pagerStress(void *p, PgHdr *pPg);
  int hasHotJournal(Pager * pPager, int *pExists);
  void pagerUnlockIfUnused(Pager * pPager);
  int pager_open_journal(Pager * pPager);
  int pager_incr_changecounter(Pager * pPager, int isDirectMode);
  __attribute__((noinline)) int pagerOpenSavepoint(Pager * pPager, int nSavepoint);
  int pagerExclusiveLock(Pager * pPager);
  int pagerOpenWal(Pager * pPager);

  extern const unsigned char aJournalMagic[8];
  void freeSuperJournal(char *zSuper);

#ifdef __cplusplus
}
#endif
