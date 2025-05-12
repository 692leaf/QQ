# 添加子模块
add_subdirectory(specificChatWindow)

# 创建库
add_library(SpecificWindow
    "DefaultWindow.h"
    "DefaultWindow.cpp"
    "Frienddetailwidget.h"
    "Frienddetailwidget.cpp"
    "NotificationManager.h"
    "NotificationManager.cpp"
)

# 链接子模块库
target_link_libraries(SpecificWindow PUBLIC
    SpecificChatWindow
    Qt::Core
    Qt::Widgets
    LocalDatabase
    Component_Zone
)

# 设置包含目录（供其他模块使用）
target_include_directories(SpecificWindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
