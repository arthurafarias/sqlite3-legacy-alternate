cmake_minimum_required(VERSION 3.24)
project(srs-009-coverage-harness-example C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(record_store STATIC record_store.c)
target_include_directories(record_store PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(record_store_test test_record_store.c)
target_link_libraries(record_store_test PRIVATE record_store)

add_executable(record_store_bench_api bench_api_functions.c)
target_link_libraries(record_store_bench_api PRIVATE record_store)

add_executable(record_store_bench_insert_retrieve bench_record_insert_retrieve.c)
target_link_libraries(record_store_bench_insert_retrieve PRIVATE record_store)

enable_testing()
add_test(NAME record_store_test COMMAND record_store_test)
