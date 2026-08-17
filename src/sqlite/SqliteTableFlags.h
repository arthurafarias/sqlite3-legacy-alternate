#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Table.tabFlags Bit Values (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). Only the subset referenced by this
   library's sources is listed here; extend as more of them are needed. */
enum {
  TF_HasGenerated = 0x00000060, /* Combo: HasVirtual + HasStored */
};

#ifdef __cplusplus
}
#endif
