#ifndef CHATLISTITEMDELEGATE_H
#define CHATLISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>

class ChatListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatListItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // CHATLISTITEMDELEGATE_H
