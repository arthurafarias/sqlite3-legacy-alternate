cmake_minimum_required(VERSION 3.24)
project(sqlite3-legacy-alternate VERSION 3.53.4)

find_package(Threads REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_C_SOURCES src/*.c)
file(GLOB_RECURSE ${PROJECT_NAME}_CXX_SOURCES src/*.cpp)
set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_C_SOURCES} ${${PROJECT_NAME}_CXX_SOURCES})

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -pedantic -Werror)
find_library(SQLITE3_LEGACY_AMALGAMATION_MATH_LIB m)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${SQLITE3_LEGACY_AMALGAMATION_MATH_LIB})

# SRS-003 verification executable: a minimal sqlite3 application built
# against nothing but the public sqlite3 C API, proving the façade
# visibility pass still produces a working, linkable library.
add_executable(${PROJECT_NAME}-verify verify/main.c)
target_compile_options(${PROJECT_NAME}-verify PRIVATE -Wall -pedantic -Werror)
target_link_libraries(${PROJECT_NAME}-verify PRIVATE ${PROJECT_NAME})

enable_testing()
add_test(NAME ${PROJECT_NAME}-verify COMMAND ${PROJECT_NAME}-verify)