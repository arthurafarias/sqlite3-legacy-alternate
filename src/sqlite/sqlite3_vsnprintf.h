#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

char *sqlite3_vsnprintf(int, char *, const char *, va_list);

#ifdef __cplusplus
}
#endif
