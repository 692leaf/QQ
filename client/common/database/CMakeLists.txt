# 收集源文件
file(GLOB_RECURSE LOCALDATABASE_SOURCES *.cpp)
file(GLOB_RECURSE LOCALDATABASE_HEADERS *.h)

# 创建库
add_library(LocalDatabase
    ${LOCALDATABASE_SOURCES}
    ${LOCALDATABASE_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(LocalDatabase PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Sql
    # 直接链接库
    IMClient
    Emoji_Unicode_Mapping_Utils
)


# 设置包含目录（供其他模块使用）
target_include_directories(LocalDatabase PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}  # 添加根目录路径
)
