#ifndef FRIENDLISTITEMDELEGATE_H
#define FRIENDLISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>

class FriendListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FriendListItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // FRIENDLISTITEMDELEGATE_H
