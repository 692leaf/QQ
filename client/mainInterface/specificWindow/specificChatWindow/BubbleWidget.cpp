#include "BubbleWidget.h"
#include <QTimer>
#include <QApplication>
#include <QRegularExpression>

BubbleWidget::BubbleWidget(const Packege& fullPkg,QWidget *parent)
    : QWidget{parent}
{
    setupUI(fullPkg);
}

void BubbleWidget::setupUI(const Packege& fullPkg)
{
    bool isSelfMessage=fullPkg.sender==qApp->property("username").toString();

    QString messageHtml;
    if(fullPkg.messageInfo.message_type==RICHTEXTCONTENT_TRANSFERS)
    {
        messageHtml=generateTextMessageHtml(fullPkg);
    }
    else if(fullPkg.messageInfo.message_type==BLOCK_FILE_TRANSFERS)
    {
        messageHtml=generateFileMessageHtml(fullPkg);
    }
    else
    {
        qDebug()<<"bublleWdidget type: NONE";
    }

    QLabel *msgLabel = new QLabel(messageHtml, this);
    msgLabel->setWordWrap(true);                                    // 自动换行
    msgLabel->setTextFormat(Qt::RichText);                          // 启用富文本
    msgLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);  // 允许鼠标点击链接
    msgLabel->setOpenExternalLinks(false);                          // 禁用自动打开外部链接
    msgLabel->setWordWrap(true);                                    // 允许自动换行


    // ===== 布局管理 =====
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(5, 5, 5, 5); // 外间距

    // 对齐控制：自己消息右对齐，他人左对齐
    if (isSelfMessage)
    {
        hLayout->addStretch();      // 左侧弹簧
        hLayout->addWidget(msgLabel);
    }
    else
    {
        hLayout->addWidget(msgLabel);
        hLayout->addStretch();      // 右侧弹簧
    }

    // 设置文本浏览器不自动打开链接
    msgLabel->setOpenExternalLinks(false);
    // 连接锚点点击信号
    connect(msgLabel, &QLabel::linkActivated,this,[this](const QString& link){
        handleAnchorClicked(QUrl(link));
    });

    this->setLayout(hLayout);
}

QString BubbleWidget::generateTextMessageHtml(const Packege& fullPkg)
{
    bool isSelfMessage=fullPkg.sender==qApp->property("username").toString();
    QString richText = fullPkg.messageInfo.richText;
    QString escapedText = richText
                              // 清理完整的HTML文档标签
                              .remove(QRegularExpression("<(!DOCTYPE|html|head|body)[^>]*>", QRegularExpression::CaseInsensitiveOption))
                              .replace(QRegularExpression("</(html|head|body)>", QRegularExpression::CaseInsensitiveOption), "")
                              // URL处理保持不变
                              .replace(QRegularExpression("((?:https?|ftp)://\\S+)"), "<a href=\"\\1\">\\1</a>")
                              .replace(QRegularExpression("\\b(www\\.\\S+\\.\\w{2,})\\b"), "<a href=\"http://\\1\">\\1</a>");

    // 将 <p> 标签替换为 <div> 标签
    escapedText.replace(QRegularExpression("<p([^>]*)>"), "<div\\1>");
    escapedText.replace(QRegularExpression("</p>"), "</div>");

    // 移除所有width/height属性并添加固定样式
    escapedText.replace(QRegularExpression("<img([^>]*)width\\s*=\\s*['\"][^'\"]*['\"]", QRegularExpression::CaseInsensitiveOption), "<img\\1");
    escapedText.replace(QRegularExpression("<img([^>]*)height\\s*=\\s*['\"][^'\"]*['\"]", QRegularExpression::CaseInsensitiveOption), "<img\\1");

    // === 1. 先处理图片 ===
    QRegularExpression imgRegex(R"(<img\s+([^>]*?)src\s*=\s*['"]([^'"]+)['"]([^>]*?)/?>)");
    QRegularExpressionMatchIterator it = imgRegex.globalMatch(escapedText);
    int offset = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString src = match.captured(2);
        QString attrs = match.captured(1) + match.captured(3);
        QString replacement;

        if (src.startsWith(":/resource/image/emojis/")) {
            // 表情包：用 span 包裹保持内联
            replacement = QString("<span style=\"display: inline;\">"
                                  "<img src=\"%1\" %2 style=\"display: inline; vertical-align: middle; max-width: 32px; max-height: 32px;\"/>"
                                  "</span>")
                              .arg(src, attrs);
        } else {
            // 普通图片：单独一行
            replacement = QString("<div style='display: block; margin: 5px 0;'>"
                                  "<img src=\"%1\" %2 width=\"400\" height=\"366\"/></div>")
                              .arg(src, attrs);
        }

        // 替换逻辑
        int start = match.capturedStart() + offset;
        int length = match.capturedLength();
        escapedText.replace(start, length, replacement);
        offset += replacement.length() - length;
    }

    // === 2. 再处理纯文本包裹（跳过已处理的内容）===
    escapedText.replace(QRegularExpression(R"((?<!<\/?\w+)(?<!>)(^|>)([^<]+?)(?=<|$))"),
                        R"(\1<div style="
                                max-width:80%;                           // 最大宽度
                                color: #333333;                          // 文字颜色
                                font: 14px '微软雅黑';                    // 字体
                                text-align: left;                        // 文字左对齐
                                white-space: pre-wrap;                   // 保留空格--自动换行--忽略 word-break 限制
                                word-wrap:break-word;                    // 自动换行
                                overflow-wrap: anywhere !important;      // 强制任意位置换行
                                word-break:break-all;                    // 强制换行
                                ">\2</div>)");

    // === 新增关键处理 ===
    // 在连续字符中插入零宽空格（每20字符）
    escapedText.replace(
        QRegularExpression(R"((\d{20}|\w{20}))"),
        "\\1&#8203;"  // Unicode Zero-Width Space
        );

    // ===== 文本气泡设置 =====
    return QString(
               "<div style='"
               "display: inline-block;"        // 让 div 根据内容自适应
               "background: %1;"               // 背景色(自己消息绿色，他人消息白色)
               "border-radius: 8px;"           // 圆角
               "padding: 8px 12px;"            // 四周内边距
               "'>"
               "%2"                            // 消息内容
               "</div>"
               ).arg(isSelfMessage ? "#DCF8C6" : "#63A9E6")  // %1
                .arg(escapedText);             // %2

    // =======================
}

