#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* SrcItem.fg.jointype Bit Values (from the sqlite3 amalgamation's
   sqliteInt.h, internal/private - not part of the public sqlite3.h API
   and not guaranteed stable across versions). */
enum {
  JT_INNER = 0x01,   /* Any kind of inner or cross join */
  JT_CROSS = 0x02,    /* Explicit use of the CROSS keyword */
  JT_NATURAL = 0x04, /* True for a "natural" join */
  JT_LEFT = 0x08,     /* Left outer join */
  JT_RIGHT = 0x10,    /* Right outer join */
  JT_OUTER = 0x20,    /* The "OUTER" keyword is present */
  JT_LTORJ = 0x40,    /* One of the LEFT operands of a RIGHT JOIN */
  JT_ERROR = 0x80,    /* unknown or unsupported join type */
};

#ifdef __cplusplus
}
#endif
