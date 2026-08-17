#define _GNU_SOURCE 1

#include "sqlite/sqlite3_io_methods.h"

#include "sqlite/sqlite3_file.h"
const sqlite3_io_methods posixIoMethods = {
    3, unixClose, unixRead, unixWrite, unixTruncate, unixSync, unixFileSize, unixLock, unixUnlock, unixCheckReservedLock, unixFileControl, unixSectorSize, unixDeviceCharacteristics, unixShmMap, unixShmLock, unixShmBarrier, unixShmUnmap, unixFetch, unixUnfetch,
};

const sqlite3_io_methods nolockIoMethods = {
    3, nolockClose, unixRead, unixWrite, unixTruncate, unixSync, unixFileSize, nolockLock, nolockUnlock, nolockCheckReservedLock, unixFileControl, unixSectorSize, unixDeviceCharacteristics, 0, unixShmLock, unixShmBarrier, unixShmUnmap, unixFetch, unixUnfetch,
};

const sqlite3_io_methods dotlockIoMethods = {
    1, dotlockClose, unixRead, unixWrite, unixTruncate, unixSync, unixFileSize, dotlockLock, dotlockUnlock, dotlockCheckReservedLock, unixFileControl, unixSectorSize, unixDeviceCharacteristics, 0, unixShmLock, unixShmBarrier, unixShmUnmap, unixFetch, unixUnfetch,
};

const sqlite3_io_methods memdb_io_methods = {3, memdbClose, memdbRead, memdbWrite, memdbTruncate, memdbSync, memdbFileSize, memdbLock, memdbUnlock, 0, memdbFileControl, 0, memdbDeviceCharacteristics, 0, 0, 0, 0, memdbFetch, memdbUnfetch};

const struct sqlite3_io_methods MemJournalMethods = {1, memjrnlClose, memjrnlRead, memjrnlWrite, memjrnlTruncate, memjrnlSync, memjrnlFileSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
