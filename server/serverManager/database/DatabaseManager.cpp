#include "DatabaseManager.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QStringList>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject{parent}
{
    //连接数据库
    db=QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("newdatabase");
    db.setUserName("root");
    db.setPassword("root");
    //尝试打开数据库
    if(!db.open())
    {
        qDebug()<<"Failed to connect to the database:"<<db.lastError();
    }

    //创建user表
    QSqlQuery query(db);  // 指定数据库连接
    QString createUserTableQuery = "CREATE TABLE IF NOT EXISTS users ("
                               "id INT AUTO_INCREMENT PRIMARY KEY, "
                               "account VARCHAR(255),"
                               "password VARCHAR(255),"
                               "nickname VARCHAR(255),"
                               "ipv4 VARCHAR(20),"
                               "online TINYINT(1),"
                               "avatar_path VARCHAR(255),"
                               "version VARCHAR(255)"
                               ")";
    if (query.exec(createUserTableQuery)) {
        qDebug() << "Table(users) created successfully.";
    } else {
        qDebug() << "Error creating table(users): " << query.lastError().text();
    }


    //创建friendlist表
    QString createFrdTableQuery = "CREATE TABLE IF NOT EXISTS friendlist ("
                               "id INT AUTO_INCREMENT PRIMARY KEY,"
                               "user VARCHAR(255), "
                               "friend VARCHAR(255),"
                               "request TINYINT(1),"
                                "answer TINYINT(1))";
    if (query.exec(createFrdTableQuery)) {
        qDebug() << "Table(friendlist) created successfully.";
    } else {
        qDebug() << "Error creating table(friendlist): " << query.lastError().text();
    }

    /*主表(基本内容)*/
    QString createChrTableQuery = "CREATE TABLE IF NOT EXISTS chatlist ("
                                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                                  "type INT,"
                                  "sender VARCHAR(255),"
                                  "receiver VARCHAR(255),"
                                  "sender_del TINYINT(1),"
                                  "receiver_del TINYINT(1),"
                                  "is_synced TINYINT(1),"
                                  "send_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                                  ")";
    if (query.exec(createChrTableQuery)) {
        qDebug() << "Table(chatlist) created successfully.";
    } else {
        qDebug() << "Error creating table(chatlist): " << query.lastError().text();
    }

    /*保存消息(文本，文件)的两个次表(具体内容)*/
    //创建texts_transfers表
    QString createTextsTableQuery = "CREATE TABLE IF NOT EXISTS texts_transfers ("
                               "id INT AUTO_INCREMENT PRIMARY KEY,"
                               "chatlist_id INT,"
                               "content TEXT,"
                               "FOREIGN KEY (chatlist_id) REFERENCES chatlist(id)"
                               "ON DELETE CASCADE ON UPDATE CASCADE"
                                  ")";
    if (query.exec(createTextsTableQuery)) {
        qDebug() << "Table(texts_transfers) created successfully.";
    } else {
        qDebug() << "Error creating table(texts_transfers): " << query.lastError().text();
    }

    //创建file_transfers表
    QString createFilesTableQuery  = "CREATE TABLE IF NOT EXISTS file_transfers ("
                          "id INT AUTO_INCREMENT PRIMARY KEY,"
                          "chatlist_id INT,"
                          "file_path TEXT,"
                          "file_name TEXT,"
                          "FOREIGN KEY (chatlist_id) REFERENCES chatlist(id)"
                          "ON DELETE CASCADE ON UPDATE CASCADE"
                          ")";
    if (query.exec(createFilesTableQuery)) {
        qDebug() << "Table(file_transfers) created successfully.";
    } else {
        qDebug() << "Error creating table(file_transfers): " << query.lastError().text();
    }

    /*音频独立出来，作为媒体库*/
    //创建media_transfers表
    QString createMediaTableQuery = "CREATE TABLE IF NOT EXISTS media_transfers ("
                                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                                    "account VARCHAR(255),"
                                    "media_path TEXT,"
                                    "media_name TEXT"
                                    ")";
    if (query.exec(createMediaTableQuery)) {
        qDebug() << "Table(media_transfers) created successfully.";
    } else {
        qDebug() << "Error creating table(media_transfers): " << query.lastError().text();
    }
}

DatabaseManager::~DatabaseManager()
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

void DatabaseManager::available_Drivers()
{
    QStringList drivers = QSqlDatabase::drivers();
    qInfo()<<drivers;
}

