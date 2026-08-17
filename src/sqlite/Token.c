#define _GNU_SOURCE 1

#include "sqlite/Token.h"

#include "sqlite/sqlite3.h"
void sqlite3DequoteToken(Token *p) {
  unsigned int i;
  if (p->n < 2)
    return;
  if (!(sqlite3CtypeMap[(unsigned char)(p->z[0])] & 0x80))
    return;
  for (i = 1; i < p->n - 1; i++) {
    if ((sqlite3CtypeMap[(unsigned char)(p->z[i])] & 0x80))
      return;
  }
  p->n -= 2;
  p->z++;
}

void sqlite3TokenInit(Token *p, char *z) {
  p->z = z;
  p->n = sqlite3Strlen30(z);
}
