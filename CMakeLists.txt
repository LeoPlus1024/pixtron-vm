cmake_minimum_required(VERSION 3.31)
project(PixotronVM C)

set(CMAKE_C_STANDARD 11)
set(VM_MAX_STACK_DEPTH "1024")

enable_testing()

configure_file(${PROJECT_SOURCE_DIR}/Config.h.in ${CMAKE_BINARY_DIR}/include/Config.h)

file(GLOB SOURCES *.c)

add_subdirectory(tests)
include_directories(AFTER "include" ${CMAKE_BINARY_DIR}/include)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
