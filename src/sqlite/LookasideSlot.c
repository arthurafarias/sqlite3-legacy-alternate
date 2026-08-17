#define _GNU_SOURCE 1
#include "sqlite/LookasideSlot.h"
#include "sqlite/u32.h"
u32 countLookasideSlots(LookasideSlot *p) {
  u32 cnt = 0;
  while (p) {
    p = p->pNext;
    cnt++;
  }
  return cnt;
}
