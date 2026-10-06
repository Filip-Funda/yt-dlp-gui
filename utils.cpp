#include "maingui.h"
#include "./ui_maingui.h"
#include <QStandardPaths>
#include <QMessageBox>
#include <QRegularExpression>
#include <QUrl>
#include <QDir>

void MainGUI::detectBinaries(bool silent) {
    bool ytdlpFound = false;
    bool ffmpegFound = false;

    // find needed binaries, disable download if not found
    ytdlpPath = QStandardPaths::findExecutable("yt-dlp");
    if (ytdlpPath.isEmpty()) {
        QStringList extraPaths;

#if defined(Q_OS_MACOS)
        extraPaths << "/opt/homebrew/bin";   // for homebrew downloads (Apple Silicon)
        extraPaths << "/usr/local/bin";      // for homebrew downloads (Intel) and MacPorts
#endif

        if (!extraPaths.isEmpty()) {
            ytdlpPath = QStandardPaths::findExecutable("yt-dlp", extraPaths);
        }
    }

    if (!ytdlpPath.isEmpty()) {
        ytdlpFound = true;
    }

    ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
    if (ffmpegPath.isEmpty()) {
        QStringList extraPaths;

#if defined(Q_OS_MACOS)
        extraPaths << "/opt/homebrew/bin";   // for homebrew downloads (Apple Silicon)
        extraPaths << "/usr/local/bin";      // for homebrew downloads (Intel) and MacPorts
#endif

        if (!extraPaths.isEmpty()) {
            ffmpegPath = QStandardPaths::findExecutable("ffmpeg", extraPaths);
        }
    }

    if (!ffmpegPath.isEmpty()) {
        ffmpegFound = true;
    }

    jsRuntimePath = QStandardPaths::findExecutable("node");
    if (jsRuntimePath.isEmpty()) {
        jsRuntimePath = QStandardPaths::findExecutable("deno");
    }
#if defined(Q_OS_MACOS)
    if (jsRuntimePath.isEmpty()) {
        QStringList extraPaths = {"/opt/homebrew/bin", "/usr/local/bin"};
        jsRuntimePath = QStandardPaths::findExecutable("node", extraPaths);
        if (jsRuntimePath.isEmpty()) {
            jsRuntimePath = QStandardPaths::findExecutable("deno", extraPaths);
        }
    }
#endif

    if (!ffmpegFound || !ytdlpFound) {
        if (!silent) {
            if (!ffmpegFound && !ytdlpFound) {
                QMessageBox::warning(this, tr("Warning"), tr("yt-dlp and ffmpeg not found!"));
            } else if (!ytdlpFound) {
                QMessageBox::warning(this, tr("Warning"), tr("yt-dlp not found!"));
            } else {
                QMessageBox::warning(this, tr("Warning"), tr("ffmpeg not found!"));
            }
        }

        ui->downloadButton->setEnabled(false);

        setLabelColor(ui->status, errorColor);
        ui->status->setText(tr("Disabled, binaries not found."));

        ui->detectButton->show();
        ui->detectButton->setEnabled(true);
    } else {
        ui->downloadButton->setEnabled(true);

        ui->status->setPalette(QPalette());
        ui->status->setText(tr("Waiting"));

        ui->detectButton->hide();
        ui->detectButton->setEnabled(false);
    }
}

QString MainGUI::getSelectedFormat() const {
    // Audio tab
    if (ui->formatTabs->currentIndex() == 0) {
        if (ui->bestButton->isChecked()) return "best";
        else if (ui->m4aButton->isChecked()) return "m4a";
        else if (ui->opusButton->isChecked()) return "opus";
        else if (ui->mp3Button->isChecked()) return "mp3";
        else if (ui->flacButton->isChecked()) return "flac";
        else if (ui->wavButton->isChecked()) return "wav";

        return "mp3";  // Default
    }
    // Video tab
    else {
        if (ui->mp4Button->isChecked()) return "mp4";
        else if (ui->mkvButton->isChecked()) return "mkv";
        else if (ui->webmButton->isChecked()) return "webm";
        else if (ui->movButton->isChecked()) return "mov";
        else if (ui->aviButton->isChecked()) return "avi";
        else if (ui->flvButton->isChecked()) return "flv";

        return "mkv";  // Default
    }
}

QStringList MainGUI::getAdvancedMediaInfo() const {
    QStringList mediaInfo;

    // Audio tab
    if (ui->advancedFormatTabs->currentIndex() == 0) {
        mediaInfo << (ui->audioContainerDropdown->currentText());
        mediaInfo << (ui->audioAudioCodecDropdown->currentText());
    }
    // Video tab
    else {
        mediaInfo << (ui->videoContainerDropdown->currentText());

        QString audioCodec = ui->videoAudioCodecDropdown->currentText();
        int index = audioCodec.indexOf(" (");
        if (index != -1) {
            audioCodec.truncate(index);
        }

        mediaInfo << audioCodec;

        QString videoCodec = ui->videoVideoCodecDropdown->currentText();
        index = videoCodec.indexOf(" (");
        if (index != -1) {
            videoCodec.truncate(index);
        }

        mediaInfo << videoCodec;
    }

    return mediaInfo;
}

