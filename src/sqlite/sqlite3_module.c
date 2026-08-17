#define _GNU_SOURCE 1

#include "sqlite/sqlite3_module.h"

#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/sqlite3_vtab_cursor.h"
const sqlite3_module pragmaVtabModule = {0, 0, pragmaVtabConnect, pragmaVtabBestIndex, pragmaVtabDisconnect, 0, pragmaVtabOpen, pragmaVtabClose, pragmaVtabFilter, pragmaVtabNext, pragmaVtabEof, pragmaVtabColumn, pragmaVtabRowid, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

sqlite3_module jsonEachModule = {0, 0, jsonEachConnect, jsonEachBestIndex, jsonEachDisconnect, 0, jsonEachOpen, jsonEachClose, jsonEachFilter, jsonEachNext, jsonEachEof, jsonEachColumn, jsonEachRowid, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
