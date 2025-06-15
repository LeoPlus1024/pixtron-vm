file(GLOB TEST_SOURCE *.c)

find_package(PkgConfig REQUIRED)

pkg_search_module(GIO REQUIRED gio-2.0)
pkg_search_module(GLIB REQUIRED glib-2.0)

# 添加测试文件
add_executable(Tests ${TEST_SOURCE})


# 链接测试目标与项目源文件
target_link_libraries(Tests
        PRIVATE
        ${PROJECT_NAME}
        ${GLIB_LIBRARIES}
        ${GIO_LIBRARIES}
)

target_include_directories(Tests SYSTEM AFTER PRIVATE ${CMAKE_SOURCE_DIR}/include ${GLIB_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS})

# 添加测试
add_test(NAME Add COMMAND Tests)

