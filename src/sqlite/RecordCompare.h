#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UnpackedRecord UnpackedRecord;

typedef int (*RecordCompare)(int, const void *, UnpackedRecord *);

#ifdef __cplusplus
}
#endif
