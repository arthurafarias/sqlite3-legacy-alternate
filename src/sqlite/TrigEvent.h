#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/IdList.h"
typedef struct TrigEvent TrigEvent;
struct TrigEvent {
  int a;
  IdList *b;
};

#ifdef __cplusplus
}
#endif