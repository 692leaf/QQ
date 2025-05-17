#ifndef LOCALDATABASE_H
#define LOCALDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "MessageType.h"
#include "Packege.h"
#include "EmojiUnicodeMapper.h"

class LocalDatabase : public QObject
{
    Q_OBJECT
public:
    explicit LocalDatabase(QObject *parent = nullptr);
    ~LocalDatabase();

    void available_Drivers();

    // Table1:user_profile
public:
    void userProfile_Table_Compare_Version(const QVector<Account_Message> &friend_List_Data);
    void userProfile_Table_Create_UserInfo(const Account_Message &friend_Data);
    bool userProfile_Table_Update_UserInfo(const Account_Message &userInfo);
    void userProfile_Table_Update_FriendStatus(const Account_Message &user_Info);
    Account_Message userProfile_Table_Load_localAccountInfo(const QString &account);
    QVector<Account_Message> userProfile_Table_Load_ChatListInfo(const QString &account);
    QVector<Account_Message> userProfile_Table_Load_FriendListInfo(const QString &account);

    // 功能模块
    QString storePictureLocally(const Account_Message &account_Data);

    // Table2:chatlist
public:
    void local_ChrTable_Create_ChatRecord(const Packege &chatSyncPkg);
    QString local_ChrTable_Load_TipMessage(const QString &peerUser);
    QVector<Packege> local_ChrTable_Load_ChatHistory(const QString &peerUser);

    // %Table3:texts
public:
    void local_TextsTable_Create_TextsRecord(int chatlist_Id, const Packege &chatSyncPkg);
    QString local_TextsTable_Load_TextsRecord(int chatlist_Id);

    // %Table4:file_transfers
public:
    void local_FilesTable_Create_FilesRecord(int chatlist_Id, const Packege &chatSyncPkg);
    QVector<File> local_FilesTable_Load_FilesRecord(int chatlist_Id);

    // Table5:media_files
public:
    QVector<Image> local_MediaTable_Create_MediaRecord(const Packege &send_Pkg);

private:
    QString replaceHtmlImagePathsWithMark(const QString &html);

private:
    QSqlDatabase db;
    EmojiUnicodeMapper *emojiToUnicodeConverter;
signals:
    void chatHistoryFetched(const std::vector<Packege> &chatHistory);
};

#endif // LOCALDATABASE_H
