# 收集源文件
file(GLOB_RECURSE IMSERVER_SOURCES *.cpp) 
file(GLOB_RECURSE IMSERVER_HEADERS *.h)

# 创建库
add_library(IMServer
    ${IMSERVER_SOURCES}
    ${IMSERVER_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(IMServer PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Network
    Database
)

# 设置包含目录
target_include_directories(IMServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
