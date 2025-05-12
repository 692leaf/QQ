# 收集源文件
file(GLOB_RECURSE IMCLEINT_SOURCES *.cpp) 
file(GLOB_RECURSE IMCLEINT_HEADERS *.h)

# 创建库
add_library(IMClient
    ${IMCLEINT_SOURCES}
    ${IMCLEINT_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(IMClient PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Network
)

# 设置包含目录（供其他模块使用）
target_include_directories(IMClient PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/client  # 直接指定父项目路径
)
