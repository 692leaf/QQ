#ifndef BUBBLEWIDGET_H
#define BUBBLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QUrl>
#include "Packege.h"
#include "MessageType.h"

class BubbleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BubbleWidget(const Packege &fullPkg, QWidget *parent = nullptr);

private:
    void setupUI(const Packege &fullPkg); // 初始化界面
    // 标签内容和样式设置
    QString generateTextMessageHtml(const Packege &fullPkg);
    QString generateFileMessageHtml(const Packege &fullPkg);
public slots:
    // 连接锚点点击信号
    void handleAnchorClicked(const QUrl &url);
signals:
    void downloadRequested(const QString &fileID);
    void openFolderRequested(const QString &fileID);
};

#endif // BUBBLEWIDGET_H
