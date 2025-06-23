file(GLOB TEST_SOURCE *.c)

find_package(PkgConfig REQUIRED)

# 添加测试文件
add_executable(Tests ${TEST_SOURCE})


# 链接测试目标与项目源文件
target_link_libraries(Tests
        PRIVATE
        ${PROJECT_NAME}
        m
)


target_include_directories(Tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GLIB_DEP_INCLUDE_DIRS}
)

# 添加测试
add_test(NAME Add COMMAND Tests)