QString DatabaseManager::userTable_Create_AccountRecord(const Packege& send_Pkg)
{
    //插入数据
    QSqlQuery query(db);
    QString insertQuery ="INSERT INTO users (account, password,avatar_path,version) "
                          "VALUES (:account, :password, :avatar_path, :version)";
    query.prepare(insertQuery);
    query.bindValue(":account",send_Pkg.sender);
    query.bindValue(":password",send_Pkg.sender_Passwd);
    query.bindValue(":avatar_path",":/resource/默认头像框.png"); // 默认路径
    QString uuid=QUuid::createUuid().toString(QUuid::WithoutBraces); // 随机生成版本号
    query.bindValue(":version",uuid);

    if(query.exec())
    {
        qDebug() << "Data inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data: " << query.lastError().text();
        return "";
    }
    return uuid;
}

bool DatabaseManager::userTable_Read_UserLogin(const QString& condition)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM users WHERE %1").arg(condition));
    if(query.exec())
    {
        if (query.next())
        {
            // 如果能移动到下一条记录，说明有符合条件的记录，返回 true
            userTable_Update_OnlineStatus(query.value("account").toString(),true);
            return true;
        }
    }
    return false;
}

bool DatabaseManager::userTable_Read_AccountExists(const QString &account)
{
    QSqlQuery query(db);
    QString condition=QString("account = '%1'").arg(account);
    query.prepare(QString("SELECT * FROM users WHERE %1").arg(condition));
    if(query.exec())
    {
        if (query.next())
        {
            // 如果能移动到下一条记录，说明有符合条件的记录，返回 true
            return true;
        }
    }
    return false;
}

QVector<Account_Message> DatabaseManager::userTable_Read_AccountMessage(const QString& part)
{
    QVector<Account_Message> search_Page_Data;
    QSqlQuery query(db);
    if(part=="")// 当 part 为空时，查询前 6 条记录
    {
        query.prepare("SELECT * FROM users LIMIT 6");
    }
    else// 使用 LIKE 操作符进行部分匹配
    {
        QString condition = QString("account LIKE '%%1%'").arg(part);
        query.prepare(QString("SELECT * FROM users WHERE %1").arg(condition));
    }

    if(query.exec())
    {
        while (query.next())
        {
            Account_Message user_Info;
            // 如果能移动到下一条记录，说明有符合条件的记录
            user_Info.account = query.value("account").toString();
            user_Info.nickname = query.value("nickname").toString();
            user_Info.avatar_Path = query.value("avatar_path").toString();

            search_Page_Data.push_back(user_Info);
        }
    }

    return search_Page_Data;
}

Account_Message DatabaseManager::userTable_Read_UserMessage(const QString &account)
{
    Account_Message user_Info;

    QSqlQuery query(db);
    QString condition = QString("SELECT * FROM users WHERE account=:account");
    query.prepare(condition);
    query.bindValue(":account",account);

    if(query.exec())
    {
        if(query.next())
        {
            user_Info.account = account;
            user_Info.nickname = query.value("nickname").toString();
            user_Info.avatar_Path = query.value("avatar_path").toString();
            user_Info.version = query.value("version").toString();
            user_Info.status = query.value("online").toBool();
        }
        else
        {
            qDebug()<<"未找到匹配的记录";
        }
    }
    else
    {
        qDebug() << "查询失败: " << query.lastError().text();
    }
    return user_Info;
}

QString DatabaseManager::userTable_Read_IPv4(const QString &account)
{
    QSqlQuery query(db);
    QString condition=QString("SELECT ipv4 FROM users WHERE account=:account");
    //准备SQL语句
    query.prepare(condition);
    query.bindValue(":account",account);

    if(query.exec())
    {
        if(query.next())
        {
            return query.value("ipv4").toString();
        }
        else
        {
            qDebug()<<"未找到匹配的记录";
        }
    }
    else
    {
        qDebug() << "查询失败: " << query.lastError().text();
    }
    return "";
}

void DatabaseManager::userTable_Update_IPv4(const QString& sender,const QString& ipAddress)
{
    QSqlQuery query(db);
    query.prepare(QString("UPDATE users SET ipv4=:ipv4 WHERE account=:account"));
    query.bindValue(":account",sender);
    query.bindValue(":ipv4",ipAddress);

    if(!query.exec())
    {
        qDebug()<<"IPv4 Update failed";
        return;
    }
}

