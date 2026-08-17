#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Database Connection Configuration Options (from sqlite3.h) */
enum {
  SQLITE_DBCONFIG_MAINDBNAME = 1000,            /* const char* */
  SQLITE_DBCONFIG_LOOKASIDE = 1001,             /* void* int int */
  SQLITE_DBCONFIG_ENABLE_FKEY = 1002,           /* int int* */
  SQLITE_DBCONFIG_ENABLE_TRIGGER = 1003,        /* int int* */
  SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER = 1004, /* int int* */
  SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION = 1005, /* int int* */
  SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE = 1006,      /* int int* */
  SQLITE_DBCONFIG_ENABLE_QPSG = 1007,           /* int int* */
  SQLITE_DBCONFIG_TRIGGER_EQP = 1008,           /* int int* */
  SQLITE_DBCONFIG_RESET_DATABASE = 1009,        /* int int* */
  SQLITE_DBCONFIG_DEFENSIVE = 1010,             /* int int* */
  SQLITE_DBCONFIG_WRITABLE_SCHEMA = 1011,       /* int int* */
  SQLITE_DBCONFIG_LEGACY_ALTER_TABLE = 1012,    /* int int* */
  SQLITE_DBCONFIG_DQS_DML = 1013,               /* int int* */
  SQLITE_DBCONFIG_DQS_DDL = 1014,               /* int int* */
  SQLITE_DBCONFIG_ENABLE_VIEW = 1015,           /* int int* */
  SQLITE_DBCONFIG_LEGACY_FILE_FORMAT = 1016,    /* int int* */
  SQLITE_DBCONFIG_TRUSTED_SCHEMA = 1017,        /* int int* */
  SQLITE_DBCONFIG_STMT_SCANSTATUS = 1018,       /* int int* */
  SQLITE_DBCONFIG_REVERSE_SCANORDER = 1019,     /* int int* */
  SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE = 1020,  /* int int* */
  SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE = 1021,   /* int int* */
  SQLITE_DBCONFIG_ENABLE_COMMENTS = 1022,       /* int int* */
  SQLITE_DBCONFIG_FP_DIGITS = 1023,             /* int int* */
  SQLITE_DBCONFIG_MAX = 1023,                   /* Largest DBCONFIG */
};

#ifdef __cplusplus
}
#endif
