#ifndef EMOJIUNICODEMAPPER_H
#define EMOJIUNICODEMAPPER_H

#include <QObject>
#include <QMap>

class EmojiUnicodeMapper : public QObject
{
    Q_OBJECT
public:
    explicit EmojiUnicodeMapper(QObject *parent = nullptr);
    void emoji_Unicode_Mapping();
    QString get_Unicode(const QString &emojiName);

private:
    // 存储表情到 Unicode 编码的映射
    QMap<QString, QString> emojiToUnicodeMap;
};

#endif // EMOJIUNICODEMAPPER_H
