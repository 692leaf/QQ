#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H
#include <QtCore/Qt>

// 声明自定义角色（使用枚举或宏，避免魔法数字）
enum CustomRoles
{
    // 用户状态角色（值为 33）
    StatusRole = Qt::UserRole + 1,
    TipMessageRole = Qt::UserRole + 2
};

enum CurrentSecondPage
{
    CHATLIST_PAGE,
    FRIENDLIST_PAGE
};

enum CurrentThirdPage
{
    DEFAULT_PAGE,
    CHAT_PAGE,
    NOTIFICATION_PAGE,
    FRIEND_PAGE
};

enum MessageType
{
    NONE,
    TEXTONLY_TRANSFERS,        // 纯文本
    RICHTEXTCONTENT_TRANSFERS, // 富文本
    BLOCK_FILE_TRANSFERS,      // 分块文件传输
};

enum PackegeType
{
    HEARTBEAT_MONITORING,             // 心跳监测
    LOGIN,                            // 登录
    REGISTER,                         // 注册
    GET_FRIENDBAR_DATA,               // 获取好友申请栏数据
    FRIEND_REQUEST_SENT,              // 好友申请
    HANDLE_FRIEND_REQUEST,            // 处理好友申请
    SEARCH_PAGE_DATA,                 // 搜索
    LOAD_CHAT_LIST,                   // 加载本地聊天列表
    LOAD_FRIEND_LIST,                 // 加载本地好友列表
    UPDATE_ACCOUNT_INFO,              // 更新用户信息
    UPDATE_ONLINE_STATUS,             // 更新在线状态
    SEND_RICHTEXT_MESSAGE,            // 发送富文本
    SEND_FILE_MESSAGE,                // 发送文件消息
    ASYNC_FETCH_FRIEND_LIST_INFOS,    // 异步获取好友列表信息
    LOAD_CHATHISTORY,                 // 加载本地的聊天数据
    ASYNC_FETCH_CHATHISTORY,          // 异步拉取服务器的聊天数据
    VIDEO_SIGNALING_MESSAGE_SEND,     // 发送的信令消息(视频)
    VIDEO_SIGNALING_MESSAGE_RECEIVER, // 接收的信令消息(视频)
    AUDIO_SIGNALING_MESSAGE_SEND,     // 发送的信令消息(语音通话)
    AUDIO_SIGNALING_MESSAGE_RECEIVER  // 接收的信令消息(语音通话)
};

#endif // MESSAGETYPE_H
