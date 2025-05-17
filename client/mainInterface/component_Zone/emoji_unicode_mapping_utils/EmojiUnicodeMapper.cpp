#include "EmojiUnicodeMapper.h"

EmojiUnicodeMapper::EmojiUnicodeMapper(QObject *parent)
    : QObject{parent}
{
    emoji_Unicode_Mapping();
}

void EmojiUnicodeMapper::emoji_Unicode_Mapping()
{
    // 面部表情类
    emojiToUnicodeMap["咧嘴笑"] = "1F600";
    emojiToUnicodeMap["笑哭"] = "1F602";
    emojiToUnicodeMap["微笑"] = "1F60A";
    emojiToUnicodeMap["飞吻"] = "1F618";
    emojiToUnicodeMap["号啕大哭"] = "1F62D";
    emojiToUnicodeMap["愤怒"] = "1F621";
    emojiToUnicodeMap["呕吐"] = "1F92E";
    emojiToUnicodeMap["天使笑"] = "1F607";
    emojiToUnicodeMap["耍帅"] = "1F60E";
    emojiToUnicodeMap["可怜"] = "1F97A";
    emojiToUnicodeMap["头昏眼花"] = "1F635";
    emojiToUnicodeMap["头晕"] = "1F635 FE0F 200D 1F4AB";

    // 手势和身体动作类
    emojiToUnicodeMap["点赞"] = "1F44D";
    emojiToUnicodeMap["OK"] = "1F44C";
    emojiToUnicodeMap["握手"] = "1F91D";
    emojiToUnicodeMap["肌肉"] = "1F4AA";
    emojiToUnicodeMap["挥手"] = "1F44B";

    // 自然和动物类
    emojiToUnicodeMap["太阳"] = "1F31E";
    emojiToUnicodeMap["月亮"] = "1F319";
    emojiToUnicodeMap["地球"] = "1F30D";
    emojiToUnicodeMap["下雨"] = "1F327 FE0F";
    emojiToUnicodeMap["猴子"] = "1F435";
    emojiToUnicodeMap["狗"] = "1F436";

    // 食物和饮料类
    emojiToUnicodeMap["汉堡包"] = "1F354";
    emojiToUnicodeMap["葡萄酒"] = "1F377";

    // 其他常见类
    emojiToUnicodeMap["炸弹"] = "1F4A3";
    emojiToUnicodeMap["电脑"] = "1F4BB";
    emojiToUnicodeMap["电话"] = "1F4DE";
    emojiToUnicodeMap["电视机"] = "1F4FA";
    emojiToUnicodeMap["礼物"] = "1F381";
}

QString EmojiUnicodeMapper::get_Unicode(const QString &emojiName)
{
    // 获取码点字符串（如"1F635 FE0F 200D 1F4AB"）
    QString unicodeStr = emojiToUnicodeMap.value(emojiName, "");
    if (unicodeStr.isEmpty())
        return "[动画表情]";

    // 将码点字符串转换为Emoji字符
    QStringList codePoints = unicodeStr.split(' ', Qt::SkipEmptyParts);
    QString emoji;

    for (const QString &code : codePoints)
    {
        bool ok;
        uint32_t codePoint = code.toUInt(&ok, 16);
        if (!ok)
            continue;

        if (codePoint <= 0xFFFF)
        {
            emoji += QChar(codePoint);
        }
        else
        {
            emoji += QChar::highSurrogate(codePoint);
            emoji += QChar::lowSurrogate(codePoint);
        }
    }

    return emoji.isEmpty() ? "[动画表情]" : emoji;
}
