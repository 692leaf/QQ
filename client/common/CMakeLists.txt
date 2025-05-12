# 添加子模块
add_subdirectory(database)
add_subdirectory(tcpClient)

add_library(Common INTERFACE)  # 定义接口库

# 链接子模块库
target_link_libraries(Common INTERFACE
    LocalDatabase
    IMClient
    Emoji_Unicode_Mapping_Utils
)

# 设置包含目录（供上层使用）
target_include_directories(Common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
