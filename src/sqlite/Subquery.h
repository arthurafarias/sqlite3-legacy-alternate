
#pragma once

  typedef struct Select Select;
  typedef struct Subquery Subquery;

  struct Subquery {
    Select *pSelect;
    int addrFillSub;
    int regReturn;
    int regResult;
  };


