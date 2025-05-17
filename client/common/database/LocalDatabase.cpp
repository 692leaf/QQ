#include "LocalDatabase.h"
#include <QApplication>
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QRegularExpression>

LocalDatabase::LocalDatabase(QObject *parent)
    : QObject{parent},
      // 初始话表情转换器
      emojiToUnicodeConverter(new EmojiUnicodeMapper(this))
{
    // 连接数据库
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("newdatabase");
    db.setUserName("root");
    db.setPassword("root");
    // 尝试打开数据库
    if (!db.open())
    {
        qDebug() << "Failed to connect to the database:" << db.lastError();
    }

    QSqlQuery query(db); // 指定数据库连接
    QString createUserProfileTableQuery = "CREATE TABLE IF NOT EXISTS user_profile ("
                                          "id INT AUTO_INCREMENT PRIMARY KEY, "
                                          "account VARCHAR(255),"
                                          "nickname VARCHAR(255),"
                                          "online TINYINT(1),"
                                          "avatar_path VARCHAR(255),"
                                          "version VARCHAR(255)"
                                          ")";
    if (query.exec(createUserProfileTableQuery))
    {
        qDebug() << "Table(user_profile) created successfully.";
    }
    else
    {
        qDebug() << "Error creating table(user_profile): " << query.lastError().text();
    }

    /*主表(基本内容)*/
    QString createChrTableQuery = "CREATE TABLE IF NOT EXISTS local_chatlist ("
                                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                                  "type INT,"
                                  "sender VARCHAR(255),"
                                  "receiver VARCHAR(255),"
                                  "sender_del TINYINT(1),"
                                  "receiver_del TINYINT(1),"
                                  "send_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                                  ")";
    if (query.exec(createChrTableQuery))
    {
        qDebug() << "Table(local_chatlist) created successfully.";
    }
    else
    {
        qDebug() << "Error creating table(local_chatlist): " << query.lastError().text();
    }

    /*保存消息(文本，文件)的两个次表(具体内容)*/
    // 创建local_texts_transfers表
    QString createTextsTableQuery = "CREATE TABLE IF NOT EXISTS local_texts_transfers ("
                                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                                    "chatlist_id INT,"
                                    "content TEXT,"
                                    "FOREIGN KEY (chatlist_id) REFERENCES local_chatlist(id)"
                                    "ON DELETE CASCADE ON UPDATE CASCADE"
                                    ")";
    if (query.exec(createTextsTableQuery))
    {
        qDebug() << "Table(local_texts_transfers) created successfully.";
    }
    else
    {
        qDebug() << "Error creating table(local_texts_transfers): " << query.lastError().text();
    }

    // 创建local_file_transfers表
    QString createFilesTableQuery = "CREATE TABLE IF NOT EXISTS local_file_transfers ("
                                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                                    "chatlist_id INT,"
                                    "file_path TEXT,"
                                    "file_name TEXT,"
                                    "FOREIGN KEY (chatlist_id) REFERENCES local_chatlist(id)"
                                    "ON DELETE CASCADE ON UPDATE CASCADE"
                                    ")";
    if (query.exec(createFilesTableQuery))
    {
        qDebug() << "Table(local_file_transfers) created successfully.";
    }
    else
    {
        qDebug() << "Error creating table(local_file_transfers): " << query.lastError().text();
    }

    /*音频独立出来，作为媒体库*/
    // 创建local_media_transfers表
    QString createMediaTableQuery = "CREATE TABLE IF NOT EXISTS local_media_transfers ("
                                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                                    "media_path TEXT,"
                                    "media_name TEXT"
                                    ")";
    if (query.exec(createMediaTableQuery))
    {
        qDebug() << "Table(local_media_transfers) created successfully.";
    }
    else
    {
        qDebug() << "Error creating table(local_media_transfers): " << query.lastError().text();
    }
}

