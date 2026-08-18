#pragma once

typedef struct sqlite3_io_methods sqlite3_io_methods;
typedef struct unixFile unixFile;

typedef const sqlite3_io_methods *(*finder_type)(const char *, unixFile *);


