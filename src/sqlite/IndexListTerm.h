
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct Index Index;
  typedef struct IndexListTerm IndexListTerm;
  struct IndexListTerm {
    Index *p;
    int ix;
  };

#ifdef __cplusplus
}
#endif
