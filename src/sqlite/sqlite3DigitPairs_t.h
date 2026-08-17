#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef union sqlite3DigitPairs_t sqlite3DigitPairs_t;

union sqlite3DigitPairs_t {
  char a[201];
  short int forceAlignment;
};

#ifdef __cplusplus
}
#endif
