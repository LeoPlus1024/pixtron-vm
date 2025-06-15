cmake_minimum_required(VERSION 3.10)
project(PixotronVM C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
set(VM_MAX_STACK_DEPTH "1024")

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO REQUIRED gio-2.0)

configure_file(${PROJECT_SOURCE_DIR}/Config.h.in ${CMAKE_BINARY_DIR}/include/Config.h)

file(GLOB SOURCES *.c)

enable_testing()

add_subdirectory(tests)
include_directories(AFTER "include" ${CMAKE_BINARY_DIR}/include ${GLIB_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${SOURCES})
