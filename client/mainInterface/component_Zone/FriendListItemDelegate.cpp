#include "FriendListItemDelegate.h"
#include "MessageType.h"

FriendListItemDelegate::FriendListItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
}

void FriendListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // 获取数据
    QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>();
    QString name = index.data(Qt::DisplayRole).toString();
    bool isOnline = index.data(StatusRole).toBool();

    // 背景高亮
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->fillRect(option.rect, option.palette.brush(QPalette::AlternateBase));
    }

    // 布局参数
    const int padding = 8;
    const int avatarSize = 40;
    const int statusDotSize = 8;
    const int bracketWidth = 12;

    // 绘制头像
    QRect avatarRect = option.rect.adjusted(padding, padding, 0, -padding);
    avatarRect.setSize(QSize(avatarSize, avatarSize));
    if (!avatar.isNull())
    {
        painter->drawPixmap(avatarRect, avatar.scaled(avatarSize, avatarSize,
                                                      Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 文字区域
    QRect textRect = option.rect.adjusted(
        avatarSize + 2 * padding,
        padding,
        -padding - bracketWidth * 2,
        -padding);

    // 用户名
    QFont nameFont = QApplication::font();
    nameFont.setBold(true);
    painter->setFont(nameFont);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, name);

    // 状态区域
    QFontMetrics bracketMetrics(QApplication::font());
    const int statusHeight = bracketMetrics.height();
    QRect statusRect = QRect(
        textRect.left(),
        textRect.bottom() - statusHeight - padding / 2,
        textRect.width(),
        statusHeight);

    // 绘制括号
    QFont bracketFont = QApplication::font();
    bracketFont.setPointSize(10);
    painter->setFont(bracketFont);
    painter->setPen(QColor(150, 150, 150));

    // 左括号
    painter->drawText(
        statusRect.left(),
        statusRect.top() + bracketMetrics.ascent(),
        "[");

    // 状态内容
    QString statusText = isOnline ? "● 在线" : "\u25CF 离线"; // 实心圆圈的Unicode字符编码
    int textWidth = painter->fontMetrics().horizontalAdvance(statusText);

    // 右括号
    painter->drawText(
        statusRect.left() + bracketWidth + textWidth + 4,
        statusRect.top() + bracketMetrics.ascent(),
        "]");

    // 绘制状态内容（居中处理）
    QRect statusContentRect(
        statusRect.left() + bracketWidth,
        statusRect.top(),
        textWidth,
        statusHeight);

    // 垂直居中绘制
    painter->setPen(isOnline ? QColor("#00C800") : QColor("#3D3D3D"));
    painter->drawText(
        statusContentRect,
        Qt::AlignVCenter | Qt::AlignLeft,
        statusText);

    painter->restore();
}

QSize FriendListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 56);
}
