# 收集源文件
file(GLOB_RECURSE QQ_NAVTOOLBAR_SOURCES *.cpp)
file(GLOB_RECURSE QQ_NAVTOOLBAR_HEADERS *.h)

# 创建库（或直接添加到可执行文件）
add_library(QQNavToolBar 
    ${QQ_NAVTOOLBAR_SOURCES} 
    ${QQ_NAVTOOLBAR_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(QQNavToolBar PUBLIC
    Qt::Core
    Qt::Widgets
    IMClient
    LocalDatabase
)

# 设置包含目录（供其他模块使用）
target_include_directories(QQNavToolBar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
