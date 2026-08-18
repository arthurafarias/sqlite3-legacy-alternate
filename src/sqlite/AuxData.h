
#pragma once

  typedef struct AuxData AuxData;

  struct AuxData {
    int iAuxOp;
    int iAuxArg;
    void *pAux;
    void (*xDeleteAux)(void *);
    AuxData *pNextAux;
  };


