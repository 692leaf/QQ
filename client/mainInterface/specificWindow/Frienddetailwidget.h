#ifndef FRIENDDETAILWIDGET_H
#define FRIENDDETAILWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include "LocalDatabase.h"

class FriendDetailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FriendDetailWidget(QWidget *parent = nullptr,LocalDatabase* localBase=nullptr);
public slots:
    void receiveFriendInfo(const QString &account);
private:
    LocalDatabase* localBase;
    QLabel* avatarLabel;
    QLabel* nicknameLabel;
    QLabel* qqNumberLabel;
    QLabel* remarkLabel;
    QLabel* signatureLabel;
    QLabel* qqSpaceLabel;
};



#endif // FRIENDDETAILWIDGET_H
