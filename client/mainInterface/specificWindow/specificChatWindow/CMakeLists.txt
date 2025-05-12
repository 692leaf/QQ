# 收集源文件
file(GLOB_RECURSE SPECIFIC_CHAT_WINDOW_SOURCES *.cpp)
file(GLOB_RECURSE SPECIFIC_CHAT_WINDOW_HEADERS *.h)

# 创建库
add_library(SpecificChatWindow
    ${SPECIFIC_CHAT_WINDOW_SOURCES} 
    ${SPECIFIC_CHAT_WINDOW_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(SpecificChatWindow PUBLIC 
    Qt::Core
    Qt::Widgets
    Qt::Network
    Qt::Multimedia
    IMClient
    LocalDatabase
)

# 设置包含目录（供其他模块使用）
target_include_directories(SpecificChatWindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
