#ifndef PACKEGE_H
#define PACKEGE_H

#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QMap>
#include <QHostAddress>

/*******************************************************************************************
 *                                     tcp                                                 *
 *******************************************************************************************/
class Image
{
public:
    QString uniqueId;            // 唯一标识符
    QString imageName;           // 图片名称
    QString imageType;           // 图片路径
    QByteArray imageData;        // 原始图片
    QString localPath;           // 本地路径
    QString url;                 // 对端url

    // 声明 << 操作符
    friend QDataStream& operator<<(QDataStream& out, const Image& image);
    // 声明 >> 操作符
    friend QDataStream& operator>>(QDataStream& in, Image& image);
};

class File
{
public:
    QString fileName;              //文件名
    QByteArray fileContent;        // 文件内容
    int fileSize;                  // 文件大小
    QString fileType;              // 文件类型
    QString fileID;                // 唯一文件标识
    int currentBlock;              // 当前块序号（0=元数据，1+为数据块）
    int totalBlocks;               // 总块数
    int receivedBlocks=0;          // 接收到的块
    QMap<int,QByteArray> blocks;   //传输到对方后,保存各个块的数据

    // 声明 << 操作符
    friend QDataStream& operator<<(QDataStream& out, const File& file);
    // 声明 >> 操作符
    friend QDataStream& operator>>(QDataStream& in, File& file);
};

class Account_Message
{
public:
    QString account;              // 用户
    QString nickname;             // 昵称
    QString avatar_Path;          // 图片路径
    QByteArray avatarData;        // 图片数据
    QString imageType;            // 图片类型
    QString version;              // 版本类型(客户端服务器同步)
    bool status;                  // 状态(是否在线)
    QString tipMessage;           // 提示消息

    // 声明 << 操作符
    friend QDataStream& operator<<(QDataStream& out, const Account_Message& msg);
    // 声明 >> 操作符
    friend QDataStream& operator>>(QDataStream& in, Account_Message& msg);
};

class MessageInfo
{
public:
    QString textOnly;           // 纯文本
    QString richText;           // 富文本
    QVector<Image> images;      // 图片
    File file;                  // 文件

    // 是否已经删除
    bool sender_del;
    bool receiver_del;

    int message_type;           //文件类型

    // 声明 << 操作符
    friend QDataStream& operator<<(QDataStream& out, const MessageInfo& msg);
    // 声明 >> 操作符
    friend QDataStream& operator>>(QDataStream& in, MessageInfo& msg);
};

// 声明 QVector<QVector<Account_Message>> 的 << 操作符
QDataStream& operator<<(QDataStream& out, const QVector<QVector<Account_Message>>& vec);
// 声明 QVector<Account_Message> 的 << 操作符
QDataStream& operator<<(QDataStream& out, const QVector<Account_Message>& vec);
// 声明 QVector<Account_Message> 的 >> 操作符
QDataStream& operator>>(QDataStream& in, QVector<Account_Message>& vec);
// 声明 QVector<QVector<Account_Message>> 的 >> 操作符
QDataStream& operator>>(QDataStream& in, QVector<QVector<Account_Message>>& vec);
QDataStream& operator>>(QDataStream& in, QVector<QVector<QString>>& vec);
// 声明 QMap<int, QByteArray> 的 << 操作符
QDataStream& operator<<(QDataStream& out, const QMap<int, QByteArray>& map);
// 声明 QMap<int, QByteArray> 的 >> 操作符
QDataStream& operator>>(QDataStream& in, QMap<int, QByteArray>& map);



class Packege
{
public:
    QString sender;
    QString sender_Passwd;
    QString sender_Nickname;
    QString receiver;
    QString receiver_Nickname;
    MessageInfo messageInfo;
    Account_Message user_Info;
    QVector<Account_Message> search_Page_Data;
    QVector<Account_Message> friend_List_Data;
    QVector<QVector<Account_Message>> friendListBarItemTextStates;
    int type;
    quint64 timeStamp;
    QString ip;
    quint16 videoPort;
    quint16 audioPort;

    // 声明 << 操作符
    friend QDataStream& operator<<(QDataStream& out, const Packege& pkg);
    // 声明 >> 操作符
    friend QDataStream& operator>>(QDataStream& in, Packege& pkg);
};

#endif // PACKEGE_H