LocalDatabase::~LocalDatabase()
{
    // 关闭数据库连接........子线程
    if (QSqlDatabase::contains("newdatabase"))
    {
        QSqlDatabase db = QSqlDatabase::database("newdatabase");
        if (db.isOpen())
        {
            db.close();
        }
        QSqlDatabase::removeDatabase("newdatabase");
    }
}

void LocalDatabase::userProfile_Table_Compare_Version(const QVector<Account_Message> &friend_List_Data)
{
    QSqlQuery query(db);
    QString queryStr = "SELECT * FROM user_profile WHERE account = :account";

    for (auto friend_Data : friend_List_Data)
    {
        query.prepare(queryStr);
        query.bindValue(":account", friend_Data.account);

        if (!query.exec())
        {
            // 插入新记录
            userProfile_Table_Create_UserInfo(friend_Data);
            continue;
        }

        if (query.next())
        {
            if (query.value("version").toString() != friend_Data.version)
            {
                userProfile_Table_Update_UserInfo(friend_Data);
            }
            else if (query.value("online").toBool() != friend_Data.status)
            {
                userProfile_Table_Update_FriendStatus(friend_Data);
            }
        }
    }
}

void LocalDatabase::userProfile_Table_Create_UserInfo(const Account_Message &friend_Data)
{
    QString localPath = storePictureLocally(friend_Data);

    // 插入数据
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO user_profile (account,nickname,online,avatar_path,version) "
                        "VALUES (:account,:nickname,:isOnline,:avatar_path,:version)");

    insertQuery.bindValue(":account", friend_Data.account);
    insertQuery.bindValue(":nickname", friend_Data.nickname);
    insertQuery.bindValue(":isOnline", friend_Data.status);
    insertQuery.bindValue(":avatar_path", localPath);
    insertQuery.bindValue(":version", friend_Data.version);

    if (!insertQuery.exec())
    {
        qDebug() << "新好友数据插入失败!";
    }
}

bool LocalDatabase::userProfile_Table_Update_UserInfo(const Account_Message &userInfo)
{
    QString localPath = storePictureLocally(userInfo);

    QSqlQuery updateQuery(db);

    updateQuery.prepare("UPDATE user_profile SET "
                        "nickname=:nickname,"
                        "avatar_path=:avatar_path,"
                        "version=:version,"
                        "online=:isOnline "
                        "WHERE account=:account");

    updateQuery.bindValue(":account", userInfo.account);
    updateQuery.bindValue(":nickname", userInfo.nickname);
    updateQuery.bindValue(":avatar_path", localPath);
    updateQuery.bindValue(":version", userInfo.version);
    updateQuery.bindValue(":isOnline", 1);

    if (!updateQuery.exec())
    {
        qDebug() << "用户信息更新失败!";
        return false;
    }
    return true;
}

void LocalDatabase::userProfile_Table_Update_FriendStatus(const Account_Message &user_Info)
{
    QSqlQuery updateQuery(db);

    updateQuery.prepare("UPDATE user_profile SET "
                        "online=:isOnline "
                        "WHERE account=:account");

    updateQuery.bindValue(":account", user_Info.account);
    updateQuery.bindValue(":isOnline", user_Info.status);

    if (!updateQuery.exec())
    {
        qDebug() << "用户状态更新失败!";
        return;
    }
}

Account_Message LocalDatabase::userProfile_Table_Load_localAccountInfo(const QString &account)
{
    // 接收方消息同步
    Account_Message user_Info;
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM user_profile WHERE "
                          "account=:account"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":account", account);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch account_Info:" << query.lastError().text();
        return user_Info;
    }

    // 如果能移动到下一条记录，说明有符合条件的记录
    if (query.next())
    {
        user_Info.account = account;
        user_Info.nickname = query.value("nickname").toString();
        user_Info.avatar_Path = query.value("avatar_path").toString();
        user_Info.status = query.value("online").toBool();
    }

    return user_Info;
}

