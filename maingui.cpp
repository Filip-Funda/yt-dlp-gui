#include "maingui.h"
#include "./ui_maingui.h"
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QProgressBar>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QColor>
#include <QPalette>
#include <QGuiApplication>
#include <QStyleHints>
#include <QMessageBox>
#include <QUrl>
#include <QFile>
#include <QKeyEvent>
#include <QFontMetrics>

MainGUI::MainGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainGUI)
    , process(nullptr) {

    ui->setupUi(this);

    // set error label invisible by default and remain size
    auto sp = ui->errorLabel->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    ui->errorLabel->setSizePolicy(sp);
    ui->errorLabel->hide();

    // set detect binaries button invsible by default
    ui->detectButton->hide();
    ui->detectButton->setEnabled(false);

    // set language automatically from system
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "yt-dlp-gui_" + QLocale(locale).name();

        if (appTranslator.load(":/i18n/" + baseName)) {
            qApp->installTranslator(&appTranslator);
            break;
        }
    }

    // connect color mode change in OS
    auto hints = QGuiApplication::styleHints();
    connect(hints, &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme scheme) {
        updateUIColors(scheme == Qt::ColorScheme::Dark);
    });

    // initial color update
    updateUIColors(hints->colorScheme() == Qt::ColorScheme::Dark);

    // progress bar not natively rendering in macOS (idk why), using Fusion style
    ui->progressBar->setStyle(QStyleFactory::create("Fusion"));

    // automatically update available codecs
    connect(ui->audioContainerDropdown, &QComboBox::currentTextChanged, this, [this]() {
        showAvailableCodecs();
    });

    connect(ui->videoContainerDropdown, &QComboBox::currentTextChanged, this, [this]() {
        showAvailableCodecs();
    });

    connect(ui->advancedFormatTabs, &QTabWidget::currentChanged, this, [this]() {
        showAvailableCodecs();
    });

    connect(ui->settingsTab, &QTabWidget::currentChanged, this, [this]() {
        showAvailableCodecs();
    });

    showAvailableCodecs();

    // install event filter so user cant create a new line
    ui->advancedFileNameInput->installEventFilter(this);

    highlighter = new FilenameHighlighter(ui->advancedFileNameInput->document());

    // automatically detect binaries
    detectBinaries();
}

