cmake_minimum_required(VERSION 3.16)
project(microxcode)

set(CMAKE_C_STANDARD 11)

add_library(microxcode SHARED
        src/developer_path.c
        src/getters.c
        src/logging_printf.c
        src/runners.c
        src/stripext.c
        src/typedefs.c
        src/validators.c
        src/verbose_printf.c)

add_executable(runner_example examples/git.c)

set(LIBPLIST_INCLUDE "/usr/local/include" CACHE FILEPATH "Sets the libplist include directory")
set(LIBPLIST_LIB "/usr/local/lib" CACHE FILEPATH "Sets the libplist lib location")

add_library(microxcode::library ALIAS microxcode)

target_include_directories(microxcode
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${LIBPLIST_INCLUDE})

target_link_directories(microxcode
        PUBLIC
        ${LIBPLIST_LIB})

target_link_libraries(microxcode
        -lplist-2.0)

target_link_libraries(runner_example microxcode)

add_subdirectory(src)
add_subdirectory(examples)