void DatabaseManager::userTable_Update_Account_Info(const Packege& send_Pkg)
{
    /*==================================将图片保存到本地==================================*/

    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty())
    {
        qDebug() << "无法获取桌面路径";
        return;
    }
    // 构建目标目录路径
    QString account = send_Pkg.user_Info.account;
    QString dirPath = desktopPath + "/QQ_Application_Data/" + account;
    // 创建 QDir 对象
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        qDebug() << "无法创建目录：" << dirPath;
        return;
    }

    Account_Message user_Info = send_Pkg.user_Info;
    // 构建完整图片路径
    QString uuid=QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString imageType=user_Info.imageType;
    QString imagePath = dirPath + "/" + uuid + "." + imageType;  // 注意文件扩展名前的点
    QFile image(imagePath);
    if(image.open(QIODevice::WriteOnly))
    {
        image.write(user_Info.avatarData);
        image.close();
        qDebug()<<"图片保存成功";
    }
    else
    {
        qDebug()<<"无法打开图片进行写入:"<<image.errorString();
        return;
    }


    QSqlQuery query(db);
    query.prepare(QString("UPDATE users SET "
                          "nickname=:nickname,"
                          "avatar_path=:avatar_path,"
                          "version=:version "
                          "WHERE account=:account"));
    query.bindValue(":account", user_Info.account);
    query.bindValue(":nickname", user_Info.nickname);
    query.bindValue(":avatar_path", imagePath);
    query.bindValue(":version", user_Info.version);

    if(!query.exec())
    {
        //更新出错
        qDebug()<<"user_Info Update failed!";
        return;
    }
}

void DatabaseManager::userTable_Update_OnlineStatus(const QString &account, bool isOnline)
{
    QSqlQuery query(db);
    query.prepare(QString("UPDATE users SET online=:online WHERE account=:account"));
    query.bindValue(":account",account);
    query.bindValue(":online",isOnline);

    if(!query.exec())
    {
        qDebug()<<"Online Status Update failed!";
        return;
    }
}


/*void DatabaseManager::userTable_Update_Record(const QString& condition,const QString& newPassword)
{
    QSqlQuery query(db);
    QString account;
    // 从 condition 中提取用户名
    QStringList parts=account.split("=");
    if(parts.size()==3)
    {
        account=parts[1].trimmed();
        int index = account.indexOf(" AND ");
        if (index != -1) {
            account=account.left(index);
        }
        else
        {
            qDebug()<<"update_Record wrong!";
            return;
        }
    }
    QString updateQuery = "UPDATE users SET password=:newPassword WHERE account=:account";
    query.prepare(updateQuery);
    query.bindValue(":account", account);
    query.bindValue(":newPassword", newPassword);
    if(read_UserLogin(condition))
    {
        if (query.exec()) {
            qDebug() << "Data updated successfully";
            return;
        } else {
            qDebug() << "Data update failed:" << query.lastError().text();
            return;
        }
    }
    qDebug()<<"update_Record not find!";
}

void DatabaseManager::userTable_Delete_Record(const QString& condition)
{
    QSqlQuery query(db);
    QString deleteQuery="DELETE FROM users WHERE account=:account AND password=:password";
    query.prepare(deleteQuery);
    split_BlindValue(query,condition);
    if(read_UserLogin(condition))
    {
        if(query.exec())
        {
            qDebug()<<"Data deleted successfully";
            return;
        }
        qDebug()<<"Data deleted failed:"<<query.lastError().text();
    }
    qDebug()<<"deleted_Data not find!";
}*/




//Table2:friendlist
void DatabaseManager::frdTable_Create_FriendListRecord(const Packege& send_Pkg)
{
    //插入数据
    QSqlQuery query(db);
    QString insertQuery ="INSERT INTO friendlist (user,friend,request,answer) VALUES (:user,:friend,:request,:answer)";

    query.prepare(insertQuery);
    query.bindValue(":user",send_Pkg.sender);
    query.bindValue(":friend",send_Pkg.receiver);
    query.bindValue(":request",1);
    query.bindValue(":answer",0);

    if(query.exec())
    {
        qDebug() << "Data(create_FriendListRecord) inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data(create_FriendListRecord): " << query.lastError().text();
    }
}



