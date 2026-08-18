
#pragma once

  struct AuxData;

  struct AuxData {
    int iAuxOp;
    int iAuxArg;
    void *pAux;
    void (*xDeleteAux)(void *);
    AuxData *pNextAux;
  };


