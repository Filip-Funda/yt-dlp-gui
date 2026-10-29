#include "maingui.h"
#include "./ui_maingui.h"

void MainGUI::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange) {
        if (ui && ui->status) {
            ui->retranslateUi(this);

            switch (currentStatusState) {
                case StatusState::Waiting:
                    ui->status->setPalette(QPalette());
                    ui->status->setText(tr("Waiting"));
                    break;
                case StatusState::BinariesNotFound:
                    setLabelColor(ui->status, errorColor);
                    ui->status->setText(tr("Disabled, binaries not found."));
                    break;
                case StatusState::Starting:
                    setLabelColor(ui->status, downloadColor);
                    ui->status->setText(tr("Starting download..."));
                    break;
                case StatusState::Downloading:
                    setLabelColor(ui->status, downloadColor);
                    ui->status->setText(tr("Downloading..."));
                    break;
                case StatusState::FailedToStart:
                    setLabelColor(ui->status, errorColor);
                    ui->status->setText(tr("Failed to start download!"));
                    break;
                case StatusState::Finished:
                    setLabelColor(ui->status, finishedColor);
                    ui->status->setText(tr("Download finished!"));
                    break;
                case StatusState::DownloadError:
                    setLabelColor(ui->status, errorColor);
                    ui->status->setText(tr("Error, download failed!"));
                    break;
                default:
                    break;
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