#ifndef BUTTONDELEGATE_H
#define BUTTONDELEGATE_H

#include <QApplication>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QStyle>

class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ButtonDelegate(QObject *parent = nullptr, bool isJoinButton = true);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    // 设置特定索引的按钮文本
    void setButtonText(const QModelIndex &index, const QString &text);
    // 获取特定索引的按钮文本
    QString getButtonText(const QModelIndex &index) const;

    void setButtonEnabled(const QModelIndex &index, bool enabled);
signals:
    void buttonClicked(const QModelIndex &index);

private:
    bool isJoinButton;
    QMap<QModelIndex, QString> buttonTexts; // 存储每个索引对应的按钮文本
    QMap<QModelIndex, bool> buttonEnabledStates;
};

#endif // BUTTONDELEGATE_H
