#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*sqlite3_callback)(void *, int, char **, char **);

#ifdef __cplusplus
}
#endif