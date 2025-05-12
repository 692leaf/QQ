# 收集源文件
file(GLOB_RECURSE SERVERTRAY_SOURCES *.cpp)
file(GLOB_RECURSE SERVERTRAY_HEADERS *.h)

# 创建库
add_library(ServerTray
    ${SERVERTRAY_SOURCES}
    ${SERVERTRAY_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(ServerTray PUBLIC
    Qt::Core
    Qt::Widgets
)

# 设置包含目录（供其他模块使用）
target_include_directories(ServerTray PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