bool MainGUI::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            return true;
        }

        if (obj == ui->advancedFileNameInput) {
            QString text = keyEvent->text();
            if (!text.isEmpty()) {
                static const QString forbiddenChars = "<>:\"/\\|?*";
                for (int i = 0; i < text.length(); ++i) {
                    if (forbiddenChars.contains(text.at(i))) {
                        return true;
                    }
                }
            }

            int maxLen = 256;

            if (ui->advancedFileNameInput->toPlainText().length() >= maxLen) {
                if (keyEvent->key() != Qt::Key_Backspace &&
                    keyEvent->key() != Qt::Key_Delete &&
                    keyEvent->key() != Qt::Key_Left &&
                    keyEvent->key() != Qt::Key_Right &&
                    keyEvent->key() != Qt::Key_Up &&
                    keyEvent->key() != Qt::Key_Down &&
                    !keyEvent->modifiers().testFlag(Qt::ControlModifier))
                {
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

MainGUI::~MainGUI() {
    if (process) {
        process->kill();
        delete process;
    }

    delete highlighter;

    delete ui;
}

void MainGUI::on_directoryButton_clicked() {
    QString directoryPath = QFileDialog::getExistingDirectory(this, tr("Select destination directory"), QDir::homePath());
    if (!directoryPath.isEmpty())
    {
        ui->pathPrint->setText(directoryPath);
    }
}

void MainGUI::on_downloadButton_clicked() {
    if (!QFile::exists(ytdlpPath) || !QFile::exists(ffmpegPath)) {
        QMessageBox::warning(this, tr("Warning"), tr("Binaries disappeared! Checking again..."));

        detectBinaries(false);
        return;
    }

    // reset label
    ui->errorLabel->hide();

    // reset progress bar
    ui->progressBar->setValue(0);

    QString url = ui->urlInput->text().trimmed();
    if (url.isEmpty()) {
        ui->errorLabel->setText(tr("URL is missing!"));
        ui->errorLabel->show();
        return;
    }

    if (!isValidUrl(url)) {
        ui->errorLabel->setText(tr("Invalid URL!"));
        ui->errorLabel->show();
        return;
    }

    QString directoryPath = ui->pathPrint->text().trimmed();
    if (directoryPath.isEmpty()) {
        ui->errorLabel->setText(tr("Directory path is missing!"));
        ui->errorLabel->show();
        return;
    }

    if (!isValidDirectory(directoryPath)) {
        ui->errorLabel->setText(tr("Cannot write into selected directory!"));
        ui->errorLabel->show();
        return;
    }

    ui->errorLabel->hide();

    if (process) {
        process->kill();
        delete process;
    }

    process = new QProcess(this);

    // connect to finished and newOutput events
    connect(process, &QProcess::finished, this, &MainGUI::onProcessFinished);
    connect(process, &QProcess::readyReadStandardOutput, this, &MainGUI::onProcessNewOutput);

    process->setWorkingDirectory(directoryPath);

    // arguments for download
    args.clear();
    addArguments(url, directoryPath);

    setLabelColor(ui->status, downloadColor);
    ui->status->setText(tr("Starting download..."));

    process->start(ytdlpPath, args);

    if (!process->waitForStarted(3000)) {
        setLabelColor(ui->status, errorColor);

        ui->status->setText(tr("Failed to start download!"));
        ui->downloadButton->setEnabled(true);
        //ui->downloadButton->setCursor(Qt::PointingHandCursor);

        ui->downloadButton->setText(tr("Download"));
        ui->directoryButton->setEnabled(true);
        //ui->directoryButton->setCursor(Qt::PointingHandCursor);
        return;
    }

    ui->status->setText(tr("Downloading..."));
    setButtonsEnabled(false);
    ui->downloadButton->setText(tr("Downloading..."));
}

void MainGUI::on_detectButton_clicked() {
    detectBinaries(false);
}

void MainGUI::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    setButtonsEnabled(true);
    ui->downloadButton->setText(tr("Download"));

    // set status according to exit code
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        setLabelColor(ui->status, finishedColor);
        ui->status->setText(tr("Download finished!"));
    } else {
        setLabelColor(ui->status, errorColor);
        ui->status->setText(tr("Error, download failed!"));

        QString errorOutput = process->readAllStandardError();
        if (errorOutput.isEmpty()) {
            errorOutput = tr("Unknown error (exit code: %1)").arg(exitCode);
        }

        QMessageBox::critical(this, tr("Download Failed"),
                              tr("The download process failed.\n\nError details:\n%1").arg(errorOutput));
    }
}

const QRegularExpression MainGUI::progressRegex(R"((\d+\.?\d*)%)");

void MainGUI::onProcessNewOutput() {
    while (process->canReadLine()) {
        QString line = process->readLine().trimmed();

        // match download progress with regexp, update progress bar
        if (line.contains("[download]")) {
            QRegularExpressionMatch match = progressRegex.match(line);

            if (match.hasMatch()) {
                double percentage = match.captured(1).toDouble();
                int value = qRound(percentage);
                ui->progressBar->setValue(value);
            }
        }
    }
}

// update label color
void MainGUI::setLabelColor(QLabel * label, QColor color) {
    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
}

void MainGUI::updateUIColors(bool isDark) {
    if (isDark) {
        errorColor = QColor(255, 100, 100);
        downloadColor = QColor(255, 214, 102);
        finishedColor = QColor(144, 238, 144);

        if (ui->status->palette().color(QPalette::WindowText) == QColor(200, 0, 0)) {
            setLabelColor(ui->status, errorColor);
        } else if (ui->status->palette().color(QPalette::WindowText) == QColor(184, 134, 11)) {
            setLabelColor(ui->status, downloadColor);
        } else if (ui->status->palette().color(QPalette::WindowText) == QColor(34, 139, 34)) {
            setLabelColor(ui->status, finishedColor);
        }

    } else {
        errorColor = QColor(200, 0, 0);
        downloadColor = QColor(184, 134, 11);
        finishedColor =  QColor(34, 139, 34);

        if (ui->status->palette().color(QPalette::WindowText) == QColor(255, 100, 100)) {
            setLabelColor(ui->status, errorColor);
        } else if (ui->status->palette().color(QPalette::WindowText) == QColor(255, 214, 102)) {
            setLabelColor(ui->status, downloadColor);
        } else if (ui->status->palette().color(QPalette::WindowText) == QColor(144, 238, 144)) {
            setLabelColor(ui->status, finishedColor);
        }
    }

    setLabelColor(ui->errorLabel, errorColor);
}

void MainGUI::setButtonsEnabled(bool enabled) {
    ui->downloadButton->setEnabled(enabled);
    //ui->downloadButton->setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
    ui->directoryButton->setEnabled(enabled);
    //ui->directoryButton->setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void MainGUI::showAvailableCodecs() {
    // ADVANCED tab has to be selected
    if (ui->settingsTab->currentIndex() == 1) {
        if (ui->advancedFormatTabs->currentIndex() == 0) {
            // AUDIO is selected, set audio codecs
            ui->audioAudioCodecDropdown->clear();
            ui->audioAudioCodecDropdown->setEnabled(true);

            QString currentContainer = ui->audioContainerDropdown->currentText();
            if (currentContainer == "M4A") {
                ui->audioAudioCodecDropdown->addItems({"mp4a", "aac", "alac"});
            } else if (currentContainer == "WAV") {
                ui->audioAudioCodecDropdown->addItems({"pcm"});
            } else if (currentContainer == "OGG") {
                ui->audioAudioCodecDropdown->addItems({"opus", "vorbis"});
            } else {
                ui->audioAudioCodecDropdown->addItems({currentContainer.toLower()});
                ui->audioAudioCodecDropdown->setEnabled(false);
            }
        } else {
            // VIDEO is selected, set audio and video codecs
            ui->videoAudioCodecDropdown->clear();
            ui->videoVideoCodecDropdown->clear();

            ui->videoAudioCodecDropdown->setEnabled(true);
            ui->videoVideoCodecDropdown->setEnabled(true);

            QString currentContainer = ui->videoContainerDropdown->currentText();
            if (currentContainer == "MP4") {
                ui->videoVideoCodecDropdown->addItems({"avc1 (H.264)", "av01 (AV1)", "hev1 (H.265)"});
                ui->videoAudioCodecDropdown->addItems({"mp4a (AAC)", "mp3"});
            } else if (currentContainer == "MKV") {
                ui->videoVideoCodecDropdown->addItems({"avc1 (H.264)", "vp9", "av01 (AV1)", "hev1 (H.265)"});
                ui->videoAudioCodecDropdown->addItems({"mp4a (AAC)", "opus", "mp3", "flac"});
            } else if (currentContainer == "WebM") {
                ui->videoVideoCodecDropdown->addItems({"vp9", "av01 (AV1)"});
                ui->videoAudioCodecDropdown->addItems({"opus", "vorbis"});
            } else if (currentContainer == "MOV") {
                ui->videoVideoCodecDropdown->addItems({"avc1 (H.264)", "hev1 (H.265)", "prores"});
                ui->videoAudioCodecDropdown->addItems({"mp4a (AAC)", "pcm"});
            } else if (currentContainer == "AVI") {
                ui->videoVideoCodecDropdown->addItems({"mpeg4 (Xvid)", "avc1 (H.264)"});
                ui->videoAudioCodecDropdown->addItems({"mp3", "pcm"});
            } else if (currentContainer == "FLV") {
                ui->videoVideoCodecDropdown->addItems({"avc1 (H.264)", "flv1"});
                ui->videoAudioCodecDropdown->addItems({"mp4a (AAC)", "mp3"});
            }
        }
    }
}

void MainGUI::on_addTitleButton_clicked() {
    if (ui->advancedFileNameInput->toPlainText().length() < 247) {
        ui->advancedFileNameInput->insertPlainText("%(title)s");
        ui->advancedFileNameInput->setFocus();
    }
}


void MainGUI::on_addUploaderButton_clicked() {
    if (ui->advancedFileNameInput->toPlainText().length() < 244) {
        ui->advancedFileNameInput->insertPlainText("%(uploader)s");
        ui->advancedFileNameInput->setFocus();
    }
}


void MainGUI::on_addUploadDateButton_clicked() {
    if (ui->advancedFileNameInput->toPlainText().length() < 241) {
        ui->advancedFileNameInput->insertPlainText("%(upload_date)s");
        ui->advancedFileNameInput->setFocus();
    }
}


void MainGUI::on_addResolutionButton_clicked() {
    if (ui->advancedFileNameInput->toPlainText().length() < 242) {
        ui->advancedFileNameInput->insertPlainText("%(resolution)s");
        ui->advancedFileNameInput->setFocus();
    }
}


void MainGUI::on_addExtensionButton_clicked() {
    if (ui->advancedFileNameInput->toPlainText().length() < 249) {
        ui->advancedFileNameInput->insertPlainText("%(ext)s");
        ui->advancedFileNameInput->setFocus();
    }
}