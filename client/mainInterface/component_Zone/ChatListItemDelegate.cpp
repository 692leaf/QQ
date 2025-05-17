#include "ChatListItemDelegate.h"
#include "MessageType.h"

ChatListItemDelegate::ChatListItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
}

void ChatListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // 获取数据
    QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>(); // 头像
    QString name = index.data(Qt::DisplayRole).toString();            // 昵称
    QString latestMsg = index.data(TipMessageRole).toString();        // 最新消息

    // 高亮背景（选中或悬停）
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->fillRect(option.rect, option.palette.brush(QPalette::AlternateBase));
    }

    // 布局参数
    const int padding = 8;       // 内边距
    const int avatarSize = 40;   // 头像大小
    const int textSpacing = 2;   // 文字间距
    const int maxMsgLength = 20; // 最大显示消息长度

    // 绘制头像
    QRect avatarRect = option.rect.adjusted(padding, padding, 0, -padding);
    avatarRect.setWidth(avatarSize);
    avatarRect.setHeight(avatarSize);
    if (!avatar.isNull())
    {
        painter->drawPixmap(avatarRect, avatar.scaled(avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 绘制用户名和最新消息
    QRect textRect = option.rect.adjusted(avatarSize + 2 * padding, padding, -padding, -padding);
    QFont nameFont = QApplication::font();
    nameFont.setBold(true);
    painter->setFont(nameFont);

    // 用户名（上方）
    QRect nameRect = textRect;
    nameRect.setHeight(textRect.height() / 2);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignTop, name);

    // 最新消息（下方）
    QFont msgFont = QApplication::font();
    msgFont.setPointSize(10);
    painter->setFont(msgFont);
    painter->setPen(Qt::darkGray);

    // 消息截断处理
    QString displayMsg = latestMsg;
    if (latestMsg.length() > maxMsgLength)
    {
        displayMsg = latestMsg.left(maxMsgLength - 3) + "...";
    }

    QRect msgRect = textRect;
    msgRect.setTop(nameRect.bottom() + textSpacing);
    painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignTop, displayMsg);

    painter->restore();
}

QSize ChatListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 增加项高度以适应两行文本
    const int itemHeight = 40 + 2 * 8 + 20; // 头像40px + 边距 + 消息行高度
    return QSize(option.rect.width(), itemHeight);
}
