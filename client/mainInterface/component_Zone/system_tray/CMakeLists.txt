# 收集源文件
file(GLOB_RECURSE SYSTRAY_SOURCES *.cpp)
file(GLOB_RECURSE SYSTRAY_HEADERS *.h)

# 创建库
add_library(SysTray
    ${SYSTRAY_SOURCES} 
    ${SYSTRAY_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(SysTray PUBLIC 
    Qt::Core
    Qt::Widgets
)

# 设置包含目录（供其他模块使用）
target_include_directories(SysTray PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
