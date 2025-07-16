add_executable(Tests test.c)


target_link_libraries(Tests
        PRIVATE
        ${PROJECT_NAME}
)

target_include_directories(Tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GLIB_DEP_INCLUDE_DIRS}
)


add_test(NAME Add COMMAND Tests)

