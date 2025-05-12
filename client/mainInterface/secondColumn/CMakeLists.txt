# 收集源文件
file(GLOB_RECURSE SECONDCOLUMN_SOURCES *.cpp)    
file(GLOB_RECURSE SECONDCOLUMN_HEADERS *.h)

# 创建库（或直接添加到可执行文件）
add_library(SecondColumn 
    ${SECONDCOLUMN_SOURCES} 
    ${SECONDCOLUMN_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(SecondColumn PUBLIC 
    Qt::Core
    Qt::Widgets
    Component_Zone
    IMClient
    LocalDatabase
)

# 设置包含目录（供其他模块使用）
target_include_directories(SecondColumn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
