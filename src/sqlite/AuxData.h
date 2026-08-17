
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct AuxData AuxData;

  struct AuxData {
    int iAuxOp;
    int iAuxArg;
    void *pAux;
    void (*xDeleteAux)(void *);
    AuxData *pNextAux;
  };

#ifdef __cplusplus
}
#endif
