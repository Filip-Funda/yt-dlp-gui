#ifndef TEMPLATE_HIGHLIGHTER_H
#define TEMPLATE_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class FilenameHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit FilenameHighlighter(QTextDocument * parent = nullptr) : QSyntaxHighlighter(parent) {
        specialFormat.setForeground(QBrush(QColor(144, 238, 144)));

        QStringList keywords = {
            "%(title)s",
            "%(uploader)s",
            "%(upload_date)s",
            "%(resolution)s",
            "%(ext)s"
        };

        for (const QString &word : keywords) {
            QRegularExpression pattern(QRegularExpression::escape(word));
            highlightingRules.append(pattern);
        }
    }

protected:
    void highlightBlock(const QString &text) override {
        for (const QRegularExpression &pattern : std::as_const(highlightingRules)) {
            QRegularExpressionMatchIterator matchIterator = pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), specialFormat);
            }
        }
    }

private:
    QTextCharFormat specialFormat;
    QVector<QRegularExpression> highlightingRules;
};

#endif // TEMPLATE_HIGHLIGHTER_H
