#pragma once

struct sqlite3_io_methods;
struct unixFile;

typedef const sqlite3_io_methods *(*finder_type)(const char *, unixFile *);


