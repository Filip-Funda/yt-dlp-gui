#ifndef MAINGUI_H
#define MAINGUI_H

#include <QMainWindow>
#include <QProcess>
#include <QtWidgets/qlabel.h>
#include <QTranslator>
#include <QEvent>
#include "template_highlighter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainGUI;
}
QT_END_NAMESPACE

class AppTests;

class MainGUI : public QMainWindow {
    Q_OBJECT

    friend class AppTests;

public:
    MainGUI(QWidget *parent = nullptr);
    ~MainGUI();

protected:
    void changeEvent(QEvent * event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_directoryButton_clicked();

    void on_downloadButton_clicked();

    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onProcessNewOutput();

    void on_enButton_clicked();

    void on_czButton_clicked();

    void on_detectButton_clicked();

    void on_addTitleButton_clicked();

    void on_addUploaderButton_clicked();

    void on_addUploadDateButton_clicked();

    void on_addResolutionButton_clicked();

    void on_addExtensionButton_clicked();

private:
    Ui::MainGUI *ui;
    QProcess *process;
    QTranslator appTranslator;

    QString ytdlpPath;
    QString ffmpegPath;
    QString jsRuntimePath;
    QStringList args;

    static constexpr const char * RATE_LIMIT = "3M";
    static constexpr int SLEEP_MIN = 3;
    static constexpr int SLEEP_MAX = 10;

    QColor errorColor;
    QColor downloadColor;
    QColor finishedColor;

    static const QRegularExpression progressRegex;

    void detectBinaries(bool silent = true);

    QString getSelectedFormat() const;

    QStringList getAdvancedMediaInfo() const;

    void addArguments(const QString & url, const QString & directoryPath);

    void setLabelColor(QLabel * label, QColor color);

    void updateUIColors(bool isDark);

    void setButtonsEnabled(bool enabled);

    void showAvailableCodecs();

    static QString sanitizeFilename(const QString & filename);

    static bool isValidUrl(const QString & url);

    static bool isValidDirectory(const QString & path);

    FilenameHighlighter * highlighter = nullptr;
};
#endif // MAINGUI_H
