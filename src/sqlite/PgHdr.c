#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/PgHdr.h"

#include "sqlite/Bitvec.h"
#include "sqlite/DbPage.h"
#include "sqlite/PCache.h"
#include "sqlite/Pager.h"
#include "sqlite/PagerSavepoint.h"
#include "sqlite/Pgno.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/Wal.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_pcache.h"
#include "sqlite/sqlite3_pcache_methods2.h"
#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void pcacheManageDirtyList(PgHdr *pPage, u8 addRemove) {
  PCache *p = pPage->pCache;

  ;
  if (addRemove & 1) {





    if (p->pSynced == pPage) {
      p->pSynced = pPage->pDirtyPrev;
    }

    if (pPage->pDirtyNext) {
      pPage->pDirtyNext->pDirtyPrev = pPage->pDirtyPrev;
    } else {


      p->pDirtyTail = pPage->pDirtyPrev;
    }
    if (pPage->pDirtyPrev) {
      pPage->pDirtyPrev->pDirtyNext = pPage->pDirtyNext;
    } else {


      p->pDirty = pPage->pDirtyNext;


      if (p->pDirty == 0) {


        p->eCreate = 2;
      }
    }
  }
  if (addRemove & 2) {
    pPage->pDirtyPrev = 0;
    pPage->pDirtyNext = p->pDirty;
    if (pPage->pDirtyNext) {


      pPage->pDirtyNext->pDirtyPrev = pPage;
    } else {
      p->pDirtyTail = pPage;
      if (p->bPurgeable) {


        p->eCreate = 1;
      }
    }
    p->pDirty = pPage;

    if (!p->pSynced && 0 == (pPage->flags & 0x008)) {
      p->pSynced = pPage;
    }
  };
}

void pcacheUnpin(PgHdr *p) {
  if (p->pCache->bPurgeable) {
    ;
    sqlite3Config.pcache2.xUnpin(p->pCache->pCache, p->pPage, 0);
    ;
  }
}

void __attribute__((noinline)) sqlite3PcacheRelease(PgHdr *p) {

  p->pCache->nRefSum--;
  if ((--p->nRef) == 0) {
    if (p->flags & 0x001) {
      pcacheUnpin(p);
    } else {
      pcacheManageDirtyList(p, 3);


    }
  }
}

void sqlite3PcacheRef(PgHdr *p) {

  p->nRef++;
  p->pCache->nRefSum++;
}

void sqlite3PcacheDrop(PgHdr *p) {

  if (p->flags & 0x002) {
    pcacheManageDirtyList(p, 1);
  }
  p->pCache->nRefSum--;
  sqlite3Config.pcache2.xUnpin(p->pCache->pCache, p->pPage, 1);
}

void sqlite3PcacheMakeDirty(PgHdr *p) {

  if (p->flags & (0x001 | 0x010)) {
    p->flags &= ~0x010;
    if (p->flags & 0x001) {
      p->flags ^= (0x002 | 0x001);
      ;


      pcacheManageDirtyList(p, 2);


    }


  }
}

void sqlite3PcacheMakeClean(PgHdr *p) {

  pcacheManageDirtyList(p, 1);
  p->flags &= ~(0x002 | 0x008 | 0x004);
  p->flags |= 0x001;
  ;

  if (p->nRef == 0) {
    pcacheUnpin(p);
  }
}

void sqlite3PcacheMove(PgHdr *p, Pgno newPgno) {
  PCache *pCache = p->pCache;
  sqlite3_pcache_page *pOther;

  ;
  pOther = sqlite3Config.pcache2.xFetch(pCache->pCache, newPgno, 0);
  if (pOther) {
    PgHdr *pXPage = (PgHdr *)pOther->pExtra;


    pXPage->nRef++;
    pCache->nRefSum++;
    sqlite3PcacheDrop(pXPage);
  }
  sqlite3Config.pcache2.xRekey(pCache->pCache, p->pPage, p->pgno, newPgno);
  p->pgno = newPgno;
  if ((p->flags & 0x002) && (p->flags & 0x008)) {
    pcacheManageDirtyList(p, 3);


  }
}

PgHdr *pcacheMergeDirtyList(PgHdr *pA, PgHdr *pB) {
  PgHdr result, *pTail;
  pTail = &result;

  for (;;) {
    if (pA->pgno < pB->pgno) {
      pTail->pDirty = pA;
      pTail = pA;
      pA = pA->pDirty;
      if (pA == 0) {
        pTail->pDirty = pB;
        break;
      }
    } else {
      pTail->pDirty = pB;
      pTail = pB;
      pB = pB->pDirty;
      if (pB == 0) {
        pTail->pDirty = pA;
        break;
      }
    }
  }
  return result.pDirty;
}

