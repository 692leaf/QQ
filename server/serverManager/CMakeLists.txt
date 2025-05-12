# 添加子模块
add_subdirectory(server_tray)
add_subdirectory(database)
add_subdirectory(tcpServer)


# 创建库
add_library(ServerManager
    "ServerManager.h"
    "ServerManager.cpp"
)

# 链接子模块库
target_link_libraries(ServerManager PUBLIC
    ServerTray
    Database
    IMServer
    # 添加其他需要链接的库
    Qt::Core
    Qt::Widgets
    Qt::Sql
    Qt::Network
    Qt::Multimedia
)

# 设置包含目录（供其他模块使用）
target_include_directories(ServerManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
