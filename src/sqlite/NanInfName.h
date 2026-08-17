
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NanInfName NanInfName;

struct NanInfName {
  char c1;
  char c2;
  char n;
  char eType;
  char nRepl;
  char *zMatch;
  char *zRepl;
};

#ifdef __cplusplus
}
#endif
