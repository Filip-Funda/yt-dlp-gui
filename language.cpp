#include "maingui.h"
#include "./ui_maingui.h"

void MainGUI::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange) {
        if (ui && ui->status) {
            QColor currentColor = ui->status->palette().color(QPalette::WindowText);

            ui->retranslateUi(this);

            if (ytdlpPath.isEmpty() || ffmpegPath.isEmpty()) {
                setLabelColor(ui->status, errorColor);
                ui->status->setText(tr("Disabled, binaries not found."));
            }
            else if (currentColor == finishedColor) {
                ui->status->setText(tr("Download finished!"));
            }
            else if (process && process->state() == QProcess::Running) {
                ui->status->setText(tr("Downloading..."));
            }
            else if (currentColor == errorColor) {
                setLabelColor(ui->status, errorColor);
                ui->status->setText(tr("Error, download failed!"));
            }
        }
    }

    QMainWindow::changeEvent(event);
}

void MainGUI::on_enButton_clicked() {
    qApp->removeTranslator(&appTranslator);
}

void MainGUI::on_czButton_clicked() {
    qApp->removeTranslator(&appTranslator);

    if (appTranslator.load(":/i18n/yt-dlp-gui_cs")) {
        qApp->installTranslator(&appTranslator);
    }
}