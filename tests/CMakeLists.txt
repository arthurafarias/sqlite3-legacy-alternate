# Each test is a small standalone C program exercising one slice of the
# SQLite C API surface. Keeping them as one-binary-per-concern makes CTest's
# pass/fail/timing granularity meaningful and keeps failures easy to isolate.

function(add_sqlite_test name)
  add_executable(${name} ${name}.c)
  target_link_libraries(${name} PRIVATE sqlite3)
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES TIMEOUT 60 LABELS "unit")
endfunction()

# Core connection / API surface
add_sqlite_test(test_version_info)
add_sqlite_test(test_open_close)
add_sqlite_test(test_exec_callback)
add_sqlite_test(test_prepare_bind_step)
add_sqlite_test(test_transactions_savepoints)
add_sqlite_test(test_error_handling)

# Data access surface
add_sqlite_test(test_blob_io)
add_sqlite_test(test_backup_api)

# Extensibility surface
add_sqlite_test(test_custom_scalar_function)
add_sqlite_test(test_custom_aggregate_function)
add_sqlite_test(test_custom_collation)
add_sqlite_test(test_virtual_table)

# Observability / control surface
add_sqlite_test(test_hooks)
add_sqlite_test(test_busy_progress_handler)
# test_preupdate_hook: omitted -- sqlite3_preupdate_hook() and friends
# (SQLITE_ENABLE_PREUPDATE_HOOK) aren't part of this build.

# Schema / introspection surface
add_sqlite_test(test_pragma_and_meta)
# test_dbstat_vtab: omitted -- this build has no dbstat virtual table
# (SQLITE_ENABLE_DBSTAT_VTAB wasn't on when the amalgamation this project
# decomposes was preprocessed; see docs/architecture.md).

# Bundled extensions
add_sqlite_test(test_json_functions)
# test_fts5_search, test_rtree_index: omitted -- this build has no
# registered virtual table modules at all (no FTS5, RTREE, dbstat, or
# stmt vtab; see test_pragma_extras, which checks pragma_module_list is
# otherwise empty). Only auxiliary struct declarations for FTS5/RTREE
# (Fts5ExtensionApi, sqlite3_rtree_geometry, ...) made it into
# src/sqlite/ -- their virtual table engines did not.
add_sqlite_test(test_math_functions)

# Durability / concurrency surface
add_sqlite_test(test_wal_mode)
add_sqlite_test(test_multithread)

# Schema DDL / SQL-language surface
add_sqlite_test(test_triggers)
add_sqlite_test(test_views)
add_sqlite_test(test_upsert_returning)
add_sqlite_test(test_alter_table)
add_sqlite_test(test_attach_detach)
add_sqlite_test(test_conflict_clauses)
add_sqlite_test(test_explain_query_plan)

# Encoding / introspection / lifecycle surface
add_sqlite_test(test_utf16)
add_sqlite_test(test_status_counters)
add_sqlite_test(test_config_and_init)
# test_stmt_vtab: omitted -- the "sqlite_stmt" eponymous virtual table
# (SQLITE_ENABLE_STMTVTAB) isn't part of this build.

# Storage maintenance
add_sqlite_test(test_vacuum_and_serialize)

# Bundled extensions (cont'd)
# test_geopoly: omitted -- geopoly isn't part of this build (no geopoly_*
# module in src/sqlite/; not compiled into the amalgamation this project
# decomposes).
# test_fts5_advanced: omitted -- see the test_fts5_search note above.

# Access control / resource limits / introspection (cont'd)
add_sqlite_test(test_authorizer_and_limits)
add_sqlite_test(test_introspection)

# SQL-language surface (cont'd)
add_sqlite_test(test_without_rowid_strict)
add_sqlite_test(test_cte_setops_window)

# WAL snapshot isolation
# test_snapshot_api: omitted -- sqlite3_snapshot_get()/_open() (full
# SQLITE_ENABLE_SNAPSHOT support) aren't part of this build; only the
# _free()/_cmp() declarations survived.

# Fault-injection: out-of-memory handling across a broad workload
add_sqlite_test(test_oom_fault_injection)

# Connection configuration / pragma surface (cont'd)
add_sqlite_test(test_db_config)
add_sqlite_test(test_pragma_extras)

# SQL-language surface (cont'd)
add_sqlite_test(test_foreign_key_actions)
add_sqlite_test(test_index_features)

# Extensibility surface (cont'd)
# test_rtree_geometry_callback: omitted -- see the test_rtree_index note
# above; neither the R*Tree module nor sqlite3_rtree_geometry_callback()
# is part of this build.
add_sqlite_test(test_value_subtype_and_dup)

# Runtime extension loading: built as a real shared object so
# sqlite3_load_extension()'s dlopen/dlsym path is genuinely exercised.
add_library(sample_extension MODULE extensions/sample_extension.c)
target_link_libraries(sample_extension PRIVATE sqlite3)
set_target_properties(sample_extension PROPERTIES
  PREFIX ""
  POSITION_INDEPENDENT_CODE ON
)

add_executable(test_load_extension test_load_extension.c)
target_link_libraries(test_load_extension PRIVATE sqlite3)
target_include_directories(test_load_extension PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)
add_dependencies(test_load_extension sample_extension)
target_compile_definitions(test_load_extension PRIVATE
  SAMPLE_EXTENSION_PATH="$<TARGET_FILE:sample_extension>")
add_test(NAME test_load_extension COMMAND test_load_extension)
set_tests_properties(test_load_extension PROPERTIES TIMEOUT 60 LABELS "unit")

# Shared-cache mode / unlock_notify (process-wide state, kept isolated)
# test_shared_cache_unlock_notify: omitted -- sqlite3_unlock_notify()
# (SQLITE_ENABLE_UNLOCK_NOTIFY) isn't part of this build.

# Legacy convenience API / connection introspection
add_sqlite_test(test_get_table)
add_sqlite_test(test_txn_state_and_memory)
add_sqlite_test(test_column_metadata_extras)

# SQL-language / built-in function surface (cont'd)
add_sqlite_test(test_date_time_functions)
add_sqlite_test(test_format_and_literals)

# Extensibility / connection-internals surface (cont'd)
add_sqlite_test(test_custom_vfs_registration)
add_sqlite_test(test_udf_error_reporting_and_auxdata)

# Fault-injection (cont'd): a second, differently-shaped workload sweep
add_sqlite_test(test_oom_fault_injection_extended)
