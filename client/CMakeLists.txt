cmake_minimum_required(VERSION 3.19)
project(Client LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 UI、资源和 MOC（统一在此配置）
set(CMAKE_AUTOUIC ON)   # 自动处理 .ui 文件
set(CMAKE_AUTORCC ON)   # 自动处理 .qrc 文件
set(CMAKE_AUTOMOC ON)   # 自动处理 Q_OBJECT 宏

# 添加 Qt 依赖（必须放在 add_subdirectory 前）
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Sql
    Network
    Multimedia
)

# 可执行文件
add_executable(Client
    "main.cpp"              # 主入口文件
    "resource.qrc"          # 资源文件
    "MessageType.h"         # 枚举类型

    # 数据包
    "Packege.h"
    "Packege.cpp"
    icon.rc
)

# 添加子模块
add_subdirectory(mainInterface)
add_subdirectory(loginInterface)
add_subdirectory(common)

# 添加 Qt 模块依赖
target_link_libraries(Client PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
    Qt::Sql
    Qt::Multimedia
)

# 链接子模块库
target_link_libraries(Client PRIVATE
    MainInterface
    LoginInterface
    Common
)

