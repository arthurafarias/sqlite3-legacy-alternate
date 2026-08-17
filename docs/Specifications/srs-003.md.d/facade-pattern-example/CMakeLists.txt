cmake_minimum_required(VERSION 3.24)
project(srs-003-facade-example C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(facade_example STATIC db_limits.c hash.c db.c stmt.c)
target_include_directories(facade_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(facade_example_verify verify.c)
target_link_libraries(facade_example_verify PRIVATE facade_example)

enable_testing()
add_test(NAME facade_example_verify COMMAND facade_example_verify)
