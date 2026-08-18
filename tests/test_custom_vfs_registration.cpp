#include "test_common.h"

/* A pass-through VFS: every method just forwards to the default VFS. It
 * doesn't need to do anything interesting itself -- the point is to
 * exercise sqlite3_vfs_register()/_unregister()/_find() and the VFS
 * dispatch-by-name machinery, none of which is reachable while every test
 * in this suite uses the implicit default VFS. */

static sqlite3_vfs *g_default_vfs;

static int shim_open(sqlite3_vfs *vfs, sqlite3_filename zName, sqlite3_file *file,
                      int flags, int *outFlags) {
  return g_default_vfs->xOpen(g_default_vfs, zName, file, flags, outFlags);
}
static int shim_delete(sqlite3_vfs *vfs, const char *zName, int syncDir) {
  return g_default_vfs->xDelete(g_default_vfs, zName, syncDir);
}
static int shim_access(sqlite3_vfs *vfs, const char *zName, int flags, int *pRes) {
  return g_default_vfs->xAccess(g_default_vfs, zName, flags, pRes);
}
static int shim_fullpathname(sqlite3_vfs *vfs, const char *zName, int nOut, char *zOut) {
  return g_default_vfs->xFullPathname(g_default_vfs, zName, nOut, zOut);
}
static void *shim_dlopen(sqlite3_vfs *vfs, const char *zFilename) {
  return g_default_vfs->xDlOpen(g_default_vfs, zFilename);
}
static void shim_dlerror(sqlite3_vfs *vfs, int nByte, char *zErrMsg) {
  g_default_vfs->xDlError(g_default_vfs, nByte, zErrMsg);
}
static void (*shim_dlsym(sqlite3_vfs *vfs, void *p, const char *zSym))(void) {
  return g_default_vfs->xDlSym(g_default_vfs, p, zSym);
}
static void shim_dlclose(sqlite3_vfs *vfs, void *p) {
  g_default_vfs->xDlClose(g_default_vfs, p);
}
static int shim_randomness(sqlite3_vfs *vfs, int nByte, char *zOut) {
  return g_default_vfs->xRandomness(g_default_vfs, nByte, zOut);
}
static int shim_sleep(sqlite3_vfs *vfs, int microseconds) {
  return g_default_vfs->xSleep(g_default_vfs, microseconds);
}
static int shim_currenttime(sqlite3_vfs *vfs, double *pTime) {
  return g_default_vfs->xCurrentTime(g_default_vfs, pTime);
}
static int shim_getlasterror(sqlite3_vfs *vfs, int n, char *z) {
  return g_default_vfs->xGetLastError ? g_default_vfs->xGetLastError(g_default_vfs, n, z) : 0;
}

static sqlite3_vfs g_shim_vfs; /* zeroed; filled in main() from g_default_vfs */

int main(void) {
  g_default_vfs = sqlite3_vfs_find(NULL); /* the current default */
  TEST_ASSERT(g_default_vfs != NULL);

  g_shim_vfs.iVersion = 1;
  g_shim_vfs.szOsFile = g_default_vfs->szOsFile;
  g_shim_vfs.mxPathname = g_default_vfs->mxPathname;
  g_shim_vfs.zName = "shim_vfs";
  g_shim_vfs.xOpen = shim_open;
  g_shim_vfs.xDelete = shim_delete;
  g_shim_vfs.xAccess = shim_access;
  g_shim_vfs.xFullPathname = shim_fullpathname;
  g_shim_vfs.xDlOpen = shim_dlopen;
  g_shim_vfs.xDlError = shim_dlerror;
  g_shim_vfs.xDlSym = shim_dlsym;
  g_shim_vfs.xDlClose = shim_dlclose;
  g_shim_vfs.xRandomness = shim_randomness;
  g_shim_vfs.xSleep = shim_sleep;
  g_shim_vfs.xCurrentTime = shim_currenttime;
  g_shim_vfs.xGetLastError = shim_getlasterror;

  TEST_ASSERT(sqlite3_vfs_find("shim_vfs") == NULL); /* not registered yet */
  TEST_ASSERT(sqlite3_vfs_register(&g_shim_vfs, 0 /* not default */) == SQLITE_OK);
  TEST_ASSERT(sqlite3_vfs_find("shim_vfs") == &g_shim_vfs);
  TEST_ASSERT(sqlite3_vfs_find(NULL) == g_default_vfs); /* still the process default */

  /* Open a connection explicitly through the shim by name. */
  sqlite3 *db = NULL;
  int rc = sqlite3_open_v2(":memory:", &db,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                            "shim_vfs");
  TEST_ASSERT_MSG(rc == SQLITE_OK, "open through shim_vfs failed: %d", rc);
  test_exec_ok(db, "CREATE TABLE t(x)");
  test_exec_ok(db, "INSERT INTO t VALUES (1)");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT x FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1);
  sqlite3_finalize(stmt);
  test_close_ok(db);

  /* Registering as the new default, then restoring the original. */
  TEST_ASSERT(sqlite3_vfs_register(&g_shim_vfs, 1 /* make default */) == SQLITE_OK);
  TEST_ASSERT(sqlite3_vfs_find(NULL) == &g_shim_vfs);
  TEST_ASSERT(sqlite3_vfs_register(g_default_vfs, 1) == SQLITE_OK); /* restore */
  TEST_ASSERT(sqlite3_vfs_find(NULL) == g_default_vfs);

  TEST_ASSERT(sqlite3_vfs_unregister(&g_shim_vfs) == SQLITE_OK);
  TEST_ASSERT(sqlite3_vfs_find("shim_vfs") == NULL);

  TEST_PASS("test_custom_vfs_registration");
  return 0;
}
