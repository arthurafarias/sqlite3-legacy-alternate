
#pragma once

  struct Expr;
  struct IdList;
  struct OnOrUsing;

  struct OnOrUsing {
    Expr *pOn;
    IdList *pUsing;
  };