PgHdr *pcacheSortDirtyList(PgHdr *pIn) {
  PgHdr *a[32], *p;
  int i;
  memset(a, 0, sizeof(a));
  while (pIn) {
    p = pIn;
    pIn = p->pDirty;
    p->pDirty = 0;
    for (i = 0; (i < 32 - 1); i++) {
      if (a[i] == 0) {
        a[i] = p;
        break;
      } else {
        p = pcacheMergeDirtyList(a[i], p);
        a[i] = 0;
      }
    }
    if ((i == 32 - 1)) {

      a[i] = pcacheMergeDirtyList(a[i], p);
    }
  }
  p = a[0];
  for (i = 1; i < 32; i++) {
    if (a[i] == 0)
      continue;
    p = p ? pcacheMergeDirtyList(p, a[i]) : a[i];
  }
  return p;
}

i64 sqlite3PcachePageRefcount(PgHdr *p) { return p->nRef; }

int subjRequiresPage(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;
  PagerSavepoint *p;
  Pgno pgno = pPg->pgno;
  int i;
  for (i = 0; i < pPager->nSavepoint; i++) {
    p = &pPager->aSavepoint[i];
    if (p->nOrig >= pgno && 0 == sqlite3BitvecTestNotNull(p->pInSavepoint, pgno)) {
      for (i = i + 1; i < pPager->nSavepoint; i++) {
        pPager->aSavepoint[i].bTruncateOnRelease = 0;
      }
      return 1;
    }
  }
  return 0;
}

int readDbPage(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;
  int rc = 0;

  u32 iFrame = 0;

  if (((pPager)->pWal != 0)) {
    rc = sqlite3WalFindFrame(pPager->pWal, pPg->pgno, &iFrame);
    if (rc)
      return rc;
  }
  if (iFrame) {
    rc = sqlite3WalReadFrame(pPager->pWal, iFrame, pPager->pageSize, pPg->pData);
  } else

  {
    i64 iOffset = (pPg->pgno - 1) * (i64)pPager->pageSize;
    rc = sqlite3OsRead(pPager->fd, pPg->pData, pPager->pageSize, iOffset);
    if (rc == (10 | (2 << 8))) {
      rc = 0;
    }
  }

  if (pPg->pgno == 1) {
    if (rc) {

      memset(pPager->dbFileVers, 0xff, sizeof(pPager->dbFileVers));
    } else {
      u8 *dbFileVers = &((u8 *)pPg->pData)[24];
      memcpy(&pPager->dbFileVers, dbFileVers, sizeof(pPager->dbFileVers));
    }
  };
  ;
  ;

  ;

  return rc;
}

void pager_write_changecounter(PgHdr *pPg) {
  u32 change_counter;
  if ((pPg == 0))
    return;

  change_counter = sqlite3Get4byte((u8 *)pPg->pPager->dbFileVers) + 1;
  sqlite3Put4byte((u8 *)((char *)pPg->pData) + 24, change_counter);

  sqlite3Put4byte((u8 *)((char *)pPg->pData) + 92, change_counter);
  sqlite3Put4byte((u8 *)((char *)pPg->pData) + 96, 3053004);
}

void pagerReleaseMapPage(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;
  pPager->nMmapOut--;
  pPg->pDirty = pPager->pMmapFreelist;
  pPager->pMmapFreelist = pPg;

  sqlite3OsUnfetch(pPager->fd, (i64)(pPg->pgno - 1) * pPager->pageSize, pPg->pData);
}

int subjournalPage(PgHdr *pPg) {
  int rc = 0;
  Pager *pPager = pPg->pPager;
  if (pPager->journalMode != 2) {








    rc = openSubJournal(pPager);

    if (rc == 0) {
      void *pData = pPg->pData;
      i64 offset = (i64)pPager->nSubRec * (4 + pPager->pageSize);
      char *pData2;
      pData2 = pData;
      ;
      rc = write32bits(pPager->sjfd, offset, pPg->pgno);
      if (rc == 0) {
        rc = sqlite3OsWrite(pPager->sjfd, pData2, pPager->pageSize, offset + 4);
      }
    }
  }
  if (rc == 0) {
    pPager->nSubRec++;


    rc = addToSavepointBitvecs(pPager, pPg->pgno);
  }
  return rc;
}

int subjournalPageIfRequired(PgHdr *pPg) {
  if (subjRequiresPage(pPg)) {
    return subjournalPage(pPg);
  } else {
    return 0;
  }
}