void DatabaseManager::frdTable_ReadMyListRecord(QVector<QVector<Account_Message>>& friendListBarItemTextStates,const QString &account)
{
    // 申请方为我
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM friendlist WHERE user=:account"));
    query.bindValue(":account",account);

    /* pending_Verification   rejected */
    if(query.exec())
    {
        while (query.next())
        {
            // 如果能移动到下一条记录，说明有符合条件的记录,然后再判断
            int request = query.value("request").toInt();
            int answer = query.value("answer").toInt();
            if(request&&!answer)
            {
                // 等待验证
                QString account=query.value("friend").toString();
                Account_Message friendInfo = userTable_Read_UserMessage(account);
                friendListBarItemTextStates[0].push_back(friendInfo);
            }
            else if(request&&answer)
            {
                // 已通过
                QString account=query.value("friend").toString();
                Account_Message friendInfo = userTable_Read_UserMessage(account);
                friendListBarItemTextStates[1].push_back(friendInfo);
            }
        }
    }
}

void DatabaseManager::frdTable_ReadOtherListRecord(QVector<QVector<Account_Message>>& friendListBarItemTextStates,const QString &account)
{
    // 申请方为对方
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM friendlist WHERE friend=:account"));
    query.bindValue(":account",account);

    /* approved   to_Agree; */
    if(query.exec())
    {
        while (query.next())
        {
            // 如果能移动到下一条记录，说明有符合条件的记录,然后再判断
            int request = query.value("request").toInt();
            int answer = query.value("answer").toInt();
            if(request&&answer)
            {
                // 已同意
                QString account=query.value("user").toString();
                Account_Message friendInfo = userTable_Read_UserMessage(account);
                friendListBarItemTextStates[2].push_back(friendInfo);
            }
            else if(request&&!answer)
            {
                // 是否同意
                QString account=query.value("user").toString();
                Account_Message friendInfo = userTable_Read_UserMessage(account);
                friendListBarItemTextStates[3].push_back(friendInfo);
            }
        }
    }
}

QVector<Account_Message> DatabaseManager::frdTable_ReadFriendRecord(const QString& account)
{

    QVector<Account_Message> friend_List_Data;

    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM friendlist WHERE (user=:user OR friend=:user) AND request=1 AND answer=1"));
    query.bindValue(":user",account);
    if(query.exec())
    {
        while(query.next())
        {
            QString frd=query.value("friend").toString()!=account?query.value("friend").toString():
                              query.value("user").toString();
            friend_List_Data.push_back(userTable_Read_UserMessage(frd));
        }
    }

    return friend_List_Data;
}

void DatabaseManager::frdTable_Update_FriendListRecord(const Packege& friendRequestAgreementPackage)
{
    // 当对方同意好友申请时，更新数据库中好友列表记录的应答状态
    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE friendlist SET answer=1 WHERE (user=:user AND friend=:friend)");
    updateQuery.bindValue(":user",friendRequestAgreementPackage.sender);
    updateQuery.bindValue(":friend", friendRequestAgreementPackage.receiver);
    if(!updateQuery.exec())
    {
        qDebug()<<"更新应答状态失败";
    }
}




//Table3:chatrecords
int DatabaseManager::chrTable_Create_ChatRecord(const Packege& send_Pkg)
{
    /*主表插入数据*/
    QSqlQuery query(db);
    QString insertQuery ="INSERT INTO chatlist (type,sender,receiver,sender_del,receiver_del,is_synced)"
                          " VALUES (:type,:sender,:receiver,:sender_del,:receiver_del,:is_synced)";
    //绑定值
    query.prepare(insertQuery);
    query.bindValue(":type",send_Pkg.messageInfo.message_type);
    query.bindValue(":sender",send_Pkg.sender);
    query.bindValue(":receiver",send_Pkg.receiver);
    query.bindValue(":sender_del",0);
    query.bindValue(":receiver_del",0);
    query.bindValue(":is_synced",0);    //默认未同步

    int chatlist_Id;
    if(query.exec())
    {
        chatlist_Id = query.lastInsertId().toInt(); //获取自增 ID
        qDebug() << "Data inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data: " << query.lastError().text();
    }


    /*次表插入数据*/
    switch(send_Pkg.messageInfo.message_type)
    {
    case RICHTEXTCONTENT_TRANSFERS:
        textsTable_Create_TextsRecord(chatlist_Id,send_Pkg);
        break;
    case BLOCK_FILE_TRANSFERS:
        filesTable_Create_FilesRecord(chatlist_Id,send_Pkg);
        break;
    }

    // 返回该条消息id
    return chatlist_Id;
}

