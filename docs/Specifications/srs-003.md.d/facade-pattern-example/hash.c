#include "hash.h"

/* Used only inside this translation unit: stays private here rather than
 * living in a shared constants header. */
static const unsigned FNV1A_OFFSET_BASIS = 2166136261u;
static const unsigned FNV1A_PRIME = 16777619u;

unsigned fnv1a_hash(const char *s) {
  unsigned h = FNV1A_OFFSET_BASIS;
  for (; s && *s; s++) {
    h ^= (unsigned char)*s;
    h *= FNV1A_PRIME;
  }
  return h;
}
