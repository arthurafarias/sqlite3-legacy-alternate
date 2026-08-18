
#pragma once

  struct Select;
  struct Subquery;

  struct Subquery {
    Select *pSelect;
    int addrFillSub;
    int regReturn;
    int regResult;
  };


