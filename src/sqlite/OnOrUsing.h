
#pragma once

  typedef struct Expr Expr;
  typedef struct IdList IdList;
  typedef struct OnOrUsing OnOrUsing;

  struct OnOrUsing {
    Expr *pOn;
    IdList *pUsing;
  };


