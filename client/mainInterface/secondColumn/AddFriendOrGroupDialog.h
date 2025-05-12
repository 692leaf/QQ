#ifndef ADDFRIENDORGROUPDIALOG_H
#define ADDFRIENDORGROUPDIALOG_H

#include <QDialog>
#include <QListView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "ButtonDelegate.h"
#include "Tcpclient.h"

class AddFriendOrGroupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddFriendOrGroupDialog(QWidget *parent = nullptr,TcpClient* client=nullptr);
    ~AddFriendOrGroupDialog();
    void initUi();
private slots:
    void onSearchTextChanged(const QString& text);
    void handleSearchPageDataResponse(const Packege& resend_Pkg);
    void onAddButtonClicked(const QModelIndex& index);
private:
    TcpClient* client;
    QListView* listView;
    QStandardItemModel* model;
    ButtonDelegate* delegate;
signals:
    void onAddNewFriendButtonClickded(const Account_Message& user_Info);
};

#endif // ADDFRIENDORGROUPDIALOG_H
