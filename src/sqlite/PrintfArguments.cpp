#define _GNU_SOURCE 1
#include "sqlite/PrintfArguments.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
sqlite3_int64 getIntArg(PrintfArguments *p) {
  if (p->nArg <= p->nUsed)
    return 0;
  return sqlite3_value_int64(p->apArg[p->nUsed++]);
}

double getDoubleArg(PrintfArguments *p) {
  if (p->nArg <= p->nUsed)
    return 0.0;
  return sqlite3_value_double(p->apArg[p->nUsed++]);
}

char *getTextArg(PrintfArguments *p) {
  if (p->nArg <= p->nUsed)
    return 0;
  return (char *)sqlite3_value_text(p->apArg[p->nUsed++]);
}
