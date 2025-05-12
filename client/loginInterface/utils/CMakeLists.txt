# 收集源文件
file(GLOB_RECURSE UTILS_SOURCES *.cpp)  
file(GLOB_RECURSE UTILS_HEADERS *.h)

# 创建库
add_library(Utils
    ${UTILS_SOURCES} 
    ${UTILS_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(Utils PUBLIC
    Qt::Core
    Qt::Widgets
)

# 设置包含目录（供其他模块使用）
target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
