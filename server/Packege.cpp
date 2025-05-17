#include "Packege.h"

// ===================== Image 序列化 =====================
QDataStream &operator<<(QDataStream &out, const Image &image)
{
    out << image.uniqueId << image.imageName << image.imageType
        << image.imageData << image.localPath << image.url;
    return out;
}

QDataStream &operator>>(QDataStream &in, Image &image)
{
    in >> image.uniqueId >> image.imageName >> image.imageType >> image.imageData >> image.localPath >> image.url;
    return in;
}

// ===================== File 序列化 =====================
QDataStream &operator<<(QDataStream &out, const File &file)
{
    out << file.fileName << file.fileContent << file.fileSize << file.fileType
        << file.fileID << file.currentBlock << file.totalBlocks
        << file.receivedBlocks;

    // 按块序号升序序列化
    QList<int> sortedKeys = file.blocks.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());
    out << static_cast<qint32>(sortedKeys.size());
    for (int key : sortedKeys)
    {
        out << key << file.blocks[key];
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, File &file)
{
    in >> file.fileName >> file.fileContent >> file.fileSize >> file.fileType >> file.fileID >> file.currentBlock >> file.totalBlocks >> file.receivedBlocks;

    qint32 blockCount;
    in >> blockCount;
    file.blocks.clear();
    for (qint32 i = 0; i < blockCount; ++i)
    {
        int key;
        QByteArray value;
        in >> key >> value;
        file.blocks.insert(key, value);
    }
    return in;
}

// ===================== Account_Message 序列化 =====================
QDataStream &operator<<(QDataStream &out, const Account_Message &msg)
{
    out << msg.account << msg.nickname << msg.avatar_Path << msg.avatarData
        << msg.imageType << msg.version << msg.status << msg.tipMessage;
    return out;
}

QDataStream &operator>>(QDataStream &in, Account_Message &msg)
{
    in >> msg.account >> msg.nickname >> msg.avatar_Path >> msg.avatarData >> msg.imageType >> msg.version >> msg.status >> msg.tipMessage;
    return in;
}

// ===================== MessageInfo 序列化 =====================
QDataStream &operator<<(QDataStream &out, const MessageInfo &msg)
{
    out << msg.textOnly << msg.richText;
    out << static_cast<qint32>(msg.images.size()); // 写入图片数量
    for (const auto &img : msg.images)
        out << img; // 序列化每张图片
    out << msg.file << msg.sender_del << msg.receiver_del << msg.message_type;
    return out;
}

QDataStream &operator>>(QDataStream &in, MessageInfo &msg)
{
    in >> msg.textOnly >> msg.richText;
    qint32 imageCount;
    in >> imageCount;
    msg.images.resize(imageCount);
    for (auto &img : msg.images)
        in >> img;
    in >> msg.file >> msg.sender_del >> msg.receiver_del >> msg.message_type;
    return in;
}

// ===================== QVector<Image> 序列化 =====================
QDataStream &operator<<(QDataStream &out, const QVector<Image> &images)
{
    out << static_cast<qint32>(images.size());
    for (const auto &img : images)
        out << img;
    return out;
}

QDataStream &operator>>(QDataStream &in, QVector<Image> &images)
{
    qint32 size;
    in >> size;
    images.resize(size);
    for (auto &img : images)
        in >> img;
    return in;
}

// ===================== QVector<Account_Message> 序列化 =====================
QDataStream &operator<<(QDataStream &out, const QVector<Account_Message> &vec)
{
    out << static_cast<qint32>(vec.size());
    for (const auto &msg : vec)
        out << msg;
    return out;
}

QDataStream &operator>>(QDataStream &in, QVector<Account_Message> &vec)
{
    qint32 size;
    in >> size;
    vec.resize(size);
    for (auto &msg : vec)
        in >> msg;
    return in;
}

// ===================== QVector<QVector<Account_Message>> 序列化 =====================
QDataStream &operator<<(QDataStream &out, const QVector<QVector<Account_Message>> &vec)
{
    out << static_cast<qint32>(vec.size());
    for (const auto &subVec : vec)
    {
        out << static_cast<qint32>(subVec.size());
        for (const auto &msg : subVec)
            out << msg;
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, QVector<QVector<Account_Message>> &vec)
{
    qint32 outerSize;
    in >> outerSize;
    vec.resize(outerSize);
    for (auto &subVec : vec)
    {
        qint32 innerSize;
        in >> innerSize;
        subVec.resize(innerSize);
        for (auto &msg : subVec)
            in >> msg;
    }
    return in;
}

// ===================== QVector<QVector<QString>> 序列化 =====================
QDataStream &operator<<(QDataStream &out, const QVector<QVector<QString>> &vec)
{
    out << static_cast<qint32>(vec.size());
    for (const auto &subVec : vec)
    {
        out << static_cast<qint32>(subVec.size());
        for (const auto &str : subVec)
            out << str;
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, QVector<QVector<QString>> &vec)
{
    qint32 outerSize;
    in >> outerSize;
    vec.resize(outerSize);
    for (auto &subVec : vec)
    {
        qint32 innerSize;
        in >> innerSize;
        subVec.resize(innerSize);
        for (auto &str : subVec)
            in >> str;
    }
    return in;
}

// ===================== QMap 序列化 =====================
QDataStream &operator<<(QDataStream &out, const QMap<int, QByteArray> &map)
{
    QList<int> sortedKeys = map.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());
    out << static_cast<qint32>(sortedKeys.size());
    for (int key : sortedKeys)
        out << key << map[key];
    return out;
}

QDataStream &operator>>(QDataStream &in, QMap<int, QByteArray> &map)
{
    qint32 size;
    in >> size;
    map.clear();
    for (qint32 i = 0; i < size; ++i)
    {
        int key;
        QByteArray value;
        in >> key >> value;
        map.insert(key, value);
    }
    return in;
}

// ===================== Package 序列化 =====================
QDataStream &operator<<(QDataStream &out, const Packege &pkg)
{
    out << pkg.sender << pkg.sender_Passwd << pkg.sender_Nickname
        << pkg.receiver << pkg.receiver_Nickname << pkg.messageInfo
        << pkg.user_Info << pkg.search_Page_Data << pkg.friend_List_Data
        << pkg.friendListBarItemTextStates << pkg.type
        << pkg.timeStamp << pkg.ip << pkg.videoPort << pkg.audioPort;
    return out;
}

QDataStream &operator>>(QDataStream &in, Packege &pkg)
{
    in >> pkg.sender >> pkg.sender_Passwd >> pkg.sender_Nickname >> pkg.receiver >> pkg.receiver_Nickname >> pkg.messageInfo >> pkg.user_Info >> pkg.search_Page_Data >> pkg.friend_List_Data >> pkg.friendListBarItemTextStates >> pkg.type >> pkg.timeStamp >> pkg.ip >> pkg.videoPort >> pkg.audioPort;
    return in;
}
