
#pragma once

#include "sqlite/u8.h"
  struct TreeView;
  struct TreeView {
    int iLevel;    /* Which level of the tree we are on */
    u8 bLine[100]; /* Draw vertical in column i if bLine[i] is true */
  };


