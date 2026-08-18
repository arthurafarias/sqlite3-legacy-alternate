
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct PGroup PGroup;
  typedef struct PgHdr1 PgHdr1;
  typedef struct PCache1 PCache1;
  struct PCache1 {
    PGroup *pGroup;
    unsigned int *pnPurgeable;
    int szPage;
    int szExtra;
    int szAlloc;
    int bPurgeable;
    unsigned int nMin;
    unsigned int nMax;
    unsigned int n90pct;
    unsigned int iMaxKey;
    unsigned int nPurgeableDummy;

    unsigned int nRecyclable;
    unsigned int nPage;
    unsigned int nHash;
    PgHdr1 **apHash;
    PgHdr1 *pFree;
    void *pBulk;
  };

  int pcache1InitBulk(PCache1 * pCache);
  PgHdr1 *pcache1AllocPage(PCache1 * pCache, int benignMalloc);
  int pcache1UnderMemoryPressure(PCache1 * pCache);
  void pcache1ResizeHash(PCache1 * p);
  void pcache1EnforceMaxPage(PCache1 * pCache);
  void pcache1TruncateUnsafe(PCache1 * pCache, unsigned int iLimit);
  __attribute__((noinline)) PgHdr1 *pcache1FetchStage2(PCache1 * pCache, unsigned int iKey, int createFlag);

  void *pcache1Alloc(int nByte);

#ifdef __cplusplus
}
#endif
