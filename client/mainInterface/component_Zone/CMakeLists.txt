# 收集源文件
file(GLOB_RECURSE COMPONENT_ZONE_SOURCES *.cpp)    
file(GLOB_RECURSE COMPONENT_ZONE_HEADERS *.h)

# 添加子模块
add_subdirectory(emoji_unicode_mapping_utils)
add_subdirectory(system_tray)

# 创建库
add_library(Component_Zone 
    ${COMPONENT_ZONE_SOURCES} 
    ${COMPONENT_ZONE_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(Component_Zone PUBLIC
    Emoji_Unicode_Mapping_Utils
    SysTray
    Qt::Core
    Qt::Widgets
    IMClient
)

# 设置包含目录（供其他模块使用）
target_include_directories(Component_Zone PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