QVector<Account_Message> LocalDatabase::userProfile_Table_Load_ChatListInfo(const QString &account)
{
    QVector<Account_Message> chat_List_Data;

    QSqlQuery query(db);
    // 修改查询语句，移除账户筛选条件，遍历整个表
    query.prepare("SELECT * FROM user_profile WHERE account != :account");
    query.bindValue(":account", account);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch chatList_Info:" << query.lastError().text();
        return chat_List_Data;
    }

    // 遍历查询结果
    while (query.next())
    {
        Account_Message data;
        data.account = query.value("account").toString();
        data.nickname = query.value("nickname").toString();
        data.avatar_Path = query.value("avatar_path").toString();
        data.tipMessage = local_ChrTable_Load_TipMessage(data.account);
        chat_List_Data.append(data);
    }

    return chat_List_Data;
}

QVector<Account_Message> LocalDatabase::userProfile_Table_Load_FriendListInfo(const QString &account)
{
    QVector<Account_Message> friend_List_Data;

    QSqlQuery query(db);
    // 修改查询语句，移除账户筛选条件，遍历整个表
    query.prepare("SELECT * FROM user_profile WHERE account != :account");
    query.bindValue(":account", account);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch friendList_Info:" << query.lastError().text();
        return friend_List_Data;
    }

    // 遍历查询结果
    while (query.next())
    {
        Account_Message data;
        data.account = query.value("account").toString();
        data.nickname = query.value("nickname").toString();
        data.avatar_Path = query.value("avatar_path").toString();
        data.status = query.value("online").toBool();
        friend_List_Data.append(data);
    }

    return friend_List_Data;
}

QString LocalDatabase::storePictureLocally(const Account_Message &account_Data)
{
    /*==================================将图片保存到本地==================================*/

    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty())
    {
        qDebug() << "无法获取桌面路径";
        return "";
    }
    // 构建目标目录路径
    QString dirPath = desktopPath + "/QQ_Localdata";
    // 创建 QDir 对象
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        qDebug() << "无法创建目录：" << dirPath;
        return "";
    }

    // 构建完整图片路径
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString imageType = account_Data.imageType;
    QString imagePath = dirPath + "/" + uuid + "." + imageType; // 注意文件扩展名前的点
    QFile image(imagePath);
    if (image.open(QIODevice::WriteOnly))
    {
        image.write(account_Data.avatarData);
        image.close();
        qDebug() << "头像保存成功";
    }
    else
    {
        qDebug() << "无法打开图片进行写入:" << image.errorString();
        return "";
    }

    return imagePath;
}

// Table1:chatrecords
void LocalDatabase::local_ChrTable_Create_ChatRecord(const Packege &chatSyncPkg)
{
    /*主表插入数据*/
    QSqlQuery query(db);
    QString insertQuery = "INSERT INTO local_chatlist (type,sender,receiver,sender_del,receiver_del)"
                          " VALUES (:type,:sender,:receiver,:sender_del,:receiver_del)";
    // 绑定值
    query.prepare(insertQuery);
    query.bindValue(":type", chatSyncPkg.messageInfo.message_type);
    query.bindValue(":sender", chatSyncPkg.sender);
    query.bindValue(":receiver", chatSyncPkg.receiver);
    query.bindValue(":sender_del", chatSyncPkg.messageInfo.sender_del);
    query.bindValue(":receiver_del", chatSyncPkg.messageInfo.receiver_del);

    int chatlist_Id;
    if (query.exec())
    {
        chatlist_Id = query.lastInsertId().toInt(); // 获取自增 ID
        qDebug() << "Data inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data: " << query.lastError().text();
        return;
    }

    /*次表插入数据*/
    switch (chatSyncPkg.messageInfo.message_type)
    {
    case RICHTEXTCONTENT_TRANSFERS:
        local_TextsTable_Create_TextsRecord(chatlist_Id, chatSyncPkg);
        break;
    case BLOCK_FILE_TRANSFERS:
        local_FilesTable_Create_FilesRecord(chatlist_Id, chatSyncPkg);
        break;
    }
}

