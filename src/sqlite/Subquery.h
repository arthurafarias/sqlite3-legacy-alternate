
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct Select Select;
  typedef struct Subquery Subquery;

  struct Subquery {
    Select *pSelect;
    int addrFillSub;
    int regReturn;
    int regResult;
  };

#ifdef __cplusplus
}
#endif
