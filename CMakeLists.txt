cmake_minimum_required(VERSION 3.10)
project(PixotronVM C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(VM_DEBUG_ENABLE "1")
set(VM_STACK_SIZE "1024*1024")
set(VM_EXCEPT_STACK_DEPTH "30")


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -O2")

find_program(Python_EXECUTABLE python NO_CMAKE_FIND_ROOT_PATH)
# if python can't find try find python3
if (NOT Python_EXECUTABLE)
    find_program(Python_EXECUTABLE python3 NO_CMAKE_FIND_ROOT_PATH)
endif ()
#if python and python3 not found in host throw error
if (NOT Python_EXECUTABLE)
    message(FATAL_ERROR "Please ensure python/python3 install in host.")
endif ()

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFI libffi REQUIRED)
pkg_check_modules(GLIB_DEP REQUIRED glib-2.0 gobject-2.0 gio-2.0)


configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config.h)

file(GLOB SOURCES *.c lib/*.c)


if (EXISTS ${CMAKE_SOURCE_DIR}/example/build)
    file(COPY ${CMAKE_SOURCE_DIR}/example/build DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()

add_library(${PROJECT_NAME} SHARED ${SOURCES})


execute_process(COMMAND ${Python_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/op_gen.py
        ${CMAKE_SOURCE_DIR}/include/engine ERROR_QUIET
)

target_link_directories(${PROJECT_NAME} PUBLIC
        ${GLIB_DEP_LIBRARY_DIRS}
        ${FFI_LIBDIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        ${GLIB_DEP_LIBRARIES}
        ffi
)


target_include_directories(${PROJECT_NAME} PUBLIC
        include/engine
        include/api
        ${FFI_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}/include
        ${GLIB_DEP_INCLUDE_DIRS}
)

enable_testing()

add_subdirectory(pva)
add_subdirectory(tests)