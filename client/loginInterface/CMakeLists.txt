# 收集源文件
file(GLOB_RECURSE LOGININTERFACE_SOURCES *.cpp)  
file(GLOB_RECURSE LOGININTERFACE_HEADERS *.h)

# 添加子模块
add_subdirectory(utils)

# 创建库
add_library(LoginInterface 
    ${LOGININTERFACE_SOURCES} 
    ${LOGININTERFACE_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(LoginInterface PUBLIC
    Qt::Core
    Qt::Widgets
    IMClient
    LocalDatabase
    Utils
)

# 设置包含目录（供其他模块使用）
target_include_directories(LoginInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
