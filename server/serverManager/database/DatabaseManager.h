#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "Packege.h"
#include "MessageType.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    void available_Drivers();

    //Table1:user
public:
    // 创建（增）
    QString userTable_Create_AccountRecord(const Packege& send_Pkg);
    // 读取（查）
    bool userTable_Read_UserLogin(const QString& condition);
    bool userTable_Read_AccountExists(const QString& condition);
    QVector<Account_Message> userTable_Read_AccountMessage(const QString& part);
    Account_Message userTable_Read_UserMessage(const QString& account);
    QString userTable_Read_IPv4(const QString& account);
    // 更新（改）
    void userTable_Update_IPv4(const QString& sender,const QString& ipAddress);
    void userTable_Update_Account_Info(const Packege& send_Pkg);
    void userTable_Update_OnlineStatus(const QString& account,bool isOnline);
    // void userTable_Update_Record(const QString& condition,const QString& newPassword);
    // // 删除（删）
    // void userTable_Delete_Record(const QString& condition);

    //分离
    void userTable_Split_BlindValue(QSqlQuery& query,const QString& condition);


    //Table2:friendlist
public:
    void frdTable_Create_FriendListRecord(const Packege& send_Pkg); // 好友申请
    void frdTable_ReadMyListRecord(QVector<QVector<Account_Message>>& friendListBarItemTextStates,const QString& account);//我的好友申请列表
    void frdTable_ReadOtherListRecord(QVector<QVector<Account_Message>>& friendListBarItemTextStates,const QString& account);//对方的好友申请列表
    QVector<Account_Message> frdTable_ReadFriendRecord(const QString& account); //我的好友
    void frdTable_Update_FriendListRecord(const Packege& friendRequestAgreementPackage); // 对方同意好友时，更新好友申请列表


    //Table3:chatlist
public:
    int chrTable_Create_ChatRecord(const Packege& send_Pkg);
    QVector<Packege> chrTable_Read_ChatHistory(const QString& account);
    void markMessageSynced(int recordId);


    //%Table4:texts
private:
    void textsTable_Create_TextsRecord(int chatlist_Id,const Packege& send_Pkg);
    QString textsTable_Read_TextsRecord(int chatlist_Id);

    //%Table5:file_transfers
private:
    void filesTable_Create_FilesRecord(int chatlist_Id,const Packege& send_Pkg);
    File filesTable_Read_FilesRecord(int chatlist_Id);

    //%Table6:media_files
public:
    QVector<Image> mediaTable_Create_MediaRecord(const Packege& send_Pkg);
    QString mediaTable_Select_Media_Name(const QString& media_Path);

private:
    QSqlDatabase db;

signals:
    void chatHistoryFetched(const std::vector<Packege>& chatHistory);
};

#endif // DATABASEMANAGER_H







