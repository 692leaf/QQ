# 收集源文件
file(GLOB_RECURSE DATABASE_SOURCES *.cpp)
file(GLOB_RECURSE DATABASE_HEADERS *.h)

# 创建库
add_library(Database 
    ${DATABASE_SOURCES} 
    ${DATABASE_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(Database PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Sql
    Qt::Network
)


# 设置包含目录（供其他模块使用）
target_include_directories(Database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server  # 直接指定父项目路径
)
