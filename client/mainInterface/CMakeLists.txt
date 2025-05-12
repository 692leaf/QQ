# 添加子模块
add_subdirectory(component_Zone)
add_subdirectory(secondColumn)
add_subdirectory(specificWindow)
add_subdirectory(toolBar)

# 创建库库
add_library(MainInterface
    "MainPage.h"
    "MainPage.cpp"
)

# 链接模块库
target_link_libraries(MainInterface PUBLIC
    Component_Zone
    SecondColumn
    SpecificWindow
    SpecificChatWindow
    Qt::Core
    Qt::Widgets
    SecondColumn
    QQNavToolBar
    SysTray
)

# 设置包含目录（供其他模块使用）
target_include_directories(MainInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
