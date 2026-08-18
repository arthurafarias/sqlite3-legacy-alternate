
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct Expr Expr;
  typedef struct IdList IdList;
  typedef struct OnOrUsing OnOrUsing;

  struct OnOrUsing {
    Expr *pOn;
    IdList *pUsing;
  };

#ifdef __cplusplus
}
#endif