QString LocalDatabase::local_ChrTable_Load_TipMessage(const QString &peerUser)
{
    QString localUser = qApp->property("username").toString();
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM local_chatlist WHERE "
                          "(sender=:peerUser AND receiver=:localUser AND receiver_del=0) "
                          "OR (receiver=:peerUser AND sender=:localUser AND sender_del=0)"
                          "ORDER BY send_time DESC LIMIT 1"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":peerUser", peerUser);
    query.bindValue(":localUser", localUser);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch chat history:" << query.lastError().text();
        return "";
    }

    // 如果能移动到下一条记录，说明有符合条件的记录
    if (query.next())
    {
        if (query.value("type").toInt() == RICHTEXTCONTENT_TRANSFERS)
        {
            QSqlQuery textsQuery(db);
            textsQuery.prepare(QString("SELECT * FROM local_texts_transfers WHERE chatlist_id=:chatlist_id LIMIT 1"));
            textsQuery.bindValue(":chatlist_id", query.value("id").toInt());
            if (textsQuery.exec() && textsQuery.next())
            {
                QString html = textsQuery.value("content").toString();
                QString processedHtml = replaceHtmlImagePathsWithMark(html);
                return processedHtml;
            }
        }
        else if (query.value("type").toInt() == BLOCK_FILE_TRANSFERS)
        {
            // 创建一个新的 QSqlQuery 对象来处理文件查询
            QSqlQuery fileQuery(db);
            fileQuery.prepare(QString("SELECT * FROM local_file_transfers WHERE chatlist_id=:chatlist_id LIMIT 1"));
            fileQuery.bindValue(":chatlist_id", query.value("id").toInt());
            if (fileQuery.exec() && fileQuery.next())
            {
                return fileQuery.value("file_name").toString();
            }
        }
    }
    return "";
}

QVector<Packege> LocalDatabase::local_ChrTable_Load_ChatHistory(const QString &peerUser)
{
    QString localUser = qApp->property("username").toString();
    // 接收方消息同步
    QVector<Packege> chatHistory_Pkgs;
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM local_chatlist WHERE "
                          "(sender=:peerUser AND receiver=:localUser AND receiver_del=0) "
                          "OR (receiver=:peerUser AND sender=:localUser AND sender_del=0)"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":peerUser", peerUser);
    query.bindValue(":localUser", localUser);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch chat history:" << query.lastError().text();
        return chatHistory_Pkgs;
    }

    // 如果能移动到下一条记录，说明有符合条件的记录
    while (query.next())
    {
        Packege dataPkg;

        dataPkg.type = LOAD_CHATHISTORY;
        dataPkg.sender = query.value("sender").toString();
        dataPkg.receiver = query.value("receiver").toString();
        dataPkg.timeStamp = query.value("send_time").toDateTime().toSecsSinceEpoch();

        switch (query.value("type").toUInt())
        {
        case RICHTEXTCONTENT_TRANSFERS:
        {
            dataPkg.messageInfo.message_type = RICHTEXTCONTENT_TRANSFERS;
            QString richText = local_TextsTable_Load_TextsRecord(query.value("id").toInt());
            if (richText == "")
                continue;
            dataPkg.messageInfo.richText = richText;
            chatHistory_Pkgs.push_back(dataPkg);
        }
        break;
        case BLOCK_FILE_TRANSFERS:
        {
            QVector<File> files = local_FilesTable_Load_FilesRecord(query.value("id").toInt());
            for (auto file : files)
            {
                dataPkg.messageInfo.message_type = BLOCK_FILE_TRANSFERS;
                dataPkg.messageInfo.file = file;
                chatHistory_Pkgs.push_back(dataPkg);
            }
        }
        break;
        }
    }

    return chatHistory_Pkgs;
}

void LocalDatabase::local_TextsTable_Create_TextsRecord(int chatlist_Id, const Packege &send_Pkg)
{
    /*将文本数据保存到表中*/
    // 插入数据
    QSqlQuery query(db);
    QString insertQuery = "INSERT INTO local_texts_transfers (chatlist_id,content)"
                          " VALUES (:chatlist_id,:content)";

    query.prepare(insertQuery);
    query.bindValue(":chatlist_id", chatlist_Id);
    query.bindValue(":content", send_Pkg.messageInfo.richText);

    if (query.exec())
    {
        qDebug() << "Data(local_create_FilesRecord) inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data(local_create_FilesRecord): " << query.lastError().text();
    }
}

