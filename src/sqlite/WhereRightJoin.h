
#pragma once

  typedef struct WhereRightJoin WhereRightJoin;
  struct WhereRightJoin {
    int iMatch;
    int regBloom;
    int regReturn;
    int addrSubrtn;
    int endSubrtn;
  };


