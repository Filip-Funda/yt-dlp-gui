#include <QTest>
#include "maingui.h"
#include "ui_maingui.h"

class AppTests : public QObject {
    Q_OBJECT

private slots:
    void testUrlValidation() {
        QVERIFY(MainGUI::isValidUrl("https://www.youtube.com/watch?v=dQw4w9WgXcQ") == true);
        QVERIFY(MainGUI::isValidUrl("http://vimeo.com/12345") == true);

        QVERIFY(MainGUI::isValidUrl("non_sense_url") == false);
        QVERIFY(MainGUI::isValidUrl("") == false);
        QVERIFY(MainGUI::isValidUrl("../../../../") == false);
    }

    void testFilenameSanitization() {
        QCOMPARE(MainGUI::sanitizeFilename("try/traversal../..mp4"), QString("trytraversal.mp4"));
        QCOMPARE(MainGUI::sanitizeFilename("dangerous*symbol?.mp4"), QString("dangeroussymbol.mp4"));
        QCOMPARE(MainGUI::sanitizeFilename("   padding at the start and end   "), QString("padding_at_the_start_and_end"));
        QCOMPARE(MainGUI::sanitizeFilename(".make visible.mp4"), QString("make_visible.mp4"));
        QCOMPARE(MainGUI::sanitizeFilename(".....very/ \\hard?*** test:<>.mp4"), QString("very_hard_test.mp4"));
    }

    void testDirectoryValidation() {
        QString tempPath = QDir::tempPath();
        QVERIFY(MainGUI::isValidDirectory(tempPath) == true);

        QVERIFY(MainGUI::isValidDirectory("/ultimatelyNonExistent/path/to/a/file") == false);
        QVERIFY(MainGUI::isValidDirectory("surely*this(isnt}//a/path") == false);
        QVERIFY(MainGUI::isValidDirectory("") == false);
    }

    void testFormatSelection() {
        MainGUI gui;

        gui.ui->settingsTab->setCurrentIndex(0);
        gui.ui->formatTabs->setCurrentIndex(0);
        gui.ui->flacButton->setChecked(true);
        QCOMPARE(gui.getSelectedFormat(), QString("flac"));

        gui.ui->formatTabs->setCurrentIndex(1);
        gui.ui->mkvButton->setChecked(true);
        QCOMPARE(gui.getSelectedFormat(), QString("mkv"));
    }

    void testArgumentsGeneration() {
        MainGUI gui;
        gui.args.clear();

        gui.ui->settingsTab->setCurrentIndex(0);
        gui.ui->formatTabs->setCurrentIndex(0);
        gui.ui->mp3Button->setChecked(true);
        gui.ui->quantityTabs->setCurrentIndex(0);

        gui.ui->fileNameInput->setText(" my awesome song? ");

        gui.addArguments("https://youtube.com/watch?v=123", "/downloads");

        QVERIFY(gui.args.contains("--audio-format"));
        QVERIFY(gui.args.contains("mp3"));
        QVERIFY(gui.args.contains("-o"));
        QVERIFY(gui.args.contains("my_awesome_song"));
    }

    void testSetLabelColor() {
        MainGUI gui;

        QLabel label1;
        QLabel label2;
        QColor color1 = QColor(255, 100, 100);
        QColor color2 = QColor(100, 255, 100);

        gui.setLabelColor(&label1, color1);
        gui.setLabelColor(&label2, color2);

        QCOMPARE(label1.palette().color(QPalette::WindowText), color1);
        QCOMPARE(label2.palette().color(QPalette::WindowText), color2);
    }

    void testAdvancedPlaylistArguments() {
        MainGUI gui;
        gui.args.clear();

        gui.ui->settingsTab->setCurrentIndex(1);
        gui.ui->advancedQuantityTabs->setCurrentIndex(1);

        gui.ui->createFolderCheckBox->setChecked(true);
        gui.ui->ignoreAdvancedErrCheckBox->setChecked(true);

        gui.addArguments("https://youtube.com/playlist?list=123", "/downloads");

        QVERIFY(gui.args.contains("--yes-playlist"));
        QVERIFY(gui.args.contains("-i"));
        QVERIFY(gui.args.contains("-o"));
        QVERIFY(gui.args.contains("%(playlist)s/%(title)s.%(ext)s"));
    }

    void testFilenameSanitizationEdgeCases() {
        QCOMPARE(MainGUI::sanitizeFilename("?<>:\"|?*"), QString("file"));
        QCOMPARE(MainGUI::sanitizeFilename("   "), QString("file"));

        QCOMPARE(MainGUI::sanitizeFilename("---bad-option.mp4"), QString("bad-option.mp4"));
    }

    void testUrlSchemes() {
        QVERIFY(MainGUI::isValidUrl("ftp://example.com/file.mp4") == false);
        QVERIFY(MainGUI::isValidUrl("file:///etc/passwd") == false);
        QVERIFY(MainGUI::isValidUrl("javascript:alert(1)") == false);
    }

    void testAdvancedMediaInfoExtraction() {
        MainGUI gui;

        gui.ui->settingsTab->setCurrentIndex(1);
        gui.ui->advancedFormatTabs->setCurrentIndex(1);

        gui.ui->videoVideoCodecDropdown->clear();
        gui.ui->videoVideoCodecDropdown->addItem("avc1 (H.264)");
        gui.ui->videoVideoCodecDropdown->setCurrentIndex(0);

        QStringList mediaInfo = gui.getAdvancedMediaInfo();

        if (!mediaInfo.isEmpty() && mediaInfo.size() >= 3) {
            QCOMPARE(mediaInfo[2], QString("avc1"));
        } else {
            QFAIL("Advanced media info list is invalid or empty.");
        }
    }
};

QTEST_MAIN(AppTests)

#include "tests.moc"