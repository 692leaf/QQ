#include "ButtonDelegate.h"

ButtonDelegate::ButtonDelegate(QObject *parent, bool isJoinButton)
    : QStyledItemDelegate(parent),
      isJoinButton(isJoinButton)
{
}

QSize ButtonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 重写 sizeHint 函数，自定义项的大小
    return QSize(option.rect.width(), 60); // 头像高度40px + 上下边距
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // 获取数据
    QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>(); // 头像
    QString qqNumber = index.data(Qt::UserRole).toString();           // QQ号
    QString name = index.data(Qt::DisplayRole).toString();            // 用户昵称

    // 绘制背景（选中或悬停）
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->fillRect(option.rect, option.palette.brush(QPalette::AlternateBase));
    }

    // 布局参数
    const int padding = 8;     // 内边距
    const int avatarSize = 40; // 头像大小
    const int textSpacing = 4; // 文字间距

    // 绘制头像
    QRect avatarRect = option.rect.adjusted(padding, padding, 0, -padding);
    avatarRect.setWidth(avatarSize);
    avatarRect.setHeight(avatarSize);
    if (!avatar.isNull())
    {
        painter->drawPixmap(avatarRect, avatar.scaled(avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 绘制用户名和QQ号
    QRect textRect = option.rect.adjusted(avatarSize + 2 * padding, padding, -100, -padding);
    textRect.setHeight(option.rect.height() - 2 * padding);

    // 用户名（第一行）
    QFont nameFont = QApplication::font();
    nameFont.setBold(true);
    painter->setFont(nameFont);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, name);

    // QQ号（第二行）
    QFont qqFont = QApplication::font();
    qqFont.setPointSize(10);
    painter->setFont(qqFont);
    painter->setPen(Qt::gray);
    QRect qqRect = textRect.adjusted(0, 20, 0, 0); // 向下偏移20px
    painter->drawText(qqRect, Qt::AlignLeft | Qt::AlignTop, "QQ: " + qqNumber);

    // 绘制按钮
    QRect buttonRect = option.rect;
    buttonRect.setLeft(buttonRect.right() - 80); // 按钮宽度80px
    buttonRect.setWidth(60);
    buttonRect.setHeight(24);
    buttonRect.moveTop(option.rect.center().y() - 12); // 垂直居中

    QStyleOptionButton buttonOption;
    buttonOption.rect = buttonRect;
    buttonOption.text = getButtonText(index);
    buttonOption.state = QStyle::State_Enabled;

    // 按钮启用状态
    if (buttonEnabledStates.contains(index) && !buttonEnabledStates[index])
    {
        buttonOption.state &= ~QStyle::State_Enabled;
    }
    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);

    painter->restore();
}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QRect buttonRect = option.rect;
        buttonRect.setLeft(buttonRect.right() - 80);
        buttonRect.setWidth(60);
        buttonRect.setHeight(24);
        buttonRect.moveTop(option.rect.center().y() - 12);

        // 检查按钮是否启用
        if (buttonEnabledStates.contains(index) && !buttonEnabledStates[index])
        {
            return false;
        }

        if (buttonRect.contains(mouseEvent->pos()))
        {
            emit buttonClicked(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ButtonDelegate::setButtonText(const QModelIndex &index, const QString &text)
{
    buttonTexts[index] = text;
}

QString ButtonDelegate::getButtonText(const QModelIndex &index) const
{
    if (buttonTexts.contains(index))
    {
        return buttonTexts.value(index);
    }
    return isJoinButton ? "添加" : "同意";
}

void ButtonDelegate::setButtonEnabled(const QModelIndex &index, bool enabled)
{
    buttonEnabledStates[index] = enabled;
}
