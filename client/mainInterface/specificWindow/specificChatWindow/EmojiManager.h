#ifndef EMOJIMANAGER_H
#define EMOJIMANAGER_H

#include <QWidget>

class EmojiManager : public QWidget
{
    Q_OBJECT
public:
    explicit EmojiManager(QWidget *parent = nullptr);
    void setupUI();
signals:
    void emojiClicked(const QString &emojiPath);
};

#endif // EMOJIMANAGER_H
