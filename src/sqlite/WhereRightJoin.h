
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct WhereRightJoin WhereRightJoin;
  struct WhereRightJoin {
    int iMatch;
    int regBloom;
    int regReturn;
    int addrSubrtn;
    int endSubrtn;
  };

#ifdef __cplusplus
}
#endif