QString LocalDatabase::local_TextsTable_Load_TextsRecord(int chatlist_Id)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM local_texts_transfers WHERE chatlist_id=:chatlist_id LIMIT 1"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":chatlist_id", chatlist_Id);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch texts history:" << query.lastError().text();
        return "";
    }

    // 通过字段索引获取数据（更高效）
    if (query.next())
    {
        const int contentIndex = query.record().indexOf("content");
        if (contentIndex == -1)
        {
            qCritical() << "Field 'content' does not exist in query result";
            return QString();
        }
        return query.value(contentIndex).toString();
    }

    return "";
}

void LocalDatabase::local_FilesTable_Create_FilesRecord(int chatlist_Id, const Packege &chatSyncPkg)
{
    /*==================================将文件保存到本地==================================*/
    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty())
    {
        qDebug() << "无法获取桌面路径";
        return;
    }

    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 构建目标目录路径
    QString fileType = chatSyncPkg.messageInfo.file.fileType;
    QString dirPath = desktopPath + "/QQ_Localdata";
    // 创建 QDir 对象
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        qDebug() << "无法创建目录：" << dirPath;
        return;
    }

    // 构建完整文件路径
    QString filePath = dirPath + "/" + uuid + "." + fileType; // 注意文件扩展名前的点
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(chatSyncPkg.messageInfo.file.fileContent);
        file.close();
        qDebug() << "文件保存成功";
    }
    else
    {
        qDebug() << "无法打开文件进行写入:" << file.errorString();
        return;
    }

    /*=============================将文件属性保存到表中===============================*/
    // 插入数据
    QSqlQuery query(db);
    QString insertQuery = "INSERT INTO local_file_transfers (chatlist_id,file_path,file_name)"
                          " VALUES (:chatlist_id,:file_path,:file_name)";

    query.prepare(insertQuery);
    query.bindValue(":chatlist_id", chatlist_Id);
    query.bindValue(":file_path", filePath);
    query.bindValue(":file_name", chatSyncPkg.messageInfo.file.fileName);

    if (query.exec())
    {
        qDebug() << "Data(local_create_FilesRecord) inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data(local_create_FilesRecord): " << query.lastError().text();
    }
}

QVector<File> LocalDatabase::local_FilesTable_Load_FilesRecord(int chatlist_Id)
{
    QVector<File> files;
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM local_file_transfers WHERE chatlist_id=:chatlist_id LIMIT 1"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":chatlist_id", chatlist_Id);

    if (!query.exec())
    {
        qCritical() << "Failed to fetch file history:" << query.lastError().text();
        return files; // 默认空值
    }

    if (query.next())
    {
        File meta_File;
        // 1. 从数据库获取路径和文件名
        QString filePath = query.value("file_path").toString();
        QString fileName = query.value("file_name").toString();
        meta_File.fileName = fileName;

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly))
        {
            // 生成唯一文件ID（示例使用QUuid）
            QString fileID = QUuid::createUuid().toString();

            const int blockSize = 1024 * 1024; // 每次读取1MB

            /*元信息*/
            QFileInfo fileInfo(filePath);
            meta_File.fileName = fileName;
            meta_File.fileSize = fileInfo.size();
            meta_File.fileType = fileInfo.suffix().toLower();

            // 该文件唯一标识符
            meta_File.fileID = fileID;
            // 0表示元数据包
            meta_File.currentBlock = 0;
            // 总块数计算
            meta_File.totalBlocks = ceil((double)fileInfo.size() / blockSize);
            // 存储元消息
            files.push_back(meta_File);

            int blockIndex = 1;
            while (!file.atEnd())
            {
                File data_File;        // 每次创建新对象
                data_File = meta_File; // 继承元数据
                data_File.currentBlock = blockIndex++;
                data_File.fileContent = file.read(blockSize);
                // 存储块信息
                files.push_back(data_File);
            }
            file.close();
        }
    }

    return files;
}

