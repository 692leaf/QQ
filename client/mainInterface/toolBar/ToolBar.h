#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QApplication>
#include <QToolBar>
#include <QAction>
#include <QBoxLayout>
#include <QPushButton>
#include "Tcpclient.h"
#include "LocalDatabase.h"

class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = nullptr, TcpClient *client = nullptr, LocalDatabase *localBase = nullptr);
    void initUi();
    QAction *setUserAvatarBar();
    QAction *setChatMessageBar();
    QAction *setFriendListBar();
    QAction *setQZoneFeedsBar();
    QAction *setTencentChannelsBar();
    QAction *setGameListBar();
    QAction *setWidgetsBar();
public slots:
    void avatarDialog();
    // avatarDialog的弹窗
    void editDialog();

    void load_Local_Account_Message();

    void getImageBinaryDataByPath(const QString &localPath, Account_Message &user_Info);

public:
    QToolBar *tBar;

private:
    TcpClient *client;
    LocalDatabase *localBase;
    QDialog *dlog;
    QDialog *editDlog;
    QString qqNumber;
    QString displayNickname;
    QString avatar_Path;
    bool isLocalDataUpdatePending = false;

signals:
    void switchSecondPageRequested(int index);
};

#endif // TOOLBAR_H