QString BubbleWidget::generateFileMessageHtml(const Packege &fullPkg)
{
    // ===== 文件气泡设置 =====
    QString fileID=fullPkg.messageInfo.file.fileID;
    QString fileName=fullPkg.messageInfo.file.fileName;
    QString fileType=fullPkg.messageInfo.file.fileType;
    QString fileSize=QString::number(fullPkg.messageInfo.file.fileSize/1024)+" KB";

    QString sendHtml=
        QString(
            "<div style='margin:5px; clear:both; max-width:70%;'>"
            "  <div style='padding:8px; border-radius:8px;'>"
            "    <table>"
            "      <tr>"
            "        <td rowspan='2'><img src=':/resource/image/fileType/%1.png' width='32'></td>"
            "        <td><b>%2</b></td>"
            "      </tr>"
            "      <tr>"
            "        <td>"
            "          %3 | "
            "          <a style='text-decoration:none;color:gray;'>已发送</a>"
            "        </td>"
            "      </tr>"
            "    </table>"
            "  </div>"
            "</div>"
            ).arg(fileType, fileName.toHtmlEscaped(), fileSize);

    /*=====================标准 URL 格式为：scheme://host/path?query#fragment=================================*
    *======================自定义协议的特殊情况:scheme:///path，即%6============================================*/
    // 使用标准锚点
    QString receiverHtml=
        QString(
            "<div style='margin:5px; clear:both; max-width:70%;'>"
            "  <div style='padding:8px; border-radius:8px;'>"
            "    <table>"
            "      <tr>"
            "        <td rowspan='2'><img src=':/resource/image/fileType/%1.png' width='32'></td>"
            "        <td><b>%2</b></td>"
            "      </tr>"
            "      <tr>"
            "        <td>"
            "          %3 | "
            "          <a href='fileid:///%4' style='text-decoration:none;color:blue;'>下载</a> | "
            "          <a href='openfolder:///%4' style='text-decoration:none;color:green;'>打开位置</a>"
            "        </td>"
            "      </tr>"
            "    </table>"
            "  </div>"
            "</div>"
            ).arg(fileType, fileName, fileSize, fileID);

    bool isSelfMessage=fullPkg.sender==qApp->property("username").toString();
    return isSelfMessage?sendHtml:receiverHtml;
    // =======================
}

// 处理锚点点击
void BubbleWidget::handleAnchorClicked(const QUrl &url)
{
    /*==================标准 URL 格式为：scheme://host/path?query#fragment============================
     *==================自定义协议的特殊情况:scheme:///path============================================*/
    const QString scheme = url.scheme();
    const QString fileID = url.path().mid(1);

    if (scheme == "fileid")
    {
        emit downloadRequested(fileID);
    }
    else if (scheme == "openfolder")
    {
        emit openFolderRequested(fileID);
    }
}