QVector<Image> LocalDatabase::local_MediaTable_Create_MediaRecord(const Packege &send_Pkg)
{
    /*==================================将图片保存到本地==================================*/
    QVector<Image> savedImagesInfo;
    // 预分配空间
    savedImagesInfo.resize(send_Pkg.messageInfo.images.size());

    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty())
    {
        qDebug() << "无法获取桌面路径";
        return savedImagesInfo;
    }
    // 构建目标目录路径
    QString dirPath = desktopPath + "/QQ_Localdata";
    // 创建 QDir 对象
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        qDebug() << "无法创建目录：" << dirPath;
        return savedImagesInfo;
    }

    QVector<Image> images = send_Pkg.messageInfo.images;
    for (int i = 0; i < images.size(); i++)
    {
        // 构建完整图片路径
        QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString imageType = images[i].imageType;
        QString imagePath = dirPath + "/" + uuid + "." + imageType; // 注意文件扩展名前的点
        QFile image(imagePath);
        if (image.open(QIODevice::WriteOnly))
        {
            image.write(images[i].imageData);
            image.close();
            qDebug() << "本地图片保存成功";
        }
        else
        {
            qDebug() << "无法打开本地图片进行写入:" << image.errorString();
            return savedImagesInfo;
        }

        /*=============================将图片属性保存到表中===============================*/
        // 插入数据
        QSqlQuery query(db);
        QString insertQuery = "INSERT INTO local_media_transfers (media_path,media_name)"
                              " VALUES (:media_path,:media_name)";

        query.prepare(insertQuery);
        query.bindValue(":media_path", imagePath);
        query.bindValue(":media_name", images[i].imageName);

        if (query.exec())
        {
            qDebug() << "Data(local_MediaTable_Create_MediaRecord) inserted successfully.";
        }
        else
        {
            qDebug() << "Error inserting data(local_MediaTable_Create_MediaRecord): " << query.lastError().text();
        }

        // 保存uuid和url
        savedImagesInfo[i].uniqueId = images[i].uniqueId;
        savedImagesInfo[i].url = imagePath;
    }

    return savedImagesInfo;
}

QString LocalDatabase::replaceHtmlImagePathsWithMark(const QString &html)
{
    QString result = html;

    // 第一步：处理所有<img>标签
    QRegularExpression imgRegex(R"(<img[^>]*src\s*=\s*["']([^"']*)["'][^>]*>)",
                                QRegularExpression::CaseInsensitiveOption);

    int offset = 0;
    QRegularExpressionMatchIterator it = imgRegex.globalMatch(result);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QString fullTag = match.captured(0); // 完整img标签
        QString srcPath = match.captured(1); // src路径值

        QString replacement;

        if (srcPath.startsWith(":/resource/image/emojis/"))
        {
            // 提取表情名称（去掉路径和扩展名）
            QFileInfo fi(srcPath);
            QString emojiName = fi.baseName();

            // 直接获取Emoji字符
            replacement = emojiToUnicodeConverter->get_Unicode(emojiName);
        }
        else
        {
            // 非表情图片统一替换
            replacement = "[图片]";
        }

        // 计算替换位置
        int start = match.capturedStart() + offset;
        int length = match.capturedLength();

        // 执行替换并更新偏移量
        result.replace(start, length, replacement);
        offset += replacement.length() - length;
    }

    // 第二步：移除所有<style>和<script>标签及其内容
    QRegularExpression styleRegex(R"(<style[^>]*>.*?</style>)", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    result.replace(styleRegex, "");
    QRegularExpression scriptRegex(R"(<script[^>]*>.*?</script>)", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    result.replace(scriptRegex, "");

    // 第三步：移除所有HTML标签
    result.replace(QRegularExpression("<[^>]*>"), "");

    // 第四步：简化空白字符
    result = result.simplified();
    return result;
}
