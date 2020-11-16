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

add_library(microxcode::library ALIAS microxcode)

target_include_directories(microxcode
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        /usr/local/include)

target_link_directories(microxcode
        PUBLIC
        /usr/local/lib)

target_link_libraries(microxcode
        -lplist-2.0)

add_subdirectory(src)