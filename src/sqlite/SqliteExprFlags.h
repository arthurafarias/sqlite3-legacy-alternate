#pragma once

/* Expr.flags Bit Values (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). This is the subset referenced by
   this library's sources. */
enum {
  EP_OuterON = 0x000001,   /* Originates in ON/USING clause of outer join */
  EP_InnerON = 0x000002,   /* Originates in ON/USING of an inner join */
  EP_HasFunc = 0x000008,   /* Contains one or more functions of any kind */
  EP_FixedCol = 0x000020,  /* TK_Column with a known fixed value */
  EP_VarSelect = 0x000040, /* pSelect is correlated, not constant */
  EP_DblQuoted = 0x000080, /* token.z was originally in "..." */
  EP_Collate = 0x000200,   /* Tree contains a TK_COLLATE operator */
  EP_IntValue = 0x000800,  /* Integer value contained in u.iValue */
  EP_xIsSelect = 0x001000, /* x.pSelect is valid (otherwise x.pList is) */
  EP_Skip = 0x002000,      /* Operator does not contribute to affinity */
  EP_Reduced = 0x004000,   /* Expr struct is EXPR_REDUCEDSIZE bytes only */
  EP_TokenOnly = 0x010000, /* Expr struct is EXPR_TOKENONLYSIZE bytes only */
  EP_FullSize = 0x020000,  /* Expr structure must remain full sized */
  EP_IfNullRow = 0x040000, /* The TK_IF_NULL_ROW opcode */
  EP_Unlikely = 0x080000,  /* unlikely() or likelihood() function */
  EP_CanBeNull = 0x200000, /* Can be null despite NOT NULL constraint */
  EP_Subquery = 0x400000,  /* Tree contains a TK_SELECT operator */
  EP_Quoted = 0x4000000,   /* TK_ID was originally quoted */
  EP_IsTrue = 0x10000000,  /* Always has boolean value of TRUE */
  EP_IsFalse = 0x20000000, /* Always has boolean value of FALSE */
};


