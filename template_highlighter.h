#ifndef TEMPLATE_HIGHLIGHTER_H
#define TEMPLATE_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QStyleHints>
#include <QGuiApplication>

class FilenameHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit FilenameHighlighter(QTextDocument * parent = nullptr, bool single = false) : QSyntaxHighlighter(parent) {

        updateColors(QGuiApplication::styleHints()->colorScheme());

        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                this, &FilenameHighlighter::updateColors);

        QStringList singleKeywords = {
            "%(title)s",
            "%(uploader)s",
            "%(upload_date)s",
            "%(resolution)s",
            "%(ext)s"
        };

        QStringList playlistKeywords = {
            "%(playlist_index)s",
            "%(title)s",
            "%(playlist_title)s",
            "%(uploader)s",
            "%(ext)s"
        };

        if (single) {
            for (const QString &word : singleKeywords) {
                QRegularExpression pattern(QRegularExpression::escape(word));
                highlightingRules.append(pattern);
            }
        } else {
            for (const QString &word : playlistKeywords) {
                QRegularExpression pattern(QRegularExpression::escape(word));
                highlightingRules.append(pattern);
            }
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

private slots:
    void updateColors(Qt::ColorScheme colorScheme) {
        QColor finishedColor;

        if (colorScheme == Qt::ColorScheme::Dark) {
            finishedColor = QColor(144, 238, 144);
        } else {
            finishedColor = QColor(34, 139, 34);
        }

        specialFormat.setForeground(finishedColor);

        if (document()) {
            rehighlight();
        }
    }

private:
    QTextCharFormat specialFormat;
    QVector<QRegularExpression> highlightingRules;
};

#endif // TEMPLATE_HIGHLIGHTER_H
