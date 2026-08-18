#pragma once

/* Parser Token Codes (from the sqlite3 amalgamation's parse.h, internal/private -
   not part of the public sqlite3.h API and not guaranteed stable across
   versions). This is the subset referenced by this library's sources. */
enum {
  TK_AND = 44,
  TK_OR = 43,
  TK_NOTNULL = 52,
  TK_ID = 60,
  TK_CAST = 36,
  TK_CONCAT = 112,
  TK_COLLATE = 114,
  TK_STRING = 118,
  TK_NULL = 122,
  TK_VARIABLE = 157,
  TK_INTEGER = 156,
  TK_FLOAT = 154,
  TK_BLOB = 155,
  TK_CASE = 158,
  TK_UPLUS = 173,
  TK_UMINUS = 174,
  TK_COLUMN = 168,
  TK_AGG_COLUMN = 170,
  TK_AGG_FUNCTION = 169,
  TK_SELECT = 139,
  TK_FUNCTION = 172,
  TK_TRUEFALSE = 171,
  TK_REGISTER = 176,
  TK_VECTOR = 177,
  TK_SELECT_COLUMN = 178,
  TK_IF_NULL_ROW = 179,
};