QVector<Packege> DatabaseManager::chrTable_Read_ChatHistory(const QString& account)
{
    // 接收方消息同步
    QVector<Packege> chatHistory_Pkgs;
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM chatlist WHERE "
                          "(receiver=:account) "
                          "AND (is_synced = 0)"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":account", account);


    if (!query.exec())
    {
        qCritical() << "Failed to fetch chat history:" << query.lastError().text();
        return chatHistory_Pkgs;
    }

    while (query.next())
    {
        // 如果能移动到下一条记录，说明有符合条件的记录
        Packege dataPkg;

        dataPkg.type=ASYNC_FETCH_CHATHISTORY;
        dataPkg.sender=query.value("sender").toString();
        dataPkg.receiver=query.value("receiver").toString();
        dataPkg.messageInfo.sender_del=query.value("sender_del").toBool();
        dataPkg.messageInfo.receiver_del=query.value("receiver_del").toBool();
        dataPkg.timeStamp=query.value("send_time").toDateTime().toSecsSinceEpoch();

        // 更新为该条消息已同步
        int recordId = query.value("id").toInt(); // 假设表中有唯一标识字段 id
        markMessageSynced(recordId);

        switch(query.value("type").toUInt())
        {
        case RICHTEXTCONTENT_TRANSFERS:
        {
            dataPkg.messageInfo.message_type=RICHTEXTCONTENT_TRANSFERS;
            QString richText=textsTable_Read_TextsRecord(query.value("id").toInt());
            if(richText=="") continue;
            dataPkg.messageInfo.richText=richText;
        }
        break;
        case BLOCK_FILE_TRANSFERS:
        {
            dataPkg.messageInfo.message_type=BLOCK_FILE_TRANSFERS;
            dataPkg.messageInfo.file=filesTable_Read_FilesRecord(query.value("id").toInt());
        }
        break;
        }

        chatHistory_Pkgs.push_back(dataPkg);
    }

    return chatHistory_Pkgs;
}

void DatabaseManager::markMessageSynced(int recordId)
{
    // 标记消息为已同步
    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE chatlist SET is_synced = 1 WHERE id = :id");
    updateQuery.bindValue(":id", recordId);
    if (!updateQuery.exec())
    {
        qCritical() << "Failed to update sync status:" << updateQuery.lastError().text();
    }
}

void DatabaseManager::textsTable_Create_TextsRecord(int chatlist_Id,const Packege& send_Pkg)
{
    /*将文本数据保存到表中*/
    //插入数据
    QSqlQuery query(db);
    QString insertQuery ="INSERT INTO texts_transfers (chatlist_id,content)"
                          " VALUES (:chatlist_id,:content)";

    query.prepare(insertQuery);
    query.bindValue(":chatlist_id",chatlist_Id);
    query.bindValue(":content",send_Pkg.messageInfo.richText);

    if(query.exec())
    {
        qDebug() << "Data(textsTable_Create_TextsRecord) inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data(textsTable_Create_TextsRecord): " << query.lastError().text();
    }
}

QString DatabaseManager::textsTable_Read_TextsRecord(int chatlist_Id)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM texts_transfers WHERE chatlist_id=:chatlist_id LIMIT 1"));

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
        if(contentIndex == -1)
        {
            qCritical() << "Field 'content' does not exist in query result";
            return QString();
        }
        return query.value(contentIndex).toString();
    }

    return "";
}


void DatabaseManager::filesTable_Create_FilesRecord(int chatlist_Id,const Packege& send_Pkg)
{
    /*==================================将文件保存到本地==================================*/
    //获取桌面路径
    QString desktopPath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty())
    {
        qDebug() << "无法获取桌面路径";
        return;
    }

    QString uuid=QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 构建目标目录路径
    QString account=send_Pkg.sender;
    QString fileType=send_Pkg.messageInfo.file.fileType;
    QString dirPath = desktopPath + "/QQ_Application_Data/" + account;
    // 创建 QDir 对象
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        qDebug() << "无法创建目录：" << dirPath;
        return;
    }


    // 构建完整文件路径
    QString filePath = dirPath + "/" + uuid + "." + fileType;  // 注意文件扩展名前的点
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(send_Pkg.messageInfo.file.fileContent);
        file.close();
        qDebug()<<"文件保存成功";
    }
    else
    {
        qDebug()<<"无法打开文件进行写入:"<<file.errorString();
        return;
    }



    /*=============================将文件属性保存到表中===============================*/
    //插入数据
    QSqlQuery query(db);
    QString insertQuery ="INSERT INTO file_transfers (chatlist_id,file_path,file_name)"
                          " VALUES (:chatlist_id,:file_path,:file_name)";

    query.prepare(insertQuery);
    query.bindValue(":chatlist_id",chatlist_Id);
    query.bindValue(":file_path",filePath);
    query.bindValue(":file_name",send_Pkg.messageInfo.file.fileName);

    if(query.exec())
    {
        qDebug() << "Data(filesTable_Create_FilesRecord) inserted successfully.";
    }
    else
    {
        qDebug() << "Error inserting data(filesTable_Create_FilesRecord): " << query.lastError().text();
    }
}