void MainGUI::addArguments(const QString & url, const QString & directoryPath) {
    args // ----- GENERAL SETTINGS -----
        << "--newline"
        << "--add-metadata"
        << "--embed-thumbnail";

    args // ----- IP BLOCK AVOIDANCE -----
        << "--limit-rate" << RATE_LIMIT
        << "--sleep-interval" << QString::number(SLEEP_MIN)
        << "--max-sleep-interval" << QString::number(SLEEP_MAX);

    args // ----- PATHS -----
        << "--ffmpeg-location" << ffmpegPath
        << "-P" << directoryPath;

    if (!jsRuntimePath.isEmpty()) {
        QString runtimeType = jsRuntimePath.contains("deno") ? "deno" : "node";
        args << "--js-runtimes" << QString("%1:%2").arg(runtimeType, jsRuntimePath);
    }

    if (ui->settingsTab->currentIndex() == 0) {
        // SIMPLE tab is selected, AUDIO and VIDEO
        QString format = getSelectedFormat();

        if (ui->formatTabs->currentIndex() == 0) {
            // AUDIO is selected
            args << "-x";
            if (format != "best") {
                args << "--audio-format" << format;
            }
            args << "-f" << "ba";

        } else {
            // VIDEO is selected
            args << "-f" << "bv+ba/b";

            if (format == "mkv" || format == "webm") {
                args << "--merge-output-format" << format;
            } else {
                // MP4, MOV, AVI, FLV - recode needed
                args << "--recode-video" << format;
            }
        }

        // SINGLE FILE AND PLAYLIST
        if (ui->quantityTabs->currentIndex() == 0) {
            // SINGLE download is selected
            QString fileName = ui->fileNameInput->text().trimmed();

            // disable path traversal and regex, protection against file rewrite
            QString cleanFileName = sanitizeFilename(fileName);

            if (!cleanFileName.isEmpty()) {
                args << "-o" << cleanFileName;
            }
        } else {
            // PLAYLIST download is selected
            if (ui->ignoreErrCheckBox->isChecked()) {
                // ignore error if one video from playlist cannot be downloaded
                args << "-i";
            }

            args << "--yes-playlist";
        }
    } else {
        // ADVANCED tab is selected, AUDIO and VIDEO
        QStringList mediaInfo = getAdvancedMediaInfo();
        QString container = mediaInfo[0].toLower();
        QString audioCodec = mediaInfo[1];

        if (ui->advancedFormatTabs->currentIndex() == 0) {
            // AUDIO is selected
            args << "-x";
            args << "--audio-format" << container;
            args << "-f" << QString("ba[acodec*=%1]").arg(audioCodec.toLower());

        } else {
            // VIDEO is selected
            QString videoCodec = mediaInfo[2];

            QString vFilter = QString("bv*[vcodec*=%1]").arg(videoCodec.toLower());
            QString aFilter = QString("ba*[acodec*=%1]").arg(audioCodec.toLower());

            args << "-f" << QString("%1+%2/b").arg(vFilter, aFilter);

            args << "--merge-output-format" << container;
        }

        if (ui->advancedQuantityTabs->currentIndex() == 0) {
            // SINGLE download is selected
            QString advancedFileName = ui->advancedFileNameInput->toPlainText();
            QString cleanFileName = sanitizeFilename(advancedFileName);

            if (!cleanFileName.isEmpty()) {
                args << "-o" << cleanFileName;
            }

        } else {
            // PLAYLIST download is selected

            QString advancedFileName = ui->advancedPlaylistFileNameInput->toPlainText();
            QString cleanFileName = sanitizeFilename(advancedFileName);


            if (ui->ignoreAdvancedErrCheckBox->isChecked()) {
                // ignore error if one video from playlist cannot be downloaded
                args << "-i";
            }

            if (ui->createFolderCheckBox->isChecked()) {
                // create a folder in destination and move in all downloaded media
                // add %(playlist)s/ to the beginning to create a folder

                if (cleanFileName.isEmpty()) {
                    cleanFileName = "%(playlist)s/%(title)s.%(ext)s";
                } else {
                    cleanFileName = "%(playlist)s/" + cleanFileName;
                }

            } else if (cleanFileName.isEmpty()) {
                cleanFileName = "%(title)s.%(ext)s";
            }

            if (!cleanFileName.isEmpty()) {
                args << "-o" << cleanFileName;
            }


            args << "--yes-playlist";
        }
    }

    args << url;
}

QString MainGUI::sanitizeFilename(const QString & filename) {
    QString clean = filename.trimmed();

    clean.remove(QRegularExpression("[/\\\\<>:\"|?*\\x00-\\x1F]"));

    clean.replace(QRegularExpression("\\.{2,}"), ".");

    clean.replace(" ", "_");

    clean.remove(QRegularExpression("^[-.]+"));

    if (clean.length() > 255) {
        clean = clean.left(255);
    }

    if (clean.isEmpty()) {
        clean = "file";
    }

    return clean;
}

bool MainGUI::isValidUrl(const QString & url) {
    QUrl urlObj(url);

    if (!urlObj.isValid() || urlObj.scheme().isEmpty()) {
        return false;
    }

    QStringList allowedSchemes = {"http", "https"};
    return allowedSchemes.contains(urlObj.scheme().toLower());
}

bool MainGUI::isValidDirectory(const QString & path) {
    QDir dir(path);

    if (!dir.exists()) {
        return false;
    }

    QFileInfo dirInfo(path);
    return dirInfo.isWritable();
}