__attribute__((noinline)) int pagerAddPageToRollbackJournal(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;
  int rc;
  u32 cksum;
  char *pData2;
  i64 iOff = pPager->journalOff;

  pData2 = pPg->pData;
  cksum = pager_cksum(pPager, (u8 *)pData2);

  pPg->flags |= 0x008;

  rc = write32bits(pPager->jfd, iOff, pPg->pgno);
  if (rc != 0)
    return rc;
  rc = sqlite3OsWrite(pPager->jfd, pData2, pPager->pageSize, iOff + 4);
  if (rc != 0)
    return rc;
  rc = write32bits(pPager->jfd, iOff + pPager->pageSize + 4, cksum);
  if (rc != 0)
    return rc;

  ;
  ;

  ;

  pPager->journalOff += 8 + pPager->pageSize;
  pPager->nRec++;

  rc = sqlite3BitvecSet(pPager->pInJournal, pPg->pgno);
  ;

  rc |= addToSavepointBitvecs(pPager, pPg->pgno);

  return rc;
}

int pager_write(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;
  int rc = 0;

  ;

  if (pPager->eState == 2) {
    rc = pager_open_journal(pPager);
    if (rc != 0)
      return rc;
  }

  sqlite3PcacheMakeDirty(pPg);

  if (pPager->pInJournal != 0 && sqlite3BitvecTestNotNull(pPager->pInJournal, pPg->pgno) == 0) {


    if (pPg->pgno <= pPager->dbOrigSize) {
      rc = pagerAddPageToRollbackJournal(pPg);
      if (rc != 0) {
        return rc;
      }
    } else {
      if (pPager->eState != 4) {
        pPg->flags |= 0x008;
      }

      ;
    }
  }

  pPg->flags |= 0x004;

  if (pPager->nSavepoint > 0) {
    rc = subjournalPageIfRequired(pPg);
  }

  if (pPager->dbSize < pPg->pgno) {
    pPager->dbSize = pPg->pgno;
  }
  return rc;
}

__attribute__((noinline)) int pagerWriteLargeSector(PgHdr *pPg) {
  int rc = 0;
  Pgno nPageCount;
  Pgno pg1;
  int nPage = 0;
  int ii;
  int needSync = 0;
  Pager *pPager = pPg->pPager;
  Pgno nPagePerSector = (pPager->sectorSize / pPager->pageSize);

  pPager->doNotSpill |= 0x04;

  pg1 = ((pPg->pgno - 1) & ~(nPagePerSector - 1)) + 1;

  nPageCount = pPager->dbSize;
  if (pPg->pgno > nPageCount) {
    nPage = (pPg->pgno - pg1) + 1;
  } else if ((pg1 + nPagePerSector - 1) > nPageCount) {
    nPage = nPageCount + 1 - pg1;
  } else {
    nPage = nPagePerSector;
  }

  for (ii = 0; ii < nPage && rc == 0; ii++) {
    Pgno pg = pg1 + ii;
    PgHdr *pPage;
    if (pg == pPg->pgno || !sqlite3BitvecTest(pPager->pInJournal, pg)) {
      if (pg != ((pPager)->lckPgno)) {
        rc = sqlite3PagerGet(pPager, pg, &pPage, 0);
        if (rc == 0) {
          rc = pager_write(pPage);
          if (pPage->flags & 0x008) {
            needSync = 1;
          }
          sqlite3PagerUnrefNotNull(pPage);
        }
      }
    } else if ((pPage = sqlite3PagerLookup(pPager, pg)) != 0) {
      if (pPage->flags & 0x008) {
        needSync = 1;
      }
      sqlite3PagerUnrefNotNull(pPage);
    }
  }

  if (rc == 0 && needSync) {


    for (ii = 0; ii < nPage; ii++) {
      PgHdr *pPage = sqlite3PagerLookup(pPager, pg1 + ii);
      if (pPage) {
        pPage->flags |= 0x008;
        sqlite3PagerUnrefNotNull(pPage);
      }
    }
  }

  pPager->doNotSpill &= ~0x04;
  return rc;
}

int sqlite3PagerWrite(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;

  if ((pPg->flags & 0x004) != 0 && pPager->dbSize >= pPg->pgno) {
    if (pPager->nSavepoint)
      return subjournalPageIfRequired(pPg);
    return 0;
  } else if (pPager->errCode) {
    return pPager->errCode;
  } else if (pPager->sectorSize > (u32)pPager->pageSize) {


    return pagerWriteLargeSector(pPg);
  } else {
    return pager_write(pPg);
  }
}

void sqlite3PagerDontWrite(PgHdr *pPg) {
  Pager *pPager = pPg->pPager;
  if (!pPager->tempFile && (pPg->flags & 0x002) && pPager->nSavepoint == 0) {
    ;

    pPg->flags |= 0x010;
    pPg->flags &= ~0x004;
    ;
    ;
  }
}
