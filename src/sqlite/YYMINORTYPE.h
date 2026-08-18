#pragma once

#include "sqlite/Cte.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FrameBound.h"
#include "sqlite/IdList.h"
#include "sqlite/OnOrUsing.h"
#include "sqlite/Select.h"
#include "sqlite/SrcList.h"
#include "sqlite/Token.h"
#include "sqlite/TrigEvent.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/Upsert.h"
#include "sqlite/Window.h"
#include "sqlite/With.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
typedef union YYMINORTYPE YYMINORTYPE;

union YYMINORTYPE {
  int yyinit;
  Token yy0;
  ExprList *yy14;
  With *yy59;
  Cte *yy67;
  Upsert *yy122;
  IdList *yy132;
  int yy144;
  const char *yy168;
  SrcList *yy203;
  Window *yy211;
  OnOrUsing yy269;
  struct TrigEvent yy286;
  struct {
    int value;
    int mask;
  } yy383;
  u32 yy391;
  TriggerStep *yy427;
  Expr *yy454;
  u8 yy462;
  struct FrameBound yy509;
  Select *yy555;
};

