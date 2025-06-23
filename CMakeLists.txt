cmake_minimum_required(VERSION 3.10)
project(PixotronVM C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(VM_MAX_STACK_DEPTH "1024")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB_DEP REQUIRED glib-2.0 gobject-2.0 gio-2.0)


configure_file(${PROJECT_SOURCE_DIR}/Config.h.in ${CMAKE_BINARY_DIR}/include/Config.h)

file(GLOB SOURCES *.c lib/*.c)


if (EXISTS ${CMAKE_SOURCE_DIR}/example/build)
    file(COPY ${CMAKE_SOURCE_DIR}/example/build DESTINATION ${CMAKE_BINARY_DIR}/tests)
endif ()


enable_testing()

add_subdirectory(tests)

add_library(${PROJECT_NAME} SHARED ${SOURCES}
        lib/math.c
        include/engine/helper.h
        helper.c)

target_link_libraries(${PROJECT_NAME} PUBLIC
        ${GLIB_DEP_LIBRARIES}
)


target_include_directories(${PROJECT_NAME} PUBLIC
        include/engine
        include/api
        ${CMAKE_BINARY_DIR}/include
        ${GLIB_DEP_INCLUDE_DIRS}
)