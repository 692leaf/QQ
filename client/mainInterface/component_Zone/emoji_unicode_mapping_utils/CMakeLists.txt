# 收集源文件
file(GLOB_RECURSE EMOJI_UNICODE_MAPPING_UTILS_SOURCES *.cpp)
file(GLOB_RECURSE EMOJI_UNICODE_MAPPING_UTILS_HEADERS *.h)

# 创建库
add_library(Emoji_Unicode_Mapping_Utils
    ${EMOJI_UNICODE_MAPPING_UTILS_SOURCES}
    ${EMOJI_UNICODE_MAPPING_UTILS_HEADERS}
)

# 添加 Qt 模块依赖
target_link_libraries(Emoji_Unicode_Mapping_Utils PUBLIC
    Qt::Core
    Qt::Widgets
)

# 设置包含目录（供其他模块使用）
target_include_directories(Emoji_Unicode_Mapping_Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