File DatabaseManager::filesTable_Read_FilesRecord(int chatlist_Id)
{
    File file;
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM file_transfers WHERE chatlist_id=:chatlist_id LIMIT 1"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":chatlist_id", chatlist_Id);

    if(!query.exec())
    {
        qCritical() << "Failed to fetch file history:" << query.lastError().text();
        return file; // 默认空值
    }

    if(query.next())
    {
        // 1. 从数据库获取路径和文件名
        QString filePath=query.value("file_path").toString();
        QString fileName=query.value("file_name").toString();
        file.fileName=fileName;

        // 2. 从本地文件读取内容和大小
        QFile localFile(filePath);
        if(localFile.open(QIODevice::ReadOnly))
        {
            file.fileContent = localFile.readAll(); // 读取二进制内容
            file.fileSize = localFile.size();       // 获取文件大小（字节）
            localFile.close();
        }
        else
        {
            qWarning() << "文件不存在或无法读取:" << filePath;
            file.fileContent.clear();
            file.fileSize = 0;
        }

        // 3. 获取文件类型
        file.fileType = QFileInfo(filePath).suffix().toLower();
    }

    return file;
}


QVector<Image> DatabaseManager::mediaTable_Create_MediaRecord(const Packege& send_Pkg)
{
    /*==================================将图片保存到本地==================================*/
    QVector<Image> savedImagesInfo;
    // 预分配空间
    savedImagesInfo.resize(send_Pkg.messageInfo.images.size());

    //获取桌面路径
    QString desktopPath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty())
    {
        qDebug() << "无法获取桌面路径";
        return savedImagesInfo;
    }
    // 构建目标目录路径
    QString account=send_Pkg.sender;
    QString dirPath = desktopPath + "/QQ_Application_Data/" + account;
    // 创建 QDir 对象
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        qDebug() << "无法创建目录：" << dirPath;
        return savedImagesInfo;
    }

    QVector<Image> images = send_Pkg.messageInfo.images;
    for(int i=0;i<images.size();i++)
    {
        // 构建完整图片路径
        QString uuid=QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString imageType=images[i].imageType;
        QString imagePath = dirPath + "/" + uuid + "." + imageType;  // 注意文件扩展名前的点
        QFile image(imagePath);
        if(image.open(QIODevice::WriteOnly))
        {
            image.write(images[i].imageData);
            image.close();
            qDebug()<<"图片保存成功";
        }
        else
        {
            qDebug()<<"无法打开图片进行写入:"<<image.errorString();
            return savedImagesInfo;
        }



        //============================= 将图片属性保存到表中 ===============================
        //插入数据
        QSqlQuery query(db);
        QString insertQuery ="INSERT INTO media_transfers (account,media_path,media_name)"
                              " VALUES (:account,:media_path,:media_name)";

        query.prepare(insertQuery);
        query.bindValue(":account",send_Pkg.sender);
        query.bindValue(":media_path",imagePath);
        query.bindValue(":media_name",images[i].imageName);

        if(query.exec())
        {
            qDebug() << "Data(mediaTable_Create_MediaRecord) inserted successfully.";
        }
        else
        {
            qDebug() << "Error inserting data(mediaTable_Create_MediaRecord): " << query.lastError().text();
        }

        // 保存uuid和url
        savedImagesInfo[i].uniqueId = images[i].uniqueId;
        savedImagesInfo[i].url = imagePath;
    }

    return savedImagesInfo;
}

QString DatabaseManager::mediaTable_Select_Media_Name(const QString &media_Path)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT * FROM media_transfers WHERE media_path=:media_path LIMIT 1"));

    // 使用命名绑定防止SQL注入
    query.bindValue(":media_path", media_Path);

    if (!query.exec())
    {
        qDebug() << "Failed to fetch media_Name:" << query.lastError().text();
        return "";
    }

    if(query.next())
    {
        QString media_Name=query.value("media_name").toString();
        return media_Name;
    }

    return "";
}



