file(GLOB TEST_SOURCE *.c)

# 添加测试文件
add_executable(Tests ${TEST_SOURCE})


# 链接测试目标与项目源文件
target_link_libraries(Tests
        PRIVATE
        ${PROJECT_NAME}
)

target_include_directories(Tests SYSTEM AFTER PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 添加测试
add_test(NAME Add COMMAND Tests)
