#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*sqlite3_xauth)(void *, int, const char *, const char *, const char *, const char *);

#ifdef __cplusplus
}
#endif